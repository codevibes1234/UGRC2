#include <algorithm>
#include "ffapp_mdp.h"
#include "ecmp_hash.h"
#include "tcp.h"
#ifdef USE_DCTCP
#include "dctcp.h"
#else
#include "rocev2.h"
#endif

#include <cassert>
#include <iostream>

MDPGroups mdp_groups(const MDPSpec& s)
{
    MDPGroups g;
    auto rank = [&](int d, int p, int e, int t) {
        return (uint64_t)(((d * s.pp + p) * s.ep + e) * s.tp + t);
    };

    for (int d = 0; d < s.dp; d++)
        for (int p = 0; p < s.pp; p++)
            for (int e = 0; e < s.ep; e++) {
                std::vector<uint64_t> grp;
                for (int t = 0; t < s.tp; t++) grp.push_back(rank(d, p, e, t));
                g.tp_groups.push_back(grp);
            }

    for (int d = 0; d < s.dp; d++)
        for (int p = 0; p < s.pp; p++)
            for (int t = 0; t < s.tp; t++) {
                std::vector<uint64_t> grp;
                for (int e = 0; e < s.ep; e++) grp.push_back(rank(d, p, e, t));
                g.ep_groups.push_back(grp);
            }

    for (int p = 0; p < s.pp; p++)
        for (int e = 0; e < s.ep; e++)
            for (int t = 0; t < s.tp; t++) {
                std::vector<uint64_t> grp;
                for (int d = 0; d < s.dp; d++) grp.push_back(rank(d, p, e, t));
                g.dp_groups.push_back(grp);
            }

    for (int d = 0; d < s.dp; d++)
        for (int e = 0; e < s.ep; e++)
            for (int t = 0; t < s.tp; t++) {
                std::vector<uint64_t> chain;
                for (int p = 0; p < s.pp; p++) chain.push_back(rank(d, p, e, t));
                g.pp_chains.push_back(chain);
            }

    return g;
}

void p2p_flow_finished(void* task_ptr)
{
    static_cast<FFPointToPointTask*>(task_ptr)->flow_done();
}

FFPointToPointTask::FFPointToPointTask(FFApplication* ffapp, Topology* top,
                                       std::vector<Transfer> t, const std::string& lbl)
    : FFTask(ffapp, FFTask::TASK_COMM),
      _topology(top), transfers(std::move(t)),
      flows_total(0), flows_finished(0), label(lbl)
{
    xfersize = 0;
    for (const auto& tr : transfers) xfersize += tr.bytes;
}

void FFPointToPointTask::doNextEvent()
{
    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;

    if (transfers.empty()) {
        finish_time = eventlist().now();
        cleanup();
        return;
    }

    flows_total    = (int)transfers.size();
    flows_finished = 0;
    for (const auto& tr : transfers) start_one_flow(tr, start_time);
}

void FFPointToPointTask::start_one_flow(const Transfer& tr, simtime_picosec t0)
{
    auto* paths_out = _topology->get_paths(tr.src, tr.dst);
    auto* paths_in  = _topology->get_paths(tr.dst, tr.src);
    Route* routeout = new Route(*paths_out->at(ecmp_path_index(tr.src, tr.dst, paths_out->size())));
    Route* routein  = new Route(*paths_in->at(ecmp_path_index(tr.dst, tr.src, paths_in->size())));
    delete paths_out;
    delete paths_in;

#ifdef USE_DCTCP
    DCTCPSrc* fs = new DCTCPSrc(nullptr, nullptr, ffapp->fstream_out,
                                eventlist(), tr.src, tr.dst,
                                p2p_flow_finished, static_cast<void*>(this));
    TcpSink* snk = new TcpSink();
    fs->set_flowsize(tr.bytes);
    fs->set_ssthresh(ffapp->ssthresh * Packet::data_packet_size());
    fs->_rto = timeFromMs(10);
    ffapp->tcpRtxScanner.registerTcp(*fs);
#else
    RoCEv2Src* fs = new RoCEv2Src(nullptr, eventlist(), tr.src, tr.dst,
                                  p2p_flow_finished, static_cast<void*>(this));
    RoCEv2Sink* snk = new RoCEv2Sink(nullptr, eventlist());
    fs->set_flowsize(tr.bytes);
    fs->set_initial_credits(RoCEv2Src::default_initial_credits);
#endif

    routeout->push_back(snk);
    routein->push_back(fs);
    fs->connect(*routeout, *routein, *snk, t0);
}

