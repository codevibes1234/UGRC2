#include "config.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <string.h>
#include <unordered_set>
#include <vector>
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
#include "flat_topology.h"
#include "dyn_net_sch.h"
#include "queue_lossless_input.h"
#include "queue_lossless_output.h"
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
              << " -topo <t> [-routing ecmp|updown] -nodes N"
                 " -jobs J -jobnodes S -jobdp D -jobtp T -jobpp P -jobep E"
                 " -dims tp,dp,ep,pp [-placement compact|scatter] [-placementseed S]"
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

static uint32_t g_placement_seed = 13;

static std::vector<int> block_placement(std::unordered_set<int>& candidates, int nodes,
                                        int bs, int n)
{
    static std::mt19937 bgen(g_placement_seed);
    std::vector<int> free_blocks;
    for (int b = 0; b * bs + bs <= nodes; b++) {
        bool all = true;
        for (int r = 0; r < bs && all; r++) all = candidates.count(b * bs + r) > 0;
        if (all) free_blocks.push_back(b);
    }
    const int need = n / bs;
    if ((int)free_blocks.size() < need) {
        std::cerr << "ERROR: block placement needs " << need << " free blocks of " << bs
                  << " nodes, only " << free_blocks.size() << " left\n";
        exit(1);
    }
    std::vector<int> chosen;
    std::sample(free_blocks.begin(), free_blocks.end(), std::back_inserter(chosen), need, bgen);
    std::vector<int> result;
    for (int b : chosen)
        for (int r = 0; r < bs; r++) {
            result.push_back(b * bs + r);
            candidates.erase(b * bs + r);
        }
    return result;
}

static std::vector<int> scatter_placement(std::unordered_set<int>& candidates, int n)
{
    std::vector<int> result;
    assert(n <= (int)candidates.size());
    static std::mt19937 gen(g_placement_seed);
    std::sample(candidates.begin(), candidates.end(), std::back_inserter(result), n, gen);
    for (int i : result) candidates.erase(candidates.find(i));
    return result;
}

