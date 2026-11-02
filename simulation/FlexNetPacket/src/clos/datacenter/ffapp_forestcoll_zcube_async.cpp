// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * ffapp_forestcoll_zcube.cpp
 * ==========================
 *
 * Self-contained FFApplication/FFTask + ForestColl AllReduce implementation
 * for the ZCube(4,2) microbenchmark.
 *
 * This file intentionally does NOT require the original ffapp.cpp.
 *
 * It contains:
 *   1. Minimal FFApplication implementation required by ffapp.h
 *   2. Minimal FFTask implementation required by the benchmark
 *   3. Minimal FFDevice/linker stubs
 *   4. Generated ForestColl schedule
 *   5. ForestColl ReduceScatter + AllGather executor
 *
 * The generated schedule is included directly:
 *
 *     zcube_fc_schedule_generated.cpp
 *
 * Therefore DO NOT compile that generated .cpp separately.
 */

#include "ecmp_hash.h"
#include "ffapp_forestcoll_zcube.h"

#include "rocev2.h"
#include "tcp.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <utility>
#include <vector>


// ============================================================================
// FFApplication static members
// ============================================================================

int FFApplication::total_apps    = 0;
int FFApplication::finished_apps = 0;


// ============================================================================
// Minimal FFApplication implementation
// ============================================================================
//
// This mirrors the pattern used by the working L2K44 benchmark.
//
// We only initialize the fields required by:
//
//     FFForestCollZCubeTask
//
// We deliberately do NOT implement workload/task-graph functionality.
// ============================================================================

FFApplication::FFApplication(
        Topology *top,
        int ss,
        ofstream *_fstream_out,
        TcpRtxTimerScanner &rtx,
        EventList &evl,
        FFAllReduceStrategy ars)
    : topology(top),
      ssthresh(ss),
      eventlist(evl),
      fstream_out(_fstream_out),
      tcpRtxScanner(rtx),
      allreduce_strategy(ars),
      final_finish_time(0),
      first_iter_time(0),
      n_finished_tasks(0),
      finished_once(false),
      fancy_ring(false),
      bfb_rto_ms(1.0),
      ngpupernode(1),
      nnodes(16),
      nswitches(0),
      cwnd(0),
      pull_rate(0.0)
{
    FFApplication::total_apps++;

    gpus.resize(16);
    std::iota(gpus.begin(), gpus.end(), 0);
}


// Second constructor declared by ffapp.h.
// Required so the linker has all FFApplication constructors.

FFApplication::FFApplication(
        Topology *top,
        int ss,
        ofstream *_fstream_out,
        std::vector<int> gpus_,
        TcpRtxTimerScanner &rtx,
        EventList &evl,
        FFAllReduceStrategy ars)
    : topology(top),
      ssthresh(ss),
      eventlist(evl),
      fstream_out(_fstream_out),
      gpus(std::move(gpus_)),
      tcpRtxScanner(rtx),
      allreduce_strategy(ars),
      final_finish_time(0),
      first_iter_time(0),
      n_finished_tasks(0),
      finished_once(false),
      fancy_ring(false),
      bfb_rto_ms(1.0),
      ngpupernode(1),
      nnodes(16),
      nswitches(0),
      cwnd(0),
      pull_rate(0.0)
{
    FFApplication::total_apps++;
}


FFApplication::~FFApplication()
{
    /*
     * The benchmark registers the ForestColl task in app.tasks.
     *
     * Therefore FFApplication owns it and deletes it here.
     */
    for (auto &kv : tasks)
        delete kv.second;

    for (auto &kv : devices)
        delete kv.second;
}


// ============================================================================
// FFApplication methods not used by this benchmark
// ============================================================================

void FFApplication::load_taskgraph_json(std::string &)
{
    assert(false && "load_taskgraph_json not used in ForestColl benchmark");
}


void FFApplication::load_taskgraph_flatbuf(std::string &)
{
    assert(false && "load_taskgraph_flatbuf not used in ForestColl benchmark");
}


void FFApplication::start_init_tasks()
{
    assert(false && "start_init_tasks not used in ForestColl benchmark");
}


void FFApplication::reset_and_restart()
{
    assert(false && "reset_and_restart not used in ForestColl benchmark");
}


