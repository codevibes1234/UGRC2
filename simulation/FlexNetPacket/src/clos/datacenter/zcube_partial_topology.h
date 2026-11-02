#ifndef ZCUBE_PARTIAL_TOPOLOGY
#define ZCUBE_PARTIAL_TOPOLOGY

#include "main.h"
#include "pipe.h"
#include "config.h"
#include "loggers.h"
#include "network.h"
#include "firstfit.h"
#include "topology.h"
#include "logfile.h"
#include "eventlist.h"
#include "zcube_topology.h"
#include <vector>

class ZCubePartialTopology : public Topology {
public:
    ZCubePartialTopology(int no_of_nodes, mem_b queuesize, Logfile* log,
                         EventList* ev, FirstFit* f, queue_type q);

    void init_network();
    virtual vector<const Route*>* get_paths(int src, int dest);
    virtual vector<int>* get_neighbours(int  ) { return NULL; }
    virtual int no_of_nodes() const { return _no_of_nodes; }

    int n;
    int NPOD;
    int NL0, NL1;
    int NC;
    int NSRV;

    int gpu_pod(int g) const { return g / (n * n); }
    int gpu_l0(int g)  const { return gpu_pod(g) * n + (g % (n * n)) / n; }
    int gpu_l1(int g)  const { return gpu_pod(g) * n + (g % (n * n)) % n; }

private:
    Queue* alloc_src_queue(QueueLogger* ql, uint64_t speed);
    Queue* alloc_queue(QueueLogger* ql, uint64_t speed);
    void add_in(Route* rt, Queue* q);

    Logfile* logfile;
    EventList* eventlist;
    FirstFit* ff;
    queue_type qt;
    mem_b _queuesize;
    int _no_of_nodes;
    uint64_t _linkspeed;

    std::vector<Queue*> q_gpu_l0, q_l0_gpu, q_gpu_l1, q_l1_gpu;
    std::vector<Pipe*>  p_gpu_l0, p_l0_gpu, p_gpu_l1, p_l1_gpu;
    std::vector<std::vector<Queue*>> q_l0_l1, q_l1_l0;
    std::vector<std::vector<Pipe*>>  p_l0_l1, p_l1_l0;
    std::vector<std::vector<Queue*>> q_l1_c, q_c_l1;
    std::vector<std::vector<Pipe*>>  p_l1_c, p_c_l1;
};

#endif
