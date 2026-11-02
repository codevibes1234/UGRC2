#include "ffapp_a2a.h"
#include "ecmp_hash.h"
#include "tcp.h"
#ifdef USE_DCTCP
#include "dctcp.h"
#else
#include "rocev2.h"
#endif

#include <cassert>
#include <cstdlib>
#include <iostream>

void a2a_flow_finished(void* task_ptr)
{
    static_cast<FFAllToAllTask*>(task_ptr)->flow_done();
}

FFAllToAllTask::FFAllToAllTask(FFApplication* ffapp, Topology* top,
                               std::vector<uint64_t> ng, uint64_t sz)
    : FFTask(ffapp, FFTask::TASK_ALLREDUCE),
      _topology(top), node_group(ng),
      operator_size(sz > 0 ? sz : 1ULL),
      flows_total(0), flows_finished(0)
{
    xfersize = sz;
    assert(node_group.size() >= 2);
}

void FFAllToAllTask::doNextEvent()
{
    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;

    const int N = (int)node_group.size();
    uint64_t per_peer = operator_size / (uint64_t)(N - 1);
    if (per_peer == 0) per_peer = 1;

    std::cerr << "[A2A] Starting All-to-All N=" << N
              << " buffer=" << operator_size << " bytes"
              << " per_peer=" << per_peer << " bytes\n";

    flows_total    = N * (N - 1);
    flows_finished = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (i != j)
                start_one_flow((int)node_group[i], (int)node_group[j], per_peer, start_time);
}

void FFAllToAllTask::start_one_flow(int src, int dst, uint64_t bytes, simtime_picosec t0)
{
    auto* paths_out = _topology->get_paths(src, dst);
    auto* paths_in  = _topology->get_paths(dst, src);
    Route* routeout = new Route(*paths_out->at(ecmp_path_index(src, dst, paths_out->size())));
    Route* routein  = new Route(*paths_in->at(ecmp_path_index(dst, src, paths_in->size())));
    delete paths_out;
    delete paths_in;

#ifdef USE_DCTCP
    DCTCPSrc* fs = new DCTCPSrc(
        nullptr, nullptr, ffapp->fstream_out,
        eventlist(), src, dst,
        a2a_flow_finished, static_cast<void*>(this));
    TcpSink* snk = new TcpSink();
    fs->set_flowsize(bytes);
    fs->set_ssthresh(ffapp->ssthresh * Packet::data_packet_size());
    fs->_rto = timeFromMs(10);
    ffapp->tcpRtxScanner.registerTcp(*fs);
#else
    RoCEv2Src* fs = new RoCEv2Src(
        nullptr, eventlist(), src, dst,
        a2a_flow_finished, static_cast<void*>(this));
    RoCEv2Sink* snk = new RoCEv2Sink(nullptr, eventlist());
    fs->set_flowsize(bytes);
    fs->set_initial_credits(RoCEv2Src::default_initial_credits);
#endif

    routeout->push_back(snk);
    routein->push_back(fs);
    fs->connect(*routeout, *routein, *snk, t0);
}

void FFAllToAllTask::flow_done()
{
    flows_finished++;
    if (flows_finished == flows_total) {
        finish_time = eventlist().now();
        std::cerr << "[A2A] All-to-All complete at " << timeAsMs(finish_time) << " ms\n";
        cleanup();
    }
}