std::vector<int> FFApplication::choose_gpus(
        std::unordered_set<int> &candidates,
        int n)
{
    (void)candidates;
    (void)n;

    assert(false && "choose_gpus not used in ForestColl benchmark");

    return {};
}


// ============================================================================
// Minimal FFTask implementation
// ============================================================================

FFTask::FFTask(
        FFApplication *app,
        FFTaskType t)
    : ffapp(app),
      EventSource(app->eventlist, "FFTask"),
      type(t),
      state(TASK_NOT_READY),
      device(nullptr),
      counter(0),
      xfersize(0),
      src_node(-1),
      dst_node(-1),
      ready_time(0),
      run_time(0),
      start_time(0),
      finish_time(0)
{
}


// Constructor declared by ffapp.h.
// Not used by this benchmark, but supplied for linker completeness.

FFTask::FFTask(
        FFApplication *app,
        std::string type_str,
        FFDevice *dev,
        uint64_t sz,
        float rt)
    : ffapp(app),
      EventSource(app->eventlist, "FFTask"),
      type(TASK_COMM),
      state(TASK_NOT_READY),
      device(dev),
      counter(0),
      xfersize(sz),
      src_node(-1),
      dst_node(-1),
      ready_time(0),
      run_time((simtime_picosec)(rt * 1e9)),
      start_time(0),
      finish_time(0)
{
    (void)type_str;
}


// FlatBuffer constructor declared by ffapp.h.
// Also unused in benchmark mode.

FFTask::FFTask(
        FFApplication *app,
        FlatBufTaskGraph::SimTaskType,
        FFDevice *dev,
        uint64_t sz,
        float rt)
    : ffapp(app),
      EventSource(app->eventlist, "FFTask"),
      type(TASK_COMM),
      state(TASK_NOT_READY),
      device(dev),
      counter(0),
      xfersize(sz),
      src_node(-1),
      dst_node(-1),
      ready_time(0),
      run_time((simtime_picosec)(rt * 1e9)),
      start_time(0),
      finish_time(0)
{
}


// Base implementation.
// ForestColl overrides this.

void FFTask::doNextEvent()
{
}


// ============================================================================
// FFTask::cleanup
// ============================================================================
//
// ForestColl calls cleanup() when the AllGather phase has completed.
//
// In benchmark mode there is exactly one task. Therefore this:
//
//   1. Marks it finished.
//   2. Stores final_finish_time.
//   3. Stops EventList once all tasks are finished.
// ============================================================================

void FFTask::cleanup()
{
    state = TASK_FINISHED;

    ffapp->n_finished_tasks++;

    if (ffapp->final_finish_time < finish_time)
        ffapp->final_finish_time = finish_time;


    // Preserve normal FFTask successor semantics.
    // There are no successors in this microbenchmark.

    for (uint64_t nid : next_tasks) {

        FFTask *t = ffapp->tasks[nid];

        if (--t->counter == 0) {

            t->ready_time = finish_time;
            t->state = TASK_READY;

            eventlist().sourceIsPending(
                *t,
                t->ready_time
            );
        }
    }


    // All registered tasks have completed.

    if (ffapp->n_finished_tasks == ffapp->tasks.size()) {

        if (!ffapp->finished_once) {

            ffapp->finished_once = true;

            ffapp->first_iter_time =
                ffapp->final_finish_time;

            FFApplication::finished_apps++;
        }


        if (FFApplication::finished_apps ==
            FFApplication::total_apps) {

            eventlist().setEndtime(
                eventlist().now()
            );
        }
    }
}


// ============================================================================
// FFTask methods declared by ffapp.h but unused here
// ============================================================================

void FFTask::taskstart()
{
}


void FFTask::start_flow()
{
}


void FFTask::execute_compute()
{
}


void FFTask::add_nextask(FFTask *)
{
}


// ============================================================================
// Generic taskfinish callback required by ffapp.h
// ============================================================================

void taskfinish(void *task)
{
    FFTask *t =
        static_cast<FFTask *>(task);

    t->finish_time =
        t->eventlist().now();

    t->cleanup();
}


// ============================================================================
// Minimal FFDevice constructors
// ============================================================================

