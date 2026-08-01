// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * ffapp_rhd.cpp
 * =============
 * Self-contained implementation for ZCube(4,2) recursive-halving (RS) +
 * recursive-doubling (AG) AllReduce microbenchmark.
 *
 * Mirrors ffapp_l2k44.cpp's isolation pattern exactly.
 *
 * Contains:
 *   1. Minimal FFApplication stub  — only the fields/methods used by the bench
 *   2. Minimal FFTask stub         — only what FFZCubeRHDAllreduceTask needs
 *   3. FFZCubeRHDAllreduceTask     — full recursive halving/doubling AllReduce
 *
 * Does NOT contain:
 *   - load_taskgraph_flatbuf / load_taskgraph_json
 *   - FFRingAllreduce, FFPSAllreduce, FFDPSAllreduce, etc.
 *   - Any BFB (Circulant/LK44/L2K44/DistReg32/etc.) AllReduce
 *   - FFRecHalvAllreduceTask (the FatTree recursive-halving variant —
 *     not used by main_tcp_zcube_rhd_bench.cpp)
 *
 * Unlike ffapp_l2k44.cpp, FFZCubeRHDAllreduceTask only needs the generic
 * Topology interface (get_paths) — no topology-specific BFB schedule
 * methods or downcasting, so no topology-specific header is required here.
 *
 * Compile against htsim core only, linked with main_tcp_zcube_rhd_bench.cpp.
 */

#include "ffapp_rhd.h"
#include "dctcp.h"
#include "topology.h"
#include <numeric>
#include <iostream>
#include <map>

// ============================================================================
// Static members
// ============================================================================

int FFApplication::total_apps    = 0;
int FFApplication::finished_apps = 0;

// ============================================================================
// FFApplication — minimal stub constructor
// Only initialises fields that FFZCubeRHDAllreduceTask touches.
// ============================================================================

