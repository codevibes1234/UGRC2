#include "ffapp_fcstep_zcube.h"
#include "ecmp_hash.h"
#include "tcp.h"
#ifdef USE_DCTCP
#include "dctcp.h"
#else
#include "rocev2.h"
#endif

#include <cassert>
#include <iostream>

void ar_finish_fcstep_zcube(void* ar_ptr)
{
    FFZCubeFCStepAllreduceTask* ar = static_cast<FFZCubeFCStepAllreduceTask*>(ar_ptr);
    ar->finished_curr_step++;
    if (ar->finished_curr_step == ar->flows_this_step)
        ar->advance();
}

FFZCubeFCStepAllreduceTask::FFZCubeFCStepAllreduceTask(
        FFApplication* ffapp, Topology* top, FCScheduleSource* sched,
        uint64_t sz, double local_runtime)
    : FFTask(ffapp, FFTask::TASK_ALLREDUCE),
      _topology(top), _sched(sched),
      operator_size(sz > 0 ? sz : 1ULL),
      curr_step(0), finished_curr_step(0), flows_this_step(0)
{
    assert(top != nullptr && sched != nullptr);
    run_time  = (simtime_picosec)(local_runtime * 1e9);
    xfersize  = sz;
    num_steps = sched->fc_num_steps();
    assert(num_steps > 0);
    assert(sched->fc_rs_total_chunks() == sched->fc_ag_total_chunks());
    chunk_bytes = operator_size / (uint64_t)sched->fc_rs_total_chunks();
    if (chunk_bytes == 0) chunk_bytes = 1;
}

void FFZCubeFCStepAllreduceTask::doNextEvent()
{
    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;

    std::cerr << "[ZCubeFC] Starting ForestColl AllReduce"
              << " steps=" << num_steps << "+" << num_steps
              << " chunks=" << _sched->fc_rs_total_chunks()
              << " chunk_bytes=" << chunk_bytes
              << " size=" << operator_size << " bytes\n";

    curr_step = 0;
    start_step(curr_step);
}

void FFZCubeFCStepAllreduceTask::start_step(int step)
{
    const bool rs = step < num_steps;
    const std::vector<ZCubeFCTransmission>& tx =
        rs ? _sched->fc_rs_step(step) : _sched->fc_ag_step(step - num_steps);

    if (tx.empty()) {
        advance();
        return;
    }

    finished_curr_step = 0;
    flows_this_step    = (int)tx.size();

    simtime_picosec t0 = (step == 0) ? start_time + run_time : eventlist().now();

    for (size_t i = 0; i < tx.size(); i++) {
        const ZCubeFCTransmission& t = tx[i];
        const std::vector<int>* sw = nullptr;
        if (use_fc_paths && _sched->fc_has_paths()) {
            const std::vector<int>& p = rs ? _sched->fc_rs_path(step, (int)i)
                                           : _sched->fc_ag_path(step - num_steps, (int)i);
            if (!p.empty()) sw = &p;
        }
        start_one_flow(t.sender, t.receiver,
                       chunk_bytes * (uint64_t)t.chunks, t0, sw);
    }
}

