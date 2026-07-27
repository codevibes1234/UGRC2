// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
#pragma once
/*
 * ffapp_bfb_circulant_tg.h
 * ========================
 * FFCirBFBAllreduceTask — task-graph-integrated BFB AllReduce
 * for Circulant C(16,{3,4}).
 *
 * Inherits FFTask so it can be stored in FFApplication::tasks and
 * participates in the task-graph dependency system.
 * Calls cleanup() when done (not setEndtime()).
 * Has reset() for multi-iteration training loops.
 *
 * Logic is identical to standalone FFCirBFBAllreduce:
 *   - 3 RS steps + 3 AG steps (diameter=3)
 *   - One TCP flow per (sender,receiver) pair per step
 *   - All flows are 1-hop G-edges
 */

// #include "ffapp.h"   ← REMOVED — ffapp.h is already parsed before this header is included
#include "circulant16_topology.h"
#include "dctcp.h"
#include "tcp.h"
#include "route.h"
#include <map>

// Forward declaration
class FFCirBFBAllreduceTask;

struct FFCirBFBTaskFlow {
    FFCirBFBAllreduceTask *ar;
    int step;
    int phase;
};

void ar_finish_circulant_bfb_task(void *arinfo);

class FFCirBFBAllreduceTask : public FFTask {
public:
    FFCirBFBAllreduceTask(FFApplication         *ffapp,
                          Circulant16Topology   *cirtop,
                          std::vector<uint64_t>  ng,
                          uint64_t               sz,
                          double                 local_runtime = 0);

    void doNextEvent() override;
    void step_done(int step, int phase);

    void reset() override {
        FFTask::reset();
        operator_size    = xfersize;   // restore original size
        current_step     = 0;
        current_phase    = 0;
        flows_in_step    = 0;
        finished_in_step = 0;
    }

private:
    void launch_step();
    bool start_bfb_flow(int snd, int rcv,
                        uint64_t total_bytes,
                        simtime_picosec t0);
    void advance();

    Circulant16Topology   *cir_topology;
    std::vector<uint64_t>  node_group;
    uint64_t               operator_size;

    int current_step;
    int current_phase;
    int flows_in_step;
    int finished_in_step;
};