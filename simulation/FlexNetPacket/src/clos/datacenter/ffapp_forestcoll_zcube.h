#ifndef FFAPP_FORESTCOLL_ZCUBE_H
#define FFAPP_FORESTCOLL_ZCUBE_H

#include "ffapp.h"

#include <cstdint>
#include <map>
#include <vector>


// ============================================================================
// One logical edge of a generated ForestColl tree.
//
// src/dst are GPU IDs: 0..15.
//
// path is the exact physical path emitted by ForestColl, for example:
//
//     {0, 16, 1}
//
// meaning:
//
//     GPU0 -> switch16 -> GPU1
//
// or:
//
//     {13, 21, 19, 22, 14}
//
// meaning:
//
//     GPU13 -> switch21 -> switch19 -> switch22 -> GPU14
// ============================================================================

struct ZCubeFCEdge {
    int src;
    int dst;

    std::vector<int> path;
};


// ============================================================================
// One batched ForestColl tree record.
//
// IMPORTANT:
//
// The XML contains 25 tree records, but nchunks can be > 1.
// Therefore:
//
//     number of XML tree records = 25
//     total unit-tree chunks     = 32
//
// Do not assume "one record = one chunk".
// ============================================================================

struct ZCubeFCTree {
    int root;
    int tree_idx;
    int nchunks;
    int height;

    std::vector<ZCubeFCEdge> edges;
};


// ============================================================================
// Which half of AllReduce a TCP flow belongs to.
// ============================================================================

enum class FCFPhase {
    REDUCE_SCATTER,
    ALL_GATHER
};


class FFForestCollZCubeTask;


// ============================================================================
// Callback context associated with one TCP flow.
//
// When DCTCPSrc finishes, ar_finish_forestcoll_zcube() receives this object
// and tells the ForestColl task which tree/node has completed.
// ============================================================================

struct FFForestCollHop {
    FFForestCollZCubeTask *task;

    int tree_id;

    // For ReduceScatter:
    //     arrival_node = parent that just received a child contribution.
    //
    // For AllGather:
    //     arrival_node = child that just received its chunk.
    int arrival_node;

    FCFPhase phase;
};


// ============================================================================
// ForestColl ZCube AllReduce task
//
// This intentionally inherits FFTask.
//
// We are NOT linking the original ffapp.cpp.  Instead,
// ffapp_forestcoll_zcube.cpp supplies the minimal FFApplication/FFTask
// implementation required by this benchmark, just like the working
// L2K44 benchmark.
// ============================================================================

class FFForestCollZCubeTask : public FFTask {

public:

    FFForestCollZCubeTask(
        FFApplication *ffapp,
        Topology *topology,
        std::vector<uint64_t> ng,
        uint64_t sz,
        double local_runtime
    );


    // EventSource entry point.
    //
    // main schedules this task at t=0 using:
    //
    //     eventlist.sourceIsPending(*ar, eventlist.now());
    //
    // This starts ReduceScatter.
    void doNextEvent() override;


    // Called when one ReduceScatter TCP flow completes.
    void rs_hop_done(
        int tree_id,
        int parent
    );


    // Called when one AllGather TCP flow completes.
    void ag_hop_done(
        int tree_id,
        int node
    );


private:

    // ========================================================================
    // Runtime dependency information for one generated tree.
    // ========================================================================

    struct TreeRuntime {

        // AllGather:
        //
        // children[parent] = outgoing ForestColl edges.
        //
        // When parent receives its chunk, these edges can fire.
        std::map<
            int,
            std::vector<ZCubeFCEdge>
        > children;


        // ReduceScatter:
        //
        // parent_edge[child] = forward ForestColl edge parent -> child.
        //
        // RS reverses this edge and sends:
        //
        //     child -> parent
        std::map<
            int,
            ZCubeFCEdge
        > parent_edge;


        // ReduceScatter dependency counter.
        //
        // pending[node] = number of child contributions that node
        // still needs before it can send toward its parent.
        std::map<
            int,
            int
        > pending;
    };


    // ========================================================================
    // Benchmark / schedule state
    // ========================================================================

    Topology *_topology;

    std::vector<uint64_t> node_group;


    // AllReduce tensor/message size supplied by -msgsize.
    uint64_t operator_size;


    // Size of one unit ForestColl chunk.
    //
    // With the generated schedule:
    //
    //     total unit chunks = 32
    //
    // approximately:
    //
    //     chunk_bytes = operator_size / 32
    //
    // A tree with nchunks=2 transfers 2 * chunk_bytes per edge.
    uint64_t chunk_bytes;


    // Number of logical tree edges across all batched tree records.
    //
    // Each spanning tree has 15 logical edges.
    int total_hops;


    // Number completed in the CURRENT phase.
    //
    // Reset to zero when transitioning:
    //
    //     ReduceScatter -> AllGather
    int finished_hops;


    // One runtime dependency structure per generated tree record.
    std::vector<TreeRuntime> runtime;


    // ========================================================================
    // Schedule setup
    // ========================================================================

    void build_schedule();

    void reset_reduce_scatter_state();


    // ========================================================================
    // Collective phases
    // ========================================================================

    void start_reduce_scatter();

    void start_allgather();


    // ========================================================================
    // Launch logical ForestColl edges
    // ========================================================================

    void launch_rs_hop(
        const ZCubeFCEdge &forward_edge,
        int tree_id,
        simtime_picosec t0
    );


    void launch_ag_hop(
        const ZCubeFCEdge &edge,
        int tree_id,
        simtime_picosec t0
    );


    // ========================================================================
    // Create one DCTCP flow.
    //
    // NOTE:
    //
    // ZCubeFCEdge::path stores the exact physical ForestColl path.
    // The current .cpp still calls Topology::get_paths(src,dst).
    //
    // That routing issue is separate from the FFApplication build issue.
    // ========================================================================

    bool start_flow(
        int src,
        int dst,
        uint64_t bytes,
        int tree_id,
        int arrival_node,
        FCFPhase phase,
        simtime_picosec t0
    );


    // ========================================================================
    // Complete the FFTask after AllGather.
    // ========================================================================

    void finish_allreduce();
};


// ============================================================================
// DCTCP completion callback
// ============================================================================

void ar_finish_forestcoll_zcube(
    void *info
);


#endif // FFAPP_FORESTCOLL_ZCUBE_H
