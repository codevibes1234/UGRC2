#include "ffapp_fctree.h"
#include "ecmp_hash.h"
#include "fc_schedule.h"
#include "tcp.h"
#ifdef USE_DCTCP
#include "dctcp.h"
#else
#include "rocev2.h"
#endif

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>


static bool xml_attr(const std::string& tag, const char* name, std::string& out)
{
    const std::string key = std::string(" ") + name + "=\"";
    size_t p = tag.find(key);
    if (p == std::string::npos) return false;
    p += key.size();
    size_t q = tag.find('"', p);
    if (q == std::string::npos) return false;
    out = tag.substr(p, q - p);
    return true;
}

static bool xml_int(const std::string& tag, const char* name, int& out)
{
    std::string v;
    if (!xml_attr(tag, name, v)) return false;
    out = std::atoi(v.c_str());
    return true;
}

bool FCTreeSchedule::load_xml(const std::string& path, int nhost_expected)
{
    std::ifstream in(path.c_str());
    if (!in) {
        std::cerr << "ERROR: cannot read ForestColl trees from " << path << "\n";
        return false;
    }
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string text = ss.str();

    nhost = nhost_expected;
    trees.clear();
    dropped_sends = 0;
    FCTree* cur = nullptr;

    size_t pos = 0;
    while ((pos = text.find('<', pos)) != std::string::npos) {
        size_t end = text.find('>', pos);
        if (end == std::string::npos) break;
        const std::string tag = text.substr(pos, end - pos + 1);
        pos = end + 1;
        if (tag.compare(0, 5, "<Algo") == 0) {
            xml_int(tag, "nchunkspernode", nchunkspernode);
        } else if (tag.compare(0, 5, "<tree") == 0) {
            trees.emplace_back();
            cur = &trees.back();
            if (!xml_int(tag, "root", cur->root)) {
                std::cerr << "ERROR: " << path << ": <tree> without root\n";
                return false;
            }
            xml_int(tag, "nchunks", cur->nchunks);
            xml_int(tag, "index", cur->index);
            cur->parent.assign(nhost, -2);
            if (cur->root < 0 || cur->root >= nhost) {
                std::cerr << "ERROR: " << path << ": tree root " << cur->root
                          << " is not a host of a " << nhost << "-host schedule\n";
                return false;
            }
            cur->parent[cur->root] = -1;
        } else if (tag.compare(0, 5, "<send") == 0) {
            int src = -1, dst = -1;
            if (cur == nullptr || !xml_int(tag, "src", src) || !xml_int(tag, "dst", dst)) {
                std::cerr << "ERROR: " << path << ": malformed <send>\n";
                return false;
            }
            if (src >= nhost || dst >= nhost) { dropped_sends++; continue; }
            if (cur->parent[dst] != -2) {
                std::cerr << "ERROR: " << path << ": host " << dst
                          << " reached twice in tree " << cur->index << "\n";
                return false;
            }
            cur->parent[dst] = src;
            cur->edges.emplace_back(src, dst);
            std::vector<int> sw;
            std::string pv;
            if (xml_attr(tag, "path", pv)) {
                std::vector<int> nodes;
                std::stringstream ps(pv);
                std::string tok;
                while (std::getline(ps, tok, ',')) if (!tok.empty()) nodes.push_back(std::atoi(tok.c_str()));
                bool ok = nodes.size() >= 3 && nodes.front() == src && nodes.back() == dst;
                for (size_t i = 1; ok && i + 1 < nodes.size(); i++) {
                    if (nodes[i] < nhost) ok = false;
                    else sw.push_back(nodes[i] - nhost);
                }
                if (!ok) sw.clear();
            }
            cur->paths.push_back(sw);
        } else if (tag.compare(0, 7, "</tree>") == 0) {
            cur = nullptr;
        }
    }

    if (trees.empty()) {
        std::cerr << "ERROR: " << path << ": no <tree> records\n";
        return false;
    }
    for (const FCTree& t : trees) {
        if ((int)t.edges.size() != nhost - 1) {
            std::cerr << "ERROR: " << path << ": tree " << t.index << " has "
                      << t.edges.size() << " host edges, a spanning tree of "
                      << nhost << " hosts needs " << nhost - 1 << "\n";
            return false;
        }
        for (int h = 0; h < nhost; h++) {
            int u = h, guard = 0;
            while (u != t.root && u >= 0 && guard++ <= nhost) u = t.parent[u];
            if (u != t.root) {
                std::cerr << "ERROR: " << path << ": host " << h
                          << " does not lead to the root of tree " << t.index << "\n";
                return false;
            }
        }
    }
    if (nchunkspernode > 0 && total_chunks() != nhost * nchunkspernode)
        std::cerr << "WARNING: " << path << ": trees carry " << total_chunks()
                  << " unit chunks, nchunkspernode x hosts = "
                  << nhost * nchunkspernode << "\n";
    return true;
}

int FCTreeSchedule::total_chunks() const
{
    int n = 0;
    for (const FCTree& t : trees) n += t.nchunks;
    return n;
}

