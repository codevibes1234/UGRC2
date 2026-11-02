// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * main_tcp_zcube_rhd_bench.cpp
 * ============================
 * Standalone AllReduce microbenchmark for ZCube(4,2) with recursive
 * halving (RS) + recursive doubling (AG) AllReduce.
 * Mirrors main_tcp_fattree_rechav_bench.cpp exactly.
 *
 * Usage:
 *   ./htsim_tcp_zcube_rhd_bench -msgsize <bytes> -simtime <sec> -ofile <out.txt>
 *                               [-speed <Mbps>] [-ssthresh <pkts>] [-q <pkts>]
 */

#include "config.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <string.h>
#include <numeric>

#include "network.h"
#include "randomqueue.h"
#include "pipe.h"
#include "eventlist.h"
#include "logfile.h"
#include "loggers.h"
#include "clock.h"
#include "tcp.h"
#include "mtcp.h"
#include "compositequeue.h"
#include "firstfit.h"
#include "topology.h"
#include "ecn.h"

#include "zcube_topology.h"
#include "ffapp_rhd.h"
#include "ffapp_fcstep_zcube.h"
#include "ffapp_fctree.h"
#ifndef USE_DCTCP
#include "rocev2.h"
#endif
#include "main.h"

// ── Globals ──────────────────────────────────────────────────────────────────
uint32_t RTT_rack  = 500;
uint32_t RTT_net   = 500;
uint32_t RTT       = 500;
uint32_t ECNTHRESH = 50;
uint32_t SPEED;
std::ofstream fct_util_out;
FirstFit *ff = NULL;

#define DEFAULT_PACKET_SIZE 9000
#define DEFAULT_HEADER_SIZE 64
#define DEFAULT_QUEUE_SIZE  200
#define DEFAULT_SPEED       200000   // 200 Gbps

EventList eventlist;
Logfile  *lg;

string ntoa(double n) { std::stringstream s; s << n; return s.str(); }
string itoa(uint64_t n) { std::stringstream s; s << n; return s.str(); }

void exit_error(char *prog) {
    std::cerr << "Usage: " << prog
              << " -msgsize <bytes> -simtime <sec> -ofile <file>"
              << " [-speed <Mbps>] [-ssthresh <pkts>] [-q <pkts>]"
              << " [-fcexec step|tree] [-fctrees <forestcoll.xml>]"
              << " [-fcpieces <n> | -fcpiecebytes <bytes>] [-fcwindow <n>]"
              << " [-fcphasebarrier 0|1]\n";
    exit(1);
}

int main(int argc, char **argv)
{
    TcpPacket::set_packet_size(DEFAULT_PACKET_SIZE - DEFAULT_HEADER_SIZE);
    mem_b queuesize = memFromPkt(DEFAULT_QUEUE_SIZE);

    int      ssthresh = 15;
    uint64_t msgsize  = 0;
    double   simtime  = 2.0;
    string   ofile    = "zcube_rhd_bench_result.txt";
    SPEED = DEFAULT_SPEED;
    string   fc_exec = "step", fc_trees_file;
    int      fc_pieces = 1, fc_window = 1, fc_phase_barrier = 1;
    uint64_t fc_piece_bytes = 0;
    bool     fc_paths = false;

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-msgsize")  && i+1 < argc) msgsize   = (uint64_t)atoll(argv[++i]);
        else if (!strcmp(argv[i], "-simtime")  && i+1 < argc) simtime   = atof(argv[++i]);
        else if (!strcmp(argv[i], "-ofile")    && i+1 < argc) ofile     = argv[++i];
        else if (!strcmp(argv[i], "-speed")    && i+1 < argc) SPEED     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-ssthresh") && i+1 < argc) ssthresh  = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-q")        && i+1 < argc) queuesize = memFromPkt(atoi(argv[++i]));
        else if (!strcmp(argv[i], "-fcexec")   && i+1 < argc) fc_exec   = argv[++i];
        else if (!strcmp(argv[i], "-fctrees")  && i+1 < argc) fc_trees_file = argv[++i];
        else if (!strcmp(argv[i], "-fcpieces") && i+1 < argc) fc_pieces = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-fcpiecebytes") && i+1 < argc) fc_piece_bytes = (uint64_t)atoll(argv[++i]);
        else if (!strcmp(argv[i], "-fcwindow") && i+1 < argc) fc_window = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-fcphasebarrier") && i+1 < argc) fc_phase_barrier = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-fcpaths")) fc_paths = true;
        else { std::cerr << "Unknown arg: " << argv[i] << "\n"; exit_error(argv[0]); }
    }

    if (msgsize == 0) {
        std::cerr << "ERROR: -msgsize required\n";
        exit_error(argv[0]);
    }
    if (fc_exec != "step" && fc_exec != "tree") {
        std::cerr << "ERROR: -fcexec must be step or tree\n";
        exit_error(argv[0]);
    }
    FCTreeSchedule fc_trees;
    if (fc_exec == "tree" && (fc_trees_file.empty() || !fc_trees.load_xml(fc_trees_file, 16)))
        exit_error(argv[0]);

    std::cerr << "=== ZCube(4,2) ForestColl (step schedule) AllReduce ===\n"
              << "  Message size : " << msgsize << " bytes ("
              << msgsize / (1024.0*1024.0) << " MB)\n"
              << "  NIC speed    : " << SPEED << " Mbps\n"
              << "  Sim time     : " << simtime << " s\n"
              << "  Algorithm    : ForestColl spanning trees, 14 RS + 14 AG barrier steps\n"
              << "  Path select  : get_paths (generic ECMP)\n";

    srand(13);
    eventlist.setEndtime(timeFromSec(simtime));
    Clock c(timeFromSec(5.0/100.0), eventlist);
    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(1), eventlist);

    fct_util_out = std::ofstream(ofile);

    // Build ZCube(4,2): 16 nodes

