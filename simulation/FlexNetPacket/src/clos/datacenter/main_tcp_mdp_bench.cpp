#include "config.h"
#include <sstream>
#include <strstream>
#include <fstream>
#include <iostream>
#include <string.h>
#include <math.h>
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

#include "fat_tree_topology.h"
#include "zcube_topology.h"
#include "zcube_partial_topology.h"
#include "graph_topology.h"
#include "graph_gen.h"
#include "ffapp_mdp.h"
#ifndef USE_DCTCP
#include "rocev2.h"
#endif
#include "main.h"

uint32_t RTT_rack  = 500;
uint32_t RTT_net   = 500;
uint32_t RTT       = 500;
uint32_t SPEED     = 0;
std::ofstream fct_util_out;
FirstFit *ff = nullptr;

#define DEFAULT_PACKET_SIZE 9000
#define DEFAULT_HEADER_SIZE 64

EventList eventlist;
Logfile *lg;
string ntoa(double n);
string itoa(uint64_t n);

static void exit_error(char *prog)
{
    std::cerr << "Usage: " << prog
              << " -topo <t> -nodes N -dp D -tp T -pp P -ep E -dims tp,dp,ep,pp"
                 " -speed Mbps [-tpbytes B] [-dpbytes B] [-epbytes B] [-ppbytes B]"
                 " [-ssthresh S] [-q pkts] [-simtime s] -ofile F\n";
    exit(1);
}

static std::vector<std::string> split(const std::string& s, char sep)
{
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, sep)) if (!item.empty()) out.push_back(item);
    return out;
}

int main(int argc, char **argv)
{
    TcpPacket::set_packet_size(DEFAULT_PACKET_SIZE - DEFAULT_HEADER_SIZE);
    mem_b queuesize = memFromPkt(200);

    MDPSpec spec;
    std::string routing = "ecmp";
    std::string topo = "fattree128", dims_s = "tp,dp,ep,pp", ofile = "mdp.txt";
    double simtime = 600;
    int ssthresh = 15;

    int i = 1;
    while (i < argc) {
        if      (!strcmp(argv[i], "-topo")     && i+1 < argc) topo = argv[++i];
        else if (!strcmp(argv[i], "-routing") && i+1 < argc) routing = argv[++i];
        else if (!strcmp(argv[i], "-dims")     && i+1 < argc) dims_s = argv[++i];
        else if (!strcmp(argv[i], "-nodes")    && i+1 < argc) spec.nnodes = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-dp")       && i+1 < argc) spec.dp = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-tp")       && i+1 < argc) spec.tp = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-pp")       && i+1 < argc) spec.pp = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-ep")       && i+1 < argc) spec.ep = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-dpbytes")  && i+1 < argc) spec.dp_bytes = strtoull(argv[++i],0,10);
        else if (!strcmp(argv[i], "-tpbytes")  && i+1 < argc) spec.tp_bytes = strtoull(argv[++i],0,10);
        else if (!strcmp(argv[i], "-epbytes")  && i+1 < argc) spec.ep_bytes = strtoull(argv[++i],0,10);
        else if (!strcmp(argv[i], "-ppbytes")  && i+1 < argc) spec.pp_bytes = strtoull(argv[++i],0,10);
        else if (!strcmp(argv[i], "-speed")    && i+1 < argc) SPEED = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-ssthresh") && i+1 < argc) ssthresh = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-q")        && i+1 < argc) queuesize = memFromPkt(atoi(argv[++i]));
        else if (!strcmp(argv[i], "-simtime")  && i+1 < argc) simtime = atof(argv[++i]);
        else if (!strcmp(argv[i], "-ofile")    && i+1 < argc) ofile = argv[++i];
        else { std::cerr << "Unknown arg: " << argv[i] << "\n"; exit_error(argv[0]); }
        i++;
    }

    if (!spec.valid()) {
        std::cerr << "ERROR: dp*tp*pp*ep (" << spec.dp*spec.tp*spec.pp*spec.ep
                  << ") must equal -nodes (" << spec.nnodes << ")\n";
        exit(1);
    }

    std::vector<std::string> dims = split(dims_s, ',');

    std::cerr << "=== Multi-dimensional parallelism collectives ===\n"
              << "  Topology     : " << topo << " with " << spec.nnodes << " GPUs\n"
              << "  Decomposition: DP=" << spec.dp << " TP=" << spec.tp
              << " PP=" << spec.pp << " EP=" << spec.ep << "\n"
              << "  Dimensions   : " << dims_s << "\n"
              << "  Sizes (MB)   : tp=" << spec.tp_bytes/1048576.0
              << " dp=" << spec.dp_bytes/1048576.0
              << " ep=" << spec.ep_bytes/1048576.0
              << " pp=" << spec.pp_bytes/1048576.0 << "\n"
              << "  NIC speed    : " << SPEED << " Mbps\n";

    srand(13);
    eventlist.setEndtime(timeFromSec(simtime));
    Clock c(timeFromSec(5.0/100.0), eventlist);
    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(1), eventlist);
    fct_util_out = std::ofstream(ofile);

