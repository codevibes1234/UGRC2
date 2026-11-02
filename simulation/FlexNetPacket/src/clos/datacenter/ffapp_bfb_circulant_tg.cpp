// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * ffapp_bfb_circulant_tg.cpp
 * ==========================
 * Implementation of FFCirBFBAllreduceTask.
 */

#include "ffapp_bfb_circulant_tg.h"

void ar_finish_circulant_bfb_task(void *arinfo)
{
    auto *f = static_cast<FFCirBFBTaskFlow*>(arinfo);
    f->ar->step_done(f->step, f->phase);
    delete f;
}

FFCirBFBAllreduceTask::FFCirBFBAllreduceTask(
        FFApplication         *ffapp,
        Circulant16Topology   *cirtop,
        std::vector<uint64_t>  ng,
        uint64_t               sz,
        double                 local_runtime)
    : FFTask(ffapp, FFTask::TASK_ALLREDUCE),
      cir_topology(cirtop),
      node_group(ng),
      operator_size(sz > 0 ? sz : 1ULL),
      current_step(0), current_phase(0),
      flows_in_step(0), finished_in_step(0)
{
    run_time  = (simtime_picosec)(local_runtime * 1e9);
    xfersize  = sz;   // store original for reset()
}

void FFCirBFBAllreduceTask::doNextEvent()
{
    if (node_group.size() == 1) {
        finish_time = start_time = ready_time;
        cleanup();
        return;
    }

    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;

    // Small tensor: scale up to match effective ring transfer size
    if (operator_size < 9000ULL * node_group.size()) {
        operator_size = (uint64_t)(
            operator_size * (2.0 * (node_group.size() - 1)
                             / (double)node_group.size()));
        if (operator_size == 0) operator_size = 1;
    }

    current_step = current_phase = flows_in_step = finished_in_step = 0;
    launch_step();
}

void FFCirBFBAllreduceTask::launch_step()
{
    const std::vector<CirBFBTransmission> *txs =
        (current_phase == 0)
        ? &cir_topology->bfb_rs_step_on_G(current_step)
        : &cir_topology->bfb_ag_step_on_G(current_step);

    flows_in_step    = 0;
    finished_in_step = 0;

    if (txs->empty()) {
        advance();
        return;
    }

    int P = (current_phase == 0)
            ? cir_topology->bfb_rs_total_chunks_on_G()
            : cir_topology->bfb_ag_total_chunks_on_G();
    if (P <= 0) P = 1;

    uint64_t N     = (uint64_t)node_group.size();
    uint64_t shard = operator_size / N;
    if (shard == 0) shard = 1;

    // Group all schedule entries by (sender,receiver) → one TCP flow per link
    std::map<std::pair<int,int>, uint64_t> link_bytes;
    for (const auto &tx : *txs) {
        uint64_t bytes;
        if (shard < (uint64_t)P * 64) {
            bytes = shard;
        } else {
            bytes = shard * (uint64_t)tx.chunks / (uint64_t)P;
            if (bytes < 1) bytes = 1;
        }
        link_bytes[{tx.sender, tx.receiver}] += bytes;
    }

    simtime_picosec t0 = (current_step == 0 && current_phase == 0)
                         ? start_time
                         : eventlist().now();

    for (const auto &[link, total_bytes] : link_bytes) {
        if (start_bfb_flow(link.first, link.second, total_bytes, t0))
            flows_in_step++;
    }

    if (flows_in_step == 0) {
        advance();
    } else {
        std::cerr << "[CirBFB-TG] step=" << current_step
                  << " phase=" << current_phase
                  << " launched=" << flows_in_step << " flows"
                  << " (from " << txs->size() << " schedule entries)\n";
    }
}

bool FFCirBFBAllreduceTask::start_bfb_flow(int snd, int rcv,
                                             uint64_t total_bytes,
                                             simtime_picosec t0)
{
    auto *f = new FFCirBFBTaskFlow{this, current_step, current_phase};

    RoCEv2Src *fs = new RoCEv2Src(
        nullptr, eventlist(), snd, rcv,
        ar_finish_circulant_bfb_task, static_cast<void*>(f));
    RoCEv2Sink *snk = new RoCEv2Sink(nullptr, eventlist());

    fs->set_flowsize(total_bytes);
    fs->set_initial_credits(RoCEv2Src::default_initial_credits);

    // Data path: snd → rcv
    auto *sp = cir_topology->get_paths(snd, rcv);
    if (sp->empty()) {
        delete sp; delete f; delete snk; delete fs;
        return false;
    }
    Route *ro = new Route(*(sp->at(rand() % sp->size())));
    ro->push_back(snk);
    delete sp;

    // ACK path: rcv → snd (always exists since G=G^T for circulant)
    auto *dp = cir_topology->get_paths(rcv, snd);
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

void FFCirBFBAllreduceTask::step_done(int step, int phase)
{
    if (step != current_step || phase != current_phase) return;
    if (++finished_in_step == flows_in_step)
        advance();
}

void FFCirBFBAllreduceTask::advance()
{
    const int D = cir_topology->bfb_num_steps();   // = 3

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
        std::cerr << "[CirBFB-TG] AllReduce complete at "
                  << timeAsMs(finish_time) << " ms\n";
        cleanup();   // signals dependent tasks, advances training loop
    }
}