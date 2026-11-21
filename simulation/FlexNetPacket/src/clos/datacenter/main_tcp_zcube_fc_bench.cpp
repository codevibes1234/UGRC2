// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * main_tcp_zcube_fc_bench.cpp
 * ===========================
 *
 * Pure ForestColl AllReduce microbenchmark for ZCube(4,2).
 *
 * Architecture mirrors the working main_tcp_l2k44_bench.cpp:
 *
 *      ZCubeTopology
 *           |
 *      FFApplication
 *           |
 *      FFForestCollZCubeTask
 *           |
 *        EventList
 *
 * IMPORTANT:
 *   This benchmark uses DCTCP + ECN queues.
 *   It does NOT use LOSSLESS/PFC queues.
 *
 * Usage:
 *
 *   ./htsim_tcp_zcube_fc_bench \
 *       -msgsize 1073741824 \
 *       -speed 200000 \
 *       -ssthresh 15 \
 *       -q 200 \
 *       -ecnthresh 50 \
 *       -rto 10.0 \
 *       -simtime 5.0 \
 *       -ofile fc_1gb.txt
 */

#include "config.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "network.h"
#include "randomqueue.h"
#include "pipe.h"
#include "eventlist.h"
#include "logfile.h"
#include "loggers.h"
#include "clock.h"
#include "tcp.h"
#include "rocev2.h"
#include "mtcp.h"
#include "compositequeue.h"
#include "firstfit.h"
#include "topology.h"
#include "ecn.h"

#include "zcube_topology.h"
#include "ffapp.h"
#include "ffapp_forestcoll_zcube.h"
#include "main.h"


// ============================================================================
// Globals required by htsim / topology code
// ============================================================================

uint32_t RTT_rack  = 500;
uint32_t RTT_net   = 500;
uint32_t RTT       = 500;
uint32_t ECNTHRESH = 50;
uint32_t SPEED     = 0;

std::ofstream fct_util_out;

FirstFit *ff = nullptr;

#define DEFAULT_PACKET_SIZE 9000
#define DEFAULT_HEADER_SIZE 64
#define DEFAULT_QUEUE_SIZE  200
#define DEFAULT_SPEED       200000

EventList eventlist;

Logfile *lg = nullptr;


// ============================================================================
// Helpers expected by existing TopoOpt / htsim code
// ============================================================================

std::string ntoa(double n)
{
    std::stringstream s;
    s << n;
    return s.str();
}

std::string itoa(uint64_t n)
{
    std::stringstream s;
    s << n;
    return s.str();
}

static void exit_error(const char *prog)
{
    std::cerr
        << "Usage: " << prog
        << " -msgsize <bytes>"
        << " [-speed <Mbps>]"
        << " [-rtt <us>]"
        << " [-ssthresh <pkts>]"
        << " [-q <pkts>]"
        << " [-ecnthresh <pkts>]"
        << " [-rto <ms>]"
        << " [-simtime <sec>]"
        << " [-ofile <file>]"
        << "\n";

    std::exit(1);
}


// ============================================================================
// Main
// ============================================================================

#ifdef USE_DCTCP
#define FABRIC_QUEUE ECN
#else
#define FABRIC_QUEUE LOSSLESS_INPUT
#endif

