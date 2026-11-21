#ifndef FFAPP_MDP_H
#define FFAPP_MDP_H

#include "ffapp_rhd.h"
#include "ffapp_a2a.h"
#include <string>
#include <vector>

struct MDPSpec {
    int nnodes = 128;
    int dp = 2, tp = 4, pp = 4, ep = 4;

    uint64_t dp_bytes = 64ULL << 20;
    uint64_t tp_bytes = 8ULL << 20;
    uint64_t ep_bytes = 8ULL << 20;
    uint64_t pp_bytes = 4ULL << 20;

    std::string allreduce = "rhd";

    bool valid() const { return dp * tp * pp * ep == nnodes; }
};

struct MDPGroups {
    std::vector<std::vector<uint64_t>> dp_groups;
    std::vector<std::vector<uint64_t>> tp_groups;
    std::vector<std::vector<uint64_t>> ep_groups;
    std::vector<std::vector<uint64_t>> pp_chains;
};

MDPGroups mdp_groups(const MDPSpec& s);

class FFPointToPointTask : public FFTask {
public:
    struct Transfer { int src, dst; uint64_t bytes; };

    Topology*             _topology;
    std::vector<Transfer> transfers;
    int                   flows_total;
    int                   flows_finished;
    std::string           label;

    FFPointToPointTask(FFApplication* ffapp, Topology* top,
                       std::vector<Transfer> t, const std::string& lbl);

    void doNextEvent() override;
    void start_one_flow(const Transfer& t, simtime_picosec t0);
    void flow_done();
};

void p2p_flow_finished(void* task_ptr);

void mdp_build_tasks(FFApplication* app, Topology* top, const MDPSpec& spec,
                     const std::vector<std::string>& dims,
                     std::vector<FFTask*>& out,
                     std::vector<std::string>& labels,
                     const std::vector<int>& placement = std::vector<int>());

class FFDelayTask : public FFTask {
public:
    simtime_picosec delay;
    bool            waiting;
    FFDelayTask(FFApplication* app, simtime_picosec d)
        : FFTask(app, FFTask::TASK_COMPUTE), delay(d), waiting(false) {}
    void doNextEvent() override;
};

void mdp_build_phased_tasks(FFApplication* app, Topology* top, const MDPSpec& spec,
                            const std::vector<std::string>& dims,
                            int microbatches,
                            simtime_picosec fwd_compute, simtime_picosec bwd_compute,
                            std::vector<FFTask*>& out,
                            std::vector<std::string>& labels,
                            const std::vector<int>& placement = std::vector<int>());

void mdp_build_1f1b_tasks(FFApplication* app, Topology* top, const MDPSpec& spec,
                          const std::vector<std::string>& dims,
                          int microbatches, int layers_per_stage, int buckets,
                          simtime_picosec fwd_layer_compute, simtime_picosec bwd_layer_compute,
                          std::vector<FFTask*>& out,
                          std::vector<std::string>& labels,
                          const std::vector<int>& placement = std::vector<int>());

#endif
