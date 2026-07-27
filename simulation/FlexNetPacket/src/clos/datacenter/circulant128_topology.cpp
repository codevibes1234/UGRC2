// circulant128_topology.cpp
#include "circulant128_topology.h"
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

Circulant128Topology::Circulant128Topology(int no_of_nodes, mem_b queuesize,
                   Logfile *lg, EventList *ev,
                   FirstFit *fit, queue_type q)
    : FlatTopology(no_of_nodes, queuesize, lg, ev, fit, q), 
      _no_of_nodes(no_of_nodes), _queuesize(queuesize),
      logfile(lg), eventlist(ev), ff(fit), qt(q)
{
    assert(no_of_nodes == 128);
    queues.assign(128, std::vector<Queue*>(128, nullptr));
    pipes .assign(128, std::vector<Pipe* >(128, nullptr));

    build_graph();
    // load_bfb_routes();
    // load_bfb_schedule();
    init_network();

    std::cerr << "[Circulant128Topology] ready: N=128"
              << " bfb_steps=" << bfb_num_steps()
              << " P_RS_G=" << _bfb_rs_total_chunks_on_G << "\n";
}

Circulant128Topology::~Circulant128Topology() {
    for (int u=0;u<128;u++) for (int v=0;v<128;v++) {
        delete queues[u][v]; delete pipes[u][v];
    }
}

void Circulant128Topology::build_graph() {
    _adj.assign(128,{});
    for (int u=0;u<128;u++) for (int s:{1,3,7,17}) _adj[u].push_back((u+s)%128);
    std::cerr << "[Circulant128Topology] Graph built: 128 nodes\n";
}

void Circulant128Topology::load_bfb_routes() {
#include "circulant128_routes_generated.cpp"
    std::cerr << "[Circulant128Topology] Routes loaded\n";
}

void Circulant128Topology::load_bfb_schedule() {
#include "circulant128_schedule_generated.cpp"
    std::cerr << "[Circulant128Topology] Schedule loaded\n";
}

void Circulant128Topology::init_network() {
    linkspeed_bps speed = (linkspeed_bps)SPEED * 1000000ULL;
std::cerr << "[TOPO] SPEED=" << SPEED
<< " Mbps, link_rate=" << speed
<< " bps" << std::endl;
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

std::vector<const Route*>* Circulant128Topology::get_paths(int src, int dst) {
    auto* v = new std::vector<const Route*>();
    if (src == dst) return v;
    uint64_t key = (uint64_t)src * 128 + dst;
    auto it = _routes.find(key);
    if (it == _routes.end() || it->second.empty()) {
        std::cerr << "[Circulant128Topology] no route " << src << "->" << dst << "\n";
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

std::vector<int>* Circulant128Topology::get_neighbours(int src) {
    return new std::vector<int>(_adj[src]);
}

std::vector<const Route*>* Circulant128Topology::get_direct_route(int src, int dst) {
    auto* v = new std::vector<const Route*>();
    if (src == dst) return v;
    if (!queues[src][dst] || !pipes[src][dst]) {
        std::cerr << "[Circulant128Topology] no direct link " << src << "->" << dst << "\n";
        return v;
    }
    Route* r = new Route();
    r->push_back(queues[src][dst]);
    r->push_back(pipes[src][dst]);
    v->push_back(r);
    return v;
}