FFDevice::FFDevice(
        FFApplication *app,
        std::string type_str,
        float bw,
        int nid,
        int gid,
        int fn,
        int tn,
        int fg,
        int tg)
    : ffapp(app),
      node_id(nid),
      gpu_id(gid),
      bandwidth(bw * 8 * 1000),
      type(DEVICE_GPU),
      state(DEVICE_IDLE),
      from_gpu(fg),
      to_gpu(tg),
      from_node(fn),
      to_node(tn),
      busy_up_to(0)
{
    (void)type_str;
}


FFDevice::FFDevice(
        FFApplication *app,
        FlatBufTaskGraph::DeviceType,
        uint64_t nid,
        uint64_t prop,
        uint64_t bw)
    : ffapp(app),
      node_id((int)nid),
      gpu_id(0),
      bandwidth((float)bw * 8 * 1000),
      type(DEVICE_GPU),
      state(DEVICE_IDLE),
      from_gpu(0),
      to_gpu(0),
      from_node(0),
      to_node(0),
      busy_up_to(0)
{
    (void)prop;
}


// ============================================================================
// Generated ForestColl schedule
// ============================================================================
//
// ZCubeFCEdge and ZCubeFCTree are declared by
// ffapp_forestcoll_zcube.h.
//
// Current generated schedule:
//
//     16 GPU roots
//     nchunkspernode = 2
//     25 batched tree records
//     32 unit-tree chunks
//
// Do NOT compile this .cpp separately.
// ============================================================================

#include "zcube_fc_schedule_generated.cpp"


// ============================================================================
// ForestColl TCP callback
// ============================================================================

void ar_finish_forestcoll_zcube(void *info)
{
    auto *h =
        static_cast<FFForestCollHop *>(info);

    assert(h != nullptr);
    assert(h->task != nullptr);


    if (h->phase == FCFPhase::REDUCE_SCATTER) {

        h->task->rs_hop_done(
            h->tree_id,
            h->arrival_node
        );

    } else {

        h->task->ag_hop_done(
            h->tree_id,
            h->arrival_node
        );
    }


    delete h;
}


// ============================================================================
// ForestColl constructor
// ============================================================================

FFForestCollZCubeTask::FFForestCollZCubeTask(
        FFApplication *ffapp,
        Topology *topology,
        std::vector<uint64_t> ng,
        uint64_t sz,
        double local_runtime)
    : FFTask(
          ffapp,
          FFTask::TASK_ALLREDUCE
      ),
      _topology(topology),
      node_group(std::move(ng)),
      operator_size(sz > 0 ? sz : 1ULL),
      chunk_bytes(1),
      total_hops(0),
      finished_hops(0)
{
    run_time =
        (simtime_picosec)(
            local_runtime * 1e9
        );

    xfersize = sz;


    assert(_topology != nullptr);

    assert(node_group.size() == 16);


    // One unit ForestColl chunk.
    //
    // The generated schedule has 32 unit chunks total.
    //
    // A batched tree with nchunks=2 therefore carries:
    //
    //     2 * chunk_bytes

    chunk_bytes =
        (
            operator_size
            + (uint64_t)_gen_fc_total_chunks
            - 1
        )
        /
        (uint64_t)_gen_fc_total_chunks;


    if (chunk_bytes == 0)
        chunk_bytes = 1;


    build_schedule();
}


// ============================================================================
// Build runtime representation of generated trees
// ============================================================================