void FFPointToPointTask::flow_done()
{
    if (++flows_finished == flows_total) {
        finish_time = eventlist().now();
        cleanup();
    }
}

static void place_groups(MDPGroups& g, const MDPSpec& spec,
                         const std::vector<int>& placement)
{
    if (placement.empty()) return;

    if ((int)placement.size() != spec.nnodes) {
        std::cerr << "ERROR: placement has " << placement.size()
                  << " entries but the decomposition needs " << spec.nnodes
                  << " (one per rank)\n";
        exit(1);
    }

    auto remap = [&](std::vector<std::vector<uint64_t>>& groups) {
        for (auto& grp : groups)
            for (auto& r : grp) {
                assert(r < placement.size());
                r = (uint64_t)placement[r];
            }
    };
    remap(g.tp_groups);
    remap(g.ep_groups);
    remap(g.dp_groups);
    remap(g.pp_chains);
}

static void register_task(FFApplication* app, FFTask* t,
                          std::vector<FFTask*>& out, std::vector<std::string>& labels,
                          const std::string& label)
{
    uint64_t id = (uint64_t)(void*)t;
    app->tasks[id]  = t;
    t->counter      = 0;
    t->state        = FFTask::TASK_READY;
    t->ready_time   = t->eventlist().now();
    t->eventlist().sourceIsPending(*t, t->ready_time);
    out.push_back(t);
    labels.push_back(label);
}

static FFTask* make_allreduce(FFApplication* app, Topology* top, const MDPSpec& spec,
                              const std::vector<uint64_t>& grp, uint64_t bytes)
{
    const size_t n = grp.size();
    const bool pow2 = (n & (n - 1)) == 0;
    bool ring = spec.allreduce == "ring" || (spec.allreduce == "auto" && !pow2);
    if (!ring && !pow2) {
        std::cerr << "ERROR: a group of " << n << " ranks is not a power of two, so "
                     "recursive halving-doubling cannot run it. Pass -allreduce ring "
                     "or -allreduce auto.\n";
        exit(1);
    }
    if (ring) return new FFGroupRingAllreduceTask(app, top, grp, bytes);
    return new FFZCubeRHDAllreduceTask(app, top, grp, bytes, 0.0);
}

void mdp_build_tasks(FFApplication* app, Topology* top, const MDPSpec& spec,
                     const std::vector<std::string>& dims,
                     std::vector<FFTask*>& out,
                     std::vector<std::string>& labels,
                     const std::vector<int>& placement)
{
    MDPGroups g = mdp_groups(spec);
    place_groups(g, spec, placement);
    auto wants = [&](const std::string& d) {
        for (const auto& x : dims) if (x == d) return true;
        return false;
    };

    if (wants("tp") && spec.tp > 1)
        for (auto& grp : g.tp_groups)
            register_task(app, make_allreduce(app, top, spec, grp, spec.tp_bytes),
                          out, labels, "tp");

    if (wants("dp") && spec.dp > 1)
        for (auto& grp : g.dp_groups)
            register_task(app, make_allreduce(app, top, spec, grp, spec.dp_bytes),
                          out, labels, "dp");

    if (wants("ep") && spec.ep > 1)
        for (auto& grp : g.ep_groups)
            register_task(app, new FFAllToAllTask(app, top, grp, spec.ep_bytes),
                          out, labels, "ep");

    if (wants("pp") && spec.pp > 1)
        for (auto& chain : g.pp_chains) {
            std::vector<FFPointToPointTask::Transfer> tr;
            for (size_t i = 0; i + 1 < chain.size(); i++)
                tr.push_back({(int)chain[i], (int)chain[i + 1], spec.pp_bytes});
            register_task(app, new FFPointToPointTask(app, top, tr, "pp"),
                          out, labels, "pp");
        }

    std::cerr << "[MDP] built " << out.size() << " tasks for dims:";
    for (const auto& d : dims) std::cerr << " " << d;
    std::cerr << std::endl;

    if (!placement.empty()) {
        std::cerr << "[MDP] placement rank->node:";
        for (size_t r = 0; r < placement.size(); r++) std::cerr << " " << placement[r];
        std::cerr << std::endl;
    }
}