#ifdef USE_DCTCP
    queue_type qt = ECN;
#else
    queue_type qt = LOSSLESS_INPUT;
    {
        uint64_t rtt_ps = 12ULL * RTT_net * 1000ULL;
        uint64_t bdp = (uint64_t)SPEED * 1000000ULL / 8ULL * rtt_ps / 1000000000000ULL;
        uint32_t credits = (uint32_t)(2 * bdp / 9000);
        RoCEv2Src::default_initial_credits = credits < 8 ? 8 : credits;
        std::cerr << "  RoCE credits : " << RoCEv2Src::default_initial_credits << "\n";
    }
#endif

    Topology* top;
    if      (topo == "fattree128")   top = new FatTreeTopology(spec.nnodes, queuesize, nullptr, &eventlist, ff, qt);
    else if (topo == "zcubepartial") top = new ZCubePartialTopology(spec.nnodes, queuesize, nullptr, &eventlist, ff, qt);
    else if (topo == "jellyfish")    top = new GraphTopology(make_jellyfish(spec.nnodes, 12, 4, 13), queuesize, nullptr, &eventlist, ff, qt, 8, routing);
    else if (topo == "flatbutterfly")top = new GraphTopology(make_flattened_butterfly(spec.nnodes, 4, 2, 8), queuesize, nullptr, &eventlist, ff, qt, 8, routing);
    else if (topo == "dragonfly")    top = new GraphTopology(make_dragonfly(spec.nnodes, 8, 4, 4), queuesize, nullptr, &eventlist, ff, qt, 8, routing);
    else if (topo == "megafly")      top = new GraphTopology(make_megafly(spec.nnodes, 4, 4, 2, 8), queuesize, nullptr, &eventlist, ff, qt, 8, routing);
    else { std::cerr << "Unknown -topo " << topo << "\n"; exit(1); }

    FFApplication app(top, ssthresh, &fct_util_out, tcpRtxScanner, eventlist, FF_DEFAULT_AR);
    app.ngpupernode = 1;
    app.nnodes      = spec.nnodes;
    app.nswitches   = 0;
    app.gpus.resize(spec.nnodes);
    std::iota(app.gpus.begin(), app.gpus.end(), 0);

    std::vector<FFTask*> tasks;
    std::vector<std::string> labels;
    mdp_build_tasks(&app, top, spec, dims, tasks, labels);

    while (eventlist.doNextEvent()) {}

    std::map<std::string, simtime_picosec> class_finish;
    std::map<std::string, int> class_count, class_incomplete;
    simtime_picosec makespan = 0;
    int incomplete = 0;
    for (size_t k = 0; k < tasks.size(); k++) {
        const std::string& L = labels[k];
        class_count[L]++;
        simtime_picosec f = tasks[k]->finish_time;
        if (f == 0) { incomplete++; class_incomplete[L]++; continue; }
        if (f > class_finish[L]) class_finish[L] = f;
        if (f > makespan) makespan = f;
    }

    for (const auto& kv : class_count) {
        fct_util_out << "Class_" << kv.first << "_ms "
                     << timeAsMs(class_finish[kv.first]) << "\n";
        fct_util_out << "Tasks_" << kv.first << " " << kv.second << "\n";
        fct_util_out << "Incomplete_" << kv.first << " " << class_incomplete[kv.first] << "\n";
    }
    fct_util_out << "Makespan_ms " << timeAsMs(makespan) << "\n";
    fct_util_out << "Incomplete " << incomplete << "\n";

    std::cerr << "\n=== Results ===\n";
    for (const auto& kv : class_count)
        std::cerr << "  " << kv.first << ": " << timeAsMs(class_finish[kv.first])
                  << " ms over " << kv.second << " groups ("
                  << class_incomplete[kv.first] << " incomplete)\n";
    std::cerr << "  makespan: " << timeAsMs(makespan) << " ms, "
              << incomplete << " incomplete tasks\n";
    return 0;
}

string ntoa(double n) { stringstream s; s << n; return s.str(); }
string itoa(uint64_t n) { stringstream s; s << n; return s.str(); }
