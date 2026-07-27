// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * ffapp_multiring_tg.cpp
 * ========================
 * Implementation of FFMultiRingAllreduceTask.
 */

#include "ffapp_multiring_tg.h"
#include <numeric>

static int gcd_int(int a, int b) { while (b) { int t = a % b; a = b; b = t; } return a; }

void ar_finish_multiring_task(void *arinfo)
{
    auto *f = static_cast<FFMultiRingTaskFlow*>(arinfo);
    f->ar->step_done(f->ring_idx, f->step, f->phase);
    delete f;
}

FFMultiRingAllreduceTask::FFMultiRingAllreduceTask(
        FFApplication          *ffapp,
        Circulant128Topology   *topology,
        std::vector<uint64_t>   node_group,
        std::vector<int>        chords,
        uint64_t                arsize,
        double                  local_runtime,
        double                  rto_ms)
    : FFTask(ffapp, FFTask::TASK_ALLREDUCE),
      topology(topology),
      node_group(node_group),
      total_arsize(arsize > 0 ? arsize : 1ULL),
      rto_ms(rto_ms),
      rings_finished(0)
{
    run_time = (simtime_picosec)(local_runtime * 1e9);
    xfersize = arsize;   // store original for reset()

    uint64_t N = (uint64_t)node_group.size();
    uint64_t num_rings = (uint64_t)chords.size();
    uint64_t partition  = total_arsize / (num_rings > 0 ? num_rings : 1);
    if (partition == 0) partition = 1;

    rings.reserve(chords.size());
    for (int c : chords) {
        if (N > 0 && gcd_int(c % (int)N, (int)N) != 1) {
            std::cerr << "[MultiRing-TG] WARNING: chord " << c
                      << " is not coprime with N=" << N
                      << " — ring will not visit all nodes as a single cycle\n";
        }

        RingState r;
        r.chord      = c;
        r.shard_size = partition;
        r.current_step = r.current_phase = 0;
        r.flows_in_step = r.finished_in_step = 0;

        r.ring_order.reserve(N);
        for (uint64_t i = 0; i < N; i++) {
            uint64_t idx = (i * (uint64_t)c) % N;
            r.ring_order.push_back(node_group[idx]);
        }
        rings.push_back(std::move(r));
    }
}

void FFMultiRingAllreduceTask::doNextEvent()
{
    if (node_group.size() <= 1 || rings.empty()) {
        finish_time = start_time = ready_time;
        cleanup();
        return;
    }

    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;
    rings_finished = 0;

    for (size_t i = 0; i < rings.size(); i++) {
        rings[i].current_step = rings[i].current_phase = 0;
        rings[i].flows_in_step = rings[i].finished_in_step = 0;
        launch_step((int)i);
    }
}

void FFMultiRingAllreduceTask::launch_step(int ring_idx)
{
    RingState &r = rings[ring_idx];
    r.flows_in_step    = 0;
    r.finished_in_step = 0;

    uint64_t N = (uint64_t)r.ring_order.size();
    uint64_t shard = r.shard_size / N;
    if (shard == 0) shard = 1;

    simtime_picosec t0 = (r.current_step == 0 && r.current_phase == 0)
                         ? start_time
                         : eventlist().now();

    for (uint64_t i = 0; i < N; i++) {
        int snd = (int)r.ring_order[i];
        int rcv = (int)r.ring_order[(i + 1) % N];
        if (start_flow(ring_idx, snd, rcv, shard, t0))
            r.flows_in_step++;
    }

    if (r.flows_in_step == 0) {
        advance(ring_idx);
    } else {
        std::cerr << "[MultiRing-TG] ring(chord=" << r.chord << ")"
                  << " step=" << r.current_step
                  << " phase=" << r.current_phase
                  << " launched=" << r.flows_in_step << " flows\n";
    }
}

bool FFMultiRingAllreduceTask::start_flow(int ring_idx, int snd, int rcv,
                                           uint64_t total_bytes,
                                           simtime_picosec t0)
{
    RingState &r = rings[ring_idx];
    auto *f = new FFMultiRingTaskFlow{this, ring_idx, r.current_step, r.current_phase};

    DCTCPSrc *fs = new DCTCPSrc(
        nullptr, nullptr, ffapp->fstream_out,
        eventlist(), snd, rcv,
        ar_finish_multiring_task, static_cast<void*>(f));
    TcpSink *snk = new TcpSink();

    fs->set_flowsize(total_bytes);
    fs->set_ssthresh(ffapp->ssthresh * Packet::data_packet_size());
    fs->_rto = timeFromMs(rto_ms);
    ffapp->tcpRtxScanner.registerTcp(*fs);

    // Data path: snd → rcv
    auto *sp = topology->get_direct_route(snd, rcv);
    if (sp->empty()) {
        delete sp; delete f; delete snk; delete fs;
        return false;
    }
    Route *ro = new Route(*(sp->at(rand() % sp->size())));
    ro->push_back(snk);
    delete sp;

    // ACK path: rcv → snd
    auto *dp = topology->get_direct_route(rcv, snd);
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

void FFMultiRingAllreduceTask::step_done(int ring_idx, int step, int phase)
{
    RingState &r = rings[ring_idx];
    if (step != r.current_step || phase != r.current_phase) return;
    if (++r.finished_in_step == r.flows_in_step)
        advance(ring_idx);
}

void FFMultiRingAllreduceTask::advance(int ring_idx)
{
    RingState &r = rings[ring_idx];
    const int D = (int)r.ring_order.size() - 1;   // N-1 steps per phase

    if (r.current_step < D - 1) {
        r.current_step++;
        launch_step(ring_idx);
    } else if (r.current_phase == 0) {
        r.current_phase = 1;
        r.current_step  = 0;
        launch_step(ring_idx);
    } else {
        // This ring's AllReduce is complete.
        std::cerr << "[MultiRing-TG] ring(chord=" << r.chord
                  << ") complete at " << timeAsMs(eventlist().now()) << " ms\n";
        rings_finished++;
        if (rings_finished == (int)rings.size()) {
            finish_time = eventlist().now();
            std::cerr << "[MultiRing-TG] ALL " << rings.size()
                      << " disjoint rings complete at "
                      << timeAsMs(finish_time) << " ms\n";
            cleanup();   // signals dependent tasks, advances training loop
        }
    }
}