void FFForestCollZCubeTask::build_schedule()
{
    runtime.clear();

    runtime.resize(
        _gen_fc_trees.size()
    );


    total_hops = 0;


    for (size_t tid = 0;
         tid < _gen_fc_trees.size();
         ++tid) {

        const ZCubeFCTree &tree =
            _gen_fc_trees[tid];

        TreeRuntime &rt =
            runtime[tid];


        assert(tree.root >= 0);
        assert(tree.root < 16);

        assert(tree.nchunks > 0);

        assert(tree.edges.size() == 15);


        for (const ZCubeFCEdge &e :
             tree.edges) {

            // Logical endpoints must be GPUs.

            assert(e.src >= 0);
            assert(e.src < 16);

            assert(e.dst >= 0);
            assert(e.dst < 16);

            assert(e.src != e.dst);


            // Physical ForestColl route must begin/end
            // at the logical GPU endpoints.

            assert(e.path.size() >= 3);

            assert(
                e.path.front() == e.src
            );

            assert(
                e.path.back() == e.dst
            );


            // AllGather:
            //
            //     src(parent) -> dst(child)

            rt.children[e.src].push_back(e);


            // ReduceScatter needs the reverse relation.
            //
            // Every non-root node has exactly one parent.

            assert(
                rt.parent_edge.find(e.dst)
                ==
                rt.parent_edge.end()
            );

            rt.parent_edge[e.dst] = e;


            // Make sure both nodes exist in pending.

            if (!rt.pending.count(e.src))
                rt.pending[e.src] = 0;

            if (!rt.pending.count(e.dst))
                rt.pending[e.dst] = 0;


            // During RS, src cannot forward upward until
            // every child in the AG tree has arrived.

            rt.pending[e.src]++;


            total_hops++;
        }


        // Root has no parent.

        assert(
            rt.parent_edge.find(tree.root)
            ==
            rt.parent_edge.end()
        );
    }


    assert(
        (int)_gen_fc_trees.size()
        ==
        _gen_fc_num_trees
    );


    assert(
        total_hops
        ==
        _gen_fc_num_trees * 15
    );


    std::cerr
        << "[ForestColl] Loaded generated schedule"
        << " tree_records="
        << _gen_fc_num_trees
        << " unit_chunks="
        << _gen_fc_total_chunks
        << " logical_hops_per_phase="
        << total_hops
        << "\n";
}


// ============================================================================
// Restore RS dependency counters
// ============================================================================

void FFForestCollZCubeTask::reset_reduce_scatter_state()
{
    for (size_t tid = 0;
         tid < _gen_fc_trees.size();
         ++tid) {

        TreeRuntime &rt =
            runtime[tid];


        for (auto &kv : rt.pending)
            kv.second = 0;


        for (const ZCubeFCEdge &e :
             _gen_fc_trees[tid].edges) {

            rt.pending[e.src]++;
        }
    }
}


// ============================================================================
// Start ForestColl
// ============================================================================

void FFForestCollZCubeTask::doNextEvent()
{
    start_time =
        eventlist().now();

    state =
        FFTask::TASK_RUNNING;


    if (operator_size == 0)
        operator_size = 1;


    chunk_bytes =
        (
            operator_size
            + (uint64_t)_gen_fc_total_chunks
            - 1
        )
        /
        (uint64_t)_gen_fc_total_chunks;


    if (chunk_bytes == 0)
        chunk_bytes = 1;


    reset_reduce_scatter_state();

    finished_hops = 0;


    std::cerr
        << "[ForestColl] Starting AllReduce"
        << " operator_size="
        << operator_size
        << " bytes"
        << " unit_chunk_bytes="
        << chunk_bytes
        << " tree_records="
        << _gen_fc_num_trees
        << " unit_chunks="
        << _gen_fc_total_chunks
        << " hops_per_phase="
        << total_hops
        << "\n";


    start_reduce_scatter();
}


// ============================================================================
// ReduceScatter
// ============================================================================
//
// Generated ForestColl trees are OUT-trees:
//
//       root -> children
//
// ReduceScatter uses the reversed trees:
//
//       leaves -> root
//
// Every internal node waits until ALL of its children have arrived.
// ============================================================================

void FFForestCollZCubeTask::start_reduce_scatter()
{
    finished_hops = 0;


    for (size_t tid = 0;
         tid < _gen_fc_trees.size();
         ++tid) {

        const ZCubeFCTree &tree =
            _gen_fc_trees[tid];

        TreeRuntime &rt =
            runtime[tid];


        // All leaves can begin simultaneously.

        for (const auto &kv :
             rt.pending) {

            const int node =
                kv.first;

            const int pending_children =
                kv.second;


            if (node == tree.root)
                continue;


            if (pending_children != 0)
                continue;


            auto pe =
                rt.parent_edge.find(node);


            assert(
                pe != rt.parent_edge.end()
            );


            launch_rs_hop(
                pe->second,
                (int)tid,
                eventlist().now()
            );
        }
    }
}