void FFZCubeFCStepAllreduceTask::start_one_flow(int src, int dst,
                                                uint64_t bytes,
                                                simtime_picosec t0,
                                                const std::vector<int>* sw_path)
{
    Route* routeout = nullptr;
    Route* routein  = nullptr;

    if (sw_path != nullptr) {
        ExplicitRouteTopology* ert =
            dynamic_cast<ExplicitRouteTopology*>(_topology);
        if (ert != nullptr) {
            std::vector<int> rev(sw_path->rbegin(), sw_path->rend());
            routeout = ert->route_for_switch_path(src, dst, *sw_path);
            routein  = ert->route_for_switch_path(dst, src, rev);
            if (routeout == nullptr || routein == nullptr) {
                delete routeout; delete routein;
                routeout = routein = nullptr;
            }
        }
    }

    if (routeout != nullptr) {
        n_fc_routed++;
    } else {
        n_fallback_routed++;
        auto* paths_out = _topology->get_paths(src, dst);
        auto* paths_in  = _topology->get_paths(dst, src);
        routeout = new Route(*paths_out->at(ecmp_path_index(src, dst, paths_out->size())));
        routein  = new Route(*paths_in->at(ecmp_path_index(dst, src, paths_in->size())));
        delete paths_out;
        delete paths_in;
    }

#ifdef USE_DCTCP
    DCTCPSrc* fs = new DCTCPSrc(
        nullptr, nullptr, ffapp->fstream_out,
        eventlist(), src, dst,
        ar_finish_fcstep_zcube, static_cast<void*>(this));
    TcpSink* snk = new TcpSink();
    fs->set_flowsize(bytes);
    fs->set_ssthresh(ffapp->ssthresh * Packet::data_packet_size());
    fs->_rto = timeFromMs(10);
    ffapp->tcpRtxScanner.registerTcp(*fs);
#else
    RoCEv2Src* fs = new RoCEv2Src(
        nullptr, eventlist(), src, dst,
        ar_finish_fcstep_zcube, static_cast<void*>(this));
    RoCEv2Sink* snk = new RoCEv2Sink(nullptr, eventlist());
    fs->set_flowsize(bytes);
    fs->set_initial_credits(RoCEv2Src::default_initial_credits);
#endif

    routeout->push_back(snk);
    routein->push_back(fs);

    fs->connect(*routeout, *routein, *snk, t0);
}

void FFZCubeFCStepAllreduceTask::advance()
{
    curr_step++;
    if (curr_step == 2 * num_steps) {
        finish_time = eventlist().now();
        std::cerr << "[ZCubeFC] AllReduce complete at "
                  << timeAsMs(finish_time) << " ms\n";
        std::cerr << "[ZCubeFC] flows routed on ForestColl paths: " << n_fc_routed
                  << ", on topology default routing: " << n_fallback_routed << "\n";
        cleanup();
    } else {
        start_step(curr_step);
    }
}

void ar_finish_fc_dataflow(void* ctx_ptr)
{
    auto* c = static_cast<FFFCDataflowAllreduceTask::FlowCtx*>(ctx_ptr);
    c->task->tx_done(c->tx);
}

FFFCDataflowAllreduceTask::FFFCDataflowAllreduceTask(
        FFApplication* ffapp, Topology* top, FCScheduleSource* sched, uint64_t sz)
    : FFTask(ffapp, FFTask::TASK_ALLREDUCE),
      _topology(top), _sched(sched), operator_size(sz > 0 ? sz : 1ULL)
{
    assert(top != nullptr && sched != nullptr);
    xfersize = sz;
    assert(sched->fc_num_steps() > 0);
    assert(sched->fc_rs_total_chunks() == sched->fc_ag_total_chunks());
    chunk_bytes = operator_size / (uint64_t)sched->fc_rs_total_chunks();
    if (chunk_bytes == 0) chunk_bytes = 1;

    int maxnode = 0;
    for (int phase = 0; phase < 2; phase++)
        for (int k = 0; k < sched->fc_num_steps(); k++) {
            const auto& st = phase == 0 ? sched->fc_rs_step(k) : sched->fc_ag_step(k);
            for (size_t i = 0; i < st.size(); i++) {
                txs.push_back({phase, k, (int)i, st[i].sender, st[i].receiver,
                               st[i].chunks, 0, false});
                maxnode = std::max(maxnode, std::max(st[i].sender, st[i].receiver));
            }
        }
    out_of.assign(maxnode + 1, {});
    for (size_t i = 0; i < txs.size(); i++) out_of[txs[i].src].push_back((int)i);
    for (const Tx& p : txs)
        for (int s : out_of[p.dst])
            if (follows(p, txs[s])) txs[s].pending++;
    ctx.resize(txs.size());
    for (size_t i = 0; i < txs.size(); i++) ctx[i] = {this, (int)i};
}

