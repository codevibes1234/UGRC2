
#include "config.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
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
#include "graph_topology.h"
#include "graph_gen.h"
#include "ffapp_rhd.h"
#include "ffapp_fcstep_zcube.h"
#include "ffapp_fctree.h"
#include "queue_lossless_input.h"
#include "queue_lossless_output.h"
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
EventList eventlist;
Logfile *lg = nullptr;

#define DEFAULT_QUEUE_SIZE 200
#define DEFAULT_SPEED      100000
#define DEFAULT_PACKET_SIZE 9000
#define DEFAULT_HEADER_SIZE 64

std::string ntoa(double n)    { std::stringstream s; s << n; return s.str(); }
std::string itoa(uint64_t n)  { std::stringstream s; s << n; return s.str(); }

static void exit_error(const char *prog)
{
    std::cerr << "Usage: " << prog
              << " -topo <jellyfish|flatbutterfly|dragonfly|megafly>"
              << " -nodes <n> -msgsize <bytes>"
              << " [-speed <Mbps>] [-rtt <us>] [-ssthresh <pkts>] [-q <pkts>]"
              << " [-ecnthresh <pkts>] [-rto <ms>] [-simtime <sec>]"
              << " [-queue ecn|lossless] [-routing updown|ecmp]"
              << " [-algo forestcoll|rhd|ring] [-fcpaths] [-lqsize <pkts>]"
              << " [-fcexec step|dataflow|tree] [-fctrees <forestcoll.xml>]"
              << " [-fcpieces <n> | -fcpiecebytes <bytes>] [-fcwindow <n>]"
              << " [-fcphasebarrier 0|1]"
              << " [-ofile <file>]\n";
    std::exit(1);
}