void ar_finish_fc_tree(void* ctx_ptr)
{
    auto* c = static_cast<FFFCTreeAllreduceTask::FlowCtx*>(ctx_ptr);
    c->task->piece_done(c->hop, c->piece);
}

FFFCTreeAllreduceTask::FFFCTreeAllreduceTask(FFApplication* ffapp, Topology* top,
                                             FCTreeSchedule* sched, uint64_t sz)
    : FFTask(ffapp, FFTask::TASK_ALLREDUCE),
      _topology(top), _sched(sched), operator_size(sz > 0 ? sz : 1ULL)
{
    assert(top != nullptr && sched != nullptr && !sched->trees.empty());
    xfersize = sz;
    chunk_bytes = operator_size / (uint64_t)sched->total_chunks();
    if (chunk_bytes == 0) chunk_bytes = 1;
    null_out.open("/dev/null");
}

void FFFCTreeAllreduceTask::build()
{
    assert(pieces >= 1 && window >= 1);
    const int N = _sched->nhost;
    const int T = (int)_sched->trees.size();
    hops.clear();
    rs_hop_of.assign(T, std::vector<int>(N, -1));
    ag_hops_of.assign(T, std::vector<std::vector<int>>(N));
    nchild.assign(T, std::vector<int>(N, 0));
    rs_have.assign(T, std::vector<uint16_t>((size_t)N * pieces, 0));
    ag_have.assign(T, std::vector<uint8_t>((size_t)N * pieces, 0));
    tree_bytes.assign(T, 0);

    for (int t = 0; t < T; t++) {
        const FCTree& tr = _sched->trees[t];
        tree_bytes[t] = chunk_bytes * (uint64_t)tr.nchunks;
        for (size_t ei = 0; ei < tr.edges.size(); ei++) {
            const int p = tr.edges[ei].first, c = tr.edges[ei].second;
            Hop rs; rs.tree = t; rs.phase = 0; rs.src = c; rs.dst = p;
            rs.sw.assign(tr.paths[ei].rbegin(), tr.paths[ei].rend());
            rs_hop_of[t][c] = (int)hops.size();
            hops.push_back(rs);
            Hop ag; ag.tree = t; ag.phase = 1; ag.src = p; ag.dst = c;
            ag.sw = tr.paths[ei];
            ag_hops_of[t][p].push_back((int)hops.size());
            hops.push_back(ag);
            nchild[t][p]++;
        }
    }
    rs_total = ag_total = (long)(hops.size() / 2) * pieces;
    rs_done = ag_done = 0;
    ag_released = !phase_barrier;
    ctx.resize(hops.size() * (size_t)pieces);
    for (size_t h = 0; h < hops.size(); h++)
        for (int j = 0; j < pieces; j++)
            ctx[h * pieces + j] = {this, (int)h, j};
}

uint64_t FFFCTreeAllreduceTask::piece_bytes(int tree, int piece) const
{
    const uint64_t total = tree_bytes[tree];
    const uint64_t base = total / (uint64_t)pieces;
    uint64_t b = (piece == pieces - 1) ? total - base * (uint64_t)(pieces - 1) : base;
    return b == 0 ? 1 : b;
}

bool FFFCTreeAllreduceTask::piece_ready(const Hop& h, int j) const
{
    const FCTree& tr = _sched->trees[h.tree];
    if (h.phase == 0)
        return rs_have[h.tree][(size_t)h.src * pieces + j] == nchild[h.tree][h.src];
    if (h.src == tr.root)
        return ag_released &&
               rs_have[h.tree][(size_t)h.src * pieces + j] == nchild[h.tree][h.src];
    return ag_have[h.tree][(size_t)h.src * pieces + j] != 0;
}

void FFFCTreeAllreduceTask::try_launch(int hi)
{
    Hop& h = hops[hi];
    while (h.next_piece < pieces && h.in_flight < window && piece_ready(h, h.next_piece)) {
        start_flow(hi, h.next_piece, eventlist().now());
        h.next_piece++;
        h.in_flight++;
    }
}

void FFFCTreeAllreduceTask::doNextEvent()
{
    start_time = ready_time;
    state      = FFTask::TASK_RUNNING;
    build();
    long leaves = 0;
    for (size_t i = 0; i < hops.size(); i++)
        if (hops[i].phase == 0 && piece_ready(hops[i], 0)) leaves++;
    std::cerr << "[FCTree] Starting ForestColl AllReduce over " << _sched->trees.size()
              << " trees (" << _sched->total_chunks() << " unit chunks of " << chunk_bytes
              << " B), " << hops.size() / 2 << " tree edges, pieces=" << pieces
              << " window=" << window << " phase_barrier=" << (phase_barrier ? 1 : 0)
              << ", " << leaves << " leaf edges ready at t0\n";
    for (size_t i = 0; i < hops.size(); i++)
        if (hops[i].phase == 0) try_launch((int)i);
}

