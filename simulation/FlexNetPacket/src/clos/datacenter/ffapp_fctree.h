#ifndef FFAPP_FCTREE_H
#define FFAPP_FCTREE_H

#include "ffapp_rhd.h"
#include "topology.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

struct FCTree {
    int root = -1;
    int nchunks = 1;
    int index = 0;
    std::vector<int> parent;
    std::vector<std::pair<int, int>> edges;
    std::vector<std::vector<int>> paths;
};

struct FCTreeSchedule {
    int nhost = 0;
    int nchunkspernode = 0;
    long dropped_sends = 0;
    std::vector<FCTree> trees;

    bool load_xml(const std::string& path, int nhost_expected);
    int total_chunks() const;
};

class FFFCTreeAllreduceTask : public FFTask {
public:
    struct Hop {
        int tree, phase;
        int src, dst;
        std::vector<int> sw;
        int next_piece = 0;
        int in_flight = 0;
    };
    struct FlowCtx { FFFCTreeAllreduceTask* task; int hop; int piece; };

    Topology*        _topology;
    FCTreeSchedule*  _sched;
    uint64_t         operator_size;
    uint64_t         chunk_bytes = 1;

    int  pieces = 1;
    int  window = 1;
    bool phase_barrier = true;
    bool use_fc_paths = false;
    long n_fc_routed = 0, n_fallback_routed = 0;

    long flows_started = 0, max_in_flight = 0, in_flight = 0;
    simtime_picosec rs_finish = 0;

    FFFCTreeAllreduceTask(FFApplication* ffapp, Topology* top,
                          FCTreeSchedule* sched, uint64_t sz);

    void build();

    void doNextEvent() override;
    void piece_done(int hop, int piece);

private:
    std::vector<Hop> hops;
    std::vector<FlowCtx> ctx;
    std::vector<std::vector<int>> rs_hop_of;
    std::vector<std::vector<std::vector<int>>> ag_hops_of;
    std::vector<std::vector<int>> nchild;
    std::vector<std::vector<uint16_t>> rs_have;
    std::vector<std::vector<uint8_t>>  ag_have;
    std::vector<uint64_t> tree_bytes;
    long rs_total = 0, rs_done = 0;
    long ag_total = 0, ag_done = 0;
    bool ag_released = false;
    std::ofstream null_out;

    uint64_t piece_bytes(int tree, int piece) const;
    bool piece_ready(const Hop& h, int piece) const;
    void try_launch(int hop);
    void start_flow(int hop, int piece, simtime_picosec t0);
};

void ar_finish_fc_tree(void* ctx_ptr);

#endif