// ============================================================================
// ReduceScatter flow completion
// ============================================================================

void FFForestCollZCubeTask::rs_hop_done(
        int tree_id,
        int parent)
{
    assert(tree_id >= 0);

    assert(
        tree_id <
        (int)runtime.size()
    );


    finished_hops++;


    TreeRuntime &rt =
        runtime[tree_id];

    const ZCubeFCTree &tree =
        _gen_fc_trees[tree_id];


    auto p =
        rt.pending.find(parent);


    assert(
        p != rt.pending.end()
    );

    assert(
        p->second > 0
    );


    // One child contribution has arrived.

    p->second--;


    // All children have now arrived.
    //
    // Unless this is the tree root, send the reduced
    // chunk toward the parent.

    if (p->second == 0 &&
        parent != tree.root) {

        auto pe =
            rt.parent_edge.find(parent);


        assert(
            pe != rt.parent_edge.end()
        );


        launch_rs_hop(
            pe->second,
            tree_id,
            eventlist().now()
        );
    }


    // Every logical edge in every tree has completed.

    if (finished_hops == total_hops) {

        std::cerr
            << "[ForestColl] ReduceScatter complete at "
            << timeAsMs(eventlist().now())
            << " ms\n";


        start_allgather();
    }
}


// ============================================================================
// AllGather
// ============================================================================
//
// Execute generated ForestColl out-trees directly:
//
//       root -> children -> grandchildren -> ...
// ============================================================================

void FFForestCollZCubeTask::start_allgather()
{
    finished_hops = 0;


    for (size_t tid = 0;
         tid < _gen_fc_trees.size();
         ++tid) {

        const ZCubeFCTree &tree =
            _gen_fc_trees[tid];

        TreeRuntime &rt =
            runtime[tid];


        auto it =
            rt.children.find(tree.root);


        assert(
            it != rt.children.end()
        );


        // Root immediately launches all outgoing tree edges.

        for (const ZCubeFCEdge &e :
             it->second) {

            launch_ag_hop(
                e,
                (int)tid,
                eventlist().now()
            );
        }
    }
}


// ============================================================================
// AllGather flow completion
// ============================================================================

void FFForestCollZCubeTask::ag_hop_done(
        int tree_id,
        int node)
{
    assert(tree_id >= 0);

    assert(
        tree_id <
        (int)runtime.size()
    );


    finished_hops++;


    TreeRuntime &rt =
        runtime[tree_id];


    // As soon as node receives the chunk,
    // forward it to all children.

    auto it =
        rt.children.find(node);


    if (it != rt.children.end()) {

        for (const ZCubeFCEdge &e :
             it->second) {

            launch_ag_hop(
                e,
                tree_id,
                eventlist().now()
            );
        }
    }


    if (finished_hops == total_hops) {

        finish_allreduce();
    }
}


// ============================================================================
// Launch one ReduceScatter logical edge
// ============================================================================

void FFForestCollZCubeTask::launch_rs_hop(
        const ZCubeFCEdge &forward_edge,
        int tree_id,
        simtime_picosec t0)
{
    assert(tree_id >= 0);

    assert(
        tree_id <
        (int)_gen_fc_trees.size()
    );


    const ZCubeFCTree &tree =
        _gen_fc_trees[tree_id];


    uint64_t bytes =
        chunk_bytes
        *
        (uint64_t)tree.nchunks;


    if (bytes == 0)
        bytes = 1;


    // Generated edge:
    //
    //     parent -> child
    //
    // RS:
    //
    //     child -> parent

    const int src =
        forward_edge.dst;

    const int dst =
        forward_edge.src;


    const bool ok =
        start_flow(
            src,
            dst,
            bytes,
            tree_id,
            dst,
            FCFPhase::REDUCE_SCATTER,
            t0
        );


    if (!ok) {

        std::cerr
            << "[ForestColl] ERROR:"
            << " failed RS flow "
            << src
            << " -> "
            << dst
            << " tree="
            << tree_id
            << "\n";

        assert(false);
    }
}


// ============================================================================
// Launch one AllGather logical edge
// ============================================================================