#ifndef USE_DCTCP
    {
        uint64_t rtt_ps = 12ULL * RTT_net * 1000ULL;
        uint64_t bdp_bytes = (uint64_t)SPEED * 1000000ULL / 8ULL * rtt_ps / 1000000000000ULL;
        uint32_t credits = (uint32_t)(2 * bdp_bytes / 9000);
        RoCEv2Src::default_initial_credits = credits < 8 ? 8 : credits;
        std::cerr << "  RoCE credits : " << RoCEv2Src::default_initial_credits << "\n";
    }
#endif
#ifdef USE_DCTCP
    ZCubeTopology *top = new ZCubeTopology(
        16, queuesize, nullptr, &eventlist, ff, ECN);
#else
    ZCubeTopology *top = new ZCubeTopology(
        16, queuesize, nullptr, &eventlist, ff, LOSSLESS_INPUT);
#endif

    FFApplication app(top, ssthresh, &fct_util_out,
                      tcpRtxScanner, eventlist,
                      FF_DEFAULT_AR);
    app.ngpupernode = 1;
    app.nnodes      = 16;
    app.nswitches   = 0;

    app.gpus.resize(16);
    std::iota(app.gpus.begin(), app.gpus.end(), 0);

    // Node group: nodes 0..15
    std::vector<uint64_t> node_group(16);
    std::iota(node_group.begin(), node_group.end(), 0);

    FFTask *ar = nullptr;
    FFFCTreeAllreduceTask *tree_task = nullptr;
    if (fc_exec == "tree") {
        tree_task = new FFFCTreeAllreduceTask(&app, top, &fc_trees, msgsize);
        if (fc_piece_bytes > 0) {
            uint64_t p = tree_task->chunk_bytes / fc_piece_bytes;
            fc_pieces = (int)(p < 1 ? 1 : (p > 4096 ? 4096 : p));
        }
        tree_task->pieces = fc_pieces;
        tree_task->window = fc_window;
        tree_task->phase_barrier = (fc_phase_barrier != 0);
        tree_task->use_fc_paths = fc_paths;
        std::cerr << "  Executor     : ForestColl trees (" << fc_trees.trees.size()
                  << " from " << fc_trees_file << "), pieces=" << fc_pieces
                  << " window=" << fc_window << " phase_barrier=" << fc_phase_barrier << "\n";
        ar = tree_task;
    } else {
        ar = new FFZCubeFCStepAllreduceTask(&app, top, top, msgsize, 0.0);
    }

    uint64_t ar_id = (uint64_t)(void*)ar;
    app.tasks[ar_id] = ar;
    ar->counter    = 0;
    ar->state      = FFTask::TASK_READY;
    ar->ready_time = eventlist.now();
    eventlist.sourceIsPending(*ar, eventlist.now());

    while (eventlist.doNextEvent()) {}

    simtime_picosec finish = ar->finish_time;
    double lat_ms = timeAsMs(finish);

    // algbw = 2*(N-1)/N * msgsize / time
    double algbw = 0.0;
    if (lat_ms > 0)
        algbw = (msgsize * 2.0 * 15.0 / 16.0) / (lat_ms * 1e-3) / 1e9;

    std::cerr << "\n=== Results ===\n"
              << "  AllReduce latency : " << lat_ms << " ms\n"
              << "  Algorithm BW      : " << algbw  << " GB/s\n";

    fct_util_out << "MsgSize_bytes "  << msgsize << "\n"
                 << "Latency_ms "     << lat_ms  << "\n"
                 << "AlgBW_GBs "      << algbw   << "\n"
                 << "FinalFinish_ps " << finish   << "\n";
    if (tree_task != nullptr)
        fct_util_out << "FCExecutor tree\n"
                     << "FCTreePieces " << tree_task->pieces << "\n"
                     << "FCTreeWindow " << tree_task->window << "\n"
                     << "FCTreePhaseBarrier " << (tree_task->phase_barrier ? 1 : 0) << "\n"
                     << "FCTreeFlows " << tree_task->flows_started << "\n"
                     << "FCTreeMaxInFlight " << tree_task->max_in_flight << "\n"
                     << "FCTreeRSFinish_ms " << timeAsMs(tree_task->rs_finish) << "\n"
                     << "FCTreeFcRouted " << tree_task->n_fc_routed << "\n"
                     << "FCTreeFallbackRouted " << tree_task->n_fallback_routed << "\n";

    return 0;
}