void FFDelayTask::doNextEvent()
{
    if (!waiting) {
        start_time = ready_time;
        state      = FFTask::TASK_RUNNING;
        if (delay > 0) {
            waiting = true;
            eventlist().sourceIsPendingRel(*this, delay);
            return;
        }
    }
    finish_time = eventlist().now();
    cleanup();
}

static void register_dependent(FFApplication* app, FFTask* t,
                               std::vector<FFTask*>& out, std::vector<std::string>& labels,
                               const std::string& label)
{
    uint64_t id = (uint64_t)(void*)t;
    app->tasks[id] = t;
    t->counter     = 0;
    t->state       = FFTask::TASK_NOT_READY;
    out.push_back(t);
    labels.push_back(label);
}

static void depends_on(FFTask* succ, FFTask* pred)
{
    pred->next_tasks.push_back((uint64_t)(void*)succ);
    succ->counter++;
}

void mdp_build_phased_tasks(FFApplication* app, Topology* top, const MDPSpec& spec,
                            const std::vector<std::string>& dims,
                            int M, simtime_picosec fwd_compute, simtime_picosec bwd_compute,
                            std::vector<FFTask*>& out,
                            std::vector<std::string>& labels,
                            const std::vector<int>& placement)
{
    assert(M >= 1);
    const size_t first = out.size();
    MDPGroups g = mdp_groups(spec);
    std::vector<int> tp_stage, ep_stage, dp_stage;
    for (size_t i = 0; i < g.tp_groups.size(); i++) tp_stage.push_back((int)(i / spec.ep) % spec.pp);
    for (size_t i = 0; i < g.ep_groups.size(); i++) ep_stage.push_back((int)(i / spec.tp) % spec.pp);
    for (size_t i = 0; i < g.dp_groups.size(); i++) dp_stage.push_back((int)(i / (spec.ep * spec.tp)));
    place_groups(g, spec, placement);

    auto wants = [&](const std::string& d) {
        for (const auto& x : dims) if (x == d) return true;
        return false;
    };
    const int P = spec.pp;
    auto per_pass = [&](uint64_t b) { uint64_t x = b / (uint64_t)(2 * M); return x ? x : 1ULL; };

    struct Block { FFTask* j0; FFTask* j2; FFTask* pp; };
    auto make_block = [&](int s, bool fwd) {
        Block b;
        b.j0 = new FFDelayTask(app, fwd ? fwd_compute : bwd_compute);
        register_dependent(app, b.j0, out, labels, "compute");
        std::vector<FFTask*> tps, eps;
        if (wants("tp") && spec.tp > 1)
            for (size_t i = 0; i < g.tp_groups.size(); i++)
                if (tp_stage[i] == s) {
                    FFTask* t = make_allreduce(app, top, spec, g.tp_groups[i], per_pass(spec.tp_bytes));
                    register_dependent(app, t, out, labels, "tp");
                    depends_on(t, b.j0);
                    tps.push_back(t);
                }
        FFTask* j1 = new FFDelayTask(app, 0);
        register_dependent(app, j1, out, labels, "sync");
        if (tps.empty()) depends_on(j1, b.j0);
        for (FFTask* t : tps) depends_on(j1, t);
        if (wants("ep") && spec.ep > 1)
            for (size_t i = 0; i < g.ep_groups.size(); i++)
                if (ep_stage[i] == s) {
                    FFTask* t = new FFAllToAllTask(app, top, g.ep_groups[i], per_pass(spec.ep_bytes));
                    register_dependent(app, t, out, labels, "ep");
                    depends_on(t, j1);
                    eps.push_back(t);
                }
        b.j2 = new FFDelayTask(app, 0);
        register_dependent(app, b.j2, out, labels, "sync");
        if (eps.empty()) depends_on(b.j2, j1);
        for (FFTask* t : eps) depends_on(b.j2, t);
        b.pp = nullptr;
        int ns = fwd ? s + 1 : s - 1;
        if (wants("pp") && P > 1 && ns >= 0 && ns < P) {
            std::vector<FFPointToPointTask::Transfer> tr;
            for (const auto& chain : g.pp_chains)
                tr.push_back({(int)chain[s], (int)chain[ns], per_pass(spec.pp_bytes)});
            b.pp = new FFPointToPointTask(app, top, tr, "pp");
            register_dependent(app, b.pp, out, labels, "pp");
            depends_on(b.pp, b.j2);
        }
        return b;
    };
    auto handoff = [](const Block& b) { return b.pp ? b.pp : b.j2; };

    std::vector<std::vector<Block>> F(P, std::vector<Block>(M)), B(P, std::vector<Block>(M));
    for (int m = 0; m < M; m++)
        for (int s = 0; s < P; s++) {
            F[s][m] = make_block(s, true);
            if (s > 0) depends_on(F[s][m].j0, handoff(F[s - 1][m]));
            if (m > 0) depends_on(F[s][m].j0, F[s][m - 1].j2);
        }
    for (int m = 0; m < M; m++)
        for (int s = P - 1; s >= 0; s--) {
            B[s][m] = make_block(s, false);
            if (s < P - 1)      depends_on(B[s][m].j0, handoff(B[s + 1][m]));
            else if (m == 0)    depends_on(B[s][m].j0, F[P - 1][M - 1].j2);
            if (m > 0)          depends_on(B[s][m].j0, B[s][m - 1].j2);
        }
    if (wants("dp") && spec.dp > 1)
        for (size_t i = 0; i < g.dp_groups.size(); i++) {
            FFTask* t = make_allreduce(app, top, spec, g.dp_groups[i], spec.dp_bytes);
            register_dependent(app, t, out, labels, "dp");
            depends_on(t, B[dp_stage[i]][M - 1].j2);
        }

    int roots = 0;
    for (size_t k = first; k < out.size(); k++)
        if (out[k]->counter == 0) {
            out[k]->state      = FFTask::TASK_READY;
            out[k]->ready_time = out[k]->eventlist().now();
            out[k]->eventlist().sourceIsPending(*out[k], out[k]->ready_time);
            roots++;
        }
    std::cerr << "[MDP] phased iteration: " << (out.size() - first) << " tasks, "
              << M << " microbatches, " << roots << " root(s)" << std::endl;
}