void FFForestCollZCubeTask::launch_ag_hop(
        const ZCubeFCEdge &edge,
        int tree_id,
        simtime_picosec t0)
{
    assert(tree_id >= 0);

    assert(
        tree_id <
        (int)_gen_fc_trees.size()
    );


    const ZCubeFCTree &tree =
        _gen_fc_trees[tree_id];


    uint64_t bytes =
        chunk_bytes
        *
        (uint64_t)tree.nchunks;


    if (bytes == 0)
        bytes = 1;


    const bool ok =
        start_flow(
            edge.src,
            edge.dst,
            bytes,
            tree_id,
            edge.dst,
            FCFPhase::ALL_GATHER,
            t0
        );


    if (!ok) {

        std::cerr
            << "[ForestColl] ERROR:"
            << " failed AG flow "
            << edge.src
            << " -> "
            << edge.dst
            << " tree="
            << tree_id
            << "\n";

        assert(false);
    }
}


// ============================================================================
// Start one TCP flow
// ============================================================================
//
// IMPORTANT:
//
// The generated ForestColl schedule contains an exact physical path:
//
//     e.path = { GPU, switch, ..., GPU }
//
// However, the existing htsim Topology API used here exposes:
//
//     get_paths(src, dst)
//
// and returns Route objects.
//
// Therefore this implementation currently executes the exact ForestColl:
//
//     * tree structure
//     * tree dependencies
//     * nchunks weighting
//     * ReduceScatter reversal
//     * AllGather direction
//
// but obtains the physical Route from ZCubeTopology::get_paths().
//
// We should subsequently extend ZCubeTopology to select the Route matching
// e.path exactly. That is a routing-correctness change, separate from the
// FFApplication/linking issue.
// ============================================================================

bool FFForestCollZCubeTask::start_flow(
        int src,
        int dst,
        uint64_t bytes,
        int tree_id,
        int arrival_node,
        FCFPhase phase,
        simtime_picosec t0)
{
    auto *ctx =
        new FFForestCollHop{
            this,
            tree_id,
            arrival_node,
            phase
        };


    RoCEv2Src *fs =
        new RoCEv2Src(
            nullptr,
            eventlist(),
            src,
            dst,
            ar_finish_forestcoll_zcube,
            static_cast<void *>(ctx)
        );


    RoCEv2Sink *snk =
        new RoCEv2Sink(nullptr, eventlist());


    fs->set_flowsize(bytes);


    fs->set_initial_credits(RoCEv2Src::default_initial_credits);


    // ------------------------------------------------------------------------
    // Forward route
    // ------------------------------------------------------------------------

    auto *sp =
        _topology->get_paths(
            src,
            dst
        );


    if (!sp ||
        sp->empty()) {

        delete sp;
        delete ctx;
        delete snk;
        delete fs;

        return false;
    }


    Route *ro =
        new Route(
            *(sp->at(
                ecmp_path_index(src, dst, sp->size())
            ))
        );


    ro->push_back(snk);

    delete sp;


    // ------------------------------------------------------------------------
    // Reverse route for TCP ACKs
    // ------------------------------------------------------------------------

    auto *dp =
        _topology->get_paths(
            dst,
            src
        );


    if (!dp ||
        dp->empty()) {

        delete dp;
        delete ctx;
        delete snk;
        delete ro;
        delete fs;

        return false;
    }


    Route *ri =
        new Route(
            *(dp->at(
                ecmp_path_index(dst, src, dp->size())
            ))
        );


    ri->push_back(fs);

    delete dp;


    // ------------------------------------------------------------------------
    // Start TCP
    // ------------------------------------------------------------------------

    fs->connect(
        *ro,
        *ri,
        *snk,
        t0
    );


    return true;
}


// ============================================================================
// Finish AllReduce
// ============================================================================

void FFForestCollZCubeTask::finish_allreduce()
{
    finish_time =
        eventlist().now();


    std::cerr
        << "[ForestColl] AllGather complete at "
        << timeAsMs(finish_time)
        << " ms\n";


    std::cerr
        << "[ForestColl] AllReduce complete"
        << " logical_flows_RS="
        << total_hops
        << " logical_flows_AG="
        << total_hops
        << " total_logical_flows="
        << (2 * total_hops)
        << "\n";


    cleanup();
}