int main(int argc, char **argv)
{
    // ------------------------------------------------------------------------
    // Defaults
    // ------------------------------------------------------------------------

    TcpPacket::set_packet_size(
        DEFAULT_PACKET_SIZE - DEFAULT_HEADER_SIZE
    );

    mem_b queuesize =
        memFromPkt(DEFAULT_QUEUE_SIZE);

    int ssthresh = 15;

    double simtime = 5.0;

    double rto_ms = 10.0;

    uint64_t msgsize = 0;

    std::string ofile =
        "zcube_fc_bench_result.txt";

    SPEED = DEFAULT_SPEED;


    // ------------------------------------------------------------------------
    // Parse command-line arguments
    // ------------------------------------------------------------------------

    int i = 1;

    while (i < argc) {

        if (!std::strcmp(argv[i], "-msgsize")) {

            if (++i >= argc)
                exit_error(argv[0]);

            msgsize =
                static_cast<uint64_t>(
                    std::atoll(argv[i])
                );
        }

        else if (!std::strcmp(argv[i], "-speed")) {

            if (++i >= argc)
                exit_error(argv[0]);

            SPEED =
                static_cast<uint32_t>(
                    std::atoi(argv[i])
                );
        }

        else if (!std::strcmp(argv[i], "-rtt")) {

            if (++i >= argc)
                exit_error(argv[0]);

            RTT =
                static_cast<uint32_t>(
                    std::atoi(argv[i])
                );
        }

        else if (!std::strcmp(argv[i], "-ssthresh")) {

            if (++i >= argc)
                exit_error(argv[0]);

            ssthresh =
                std::atoi(argv[i]);
        }

        else if (!std::strcmp(argv[i], "-q")) {

            if (++i >= argc)
                exit_error(argv[0]);

            queuesize =
                memFromPkt(
                    std::atoi(argv[i])
                );
        }

        else if (!std::strcmp(argv[i], "-ecnthresh")) {

            if (++i >= argc)
                exit_error(argv[0]);

            ECNTHRESH =
                static_cast<uint32_t>(
                    std::atoi(argv[i])
                );
        }

        else if (!std::strcmp(argv[i], "-simtime")) {

            if (++i >= argc)
                exit_error(argv[0]);

            simtime =
                std::atof(argv[i]);
        }

        else if (!std::strcmp(argv[i], "-rto")) {

            if (++i >= argc)
                exit_error(argv[0]);

            rto_ms =
                std::atof(argv[i]);
        }

        else if (!std::strcmp(argv[i], "-ofile")) {

            if (++i >= argc)
                exit_error(argv[0]);

            ofile =
                argv[i];
        }

        else {

            std::cerr
                << "Unknown argument: "
                << argv[i]
                << "\n";

            exit_error(argv[0]);
        }

        ++i;
    }


    // ------------------------------------------------------------------------
    // Validate arguments
    // ------------------------------------------------------------------------

    if (msgsize == 0) {

        std::cerr
            << "ERROR: -msgsize <bytes> is required\n";

        return 1;
    }

    if (SPEED == 0) {

        std::cerr
            << "ERROR: link speed must be > 0\n";

        return 1;
    }


    // ------------------------------------------------------------------------
    // Print benchmark configuration
    // ------------------------------------------------------------------------

    std::cerr
        << "============================================================\n"
        << " ZCube(4,2) ForestColl AllReduce Benchmark\n"
        << "============================================================\n"

        << "  Message size      : "
        << msgsize
        << " bytes ("
        << msgsize / (1024.0 * 1024.0)
        << " MiB)\n"

        << "  Link speed        : "
        << SPEED
        << " Mbps\n"

        << "  RTT parameter     : "
        << RTT
        << "\n"

        << "  TCP ssthresh      : "
        << ssthresh
        << " packets\n"

        << "  Queue size        : "
        << (queuesize / DEFAULT_PACKET_SIZE)
        << " packets approx.\n"

        << "  ECN threshold     : "
        << ECNTHRESH
        << " packets\n"

        << "  RTO               : "
        << rto_ms
        << " ms\n"

        << "  Simulation limit  : "
        << simtime
        << " s\n"

        << "  GPUs              : 16\n"
        << "  GPUs/server       : 1\n"

        << "  ForestColl chunks : 32\n"
        << "  Tree records      : 25\n"

        << "  Queue model       : ECN\n"
        << "  Transport         : DCTCP\n"
        << "  LOSSLESS/PFC      : disabled\n"

        << "============================================================\n";


    // ------------------------------------------------------------------------
    // Simulator setup
    // ------------------------------------------------------------------------

    std::srand(13);

    eventlist.setEndtime(
        timeFromSec(simtime)
    );

    Clock clock(
        timeFromSec(5.0 / 100.0),
        eventlist
    );

    TcpRtxTimerScanner tcpRtxScanner(
        timeFromMs(1),
        eventlist
    );


    // ------------------------------------------------------------------------
    // Output file
    // ------------------------------------------------------------------------

    fct_util_out.open(ofile);

    if (!fct_util_out.is_open()) {

        std::cerr
            << "ERROR: could not open output file "
            << ofile
            << "\n";

        return 1;
    }


    // ------------------------------------------------------------------------
    // Build ZCube(4,2)
    //
    // 16 endpoint GPUs / servers:
    //
    //      GPU0 ... GPU15
    //
    // One GPU per physical server.
    //
    // IMPORTANT:
    //
    //     ECN
    //
    // ------------------------------------------------------------------------


    {
        uint64_t rtt_ps = 12ULL * RTT_net * 1000ULL;
        uint64_t bdp_bytes = (uint64_t)SPEED * 1000000ULL / 8ULL * rtt_ps / 1000000000000ULL;
        uint32_t credits = (uint32_t)(2 * bdp_bytes / 9000);
        RoCEv2Src::default_initial_credits = credits < 8 ? 8 : credits;
        std::cerr << "  RoCE credits : " << RoCEv2Src::default_initial_credits << "\n";
    }
    ZCubeTopology *top =
        new ZCubeTopology(
            16,
            queuesize,
            nullptr,
            &eventlist,
            ff,
            FABRIC_QUEUE
        );


    // ------------------------------------------------------------------------
    // Minimal FFApplication
    //
    // ffapp_forestcoll_zcube.cpp provides the FFApplication/FFTask
    // implementation needed by this standalone benchmark.
    //
    // The original ffapp.cpp is NOT intended to be linked into this
    // standalone executable.
    // ------------------------------------------------------------------------

    FFApplication app(
        top,
        ssthresh,
        &fct_util_out,
        tcpRtxScanner,
        eventlist,
        FFApplication::FF_DEFAULT_AR
    );

    app.bfb_rto_ms = rto_ms;

    app.ngpupernode = 1;

    app.nnodes = 16;

    app.nswitches = 0;


    // ------------------------------------------------------------------------
    // All 16 GPUs participate in the collective
    // ------------------------------------------------------------------------

    std::vector<uint64_t> node_group;

    node_group.reserve(16);

    for (int n = 0; n < 16; ++n) {

        node_group.push_back(
            static_cast<uint64_t>(n)
        );
    }

    app.gpus =
        std::vector<int>(
            node_group.begin(),
            node_group.end()
        );


    // ------------------------------------------------------------------------
    // Create ForestColl AllReduce
    //
    // Constructor:
    //
    // FFForestCollZCubeTask(
    //     FFApplication *,
    //     Topology *,
    //     std::vector<uint64_t>,
    //     uint64_t operator_size,
    //     double local_runtime
    // )
    // ------------------------------------------------------------------------

    FFForestCollZCubeTask *ar =
        new FFForestCollZCubeTask(
            &app,
            top,
            node_group,
            msgsize,
            0.0
        );


    // ------------------------------------------------------------------------
    // Register task with FFApplication
    //
    // FFTask::cleanup() expects the task to be present in app.tasks.
    // ------------------------------------------------------------------------

    app.tasks[
        reinterpret_cast<uint64_t>(ar)
    ] = ar;


    // ------------------------------------------------------------------------
    // This benchmark contains exactly one FFApplication.
    // ------------------------------------------------------------------------

    FFApplication::total_apps = 1;

    FFApplication::finished_apps = 0;


    // ------------------------------------------------------------------------
    // Schedule ForestColl at t = 0
    // ------------------------------------------------------------------------

    eventlist.sourceIsPending(
        *ar,
        timeFromSec(0.0)
    );


    std::cerr
        << "[ZCube-ForestColl] Simulation starting...\n";


    // ------------------------------------------------------------------------
    // Run simulator
    // ------------------------------------------------------------------------

    while (eventlist.doNextEvent()) {
    }


    // ------------------------------------------------------------------------
    // Get completion time
    // ------------------------------------------------------------------------

    const simtime_picosec finish =
        app.final_finish_time;


    if (finish == 0) {

        std::cerr
            << "\n"
            << "ERROR: ForestColl AllReduce did not finish "
            << "before simulation end time "
            << simtime
            << " s.\n";

        fct_util_out
            << "ERROR ForestColl_AllReduce_did_not_finish\n";

        fct_util_out.flush();

        return 2;
    }


    // ------------------------------------------------------------------------
    // ForestColl was scheduled at t=0.
    //
    // FFTask exposes start_time as a member, not a function.
    // ------------------------------------------------------------------------

    const simtime_picosec start =
        ar->start_time;


    const simtime_picosec elapsed =
        finish - start;


    const double lat_ms =
        timeAsMs(elapsed);


    // ------------------------------------------------------------------------
    // Standard AllReduce algorithm bandwidth
    //
    // N = 16
    //
    //                 2(N-1)
    // data volume =   ------ M
    //                    N
    //
    // algBW =
    //
    //       2 * (N-1)/N * M
    //       -----------------
    //            elapsed
    //
    // Result below is GB/s.
    // ------------------------------------------------------------------------

    double algbw = 0.0;

    if (lat_ms > 0.0) {

        const double useful_bytes =
            static_cast<double>(msgsize)
            * 2.0
            * 15.0
            / 16.0;

        const double elapsed_sec =
            lat_ms * 1e-3;

        algbw =
            useful_bytes
            / elapsed_sec
            / 1e9;
    }


    // ------------------------------------------------------------------------
    // Human-readable results
    // ------------------------------------------------------------------------

    std::cerr
        << "\n"
        << "============================================================\n"
        << " Results\n"
        << "============================================================\n"

        << "  Start time        : "
        << timeAsMs(start)
        << " ms\n"

        << "  Finish time       : "
        << timeAsMs(finish)
        << " ms\n"

        << "  AllReduce latency : "
        << lat_ms
        << " ms\n"

        << "  Algorithm BW      : "
        << algbw
        << " GB/s\n"

        << "============================================================\n";


    // ------------------------------------------------------------------------
    // Machine-readable output
    // ------------------------------------------------------------------------

    fct_util_out
        << "MsgSize_bytes "
        << msgsize
        << "\n"

        << "Speed_Mbps "
        << SPEED
        << "\n"

        << "Queue_pkts "
        << (queuesize / DEFAULT_PACKET_SIZE)
        << "\n"

        << "ECNThreshold_pkts "
        << ECNTHRESH
        << "\n"

        << "Start_ps "
        << start
        << "\n"

        << "Finish_ps "
        << finish
        << "\n"

        << "Latency_ms "
        << lat_ms
        << "\n"

        << "AlgBW_GBs "
        << algbw
        << "\n"

        << "FinalFinish "
        << finish
        << "\n";


    fct_util_out.flush();


    // ------------------------------------------------------------------------
    // Cleanup
    //
    // Do NOT delete ar here.
    //
    // It is registered in app.tasks, and FFApplication owns registered
    // FFTask objects.
    //
    // We intentionally also avoid:
    //
    //     delete top;
    //
    // because the existing Topology hierarchy produces the
    // non-virtual-destructor warning. This standalone process is about
    // to exit anyway.
    // ------------------------------------------------------------------------

    return 0;
}
