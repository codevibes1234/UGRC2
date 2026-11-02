// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
#pragma once
/*
 * ffapp_multiring_tg.h
 * =====================
 * FFMultiRingAllreduceTask — task-graph-integrated multi-ring AllReduce
 * over a set of disjoint circulant chords.
 *
 * For each chord c in `chords`, builds a single Hamiltonian ring over
 * node_group by visiting node_group[(start + i*c) % N] for i=0..N-1.
 * This requires gcd(c, N) == 1 for every chord (true for N=128 with
 * chords {1,3,7,17}, since all are odd and 128 is a power of two).
 *
 * The total tensor (arsize bytes) is split into `chords.size()` equal
 * partitions, one per ring. Each ring then runs standard ring allreduce
 * (N-1 reduce-scatter steps + N-1 all-gather steps) on its partition,
 * independently and concurrently with the other rings. Because each
 * chord connects a distinct set of (sender,receiver) node pairs, the
 * rings never contend for the same link.
 *
 * The task completes (calls cleanup()) once every ring has finished
 * both phases.
 */

#include "ffapp.h"
#include "circulant128_topology.h"
#include "rocev2.h"
#include "tcp.h"
#include "route.h"
#include <vector>

class FFMultiRingAllreduceTask;

struct FFMultiRingTaskFlow {
    FFMultiRingAllreduceTask *ar;
    int ring_idx;
    int step;
    int phase;
};

void ar_finish_multiring_task(void *arinfo);

class FFMultiRingAllreduceTask : public FFTask {
public:
    FFMultiRingAllreduceTask(FFApplication          *ffapp,
                              Circulant128Topology   *topology,
                              std::vector<uint64_t>   node_group,
                              std::vector<int>        chords,
                              uint64_t                arsize,
                              double                  local_runtime = 0,
                              double                  rto_ms = 100.0);

    void doNextEvent() override;
    void step_done(int ring_idx, int step, int phase);

    void reset() override {
        FFTask::reset();
        rings_finished = 0;
        for (auto &r : rings) {
            r.current_step    = 0;
            r.current_phase   = 0;
            r.flows_in_step   = 0;
            r.finished_in_step = 0;
        }
    }

private:
    struct RingState {
        int                    chord;
        std::vector<uint64_t>  ring_order;   // Hamiltonian cycle for this chord
        uint64_t                shard_size;   // this ring's partition of arsize
        int                    current_step;
        int                    current_phase;  // 0 = RS, 1 = AG
        int                    flows_in_step;
        int                    finished_in_step;
    };

    void launch_step(int ring_idx);
    void advance(int ring_idx);
    bool start_flow(int ring_idx, int snd, int rcv,
                     uint64_t total_bytes, simtime_picosec t0);

    Circulant128Topology  *topology;
    std::vector<uint64_t>  node_group;
    std::vector<RingState> rings;
    uint64_t                total_arsize;
    double                  rto_ms;
    int                     rings_finished;
};
