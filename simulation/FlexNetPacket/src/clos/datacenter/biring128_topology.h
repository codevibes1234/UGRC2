// biring128_topology.h — BiRing(2,8)□UniRing(1,4)², 128 nodes, degree 4, diameter 10
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
#include "topology.h"
#include "firstfit.h"
#include "logfile.h"
#include "eventlist.h"
#include "config.h"
#include <vector>
#include <map>
#include <unordered_map>

static const int BIRING128_N   = 128;
static const int BIRING128_DEG = 4;
static const int BIRING128_D   = 10;

struct BiRing128BFBTransmission {
    int sender, receiver, origin, chunks;
};

class BiRing128Topology : public Topology {
public:
    BiRing128Topology(int no_of_nodes, mem_b queuesize,
                      Logfile *lg, EventList *ev,
                      FirstFit *fit, queue_type q);
    ~BiRing128Topology();

    std::vector<const Route*>* get_paths(int src, int dst) override;
    int no_of_nodes() const override { return BIRING128_N; }
    std::vector<int>* get_neighbours(int src);

    int bfb_num_steps()             const { return (int)_bfb_ag_steps_on_G.size(); }
    int bfb_ag_total_chunks_on_G()  const { return _bfb_ag_total_chunks_on_G; }
    int bfb_rs_total_chunks_on_G()  const { return _bfb_rs_total_chunks_on_G; }
    int bfb_ag_total_chunks_on_GT() const { return _bfb_ag_total_chunks_on_GT; }
    int bfb_rs_total_chunks_on_GT() const { return _bfb_rs_total_chunks_on_GT; }

    const std::vector<BiRing128BFBTransmission>& bfb_ag_step_on_G(int t)  const { return _bfb_ag_steps_on_G[t]; }
    const std::vector<BiRing128BFBTransmission>& bfb_rs_step_on_G(int t)  const { return _bfb_rs_steps_on_G[t]; }
    const std::vector<BiRing128BFBTransmission>& bfb_ag_step_on_GT(int t) const { return _bfb_ag_steps_on_GT[t]; }
    const std::vector<BiRing128BFBTransmission>& bfb_rs_step_on_GT(int t) const { return _bfb_rs_steps_on_GT[t]; }

    std::vector<std::vector<Queue*>> queues;
    std::vector<std::vector<Pipe*>>  pipes;

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

    std::vector<std::vector<BiRing128BFBTransmission>> _bfb_ag_steps_on_G;
    std::vector<std::vector<BiRing128BFBTransmission>> _bfb_rs_steps_on_G;
    std::vector<std::vector<BiRing128BFBTransmission>> _bfb_ag_steps_on_GT;
    std::vector<std::vector<BiRing128BFBTransmission>> _bfb_rs_steps_on_GT;
};
