// biring128_topology.cpp
#include "biring128_topology.h"
#include "ecnqueue.h"
#include "randomqueue.h"
#include "compositequeue.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <deque>

extern uint32_t RTT;
extern uint32_t SPEED;
extern uint32_t ECNTHRESH;

// Defined in biring128_routes_generated.cpp (separate translation unit)
extern void load_biring128_routes(
    std::unordered_map<uint64_t, std::vector<std::vector<int>>>& _routes);

BiRing128Topology::BiRing128Topology(int no_of_nodes, mem_b queuesize,
                   Logfile *lg, EventList *ev,
                   FirstFit *fit, queue_type q)
    : _no_of_nodes(no_of_nodes), _queuesize(queuesize),
      logfile(lg), eventlist(ev), ff(fit), qt(q)
{
    assert(no_of_nodes == 128);
    queues.assign(128, std::vector<Queue*>(128, nullptr));
    pipes .assign(128, std::vector<Pipe* >(128, nullptr));

    build_graph();
    load_bfb_routes();
    load_bfb_schedule();
    init_network();

    std::cerr << "[BiRing128Topology] ready: N=128"
              << " bfb_steps=" << bfb_num_steps()
              << " P_RS_G=" << _bfb_rs_total_chunks_on_G << "\n";
}

BiRing128Topology::~BiRing128Topology() {
    for (int u=0;u<128;u++) for (int v=0;v<128;v++) {
        delete queues[u][v]; delete pipes[u][v];
    }
}

void BiRing128Topology::build_graph() {
    _adj.assign(128,{});
    for (int a=0;a<8;a++) for (int b=0;b<4;b++) for (int c=0;c<4;c++) {
        int u=a*16+b*4+c;
        _adj[u]={((a+1)%8)*16+b*4+c, ((a+7)%8)*16+b*4+c,
                  a*16+((b+1)%4)*4+c, a*16+b*4+(c+1)%4};
    }
    std::cerr << "[BiRing128Topology] Graph built: 128 nodes\n";
}

void BiRing128Topology::load_bfb_routes() {
    load_biring128_routes(_routes);
    std::cerr << "[BiRing128Topology] Routes loaded\n";
}

void BiRing128Topology::load_bfb_schedule() {
#include "biring128_schedule_generated.cpp"
    std::cerr << "[BiRing128Topology] Schedule loaded\n";
}

void BiRing128Topology::init_network() {
    linkspeed_bps speed = (linkspeed_bps)SPEED * 1000000ULL;
    for (int u=0;u<128;u++) for (int v:_adj[u]) {
        if (!queues[u][v]) {
            queues[u][v] = new Queue(speed, _queuesize, *eventlist, nullptr);
            pipes [u][v] = new Pipe(timeFromUs(1.0), *eventlist);
            queues[u][v]->setName("Q-"+std::to_string(u)+"->"+std::to_string(v));
            pipes [u][v]->setName("P-"+std::to_string(u)+"->"+std::to_string(v));
        }
        // reverse for ACKs
        if (!queues[v][u]) {
            queues[v][u] = new Queue(speed, _queuesize, *eventlist, nullptr);
            pipes [v][u] = new Pipe(timeFromUs(1.0), *eventlist);
            queues[v][u]->setName("Q-"+std::to_string(v)+"->"+std::to_string(u));
            pipes [v][u]->setName("P-"+std::to_string(v)+"->"+std::to_string(u));
        }
    }
}

std::vector<const Route*>* BiRing128Topology::get_paths(int src, int dst) {
    auto* v = new std::vector<const Route*>();
    if (src == dst) return v;
    uint64_t key = (uint64_t)src * 128 + dst;
    auto it = _routes.find(key);
    if (it == _routes.end() || it->second.empty()) {
        std::cerr << "[BiRing128Topology] no route " << src << "->" << dst << "\n";
        return v;
    }
    for (const auto& path : it->second) {
        Route* r = new Route();
        for (int k=0; k+1<(int)path.size(); k++) {
            int u=path[k], w=path[k+1];
            r->push_back(queues[u][w]);
            r->push_back(pipes [u][w]);
        }
        v->push_back(r);
    }
    return v;
}

std::vector<int>* BiRing128Topology::get_neighbours(int src) {
    return new std::vector<int>(_adj[src]);
}