int main(int argc, char **argv)
{
    TcpPacket::set_packet_size(DEFAULT_PACKET_SIZE - DEFAULT_HEADER_SIZE);
    mem_b queuesize = memFromPkt(200);

    MDPSpec spec;
    spec.nnodes = 32; spec.dp = 2; spec.tp = 4; spec.pp = 2; spec.ep = 2;
    int nodes = 128, njobs = 4;
    std::string routing = "ecmp", placement_policy = "compact";
    int block_size = 0;
    std::string jobmix_s;
    std::string ocs_logical = "";
    int ocs_degree = 4;
    int ocs_rdelay = 10;
    double pfc_xoff = 15, pfc_xon = 12, pfc_buf = -1;
    int phased_mb = 0;
    int f1b_mb = 0, f1b_layers = 4, f1b_buckets = 4;
    double layer_fwd_us = 0, layer_bwd_us = 0;
    double fwd_us = 0, bwd_us = 0;
    long long pfc_delay_ns = 0;
    int pfc_classes = 1;
    long long rdelay_ns = -1, rperiod_ns = 0;
    std::string topo = "fattree128", dims_s = "tp,dp,ep,pp", ofile = "mt_mdp.txt";
    double simtime = 600;
    int ssthresh = 15;

    int i = 1;
    while (i < argc) {
        if      (!strcmp(argv[i], "-topo")      && i+1 < argc) topo = argv[++i];
        else if (!strcmp(argv[i], "-routing")   && i+1 < argc) routing = argv[++i];
        else if (!strcmp(argv[i], "-logical")   && i+1 < argc) ocs_logical = argv[++i];
        else if (!strcmp(argv[i], "-rdelayns")  && i+1 < argc) rdelay_ns = atoll(argv[++i]);
        else if (!strcmp(argv[i], "-rperiodns") && i+1 < argc) rperiod_ns = atoll(argv[++i]);
        else if (!strcmp(argv[i], "-pfcdelayns") && i+1 < argc) pfc_delay_ns = atoll(argv[++i]);
        else if (!strcmp(argv[i], "-ecnqexact")) FlatTopology::ecn_queue_exact = true;
        else if (!strcmp(argv[i], "-rocedemand") && i+1 < argc) DemandRecorder::include_roce = atoi(argv[++i]) != 0;
        else if (!strcmp(argv[i], "-logicalseed") && i+1 < argc) dyn_sch_set_logical_seed((unsigned)strtoul(argv[++i], nullptr, 10));
        else if (!strcmp(argv[i], "-pfcxoff")   && i+1 < argc) pfc_xoff = atof(argv[++i]);
        else if (!strcmp(argv[i], "-pfcxon")    && i+1 < argc) pfc_xon = atof(argv[++i]);
        else if (!strcmp(argv[i], "-pfcbuf")    && i+1 < argc) pfc_buf = atof(argv[++i]);
        else if (!strcmp(argv[i], "-pfcclasses") && i+1 < argc) pfc_classes = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-deg")       && i+1 < argc) ocs_degree = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-rdelay")    && i+1 < argc) ocs_rdelay = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-dims")      && i+1 < argc) dims_s = argv[++i];
        else if (!strcmp(argv[i], "-nodes")     && i+1 < argc) nodes = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-jobs")      && i+1 < argc) njobs = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-jobnodes")  && i+1 < argc) spec.nnodes = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-jobdp")     && i+1 < argc) spec.dp = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-jobtp")     && i+1 < argc) spec.tp = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-jobpp")     && i+1 < argc) spec.pp = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-jobep")     && i+1 < argc) spec.ep = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-allreduce") && i+1 < argc) spec.allreduce = argv[++i];
        else if (!strcmp(argv[i], "-phased")    && i+1 < argc) phased_mb = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-fwdus")     && i+1 < argc) fwd_us = atof(argv[++i]);
        else if (!strcmp(argv[i], "-iter1f1b")  && i+1 < argc) f1b_mb = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-layers")    && i+1 < argc) f1b_layers = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-dpbuckets") && i+1 < argc) f1b_buckets = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-layerfwdus") && i+1 < argc) layer_fwd_us = atof(argv[++i]);
        else if (!strcmp(argv[i], "-layerbwdus") && i+1 < argc) layer_bwd_us = atof(argv[++i]);
        else if (!strcmp(argv[i], "-bwdus")     && i+1 < argc) bwd_us = atof(argv[++i]);
        else if (!strcmp(argv[i], "-dpbytes")   && i+1 < argc) spec.dp_bytes = strtoull(argv[++i],0,10);
        else if (!strcmp(argv[i], "-tpbytes")   && i+1 < argc) spec.tp_bytes = strtoull(argv[++i],0,10);
        else if (!strcmp(argv[i], "-epbytes")   && i+1 < argc) spec.ep_bytes = strtoull(argv[++i],0,10);
        else if (!strcmp(argv[i], "-ppbytes")   && i+1 < argc) spec.pp_bytes = strtoull(argv[++i],0,10);
        else if (!strcmp(argv[i], "-placement") && i+1 < argc) placement_policy = argv[++i];
        else if (!strcmp(argv[i], "-placementseed") && i+1 < argc) g_placement_seed = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-blocksize") && i+1 < argc) block_size = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-jobmix")    && i+1 < argc) jobmix_s = argv[++i];
        else if (!strcmp(argv[i], "-speed")     && i+1 < argc) SPEED = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-ssthresh")  && i+1 < argc) ssthresh = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-q")         && i+1 < argc) queuesize = memFromPkt(atoi(argv[++i]));
        else if (!strcmp(argv[i], "-simtime")   && i+1 < argc) simtime = atof(argv[++i]);
        else if (!strcmp(argv[i], "-ofile")     && i+1 < argc) ofile = argv[++i];
        else { std::cerr << "Unknown arg: " << argv[i] << "\n"; exit_error(argv[0]); }
        i++;
    }

    if (njobs < 1) { std::cerr << "ERROR: -jobs must be at least 1\n"; exit(1); }
    if (f1b_mb > 0 && phased_mb > 0) {
        std::cerr << "ERROR: -iter1f1b and -phased are two different iteration models; pick one\n";
        exit(1);
    }
    if (f1b_mb > 0 && (f1b_layers < 1 || f1b_buckets < 1)) {
        std::cerr << "ERROR: -layers and -dpbuckets must be at least 1\n"; exit(1);
    }
    if (spec.allreduce != "rhd" && spec.allreduce != "ring" && spec.allreduce != "auto") {
        std::cerr << "ERROR: -allreduce must be rhd, ring or auto, not '"
                  << spec.allreduce << "'\n";
        exit(1);
    }
    std::vector<MDPSpec> specs;
    if (!jobmix_s.empty()) {
        for (const std::string& tok : split(jobmix_s, ',')) {
            MDPSpec js = spec;
            if (sscanf(tok.c_str(), "%dx%dx%dx%d", &js.tp, &js.dp, &js.pp, &js.ep) != 4 ||
                js.tp < 1 || js.dp < 1 || js.pp < 1 || js.ep < 1) {
                std::cerr << "ERROR: -jobmix entry '" << tok << "' is not TPxDPxPPxEP\n";
                exit(1);
            }
            js.nnodes = js.tp * js.dp * js.pp * js.ep;
            specs.push_back(js);
        }
        njobs = (int)specs.size();
        spec = specs[0];
    }
    if (!spec.valid()) {
        std::cerr << "ERROR: jobdp*jobtp*jobpp*jobep ("
                  << spec.dp*spec.tp*spec.pp*spec.ep
                  << ") must equal -jobnodes (" << spec.nnodes << ")\n";
        exit(1);
    }
    if (specs.empty()) specs.assign(njobs, spec);
    int total_nodes = 0;
    for (const MDPSpec& js : specs) total_nodes += js.nnodes;
    if (total_nodes > nodes) {
        std::cerr << "ERROR: " << njobs << " jobs need " << total_nodes
                  << " nodes, which exceeds the cluster's " << nodes << " nodes\n";
        exit(1);
    }
    if (placement_policy != "compact" && placement_policy != "scatter" &&
        placement_policy != "block") {
        std::cerr << "ERROR: -placement must be compact, scatter or block, got '"
                  << placement_policy << "'\n";
        exit(1);
    }
    if (placement_policy == "block") {
        if (block_size < 1 || nodes % block_size != 0) {
            std::cerr << "ERROR: -placement block needs -blocksize dividing -nodes\n";
            exit(1);
        }
        for (const MDPSpec& js : specs)
            if (js.nnodes % block_size != 0) {
                std::cerr << "ERROR: a job of " << js.nnodes << " nodes is not a whole "
                          << "number of " << block_size << "-node blocks\n";
                exit(1);
            }
    }

    std::vector<std::string> dims = split(dims_s, ',');
    if (dims.empty()) { std::cerr << "ERROR: -dims is empty\n"; exit(1); }
    for (const auto& d : dims)
        if (d != "tp" && d != "dp" && d != "ep" && d != "pp") {
            std::cerr << "ERROR: unknown dimension '" << d
                      << "' in -dims (expected a comma-separated subset of tp,dp,ep,pp)\n";
            exit(1);
        }

    std::cerr << "=== Multi-tenant multi-dimensional parallelism ===\n"
              << "  Topology     : " << topo << " with " << nodes << " nodes\n"
              << "  Jobs         : " << njobs << " x " << spec.nnodes << " nodes"
              << " (" << njobs * spec.nnodes << " of " << nodes << " used)\n"
              << "  Per-job decomp: DP=" << spec.dp << " TP=" << spec.tp
              << " PP=" << spec.pp << " EP=" << spec.ep << "\n"
              << (jobmix_s.empty() ? std::string() : "  Job mix      : " + jobmix_s + " (TPxDPxPPxEP per job)\n")
              << "  AllReduce    : " << spec.allreduce << "\n"
              << "  Iteration    : " << (phased_mb > 0
                    ? "phased, " + std::to_string(phased_mb) + " microbatches, compute "
                      + std::to_string(fwd_us) + "/" + std::to_string(bwd_us) + " us fwd/bwd"
                    : std::string("concurrent (every collective at t = 0)")) << "\n"
              << "  Dimensions   : " << dims_s << "\n"
              << "  Placement    : " << placement_policy
              << " (seed " << g_placement_seed << ")\n"
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

    if (pfc_buf < 0) pfc_buf = pfc_xoff + 5;
    LosslessInputQueue::configure(pfc_xoff, pfc_xon, pfc_buf);
    if (pfc_classes < 1 || pfc_classes > 16) {
      std::cerr << "ERROR: -pfcclasses must be 1..16 (the class lives in 4 bits)\n";
      exit(1);
    }
    LosslessInputQueue::cfg_classes = pfc_classes;

    LosslessInputQueue::cfg_pause_delay = (simtime_picosec)pfc_delay_ns * 1000ULL;

    Topology* top;
    DynFlatScheduler* ocs_sch = nullptr;
    if (topo == "ocs") {
        dyn_sch_set_seed(g_placement_seed ? g_placement_seed : 1);
        FlatTopology* flat = new FlatTopology(nodes, queuesize, nullptr, &eventlist, ff, qt);
        DemandRecorder* dr = new DemandRecorder(ocs_degree, &tcpRtxScanner);
        DynFlatScheduler::RoutingMode rm = (routing == "shortest")
            ? DynFlatScheduler::ROUTING_SHORTEST : DynFlatScheduler::ROUTING_UPDOWN;
        simtime_picosec delay_ps = (rdelay_ns >= 0) ? (simtime_picosec)rdelay_ns * 1000ULL
                                                    : 10000000ULL * (uint64_t)ocs_rdelay;
        simtime_picosec period_ps = (simtime_picosec)rperiod_ns * 1000ULL;
        std::cout << "reconf_delay_ps " << delay_ps << std::endl;
        std::cout << "reconf_period_ps " << (period_ps ? period_ps : 4 * delay_ps) << std::endl;
        ocs_sch = new DynFlatScheduler(nodes, ocs_degree, flat,
                             DynFlatScheduler::D_HEURISTIC, dr,
                             delay_ps, eventlist, rm, ocs_logical, period_ps);
        top = flat;
    }
    else if (topo == "fattree128")   top = new FatTreeTopology(nodes, queuesize, nullptr, &eventlist, ff, qt);
    else if (topo == "zcubepartial") top = new ZCubePartialTopology(nodes, queuesize, nullptr, &eventlist, ff, qt);
    else if (topo == "jellyfish")    top = new GraphTopology(make_jellyfish(nodes, 12, 4, 13), queuesize, nullptr, &eventlist, ff, qt, 8, routing);
    else if (topo == "flatbutterfly")top = new GraphTopology(make_flattened_butterfly(nodes, 4, 2, 8), queuesize, nullptr, &eventlist, ff, qt, 8, routing);
    else if (topo == "dragonfly")    top = new GraphTopology(make_dragonfly(nodes, 8, 4, 4), queuesize, nullptr, &eventlist, ff, qt, 8, routing);
    else if (topo == "megafly")      top = new GraphTopology(make_megafly(nodes, 4, 4, 2, 8), queuesize, nullptr, &eventlist, ff, qt, 8, routing);
    else { std::cerr << "Unknown -topo " << topo << "\n"; exit(1); }

    std::unordered_set<int> candidates;
    for (int n = 0; n < nodes; n++) candidates.insert(n);

    std::vector<std::vector<int>> placements;
    int compact_base = 0;
    for (int j = 0; j < njobs; j++) {
        std::vector<int> p;
        const int S = specs[j].nnodes;
        if (placement_policy == "compact") {
            p.resize(S);
            for (int r = 0; r < S; r++) p[r] = compact_base + r;
            for (int n : p) candidates.erase(n);
            compact_base += S;
        } else if (placement_policy == "block") {
            p = block_placement(candidates, nodes, block_size, S);
        } else {
            p = scatter_placement(candidates, S);
        }
        placements.push_back(p);
    }

    {
        std::set<int> seen;
        for (int j = 0; j < njobs; j++)
            for (int n : placements[j])
                if (!seen.insert(n).second) {
                    std::cerr << "ERROR: node " << n << " assigned to more than one job\n";
                    exit(1);
                }
    }

    std::vector<FFApplication*> apps;
    std::vector<std::vector<FFTask*>> job_tasks(njobs);
    std::vector<std::vector<std::string>> job_labels(njobs);

    for (int j = 0; j < njobs; j++) {
        FFApplication* app = new FFApplication(top, ssthresh, &fct_util_out,
                                               placements[j], tcpRtxScanner, eventlist,
                                               FF_DEFAULT_AR);
        app->ngpupernode = 1;
        app->nnodes      = specs[j].nnodes;
        app->nswitches   = 0;
        apps.push_back(app);

        std::cerr << "[JOB " << j << "] ";
        if (f1b_mb > 0)
            mdp_build_1f1b_tasks(app, top, specs[j], dims, f1b_mb, f1b_layers, f1b_buckets,
                                 timeFromUs(layer_fwd_us), timeFromUs(layer_bwd_us),
                                 job_tasks[j], job_labels[j], placements[j]);
        else if (phased_mb > 0)
            mdp_build_phased_tasks(app, top, specs[j], dims, phased_mb,
                                   timeFromUs(fwd_us), timeFromUs(bwd_us),
                                   job_tasks[j], job_labels[j], placements[j]);
        else
            mdp_build_tasks(app, top, specs[j], dims, job_tasks[j], job_labels[j], placements[j]);
    }

    while (eventlist.doNextEvent()) {}

    fct_util_out << "Jobs " << njobs << "\n"
                 << "JobNodes " << spec.nnodes << "\n"
                 << "Placement " << placement_policy << "\n"
                 << (placement_policy == "block" ? "BlockSize " + std::to_string(block_size) + "\n" : std::string())
                 << "Phased " << phased_mb << "\n"
                 << "Iter1F1B " << f1b_mb << " " << f1b_layers << " " << f1b_buckets << "\n"
                 << "FwdComputeUs " << fwd_us << "\n"
                 << "BwdComputeUs " << bwd_us << "\n"
                 << "PlacementSeed " << g_placement_seed << "\n";

    simtime_picosec global_makespan = 0;
    int global_incomplete = 0;

    for (int j = 0; j < njobs; j++) {
        std::map<std::string, simtime_picosec> class_finish;
        std::map<std::string, int> class_count, class_incomplete;
        simtime_picosec makespan = 0;
        int incomplete = 0;

        for (size_t k = 0; k < job_tasks[j].size(); k++) {
            const std::string& L = job_labels[j][k];
            class_count[L]++;
            simtime_picosec f = job_tasks[j][k]->finish_time;
            if (job_tasks[j][k]->state != FFTask::TASK_FINISHED) {
                incomplete++; class_incomplete[L]++; continue;
            }
            if (f > class_finish[L]) class_finish[L] = f;
            if (f > makespan) makespan = f;
        }

        std::ostringstream nodes_s;
        for (size_t r = 0; r < placements[j].size(); r++)
            nodes_s << (r ? "," : "") << placements[j][r];
        fct_util_out << "Job" << j << "_Nodes " << nodes_s.str() << "\n";
        if (!jobmix_s.empty())
            fct_util_out << "Job" << j << "_Decomp tp" << specs[j].tp << ".dp" << specs[j].dp
                         << ".pp" << specs[j].pp << ".ep" << specs[j].ep << "\n";

        for (const auto& kv : class_count) {
            fct_util_out << "Job" << j << "_Class_" << kv.first << "_ms "
                         << timeAsMs(class_finish[kv.first]) << "\n";
            fct_util_out << "Job" << j << "_Tasks_" << kv.first << " " << kv.second << "\n";
            fct_util_out << "Job" << j << "_Incomplete_" << kv.first << " "
                         << class_incomplete[kv.first] << "\n";
        }
        fct_util_out << "Job" << j << "_Makespan_ms " << timeAsMs(makespan) << "\n";
        fct_util_out << "Job" << j << "_Incomplete " << incomplete << "\n";

        if (makespan > global_makespan) global_makespan = makespan;
        global_incomplete += incomplete;

        std::cerr << "\n=== Job " << j << " ===\n";
        for (const auto& kv : class_count)
            std::cerr << "  " << kv.first << ": " << timeAsMs(class_finish[kv.first])
                      << " ms over " << kv.second << " groups ("
                      << class_incomplete[kv.first] << " incomplete)\n";
        std::cerr << "  makespan: " << timeAsMs(makespan) << " ms, "
                  << incomplete << " incomplete\n";
    }

    fct_util_out << "Makespan_ms " << timeAsMs(global_makespan) << "\n";
    fct_util_out << "Incomplete " << global_incomplete << "\n";
    LosslessInputQueue::print_stats(fct_util_out);
    LosslessOutputQueue::print_stats(fct_util_out);
    if (ocs_sch) { ocs_sch->print_stats(fct_util_out); ocs_sch->print_stats(std::cout); }

    std::cerr << "\n=== All jobs ===\n"
              << "  makespan: " << timeAsMs(global_makespan) << " ms, "
              << global_incomplete << " incomplete tasks\n";
    return 0;
}

string ntoa(double n) { stringstream s; s << n; return s.str(); }
string itoa(uint64_t n) { stringstream s; s << n; return s.str(); }