void FFFCDataflowAllreduceTask::doNextEvent()
{
    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;
    long roots = 0;
    for (const Tx& t : txs) roots += (t.pending == 0);
    std::cerr << "[FCDataflow] Starting ForestColl AllReduce without step barriers: "
              << txs.size() << " transmissions, " << roots << " ready at t0, chunk_bytes="
              << chunk_bytes << " size=" << operator_size << " bytes\n";
    if (txs.empty()) { finish_time = eventlist().now(); cleanup(); return; }
    simtime_picosec t0 = start_time + run_time;
    for (size_t i = 0; i < txs.size(); i++)
        if (txs[i].pending == 0) start_tx((int)i, t0);
}

void FFFCDataflowAllreduceTask::start_tx(int i, simtime_picosec t0)
{
    const Tx& t = txs[i];
    if (++in_flight > max_in_flight) max_in_flight = in_flight;
    Route* routeout = nullptr;
    Route* routein  = nullptr;
    if (use_fc_paths && _sched->fc_has_paths()) {
        const std::vector<int>& p = t.phase == 0 ? _sched->fc_rs_path(t.step, t.idx)
                                                 : _sched->fc_ag_path(t.step, t.idx);
        ExplicitRouteTopology* ert = dynamic_cast<ExplicitRouteTopology*>(_topology);
        if (!p.empty() && ert != nullptr) {
            std::vector<int> rev(p.rbegin(), p.rend());
            routeout = ert->route_for_switch_path(t.src, t.dst, p);
            routein  = ert->route_for_switch_path(t.dst, t.src, rev);
            if (routeout == nullptr || routein == nullptr) {
                delete routeout; delete routein;
                routeout = routein = nullptr;
            }
        }
    }
    if (routeout != nullptr) {
        n_fc_routed++;
    } else {
        n_fallback_routed++;
        auto* paths_out = _topology->get_paths(t.src, t.dst);
        auto* paths_in  = _topology->get_paths(t.dst, t.src);
        routeout = new Route(*paths_out->at(ecmp_path_index(t.src, t.dst, paths_out->size())));
        routein  = new Route(*paths_in->at(ecmp_path_index(t.dst, t.src, paths_in->size())));
        delete paths_out;
        delete paths_in;
    }
    uint64_t bytes = chunk_bytes * (uint64_t)t.chunks;
#ifdef USE_DCTCP
    DCTCPSrc* fs = new DCTCPSrc(nullptr, nullptr, ffapp->fstream_out, eventlist(),
                                t.src, t.dst, ar_finish_fc_dataflow, &ctx[i]);
    TcpSink* snk = new TcpSink();
    fs->set_flowsize(bytes);
    fs->set_ssthresh(ffapp->ssthresh * Packet::data_packet_size());
    fs->_rto = timeFromMs(10);
    ffapp->tcpRtxScanner.registerTcp(*fs);
#else
    RoCEv2Src* fs = new RoCEv2Src(nullptr, eventlist(), t.src, t.dst,
                                  ar_finish_fc_dataflow, &ctx[i]);
    RoCEv2Sink* snk = new RoCEv2Sink(nullptr, eventlist());
    fs->set_flowsize(bytes);
    fs->set_initial_credits(RoCEv2Src::default_initial_credits);
#endif
    routeout->push_back(snk);
    routein->push_back(fs);
    fs->connect(*routeout, *routein, *snk, t0);
}

void FFFCDataflowAllreduceTask::tx_done(int i)
{
    Tx& p = txs[i];
    assert(!p.done);
    p.done = true;
    n_done++;
    in_flight--;
    simtime_picosec now = eventlist().now();
    for (int s : out_of[p.dst])
        if (follows(p, txs[s]) && --txs[s].pending == 0) start_tx(s, now);
    if (n_done == (long)txs.size()) {
        finish_time = now;
        std::cerr << "[FCDataflow] AllReduce complete at " << timeAsMs(finish_time)
                  << " ms; at most " << max_in_flight << " transmissions in flight\n";
        std::cerr << "[ZCubeFC] flows routed on ForestColl paths: " << n_fc_routed
                  << ", on topology default routing: " << n_fallback_routed << "\n";
        cleanup();
    }
}
