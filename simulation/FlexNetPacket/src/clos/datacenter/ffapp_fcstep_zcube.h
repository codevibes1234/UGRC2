#ifndef FFAPP_FCSTEP_ZCUBE_H
#define FFAPP_FCSTEP_ZCUBE_H

#include "ffapp_rhd.h"
#include "fc_schedule.h"
#include "topology.h"

class FFZCubeFCStepAllreduceTask : public FFTask {
public:
    Topology*          _topology;
    FCScheduleSource*  _sched;
    uint64_t        operator_size;
    uint64_t        chunk_bytes;

    int curr_step;
    int num_steps;
    int finished_curr_step;
    int flows_this_step;

    FFZCubeFCStepAllreduceTask(FFApplication* ffapp, Topology* top,
                               FCScheduleSource* sched,
                               uint64_t sz, double local_runtime);

    bool use_fc_paths = false;
    long n_fc_routed = 0;
    long n_fallback_routed = 0;

    void doNextEvent() override;
    void start_step(int step);
    void start_one_flow(int src, int dst, uint64_t bytes, simtime_picosec t0,
                        const std::vector<int>* sw_path);
    void advance();
};

void ar_finish_fcstep_zcube(void* ar_ptr);

class FFFCDataflowAllreduceTask : public FFTask {
public:
    struct Tx { int phase, step, idx, src, dst, chunks, pending; bool done; };
    struct FlowCtx { FFFCDataflowAllreduceTask* task; int tx; };

    Topology*         _topology;
    FCScheduleSource* _sched;
    uint64_t          operator_size;
    uint64_t          chunk_bytes;
    bool              use_fc_paths = false;
    long              n_fc_routed = 0;
    long              n_fallback_routed = 0;

    std::vector<Tx>               txs;
    std::vector<FlowCtx>          ctx;
    std::vector<std::vector<int>> out_of;
    long                          n_done = 0;
    long                          max_in_flight = 0, in_flight = 0;

    FFFCDataflowAllreduceTask(FFApplication* ffapp, Topology* top,
                              FCScheduleSource* sched, uint64_t sz);
    void doNextEvent() override;
    void start_tx(int i, simtime_picosec t0);
    void tx_done(int i);
    static bool follows(const Tx& pred, const Tx& succ) {
        return (succ.phase == pred.phase && succ.step > pred.step) ||
               (pred.phase == 0 && succ.phase == 1);
    }
};

void ar_finish_fc_dataflow(void* ctx_ptr);

#endif