FFApplication::FFApplication(Topology* top, int ss, ofstream* _fstream_out,
                             TcpRtxTimerScanner& rtx, EventList& evl,
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
      nnodes(0),
      nswitches(0),
      cwnd(0),
      pull_rate(0.0)
{
    FFApplication::total_apps++;
}

// Second constructor (with explicit gpus vector) — required by ffapp.h
FFApplication::FFApplication(Topology* top, int ss, ofstream* _fstream_out,
                             std::vector<int> gpus_,
                             TcpRtxTimerScanner& rtx, EventList& evl,
                             FFAllReduceStrategy ars)
    : topology(top),
      ssthresh(ss),
      eventlist(evl),
      fstream_out(_fstream_out),
      gpus(gpus_),
      tcpRtxScanner(rtx),
      allreduce_strategy(ars),
      final_finish_time(0),
      first_iter_time(0),
      n_finished_tasks(0),
      finished_once(false),
      fancy_ring(false),
      bfb_rto_ms(1.0),
      ngpupernode(1),
      nnodes(0),
      nswitches(0),
      cwnd(0),
      pull_rate(0.0)
{
    FFApplication::total_apps++;
}

FFApplication::~FFApplication()
{
    for (auto& kv : tasks)   delete kv.second;
    for (auto& kv : devices) delete kv.second;
}

// Stubs for methods declared in ffapp.h but not needed by the bench
void FFApplication::load_taskgraph_json(std::string&)    { assert(false && "not used in bench"); }
void FFApplication::load_taskgraph_flatbuf(std::string&) { assert(false && "not used in bench"); }
void FFApplication::start_init_tasks()                   { assert(false && "not used in bench"); }
void FFApplication::reset_and_restart()                  { assert(false && "not used in bench"); }

std::vector<int> FFApplication::choose_gpus(
    std::unordered_set<int>& candidates, int n)
{
    assert(false && "not used in bench");
    return {};
}

// ============================================================================
// FFTask — minimal implementation
// Only the methods called from FFZCubeRHDAllreduceTask::cleanup()
// ============================================================================

FFTask::FFTask(FFApplication* app, FFTaskType t)
    : EventSource(app->eventlist, "FFTask"), ffapp(app),
      type(t), state(TASK_NOT_READY), device(nullptr),
      counter(0), xfersize(0), src_node(-1), dst_node(-1),
      ready_time(0), run_time(0), start_time(0), finish_time(0)
{}

// Stub constructors required by the linker (header declares them)
FFTask::FFTask(FFApplication* app, std::string type_str,
               FFDevice* dev, uint64_t sz, float rt)
    : EventSource(app->eventlist, "FFTask"), ffapp(app),
      type(TASK_COMM), state(TASK_NOT_READY), device(dev),
      counter(0), xfersize(sz), src_node(-1), dst_node(-1),
      ready_time(0), run_time((simtime_picosec)(rt * 1e9)),
      start_time(0), finish_time(0)
{ (void)type_str; }

FFTask::FFTask(FFApplication* app, int /*tt*/,
               FFDevice* dev, uint64_t sz, float rt)
    : EventSource(app->eventlist, "FFTask"), ffapp(app),
      type(TASK_COMM), state(TASK_NOT_READY), device(dev),
      counter(0), xfersize(sz), src_node(-1), dst_node(-1),
      ready_time(0), run_time((simtime_picosec)(rt * 1e9)),
      start_time(0), finish_time(0)
{}

void FFTask::doNextEvent() {}

// cleanup() — called by FFZCubeRHDAllreduceTask when AllReduce completes.
// Records finish time, increments counters, signals simulation end.
void FFTask::cleanup()
{
    state = TASK_FINISHED;
    ffapp->n_finished_tasks++;

    if (ffapp->final_finish_time < finish_time)
        ffapp->final_finish_time = finish_time;

    // Notify successors (none in bench mode, but kept for correctness)
    for (uint64_t nid : next_tasks) {
        FFTask* t = ffapp->tasks[nid];
        if (--t->counter == 0) {
            t->ready_time = finish_time;
            t->state = TASK_READY;
            eventlist().sourceIsPending(*t, t->ready_time);
        }
    }

    // Signal simulation end when all tasks complete
    if (ffapp->n_finished_tasks == ffapp->tasks.size()) {
        if (!ffapp->finished_once) {
            ffapp->finished_once   = true;
            ffapp->first_iter_time = ffapp->final_finish_time;
            FFApplication::finished_apps++;
        }
        if (FFApplication::finished_apps == FFApplication::total_apps)
            eventlist().setEndtime(eventlist().now());
    }
}

// Stubs for methods declared in ffapp.h but unreachable in bench mode
void FFTask::taskstart()          {}
void FFTask::start_flow()         {}
void FFTask::execute_compute()    {}
void FFTask::add_nextask(FFTask*) {}

// taskfinish callback (required by linker)
void taskfinish(void* task)
{
    FFTask* t = static_cast<FFTask*>(task);
    t->finish_time = t->eventlist().now();
    t->cleanup();
}

// FFDevice stub constructors (required by linker)
FFDevice::FFDevice(FFApplication* app, std::string type_str, float bw,
                   int nid, int gid, int fn, int tn, int fg, int tg)
    : ffapp(app), node_id(nid), gpu_id(gid), bandwidth(bw * 8 * 1000),
      type(DEVICE_GPU), state(DEVICE_IDLE),
      from_gpu(fg), to_gpu(tg), from_node(fn), to_node(tn), busy_up_to(0)
{ (void)type_str; }

FFDevice::FFDevice(FFApplication* app, int /*dtype*/,
                   uint64_t nid, uint64_t prop, uint64_t bw)
    : ffapp(app), node_id((int)nid), gpu_id(0),
      bandwidth((float)bw * 8 * 1000),
      type(DEVICE_GPU), state(DEVICE_IDLE),
      from_gpu(0), to_gpu(0), from_node(0), to_node(0), busy_up_to(0)
{ (void)prop; }

// ============================================================================
// FFZCubeRHDAllreduceTask — full implementation
// Recursive halving (RS) + recursive doubling (AG) AllReduce.
// Uses only the generic Topology interface (get_paths) — works with any
// Topology subclass, including ZCubeTopology, with no downcasting needed.
// ============================================================================

void ar_finish_rhd_zcube(void* ar_ptr)
{
    FFZCubeRHDAllreduceTask* ar = static_cast<FFZCubeRHDAllreduceTask*>(ar_ptr);
    ar->finished_curr_step++;
    if (ar->finished_curr_step == ar->flows_this_step)
        ar->advance();
}

FFZCubeRHDAllreduceTask::FFZCubeRHDAllreduceTask(
        FFApplication*         ffapp,
        Topology*              top,
        std::vector<uint64_t>  ng,
        uint64_t               sz,
        double                 local_runtime)
    : FFTask(ffapp, FFTask::TASK_ALLREDUCE),
      _topology(top),
      node_group(ng),
      operator_size(sz > 0 ? sz : 1ULL),
      curr_step(0), finished_curr_step(0), flows_this_step(0)
{
    run_time = (simtime_picosec)(local_runtime * 1e9);
    xfersize = sz;

    int N = (int)ng.size();
    assert(N > 0 && (N & (N - 1)) == 0);  // must be power of 2
    num_steps = 0;
    while ((1 << num_steps) < N) num_steps++;
    // N=16 -> num_steps=4, total unified steps = 8
}

void FFZCubeRHDAllreduceTask::doNextEvent()
{
    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;

    if (operator_size == 0) operator_size = 1;

    std::cerr << "[ZCubeRHD] Starting AllReduce N=" << node_group.size()
              << " num_steps=" << num_steps
              << " size=" << operator_size << " bytes\n";

    curr_step = finished_curr_step = flows_this_step = 0;
    start_all_flows(curr_step);
}

void FFZCubeRHDAllreduceTask::start_all_flows(int step)
{
    int N = (int)node_group.size();

    // RS phase: steps 0..num_steps-1
    //   step s: dist = 1<<s, bytes = operator_size / 2^(s+1)
    // AG phase: steps num_steps..2*num_steps-1
    //   ag_s = step - num_steps
    //   dist = 1 << (num_steps - 1 - ag_s), bytes = operator_size / 2^(num_steps - ag_s)

    uint64_t flow_bytes;
    int      dist;

    if (step < num_steps) {
        dist       = 1 << step;
        flow_bytes = operator_size >> (step + 1);
    } else {
        int ag_s   = step - num_steps;
        dist       = 1 << (num_steps - 1 - ag_s);
        flow_bytes = operator_size >> (num_steps - ag_s);
    }
    if (flow_bytes == 0) flow_bytes = 1;

    std::cerr << "[ZCubeRHD] step=" << step
              << (step < num_steps ? " RS" : " AG")
              << " dist=" << dist
              << " flow_bytes=" << flow_bytes << "\n";

    finished_curr_step = 0;
    flows_this_step    = N;

    simtime_picosec t0 = (step == 0) ? start_time + run_time : eventlist().now();

    for (int i = 0; i < N; i++) {
        int src = (int)node_group[i];
        int dst = (int)node_group[i ^ dist];
        start_one_flow(src, dst, flow_bytes, t0);
    }
}

void FFZCubeRHDAllreduceTask::start_one_flow(int src, int dst,
                                              uint64_t bytes,
                                              simtime_picosec t0)
{
    auto* paths_out = _topology->get_paths(src, dst);
    auto* paths_in  = _topology->get_paths(dst, src);
    Route* routeout = new Route(*paths_out->at(0));
    Route* routein  = new Route(*paths_in->at(0));
    delete paths_out;
    delete paths_in;

    DCTCPSrc* fs = new DCTCPSrc(
        nullptr, nullptr, ffapp->fstream_out,
        eventlist(), src, dst,
        ar_finish_rhd_zcube, static_cast<void*>(this));
    TcpSink* snk = new TcpSink();
    fs->set_flowsize(bytes);
    fs->set_ssthresh(ffapp->ssthresh * Packet::data_packet_size());
    fs->_rto = timeFromMs(10);
    ffapp->tcpRtxScanner.registerTcp(*fs);

    routeout->push_back(snk);
    routein->push_back(fs);

    fs->connect(*routeout, *routein, *snk, t0);
}

void FFZCubeRHDAllreduceTask::advance()
{
    curr_step++;
    if (curr_step == 2 * num_steps) {
        finish_time = eventlist().now();
        std::cerr << "[ZCubeRHD] AllReduce complete at "
                  << timeAsMs(finish_time) << " ms\n";
        cleanup();
    } else {
        start_all_flows(curr_step);
    }
}
