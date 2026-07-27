// circulant128_topology.h — C(128,{1,3,7,17}), 128 nodes, degree 4
#pragma once

#ifndef QT
#define QT
typedef enum {
    RANDOM, ECN, COMPOSITE, CTRL_PRIO,
    LOSSLESS, LOSSLESS_INPUT, LOSSLESS_INPUT_ECN
} queue_type;
#endif

#include "queue.h"
#include "pipe.h"
#include "ecnqueue.h"
#include "switch.h"
// CHANGE 1: Swapped topology.h for flattopology.h
#include "flat_topology.h"
#include "firstfit.h"
#include "logfile.h"
#include "eventlist.h"
#include "config.h"
#include <vector>
#include <map>
#include <unordered_map>

static const int CIRC128_N   = 128;
static const int CIRC128_DEG = 4;

struct Circulant128BFBTransmission {
    int sender, receiver, origin, chunks;
};

// CHANGE 2: Inherit from FlatTopology instead of Topology
class Circulant128Topology : public FlatTopology {
public:
    Circulant128Topology(int no_of_nodes, mem_b queuesize,
                         Logfile *lg, EventList *ev,
                         FirstFit *fit, queue_type q);
    ~Circulant128Topology();

    std::vector<const Route*>* get_paths(int src, int dst) override;
    std::vector<const Route*>* get_direct_route(int src, int dst);
    int no_of_nodes() const override { return CIRC128_N; }
    std::vector<int>* get_neighbours(int src);

    int bfb_num_steps()             const { return (int)_bfb_ag_steps_on_G.size(); }
    int bfb_ag_total_chunks_on_G()  const { return _bfb_ag_total_chunks_on_G; }
    int bfb_rs_total_chunks_on_G()  const { return _bfb_rs_total_chunks_on_G; }
    int bfb_ag_total_chunks_on_GT() const { return _bfb_ag_total_chunks_on_GT; }
    int bfb_rs_total_chunks_on_GT() const { return _bfb_rs_total_chunks_on_GT; }

    const std::vector<Circulant128BFBTransmission>& bfb_ag_step_on_G(int t)  const { return _bfb_ag_steps_on_G[t]; }
    const std::vector<Circulant128BFBTransmission>& bfb_rs_step_on_G(int t)  const { return _bfb_rs_steps_on_G[t]; }
    const std::vector<Circulant128BFBTransmission>& bfb_ag_step_on_GT(int t) const { return _bfb_ag_steps_on_GT[t]; }
    const std::vector<Circulant128BFBTransmission>& bfb_rs_step_on_GT(int t) const { return _bfb_rs_steps_on_GT[t]; }

    // CHANGE 3: Deleted local 'queues' and 'pipes' vectors. 
    // They are now inherited directly from FlatTopology.

private:
    void build_graph();
    void load_bfb_routes();
    void load_bfb_schedule();
    void init_network();

    int        _no_of_nodes;
    mem_b      _queuesize;
    Logfile   *logfile;
    EventList *eventlist;
    FirstFit  *ff;
    queue_type qt;

    std::vector<std::vector<int>> _adj;
    std::unordered_map<uint64_t, std::vector<std::vector<int>>> _routes;

    int _bfb_ag_total_chunks_on_G  = 0;
    int _bfb_rs_total_chunks_on_G  = 0;
    int _bfb_ag_total_chunks_on_GT = 0;
    int _bfb_rs_total_chunks_on_GT = 0;

    std::vector<std::vector<Circulant128BFBTransmission>> _bfb_ag_steps_on_G;
    std::vector<std::vector<Circulant128BFBTransmission>> _bfb_rs_steps_on_G;
    std::vector<std::vector<Circulant128BFBTransmission>> _bfb_ag_steps_on_GT;
    std::vector<std::vector<Circulant128BFBTransmission>> _bfb_rs_steps_on_GT;
};
