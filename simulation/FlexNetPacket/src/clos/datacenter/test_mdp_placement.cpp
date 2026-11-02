#include "config.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <numeric>
#include <set>
#include <string>
#include <vector>
#include "eventlist.h"
#include "logfile.h"
#include "clock.h"
#include "tcp.h"
#include "firstfit.h"
#include "topology.h"
#include "fat_tree_topology.h"
#include "ffapp_mdp.h"

uint32_t RTT_rack = 500, RTT_net = 500, RTT = 500, SPEED = 200000;
std::ofstream fct_util_out;
FirstFit* ff = nullptr;
EventList eventlist;
Logfile* lg;
std::string ntoa(double n) { std::stringstream s; s << n; return s.str(); }
std::string itoa(uint64_t n) { std::stringstream s; s << n; return s.str(); }

static int failures = 0;
static void check(bool ok, const std::string& what)
{
    std::cout << (ok ? "  PASS  " : "  FAIL  ") << what << "\n";
    if (!ok) failures++;
}

static std::vector<std::vector<uint64_t>>
node_ids_per_task(Topology* top, TcpRtxTimerScanner& rtx, const MDPSpec& spec,
                  const std::vector<std::string>& dims,
                  const std::vector<int>& placement)
{
    FFApplication* app = new FFApplication(top, 15, &fct_util_out, rtx, eventlist,
                                           FF_DEFAULT_AR);
    app->ngpupernode = 1;
    app->nnodes      = spec.nnodes;
    app->nswitches   = 0;
    app->gpus.resize(spec.nnodes);
    std::iota(app->gpus.begin(), app->gpus.end(), 0);

    std::vector<FFTask*> tasks;
    std::vector<std::string> labels;
    mdp_build_tasks(app, top, spec, dims, tasks, labels, placement);

    std::vector<std::vector<uint64_t>> out;
    for (FFTask* t : tasks) {
        if (auto* a = dynamic_cast<FFAllToAllTask*>(t))               out.push_back(a->node_group);
        else if (auto* r = dynamic_cast<FFZCubeRHDAllreduceTask*>(t)) out.push_back(r->node_group);
        else if (auto* p = dynamic_cast<FFPointToPointTask*>(t)) {
            std::vector<uint64_t> v;
            for (const auto& tr : p->transfers) { v.push_back(tr.src); v.push_back(tr.dst); }
            out.push_back(v);
        } else {
            std::cerr << "unexpected task type\n"; exit(2);
        }
    }
    return out;
}

static std::set<uint64_t> flatten(const std::vector<std::vector<uint64_t>>& g)
{
    std::set<uint64_t> s;
    for (const auto& v : g) for (uint64_t x : v) s.insert(x);
    return s;
}

int main(int argc, char** argv)
{
    TcpPacket::set_packet_size(9000 - 64);
    eventlist.setEndtime(timeFromSec(1));
    Clock c(timeFromSec(0.05), eventlist);
    TcpRtxTimerScanner rtx(timeFromMs(1), eventlist);
    fct_util_out = std::ofstream("/dev/null");

    Topology* top = new FatTreeTopology(128, memFromPkt(200), nullptr, &eventlist,
                                        ff, LOSSLESS_INPUT);
    const std::vector<std::string> all4 = {"tp", "dp", "ep", "pp"};

    if (argc > 1 && std::string(argv[1]) == "badsize") {
        MDPSpec s; s.nnodes = 128; s.dp = 2; s.tp = 4; s.pp = 4; s.ep = 4;
        std::vector<int> wrong(64);
        std::iota(wrong.begin(), wrong.end(), 0);
        node_ids_per_task(top, rtx, s, all4, wrong);
        std::cerr << "ERROR: a 64-entry placement for 128 ranks was accepted\n";
        return 2;
    }

    MDPSpec full; full.nnodes = 128; full.dp = 2; full.tp = 4; full.pp = 4; full.ep = 4;

    std::vector<int> identity(128);
    std::iota(identity.begin(), identity.end(), 0);
    auto g_empty = node_ids_per_task(top, rtx, full, all4, std::vector<int>());
    auto g_ident = node_ids_per_task(top, rtx, full, all4, identity);
    check(g_empty == g_ident, "empty placement is identical to an explicit identity map");
    check(!g_empty.empty() && g_empty.size() == 160,
          "160 tasks built for dp=2,tp=4,pp=4,ep=4 (32 TP + 64 DP + 32 EP + 32 PP)");

    std::vector<int> mirror(128);
    for (int r = 0; r < 128; r++) mirror[r] = 127 - r;
    auto g_mirror = node_ids_per_task(top, rtx, full, all4, mirror);
    bool ok = (g_mirror.size() == g_ident.size());
    for (size_t k = 0; ok && k < g_mirror.size(); k++) {
        if (g_mirror[k].size() != g_ident[k].size()) { ok = false; break; }
        for (size_t j = 0; j < g_mirror[k].size(); j++)
            if (g_mirror[k][j] != 127 - g_ident[k][j]) { ok = false; break; }
    }
    check(ok, "a mirror permutation is applied to all four dimensions, order preserved");

    MDPSpec job; job.nnodes = 32; job.dp = 2; job.tp = 4; job.pp = 2; job.ep = 2;
    std::vector<int> at64(32);
    for (int r = 0; r < 32; r++) at64[r] = 64 + r;
    auto g_at64 = node_ids_per_task(top, rtx, job, all4, at64);
    auto s_at64 = flatten(g_at64);
    bool in_range = true;
    for (uint64_t n : s_at64) if (n < 64 || n >= 96) in_range = false;
    check(in_range, "a 32-rank job placed at offset 64 addresses only nodes 64..95");
    check(s_at64.size() == 32, "all 32 of its nodes are used, none duplicated away");
    check(g_at64.size() == 56, "56 tasks built for dp=2,tp=4,pp=2,ep=2");

    std::vector<int> at0(32);
    for (int r = 0; r < 32; r++) at0[r] = r;
    auto s_at0 = flatten(node_ids_per_task(top, rtx, job, all4, at0));
    std::vector<uint64_t> overlap;
    std::set_intersection(s_at0.begin(), s_at0.end(), s_at64.begin(), s_at64.end(),
                          std::back_inserter(overlap));
    check(overlap.empty(), "two 32-rank jobs at offsets 0 and 64 share no node");

    MDPSpec tponly = job;
    auto g_tp = node_ids_per_task(top, rtx, tponly, {"tp"}, at64);
    bool contiguous = !g_tp.empty();
    for (const auto& grp : g_tp) {
        if (grp.size() != 4) { contiguous = false; break; }
        for (size_t j = 1; j < grp.size(); j++)
            if (grp[j] != grp[j-1] + 1) { contiguous = false; break; }
    }
    check(contiguous, "under compact placement TP groups stay contiguous physical nodes");

    std::cout << (failures ? "FAILED" : "ALL PASSED") << " (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