void FFFCTreeAllreduceTask::start_flow(int hi, int j, simtime_picosec t0)
{
    const Hop& h = hops[hi];
    flows_started++;
    if (++in_flight > max_in_flight) max_in_flight = in_flight;
    Route* routeout = nullptr;
    Route* routein  = nullptr;
    if (use_fc_paths && !h.sw.empty()) {
        ExplicitRouteTopology* ert = dynamic_cast<ExplicitRouteTopology*>(_topology);
        if (ert != nullptr) {
            std::vector<int> rev(h.sw.rbegin(), h.sw.rend());
            routeout = ert->route_for_switch_path(h.src, h.dst, h.sw);
            routein  = ert->route_for_switch_path(h.dst, h.src, rev);
            if (routeout == nullptr || routein == nullptr) {
                delete routeout; delete routein;
                routeout = routein = nullptr;
            }
        }
    }
    if (routeout != nullptr) {
        n_fc_routed++;
    } else {
        n_fallback_routed++;
        auto* paths_out = _topology->get_paths(h.src, h.dst);
        auto* paths_in  = _topology->get_paths(h.dst, h.src);
        routeout = new Route(*paths_out->at(ecmp_path_index(h.src, h.dst, paths_out->size())));
        routein  = new Route(*paths_in->at(ecmp_path_index(h.dst, h.src, paths_in->size())));
        delete paths_out;
        delete paths_in;
    }
    const uint64_t bytes = piece_bytes(h.tree, j);
    void* cb = &ctx[(size_t)hi * pieces + j];
#ifdef USE_DCTCP
    DCTCPSrc* fs = new DCTCPSrc(nullptr, nullptr, &null_out, eventlist(),
                                h.src, h.dst, ar_finish_fc_tree, cb);
    TcpSink* snk = new TcpSink();
    fs->set_flowsize(bytes);
    fs->set_ssthresh(ffapp->ssthresh * Packet::data_packet_size());
    fs->_rto = timeFromMs(10);
    ffapp->tcpRtxScanner.registerTcp(*fs);
#else
    RoCEv2Src* fs = new RoCEv2Src(nullptr, eventlist(), h.src, h.dst, ar_finish_fc_tree, cb);
    RoCEv2Sink* snk = new RoCEv2Sink(nullptr, eventlist());
    fs->set_flowsize(bytes);
    fs->set_initial_credits(RoCEv2Src::default_initial_credits);
#endif
    routeout->push_back(snk);
    routein->push_back(fs);
    fs->connect(*routeout, *routein, *snk, t0);
}

void FFFCTreeAllreduceTask::piece_done(int hi, int j)
{
    Hop& h = hops[hi];
    h.in_flight--;
    in_flight--;
    const int t = h.tree;
    const int root = _sched->trees[t].root;
    const simtime_picosec now = eventlist().now();

    if (h.phase == 0) {
        rs_done++;
        const uint16_t got = ++rs_have[t][(size_t)h.dst * pieces + j];
        if (got == nchild[t][h.dst]) {
            if (h.dst != root) try_launch(rs_hop_of[t][h.dst]);
            else for (int a : ag_hops_of[t][root]) try_launch(a);
        }
        try_launch(hi);
        if (rs_done == rs_total) {
            rs_finish = now;
            std::cerr << "[FCTree] ReduceScatter complete at " << timeAsMs(now) << " ms\n";
            if (!ag_released) {
                ag_released = true;
                for (size_t tt = 0; tt < _sched->trees.size(); tt++)
                    for (int a : ag_hops_of[tt][_sched->trees[tt].root]) try_launch(a);
            }
        }
    } else {
        ag_done++;
        ag_have[t][(size_t)h.dst * pieces + j] = 1;
        for (int a : ag_hops_of[t][h.dst]) try_launch(a);
        try_launch(hi);
        if (ag_done == ag_total) {
            for (const Hop& x : hops)
                if (x.next_piece != pieces || x.in_flight != 0) {
                    std::cerr << "[FCTree] INTERNAL ERROR: tree edge " << x.src << "->" << x.dst
                              << " phase " << x.phase << " sent " << x.next_piece << "/"
                              << pieces << " pieces, " << x.in_flight << " in flight\n";
                    abort();
                }
            for (size_t tt = 0; tt < _sched->trees.size(); tt++)
                for (int n = 0; n < _sched->nhost; n++)
                    for (int q = 0; q < pieces; q++)
                        if (n != _sched->trees[tt].root && !ag_have[tt][(size_t)n * pieces + q]) {
                            std::cerr << "[FCTree] INTERNAL ERROR: host " << n << " lacks piece "
                                      << q << " of tree " << tt << "\n";
                            abort();
                        }
            finish_time = now;
            std::cerr << "[FCTree] AllReduce complete at " << timeAsMs(finish_time)
                      << " ms; " << flows_started << " flows, at most " << max_in_flight
                      << " in flight; " << n_fc_routed << " on ForestColl paths, "
                      << n_fallback_routed << " on topology routing\n";
            cleanup();
        }
    }
}
