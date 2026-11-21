// main_tcp_circulant128_bench.cpp
// ===============================
// Pure AllReduce microbenchmark for Circulant C(128,{1,3,7,17}).
// Uses FFMultiRingAllreduceTask: 4 disjoint Hamiltonian rings, one per
// chord, each carrying an independent shard of the tensor, running
// concurrently.

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

#include "circulant128_topology.h"
#include "ffapp.h"
#include "ffapp_multiring_tg.h"

uint32_t RTT       = 1000;
uint32_t SPEED     = 0;
uint32_t ECNTHRESH = 50;

std::ofstream fct_util_out;
FirstFit *ff = NULL;

#define DEFAULT_PACKET_SIZE 9000
#define DEFAULT_HEADER_SIZE 64
#define DEFAULT_QUEUE_SIZE  200
#define CIRCULANT_NODES 128

EventList eventlist;

string ntoa(double n) { stringstream s; s << n; return s.str(); }
string itoa(uint64_t n) { stringstream s; s << n; return s.str(); }

int main(int argc, char **argv)
{
    TcpPacket::set_packet_size(DEFAULT_PACKET_SIZE - DEFAULT_HEADER_SIZE);
    mem_b queuesize = DEFAULT_QUEUE_SIZE * DEFAULT_PACKET_SIZE;

    int      ssthresh = 15;
    double   simtime  = 3600.1;
    double   rto_ms   = 100.0;
    uint64_t arsize   = 0;
    linkspeed_bps speed = 0;

    int i = 1;
    while (i < argc) {
        if      (!strcmp(argv[i], "-arsize"))   { arsize   = (uint64_t)atoll(argv[++i]); }
        else if (!strcmp(argv[i], "-speed"))    { speed    = (linkspeed_bps)atoll(argv[++i]) * 1000000ULL;
                                                    SPEED    = (uint32_t)atoll(argv[i]); }
        else if (!strcmp(argv[i], "-rtt"))      { RTT      = atoi(argv[++i]); }
        else if (!strcmp(argv[i], "-ssthresh")) { ssthresh = atoi(argv[++i]); }
        else if (!strcmp(argv[i], "-q"))        { queuesize = memFromPkt(atoi(argv[++i])); }
        else if (!strcmp(argv[i], "-ecnthresh")){ ECNTHRESH = atoi(argv[++i]); }
        else if (!strcmp(argv[i], "-simtime"))  { simtime  = atof(argv[++i]); }
        else if (!strcmp(argv[i], "-rto"))      { rto_ms   = atof(argv[++i]); }
        else if (!strcmp(argv[i], "-ofile"))    { fct_util_out.open(argv[++i]); }
        else {
            cerr << "Unknown argument: " << argv[i] << "\n";
            exit(1);
        }
        i++;
    }

    if (speed == 0) {
        cerr << "ERROR: -speed <Mbps> is required\n";
        exit(1);
    }
    if (arsize == 0) {
        cerr << "ERROR: -arsize <bytes> is required\n";
        exit(1);
    }

    cerr << "[Circulant128-Bench] arsize="   << arsize
         << " (" << arsize / (1024.0*1024.0) << " MB)"
         << " speed="    << speed/1000000 << " Mbps"
         << " ssthresh=" << ssthresh << " pkts"
         << " q="        << (queuesize / DEFAULT_PACKET_SIZE) << " pkts"
         << " rto="      << rto_ms   << " ms\n";

    srand(13);
    eventlist.setEndtime(timeFromSec(simtime));
    Clock c(timeFromSec(5.0 / 100.0), eventlist);
    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(rto_ms), eventlist);

    // Build Circulant128 topology
    Circulant128Topology *top = new Circulant128Topology(
        CIRCULANT_NODES, queuesize, nullptr, &eventlist, ff, LOSSLESS
    );
    //top->init_network(); // speed is now internal to the topology based on your cpp file

    // Minimal FFApplication
    FFApplication app(top, ssthresh, &fct_util_out,
                      tcpRtxScanner, eventlist,
                      (FFApplication::FFAllReduceStrategy)0);
    app.ngpupernode = 1;
    app.nnodes      = CIRCULANT_NODES;
    app.nswitches   = 0;

    // All 128 nodes participate in AllReduce
    std::vector<uint64_t> node_group;
    node_group.reserve(CIRCULANT_NODES);
    for (int n = 0; n < CIRCULANT_NODES; n++)
        node_group.push_back((uint64_t)n);
    app.gpus = std::vector<int>(node_group.begin(), node_group.end());

    // ---------------------------------------------------------
    // 4 DISJOINT RINGS, one per chord of C(128, {1,3,7,17})
    // ---------------------------------------------------------
    // Each chord is coprime with 128 (all odd), so each forms its own
    // Hamiltonian ring over all 128 nodes. The tensor is split into 4
    // equal shards, one per ring, and the rings run concurrently on
    // disjoint links (chord c connects node i to node (i+c) mod 128,
    // a distinct edge set per chord).
    std::vector<int> chords = {1, 3, 7, 17};

    FFMultiRingAllreduceTask *ar = new FFMultiRingAllreduceTask(
        &app,
        top,
        node_group,
        chords,
        arsize,
        0.0,     // local_runtime
        rto_ms   // per-flow TCP retransmit timeout
    );
    // ---------------------------------------------------------

    // Register with app so cleanup() can find it
    app.tasks[(uint64_t)(void*)ar] = ar;
    FFApplication::total_apps      = 1;
    FFApplication::finished_apps   = 0;

    eventlist.sourceIsPending(*ar, timeFromSec(0.0));

    cerr << "[Circulant128-Bench] Simulation starting...\n";
    while (eventlist.doNextEvent()) {}

    cerr << "[Circulant128-Bench] FinalFinish "
         << timeAsMs(app.final_finish_time) << " ms\n";

    // Also write to ofile so the sweep script can parse it
    if (fct_util_out.is_open()) {
        fct_util_out << "FinalFinish " << app.final_finish_time << "\n";
        fct_util_out.close();
    }

    delete top;
    return 0;
}
