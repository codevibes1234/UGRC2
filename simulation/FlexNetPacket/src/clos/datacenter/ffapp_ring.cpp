// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * ffapp_ring_tg.cpp
 * ==================
 * Implementation of FFRingAllreduceTask.
 */

#include "ffapp_ring.h"

void ar_finish_ring_task(void *arinfo)
{
    auto *f = static_cast<FFRingTaskFlow*>(arinfo);
    f->ar->step_done(f->step, f->phase);
    delete f;
}

FFRingAllreduceTask::FFRingAllreduceTask(
        FFApplication         *ffapp,
        Circulant16Topology   *topology,
        std::vector<uint64_t>  ng,
        uint64_t               sz,
        double                 local_runtime)
    : FFTask(ffapp, FFTask::TASK_ALLREDUCE),
      topology(topology),
      node_group(ng),
      operator_size(sz > 0 ? sz : 1ULL),
      current_step(0), current_phase(0),
      flows_in_step(0), finished_in_step(0)
{
    run_time  = (simtime_picosec)(local_runtime * 1e9);
    xfersize  = sz;   // store original for reset()
}

void FFRingAllreduceTask::doNextEvent()
{
    if (node_group.size() == 1) {
        finish_time = start_time = ready_time;
        cleanup();
        return;
    }

    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;

    // No small-tensor scale-up needed here: chunking operator_size into
    // N-1 pieces of size operator_size/N per phase already reproduces
    // ring allreduce's natural 2*(N-1)/N data-movement factor.

    current_step = current_phase = flows_in_step = finished_in_step = 0;
    launch_step();
}

void FFRingAllreduceTask::launch_step()
{
    flows_in_step    = 0;
    finished_in_step = 0;

    uint64_t N = (uint64_t)node_group.size();

    // Chunk size per ring hop. RS and AG both move one chunk per
    // (N-1)-step phase; every node is active in every step.
    uint64_t shard = operator_size / N;
    if (shard == 0) shard = 1;

    simtime_picosec t0 = (current_step == 0 && current_phase == 0)
                         ? start_time
                         : eventlist().now();

    for (uint64_t i = 0; i < N; i++) {
        int snd = (int)node_group[i];
        int rcv = (int)node_group[(i + 1) % N];
        if (start_ring_flow(snd, rcv, shard, t0))
            flows_in_step++;
    }

    if (flows_in_step == 0) {
        advance();
    } else {
        std::cerr << "[Ring-TG] step=" << current_step
                  << " phase=" << current_phase
                  << " launched=" << flows_in_step << " flows"
                  << " (ring size=" << N << ")\n";
    }
}

bool FFRingAllreduceTask::start_ring_flow(int snd, int rcv,
                                           uint64_t total_bytes,
                                           simtime_picosec t0)
{
    auto *f = new FFRingTaskFlow{this, current_step, current_phase};

    DCTCPSrc *fs = new DCTCPSrc(
        nullptr, nullptr, ffapp->fstream_out,
        eventlist(), snd, rcv,
        ar_finish_ring_task, static_cast<void*>(f));
    TcpSink *snk = new TcpSink();

    fs->set_flowsize(total_bytes);
    fs->set_ssthresh(ffapp->ssthresh * Packet::data_packet_size());
    fs->_rto = timeFromMs(ffapp->bfb_rto_ms);
    ffapp->tcpRtxScanner.registerTcp(*fs);

    // Data path: snd → rcv
    auto *sp = topology->get_paths(snd, rcv);
    if (sp->empty()) {
        delete sp; delete f; delete snk; delete fs;
        return false;
    }
    Route *ro = new Route(*(sp->at(rand() % sp->size())));
    ro->push_back(snk);
    delete sp;

    // ACK path: rcv → snd
    auto *dp = topology->get_paths(rcv, snd);
    if (dp->empty()) {
        delete dp; delete f; delete snk; delete ro; delete fs;
        return false;
    }
    Route *ri = new Route(*(dp->at(rand() % dp->size())));
    ri->push_back(fs);
    delete dp;

    fs->connect(*ro, *ri, *snk, t0);
    return true;
}

void FFRingAllreduceTask::step_done(int step, int phase)
{
    if (step != current_step || phase != current_phase) return;
    if (++finished_in_step == flows_in_step)
        advance();
}

void FFRingAllreduceTask::advance()
{
    const int D = (int)node_group.size() - 1;   // N-1 steps per phase

    if (current_step < D - 1) {
        current_step++;
        launch_step();
    } else if (current_phase == 0) {
        current_phase = 1;
        current_step  = 0;
        launch_step();
    } else {
        // AllReduce complete — re-enter task graph
        finish_time = eventlist().now();
        std::cerr << "[Ring-TG] AllReduce complete at "
                  << timeAsMs(finish_time) << " ms\n";
        cleanup();   // signals dependent tasks, advances training loop
    }
}