int main(int argc, char **argv)
{
    TcpPacket::set_packet_size(DEFAULT_PACKET_SIZE - DEFAULT_HEADER_SIZE);

    std::string topo, ofile = "graph_fc_bench.txt";
    std::string queue_kind = "lossless";
    std::string routing = "updown";
    std::string algo = "forestcoll";
    bool fc_paths = false;
    int lqsize_pkts = 0;
    double pfc_xoff = 15, pfc_xon = 12, pfc_buf = -1;
    long long pfc_delay_ns = 0;
    std::string fc_exec = "step";
    std::string fc_schedule_file;
    std::string fc_trees_file;
    int fc_pieces = 1, fc_window = 1, fc_phase_barrier = 1;
    uint64_t fc_piece_bytes = 0;
    double rtx_scan_ms = 1000;
    int no_of_nodes = 0;
    uint64_t msgsize = 0;
    int ssthresh = 15;
    int queuesize_pkts = DEFAULT_QUEUE_SIZE;
    double simtime = 5.0, rto_ms = 10.0;
    SPEED = DEFAULT_SPEED;

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-topo"))      { if (++i >= argc) exit_error(argv[0]); topo = argv[i]; }
        else if (!strcmp(argv[i], "-nodes"))     { if (++i >= argc) exit_error(argv[0]); no_of_nodes = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-msgsize"))   { if (++i >= argc) exit_error(argv[0]); msgsize = std::atoll(argv[i]); }
        else if (!strcmp(argv[i], "-speed"))     { if (++i >= argc) exit_error(argv[0]); SPEED = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-rtt"))       { if (++i >= argc) exit_error(argv[0]); RTT = RTT_rack = RTT_net = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-ssthresh"))  { if (++i >= argc) exit_error(argv[0]); ssthresh = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-q"))         { if (++i >= argc) exit_error(argv[0]); queuesize_pkts = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-ecnthresh")) { if (++i >= argc) exit_error(argv[0]); ECNTHRESH = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-rto"))       { if (++i >= argc) exit_error(argv[0]); rto_ms = atof(argv[i]); }
        else if (!strcmp(argv[i], "-simtime"))   { if (++i >= argc) exit_error(argv[0]); simtime = atof(argv[i]); }
        else if (!strcmp(argv[i], "-queue"))     { if (++i >= argc) exit_error(argv[0]); queue_kind = argv[i]; }
        else if (!strcmp(argv[i], "-routing"))   { if (++i >= argc) exit_error(argv[0]); routing = argv[i]; }
        else if (!strcmp(argv[i], "-algo"))      { if (++i >= argc) exit_error(argv[0]); algo = argv[i]; }
        else if (!strcmp(argv[i], "-fcpaths"))   { fc_paths = true; }
        else if (!strcmp(argv[i], "-fcexec"))    { if (++i >= argc) exit_error(argv[0]); fc_exec = argv[i]; }
        else if (!strcmp(argv[i], "-fcschedule")) { if (++i >= argc) exit_error(argv[0]); fc_schedule_file = argv[i]; }
        else if (!strcmp(argv[i], "-fctrees"))   { if (++i >= argc) exit_error(argv[0]); fc_trees_file = argv[i]; }
        else if (!strcmp(argv[i], "-fcpieces"))  { if (++i >= argc) exit_error(argv[0]); fc_pieces = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-fcpiecebytes")) { if (++i >= argc) exit_error(argv[0]); fc_piece_bytes = std::atoll(argv[i]); }
        else if (!strcmp(argv[i], "-fcwindow"))  { if (++i >= argc) exit_error(argv[0]); fc_window = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-fcphasebarrier")) { if (++i >= argc) exit_error(argv[0]); fc_phase_barrier = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-rtxscanms")) { if (++i >= argc) exit_error(argv[0]); rtx_scan_ms = atof(argv[i]); }
        else if (!strcmp(argv[i], "-lqsize"))    { if (++i >= argc) exit_error(argv[0]); lqsize_pkts = atoi(argv[i]); }
        else if (!strcmp(argv[i], "-pfcdelayns")) { if (++i >= argc) exit_error(argv[0]); pfc_delay_ns = atoll(argv[i]); }
        else if (!strcmp(argv[i], "-pfcxoff"))   { if (++i >= argc) exit_error(argv[0]); pfc_xoff = atof(argv[i]); }
        else if (!strcmp(argv[i], "-pfcxon"))    { if (++i >= argc) exit_error(argv[0]); pfc_xon = atof(argv[i]); }
        else if (!strcmp(argv[i], "-pfcbuf"))    { if (++i >= argc) exit_error(argv[0]); pfc_buf = atof(argv[i]); }
        else if (!strcmp(argv[i], "-ofile"))     { if (++i >= argc) exit_error(argv[0]); ofile = argv[i]; }
        else { std::cerr << "Unknown argument " << argv[i] << "\n"; exit_error(argv[0]); }
    }

    if (topo.empty() || no_of_nodes <= 0 || msgsize == 0) exit_error(argv[0]);
    if (queue_kind != "ecn" && queue_kind != "lossless") {
        std::cerr << "ERROR: -queue must be ecn or lossless\n"; return 1;
    }
    if (algo != "forestcoll" && algo != "rhd" && algo != "ring") {
        std::cerr << "ERROR: -algo must be forestcoll, rhd or ring\n"; return 1;
    }
    if (fc_exec != "step" && fc_exec != "dataflow" && fc_exec != "tree") {
        std::cerr << "ERROR: -fcexec must be step, dataflow or tree\n"; return 1;
    }
    const bool tree_exec = (algo == "forestcoll" && fc_exec == "tree");
    if (tree_exec && fc_trees_file.empty()) {
        std::cerr << "ERROR: -fcexec tree needs -fctrees <forestcoll.xml>\n"; return 1;
    }
    if (tree_exec && (fc_pieces < 1 || fc_window < 1)) {
        std::cerr << "ERROR: -fcpieces and -fcwindow must be at least 1\n"; return 1;
    }

    fct_util_out.open(ofile.c_str());
    if (!fct_util_out) { std::cerr << "ERROR: cannot write " << ofile << "\n"; return 1; }

    eventlist.setEndtime(timeFromSec(simtime));
    Clock c(timeFromSec(50 / 100.), eventlist);
    mem_b queuesize = memFromPkt(queuesize_pkts);
    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(rtx_scan_ms), eventlist);

    {
        uint64_t rtt_ps = 12ULL * RTT_net * 1000ULL;
        uint64_t bdp_bytes = (uint64_t)SPEED * 1000000ULL / 8ULL * rtt_ps / 1000000000000ULL;
        uint32_t credits = (uint32_t)(2 * bdp_bytes / 9000);
        RoCEv2Src::default_initial_credits = credits < 8 ? 8 : credits;
    }

    if (pfc_buf < 0) pfc_buf = pfc_xoff + 5;
    LosslessInputQueue::configure(pfc_xoff, pfc_xon, pfc_buf);
    LosslessInputQueue::cfg_pause_delay = (simtime_picosec)pfc_delay_ns * 1000ULL;

    SwitchGraph g = make_named(topo, no_of_nodes);
    if (g.nsw == 0) { std::cerr << "ERROR: unknown -topo " << topo << "\n"; return 1; }

    queue_type qt = (queue_kind == "lossless") ? LOSSLESS_INPUT : ECN;
    GraphTopology *top = new GraphTopology(g, queuesize, nullptr, &eventlist, ff, qt,
                                           8, routing,
                                           lqsize_pkts > 0 ? memFromPkt(lqsize_pkts) : 0);

    if (!fc_schedule_file.empty() && !top->load_fc_schedule_file(fc_schedule_file))
        return 1;
    FCTreeSchedule fc_trees;
    if (tree_exec && !fc_trees.load_xml(fc_trees_file, no_of_nodes))
        return 1;
    const int sched_n = tree_exec ? fc_trees.nhost : top->fc_schedule_nhost();
    if (algo == "forestcoll" && !tree_exec && top->fc_num_steps() <= 0) {
        std::cerr << "ERROR: no ForestColl schedule is compiled into this binary.\n"
                     "       Rebuild with -DFC_SCHEDULE_FILE='\"<schedule>.cpp\"';\n"
                     "       see scripts/gap2/build_fc_bench.sh.\n";
        return 1;
    }
    if (algo == "forestcoll" && sched_n != no_of_nodes) {
        std::cerr << "ERROR: this binary has a " << sched_n << "-host ForestColl "
                  << "schedule compiled in, but -nodes is " << no_of_nodes << ".\n"
                     "       Build one binary per (family, size).\n";
        return 1;
    }

    std::cerr << "[graph-" << algo << "] " << g.name << "\n"
              << "  hosts " << no_of_nodes << "  queue " << queue_kind
              << "  routing " << routing << "  speed " << SPEED << " Mbps\n"
              << "  schedule hosts " << sched_n
              << "  msgsize " << msgsize << " B\n";

    FFApplication app(top, ssthresh, &fct_util_out, tcpRtxScanner, eventlist,
                      FF_DEFAULT_AR);
    app.bfb_rto_ms = rto_ms;
    app.ngpupernode = 1;
    app.nnodes = no_of_nodes;
    app.nswitches = 0;

    app.gpus.resize(no_of_nodes);
    for (int n = 0; n < no_of_nodes; ++n) app.gpus[n] = n;

    FFTask *ar = nullptr;
    FFFCTreeAllreduceTask *tree_task = nullptr;
    if (tree_exec) {
        FFFCTreeAllreduceTask *fc = new FFFCTreeAllreduceTask(&app, top, &fc_trees, msgsize);
        tree_task = fc;
        if (fc_piece_bytes > 0) {
            uint64_t p = fc->chunk_bytes / fc_piece_bytes;
            fc_pieces = (int)(p < 1 ? 1 : (p > 4096 ? 4096 : p));
        }
        fc->pieces = fc_pieces;
        fc->window = fc_window;
        fc->phase_barrier = (fc_phase_barrier != 0);
        fc->use_fc_paths = fc_paths;
        std::cerr << "  ForestColl trees " << fc_trees.trees.size() << " from " << fc_trees_file
                  << " (" << fc_trees.dropped_sends << " switch-endpoint sends dropped)\n";
        ar = fc;
    } else if (algo == "forestcoll" && fc_exec == "dataflow") {
        FFFCDataflowAllreduceTask *fc =
            new FFFCDataflowAllreduceTask(&app, top, top, msgsize);
        if (fc_paths) {
            if (!top->fc_has_paths()) {
                std::cerr << "ERROR: -fcpaths given, but the schedule carries no paths.\n";
                return 1;
            }
            fc->use_fc_paths = true;
        }
        ar = fc;
    } else if (algo == "forestcoll") {
        FFZCubeFCStepAllreduceTask *fc =
            new FFZCubeFCStepAllreduceTask(&app, top, top, msgsize, 0.0);
        if (fc_paths) {
            if (!top->fc_has_paths()) {
                std::cerr << "ERROR: -fcpaths given, but the compiled-in schedule "
                             "carries no paths.\n       Regenerate it with the "
                             "current generate_graph_schedule.py.\n";
                return 1;
            }
            fc->use_fc_paths = true;
        }
        ar = fc;
    } else {
        std::vector<uint64_t> ng(no_of_nodes);
        for (int n = 0; n < no_of_nodes; ++n) ng[n] = (uint64_t)n;
        if (algo == "ring")
            ar = new FFGroupRingAllreduceTask(&app, top, ng, msgsize);
        else
            ar = new FFZCubeRHDAllreduceTask(&app, top, ng, msgsize, 0.0);
    }

    app.tasks[reinterpret_cast<uint64_t>(ar)] = ar;
    FFApplication::total_apps = 1;
    FFApplication::finished_apps = 0;

    eventlist.sourceIsPending(*ar, timeFromSec(0.0));

    while (eventlist.doNextEvent()) { }

    const simtime_picosec finish = app.final_finish_time;
    if (finish == 0) {
        std::cerr << "\nERROR: ForestColl AllReduce did not finish before "
                  << simtime << " s.\n";
        fct_util_out << "ERROR ForestColl_AllReduce_did_not_finish\n";
        fct_util_out.flush();
        return 2;
    }

    const simtime_picosec start = ar->start_time;
    const double lat_ms = timeAsMs(finish - start);

    double algbw = 0.0;
    if (lat_ms > 0.0) {
        const double useful_bytes = (double)msgsize * 2.0
                                  * (double)(no_of_nodes - 1) / (double)no_of_nodes;
        algbw = useful_bytes / (lat_ms * 1e-3) / 1e9;
    }

    std::cerr << "\n============================================================\n"
              << " Results\n"
              << "============================================================\n"
              << "  algorithm         : " << algo
              << (algo == "forestcoll" ? " (" + fc_exec + " executor)" : std::string()) << "\n"
              << "  topology          : " << g.name << "\n"
              << "  AllReduce latency : " << lat_ms << " ms\n"
              << "  algBW             : " << algbw << " GB/s\n"
              << "============================================================\n";

    fct_util_out << "LosslessQueuePkts " << (lqsize_pkts > 0 ? lqsize_pkts : 200) << "\n"
                 << "Algo " << algo << "\n"
                 << "FCPaths " << (fc_paths ? 1 : 0) << "\n"
                 << "Topology " << topo << "\n"
                 << "Nodes " << no_of_nodes << "\n"
                 << "Queue " << queue_kind << "\n"
                 << "Routing " << routing << "\n"
                 << "MsgSize " << msgsize << "\n"
                 << "Latency_ms " << lat_ms << "\n"
                 << "FCExecutor " << (algo == "forestcoll" ? fc_exec : std::string("none")) << "\n"
                 << "RtxScanMs " << rtx_scan_ms << "\n";
    if (tree_task != nullptr)
        fct_util_out << "FCTreePieces " << tree_task->pieces << "\n"
                     << "FCTreeWindow " << tree_task->window << "\n"
                     << "FCTreePhaseBarrier " << (tree_task->phase_barrier ? 1 : 0) << "\n"
                     << "FCTreeFlows " << tree_task->flows_started << "\n"
                     << "FCTreeMaxInFlight " << tree_task->max_in_flight << "\n"
                     << "FCTreeRSFinish_ms " << timeAsMs(tree_task->rs_finish - start) << "\n"
                     << "FCTreeFcRouted " << tree_task->n_fc_routed << "\n"
                     << "FCTreeFallbackRouted " << tree_task->n_fallback_routed << "\n";
    fct_util_out
                 << "AlgBW_GBps " << algbw << "\n"
                 << "FinalFinish " << finish << "\n";
    LosslessInputQueue::print_stats(fct_util_out);
    LosslessOutputQueue::print_stats(fct_util_out);
    fct_util_out.flush();
    return 0;
}
