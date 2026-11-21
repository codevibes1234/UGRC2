// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * main_tcp_ldistreg128_bench.cpp
 * ===============================
 * Pure AllReduce microbenchmark for L(DistReg(4,32)), 128 nodes.
 * No task graph. Just: build topology → fire AllReduce → print time.
 *
 * Usage:
 *   ./htsim_tcp_ldistreg128_bench \
 *       -arsize 4194304 -speed 200000 \
 *       -ssthresh 15 -q 200 -rto 10 \
 *       -simtime 3600.1 -ofile /dev/null
 */

#include "config.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <string.h>
#include <math.h>
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

#include "ldistreg128_topology.h"
#include "ffapp.h"
#include "ffapp_bfb_ldistreg128_tg.h"

uint32_t RTT       = 1000;
uint32_t SPEED     = 0;
uint32_t ECNTHRESH = 50;

std::ofstream fct_util_out;
FirstFit *ff = NULL;

#define DEFAULT_PACKET_SIZE 9000
#define DEFAULT_HEADER_SIZE 64
#define DEFAULT_QUEUE_SIZE  200

EventList eventlist;

string ntoa(double n) { stringstream s; s << n; return s.str(); }
string itoa(uint64_t n) { stringstream s; s << n; return s.str(); }

#ifdef USE_DCTCP
#define FABRIC_QUEUE ECN
#else
#define FABRIC_QUEUE LOSSLESS_INPUT
#endif

int main(int argc, char **argv)
{
    TcpPacket::set_packet_size(DEFAULT_PACKET_SIZE - DEFAULT_HEADER_SIZE);
    mem_b queuesize = DEFAULT_QUEUE_SIZE * DEFAULT_PACKET_SIZE;

    int      ssthresh = 15;
    double   simtime  = 3600.1;
    double   rto_ms   = 10.0;
    uint64_t arsize   = 0;

    int i = 1;
    while (i < argc) {
        if      (!strcmp(argv[i], "-arsize"))   { arsize    = (uint64_t)atoll(argv[++i]); }
        else if (!strcmp(argv[i], "-speed"))    { SPEED     = atoi(argv[++i]); }
        else if (!strcmp(argv[i], "-rtt"))      { RTT       = atoi(argv[++i]); }
        else if (!strcmp(argv[i], "-ssthresh")) { ssthresh  = atoi(argv[++i]); }
        else if (!strcmp(argv[i], "-q"))        { queuesize = memFromPkt(atoi(argv[++i])); }
        else if (!strcmp(argv[i], "-ecnthresh")){ ECNTHRESH = atoi(argv[++i]); }
        else if (!strcmp(argv[i], "-simtime"))  { simtime   = atof(argv[++i]); }
        else if (!strcmp(argv[i], "-rto"))      { rto_ms    = atof(argv[++i]); }
        else if (!strcmp(argv[i], "-ofile"))    { fct_util_out = std::ofstream(argv[++i]); }
        else { cerr << "Unknown argument: " << argv[i] << "\n"; exit(1); }
        i++;
    }

    if (SPEED == 0) { cerr << "ERROR: -speed <Mbps> required\n"; exit(1); }
    if (arsize == 0) { cerr << "ERROR: -arsize <bytes> required\n"; exit(1); }

    cerr << "[LDistReg128-Bench] arsize=" << arsize
         << " (" << arsize / (1024.0*1024.0) << " MB)"
         << " speed="    << SPEED    << " Mbps"
         << " ssthresh=" << ssthresh << " pkts"
         << " q="        << (queuesize / DEFAULT_PACKET_SIZE) << " pkts"
         << " rto="      << rto_ms   << " ms\n";

    srand(13);
    eventlist.setEndtime(timeFromSec(simtime));
    Clock c(timeFromSec(5.0 / 100.0), eventlist);
    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(1), eventlist);

    // Build L(DistReg(4,32)) topology — 128 nodes, degree 4, diameter 6

    {
        uint64_t rtt_ps = 12ULL * RTT * 1000ULL;
        uint64_t bdp_bytes = (uint64_t)SPEED * 1000000ULL / 8ULL * rtt_ps / 1000000000000ULL;
        uint32_t credits = (uint32_t)(2 * bdp_bytes / 9000);
        RoCEv2Src::default_initial_credits = credits < 8 ? 8 : credits;
    }

    LDistReg128Topology *top = new LDistReg128Topology(
        LDISTREG128_N, queuesize,
        nullptr, &eventlist, ff, FABRIC_QUEUE);

    FFApplication app(top, ssthresh, &fct_util_out,
                      tcpRtxScanner, eventlist,
                      FFApplication::FF_BFB_LDISTREG128_AR);
    app.bfb_rto_ms  = rto_ms;
    app.ngpupernode = 1;
    app.nnodes      = LDISTREG128_N;
    app.nswitches   = 0;

    // All 128 nodes participate
    std::vector<uint64_t> node_group;
    node_group.reserve(LDISTREG128_N);
    for (int n = 0; n < LDISTREG128_N; n++)
        node_group.push_back((uint64_t)n);
    app.gpus = std::vector<int>(node_group.begin(), node_group.end());

    FFLDistReg128BFBAllreduceTask *ar = new FFLDistReg128BFBAllreduceTask(
        &app, top, node_group, arsize, 0.0);

    app.tasks[(uint64_t)(void*)ar] = ar;
    FFApplication::total_apps      = 1;
    FFApplication::finished_apps   = 0;

    eventlist.sourceIsPending(*ar, timeFromSec(0.0));

    cerr << "[LDistReg128-Bench] Simulation starting...\n";
    while (eventlist.doNextEvent()) {}

    cerr << "[LDistReg128-Bench] FinalFinish "
         << timeAsMs(app.final_finish_time) << " ms\n";

    fct_util_out << "FinalFinish " << app.final_finish_time << "\n";

    delete top;
    return 0;
}
