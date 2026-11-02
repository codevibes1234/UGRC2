#ifndef GRAPH_TOPOLOGY_H
#define GRAPH_TOPOLOGY_H

#include "main.h"
#include "fc_schedule.h"
#include "pipe.h"
#include "config.h"
#include "loggers.h"
#include "network.h"
#include "firstfit.h"
#include "topology.h"
#include "logfile.h"
#include "eventlist.h"
#include "zcube_topology.h"
#include <map>
#include <string>
#include <vector>

struct SwitchGraph {
    std::string name;
    int nsw = 0;
    int nhost = 0;
    std::vector<int> host_switch;
    std::vector<std::vector<int>> adj;
    int host_ports = 1;
};

class GraphTopology : public Topology, public FCScheduleSource,
                      public ExplicitRouteTopology {
public:
    GraphTopology(const SwitchGraph& g, mem_b queuesize, Logfile* log,
                  EventList* ev, FirstFit* f, queue_type q, int max_paths = 8,
                  const std::string& routing = "ecmp",
                  mem_b lossless_qsize = 0);

    virtual vector<const Route*>* get_paths(int src, int dest);
    virtual vector<int>* get_neighbours(int  ) { return NULL; }
    virtual int no_of_nodes() const { return _g.nhost; }

    void load_fc_schedule();
    bool load_fc_schedule_file(const std::string& path);
    virtual const std::vector<ZCubeFCTransmission>& fc_rs_step(int step) const;
    virtual const std::vector<ZCubeFCTransmission>& fc_ag_step(int step) const;
    virtual int fc_num_steps()       const;
    virtual int fc_rs_total_chunks() const;
    virtual int fc_ag_total_chunks() const;
    int fc_schedule_nhost() const { return _fc_schedule_nhost; }
    virtual bool fc_has_paths() const { return _fc_has_paths; }
    virtual const std::vector<int>& fc_rs_path(int step, int idx) const;
    virtual const std::vector<int>& fc_ag_path(int step, int idx) const;

    virtual Route* route_for_switch_path(int src, int dst,
                                         const std::vector<int> &sw);

    int diameter() const { return _diameter; }
    double mean_hops() const { return _mean_hops; }

private:
    Queue* alloc_src_queue(QueueLogger* ql, uint64_t speed);
    Queue* alloc_queue(QueueLogger* ql, uint64_t speed);
    void add_in(Route* rt, Queue* q);
    void build_links();
    void compute_distances();
    void enumerate(int a, int b, std::vector<std::vector<int>>& out);
    void build_updown_order();
    void enumerate_updown(int a, int b, std::vector<std::vector<int>>& out);
    bool is_up(int u, int v) const;

    std::vector<std::vector<ZCubeFCTransmission>> _fc_rs_steps;
    std::vector<std::vector<ZCubeFCTransmission>> _fc_ag_steps;
    int _fc_num_steps        = 0;
    int _fc_rs_total_chunks  = 0;
    int _fc_ag_total_chunks  = 0;
    int _fc_schedule_nhost   = -1;
    bool _fc_has_paths       = false;
    std::vector<std::vector<std::vector<int>>> _fc_rs_paths;
    std::vector<std::vector<std::vector<int>>> _fc_ag_paths;

    SwitchGraph _g;
    queue_type qt;
    mem_b _queuesize;
    mem_b _lossless_qsize;
    Logfile* logfile;
    EventList* eventlist;
    FirstFit* ff;
    uint64_t _linkspeed;
    int _max_paths;
    int _diameter = 0;
    double _mean_hops = 0;
    std::string _routing;
    std::vector<int> _udepth;

    std::vector<Queue*> q_host_sw, q_sw_host;
    std::vector<Pipe*>  p_host_sw, p_sw_host;
    std::map<std::pair<int,int>, Queue*> q_sw_sw;
    std::map<std::pair<int,int>, Pipe*>  p_sw_sw;
    std::vector<std::vector<int>> _dist;
};

#endif