void mdp_build_1f1b_tasks(FFApplication* app, Topology* top, const MDPSpec& spec,
                          const std::vector<std::string>& dims,
                          int M, int L, int K,
                          simtime_picosec fwd_c, simtime_picosec bwd_c,
                          std::vector<FFTask*>& out,
                          std::vector<std::string>& labels,
                          const std::vector<int>& placement)
{
    assert(M >= 1 && L >= 1 && K >= 1);
    const size_t first = out.size();
    MDPGroups g = mdp_groups(spec);
    std::vector<int> tp_stage, ep_stage, dp_stage;
    for (size_t i = 0; i < g.tp_groups.size(); i++) tp_stage.push_back((int)(i / spec.ep) % spec.pp);
    for (size_t i = 0; i < g.ep_groups.size(); i++) ep_stage.push_back((int)(i / spec.tp) % spec.pp);
    for (size_t i = 0; i < g.dp_groups.size(); i++) dp_stage.push_back((int)(i / (spec.ep * spec.tp)));
    place_groups(g, spec, placement);

    auto wants = [&](const std::string& d) {
        for (const auto& x : dims) if (x == d) return true;
        return false;
    };
    const int P = spec.pp;
    auto share = [](uint64_t b, uint64_t n) { uint64_t x = b / n; return x ? x : 1ULL; };
    const uint64_t tp_each = share(spec.tp_bytes, 4ULL * M * L);
    const uint64_t ep_each = share(spec.ep_bytes, 4ULL * M * L);
    const uint64_t pp_each = share(spec.pp_bytes, 2ULL * M);
    const uint64_t dp_each = share(spec.dp_bytes, (uint64_t)K);

    auto join = [&]() {
        FFTask* j = new FFDelayTask(app, 0);
        register_dependent(app, j, out, labels, "sync");
        return j;
    };
    auto compute = [&](simtime_picosec d, FFTask* after) {
        FFTask* c = new FFDelayTask(app, d);
        register_dependent(app, c, out, labels, "compute");
        depends_on(c, after);
        return c;
    };
    auto tp_all = [&](int s, FFTask* after) -> FFTask* {
        if (!(wants("tp") && spec.tp > 1)) return after;
        FFTask* j = join();
        for (size_t i = 0; i < g.tp_groups.size(); i++)
            if (tp_stage[i] == s) {
                FFTask* t = make_allreduce(app, top, spec, g.tp_groups[i], tp_each);
                register_dependent(app, t, out, labels, "tp");
                depends_on(t, after);
                depends_on(j, t);
            }
        return j;
    };
    auto ep_all = [&](int s, FFTask* after) -> FFTask* {
        if (!(wants("ep") && spec.ep > 1)) return after;
        FFTask* j = join();
        for (size_t i = 0; i < g.ep_groups.size(); i++)
            if (ep_stage[i] == s) {
                FFTask* t = new FFAllToAllTask(app, top, g.ep_groups[i], ep_each);
                register_dependent(app, t, out, labels, "ep");
                depends_on(t, after);
                depends_on(j, t);
            }
        return j;
    };

    struct Pass { FFTask* start; FFTask* end; FFTask* pp; std::vector<FFTask*> layer_end; };
    auto make_pass = [&](int s, bool fwd) {
        Pass p;
        p.start = join();
        FFTask* cur = p.start;
        const simtime_picosec half = (fwd ? fwd_c : bwd_c) / 2;
        for (int l = 0; l < L; l++) {
            if (fwd) {
                cur = compute(half, cur);
                cur = tp_all(s, cur);
                cur = ep_all(s, cur);
                cur = compute(half, cur);
                cur = ep_all(s, cur);
                cur = tp_all(s, cur);
            } else {
                cur = ep_all(s, cur);
                cur = compute(half, cur);
                cur = ep_all(s, cur);
                cur = tp_all(s, cur);
                cur = compute(half, cur);
                cur = tp_all(s, cur);
            }
            p.layer_end.push_back(cur);
        }
        p.end = cur;
        p.pp = nullptr;
        const int ns = fwd ? s + 1 : s - 1;
        if (wants("pp") && P > 1 && ns >= 0 && ns < P) {
            std::vector<FFPointToPointTask::Transfer> tr;
            for (const auto& chain : g.pp_chains)
                tr.push_back({(int)chain[s], (int)chain[ns], pp_each});
            p.pp = new FFPointToPointTask(app, top, tr, "pp");
            register_dependent(app, p.pp, out, labels, "pp");
            depends_on(p.pp, p.end);
        }
        return p;
    };
    auto handoff = [](const Pass& p) { return p.pp ? p.pp : p.end; };

    std::vector<std::vector<Pass>> F(P), B(P);
    for (int s = 0; s < P; s++)
        for (int m = 0; m < M; m++) {
            F[s].push_back(make_pass(s, true));
            B[s].push_back(make_pass(s, false));
        }
    for (int s = 0; s < P; s++)
        for (int m = 0; m < M; m++) {
            if (s > 0)     depends_on(F[s][m].start, handoff(F[s - 1][m]));
            if (s < P - 1) depends_on(B[s][m].start, handoff(B[s + 1][m]));
            else           depends_on(B[s][m].start, F[s][m].end);
        }
    for (int s = 0; s < P; s++) {
        const int w = std::min(P - s - 1, M);
        std::vector<Pass*> seq;
        for (int m = 0; m < w; m++) seq.push_back(&F[s][m]);
        for (int i = 0; i < M - w; i++) { seq.push_back(&F[s][w + i]); seq.push_back(&B[s][i]); }
        for (int m = M - w; m < M; m++) seq.push_back(&B[s][m]);
        assert((int)seq.size() == 2 * M);
        for (size_t i = 1; i < seq.size(); i++) depends_on(seq[i]->start, seq[i - 1]->end);
    }
    if (wants("dp") && spec.dp > 1)
        for (size_t i = 0; i < g.dp_groups.size(); i++) {
            const Pass& last = B[dp_stage[i]][M - 1];
            for (int b = 0; b < K; b++) {
                int upto = std::min(L, (int)(((long)(b + 1) * L + K - 1) / K)) - 1;
                FFTask* t = make_allreduce(app, top, spec, g.dp_groups[i], dp_each);
                register_dependent(app, t, out, labels, "dp");
                depends_on(t, last.layer_end[upto]);
            }
        }

    int roots = 0;
    for (size_t k = first; k < out.size(); k++)
        if (out[k]->counter == 0) {
            out[k]->state      = FFTask::TASK_READY;
            out[k]->ready_time = out[k]->eventlist().now();
            out[k]->eventlist().sourceIsPending(*out[k], out[k]->ready_time);
            roots++;
        }
    std::cerr << "[MDP] 1F1B iteration: " << (out.size() - first) << " tasks, "
              << M << " microbatches, " << L << " layers per stage, " << K
              << " DP buckets, " << roots << " root(s)" << std::endl;
}
