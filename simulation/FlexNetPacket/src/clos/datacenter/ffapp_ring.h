// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
#pragma once
/*
 * ffapp_ring_tg.h
 * ================
 * FFRingAllreduceTask — task-graph-integrated Ring AllReduce.
 *
 * Inherits FFTask so it can be stored in FFApplication::tasks and
 * participates in the task-graph dependency system.
 * Calls cleanup() when done (not setEndtime()).
 * Has reset() for multi-iteration training loops.
 *
 * Algorithm:
 *   - Standard ring allreduce over node_group (in the given order):
 *       node i sends to node (i+1) % N, receives from (i-1+N) % N
 *   - (N-1) reduce-scatter steps + (N-1) all-gather steps
 *   - Each step launches exactly N flows (one per ring edge), each
 *     carrying operator_size / N bytes.
 *   - Physical routing (get_paths) still goes through the underlying
 *     Circulant16Topology, so this models "ring allreduce running on
 *     top of the circulant physical fabric," not a physical ring.
 *     Swap the topology type below if you want a different substrate.
 */

// #include "ffapp.h"   ← REMOVED — ffapp.h is already parsed before this header is included
#include "circulant16_topology.h"
#include "rocev2.h"
#include "tcp.h"
#include "route.h"

// Forward declaration
class FFRingAllreduceTask;

struct FFRingTaskFlow {
    FFRingAllreduceTask *ar;
    int step;
    int phase;
};

void ar_finish_ring_task(void *arinfo);

class FFRingAllreduceTask : public FFTask {
public:
    FFRingAllreduceTask(FFApplication         *ffapp,
                         Circulant16Topology   *topology,
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
        flows_in_step     = 0;
        finished_in_step  = 0;
    }

private:
    void launch_step();
    bool start_ring_flow(int snd, int rcv,
                          uint64_t total_bytes,
                          simtime_picosec t0);
    void advance();

    Circulant16Topology   *topology;
    std::vector<uint64_t>  node_group;
    uint64_t               operator_size;

    int current_step;
    int current_phase;   // 0 = reduce-scatter, 1 = all-gather
    int flows_in_step;
    int finished_in_step;
};
