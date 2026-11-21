#ifndef FFAPP_A2A_H
#define FFAPP_A2A_H

#include "ffapp_rhd.h"

class FFAllToAllTask : public FFTask {
public:
    Topology*             _topology;
    std::vector<uint64_t> node_group;
    uint64_t              operator_size;
    int                   flows_total;
    int                   flows_finished;

    FFAllToAllTask(FFApplication* ffapp, Topology* top,
                   std::vector<uint64_t> ng, uint64_t sz);

    void doNextEvent() override;
    void start_one_flow(int src, int dst, uint64_t bytes, simtime_picosec t0);
    void flow_done();
};

void a2a_flow_finished(void* task_ptr);

#endif
