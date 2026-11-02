#include "graph_topology.h"
#include "queue.h"
#include "prioqueue.h"
#include "compositequeue.h"
#include "ecnqueue.h"
#include "queue_lossless_input.h"
#include "queue_lossless_output.h"
#include <cassert>
#include <array>
#include <deque>
#include <fstream>
#include <iterator>
#include <cstdlib>
#include <functional>
#include <iostream>

extern uint32_t RTT_rack;
extern uint32_t RTT_net;
extern uint32_t SPEED;

string ntoa(double n);

GraphTopology::GraphTopology(const SwitchGraph& g, mem_b queuesize, Logfile* log,
                             EventList* ev, FirstFit* f, queue_type q, int max_paths,
                             const std::string& routing, mem_b lossless_qsize)
    : Topology(), _g(g), qt(q), _queuesize(queuesize),
      _lossless_qsize(lossless_qsize > 0 ? lossless_qsize : memFromPkt(200)),
      logfile(log), eventlist(ev), ff(f), _max_paths(max_paths), _routing(routing)
{
    assert((int)_g.host_switch.size() == _g.nhost);
    _linkspeed = (uint64_t)SPEED / (uint64_t)_g.host_ports;

    compute_distances();
    if (_routing == "updown") build_updown_order();
    build_links();
    load_fc_schedule();

    long links = 0;
    for (int s = 0; s < _g.nsw; s++) links += _g.adj[s].size();
    std::cout << "Building " << _g.name << " with " << _g.nhost << " hosts, " << _g.nsw
              << " switches, " << links / 2 << " switch-switch links, diameter " << _diameter
              << ", mean switch hops " << _mean_hops << ", links at " << _linkspeed
              << " Mbps, routing " << _routing << std::endl;
}

Queue* GraphTopology::alloc_src_queue(QueueLogger* ql, uint64_t speed)
{
    return new PriorityQueue(speedFromMbps(speed), memFromPkt(FEEDER_BUFFER), *eventlist, ql);
}

Queue* GraphTopology::alloc_queue(QueueLogger* ql, uint64_t speed)
{
    if (qt == ECN)
        return new ECNQueue(speedFromMbps(speed), _queuesize, *eventlist, ql, memFromPkt(50));
    if (qt == LOSSLESS_INPUT)
        return new LosslessOutputQueue(speedFromMbps(speed), _lossless_qsize, *eventlist, ql);
    if (qt == COMPOSITE)
        return new CompositeQueue(speedFromMbps(speed), _queuesize, *eventlist, ql);
    std::cerr << "GraphTopology: unsupported queue type (use ECN or LOSSLESS_INPUT)\n";
    assert(0);
    return NULL;
}

void GraphTopology::add_in(Route* rt, Queue* q)
{
    if (qt == LOSSLESS_INPUT) rt->push_back(q->getRemoteEndpoint());
}

void GraphTopology::build_links()
{
    QueueLogger* ql = NULL;

    q_host_sw.assign(_g.nhost, NULL); q_sw_host.assign(_g.nhost, NULL);
    p_host_sw.assign(_g.nhost, NULL); p_sw_host.assign(_g.nhost, NULL);

    for (int h = 0; h < _g.nhost; h++) {
        int s = _g.host_switch[h];
        q_host_sw[h] = alloc_src_queue(ql, _linkspeed);
        q_host_sw[h]->setName("H" + ntoa(h) + "->SW" + ntoa(s));
        p_host_sw[h] = new Pipe(timeFromNs(RTT_rack), *eventlist);

        q_sw_host[h] = alloc_queue(ql, _linkspeed);
        q_sw_host[h]->setName("SW" + ntoa(s) + "->H" + ntoa(h));
        p_sw_host[h] = new Pipe(timeFromNs(RTT_rack), *eventlist);

        if (qt == LOSSLESS_INPUT)
            new LosslessInputQueue(*eventlist, q_host_sw[h]);
    }

    for (int a = 0; a < _g.nsw; a++)
        for (int b : _g.adj[a]) {
            if (q_sw_sw.count({a, b})) continue;
            Queue* q = alloc_queue(ql, _linkspeed);
            q->setName("SW" + ntoa(a) + "->SW" + ntoa(b));
            q_sw_sw[{a, b}] = q;
            p_sw_sw[{a, b}] = new Pipe(timeFromNs(RTT_net), *eventlist);
            if (qt == LOSSLESS_INPUT)
                new LosslessInputQueue(*eventlist, q);
        }
}

void GraphTopology::compute_distances()
{
    _dist.assign(_g.nsw, std::vector<int>(_g.nsw, -1));
    long total = 0, pairs = 0;
    for (int s = 0; s < _g.nsw; s++) {
        std::deque<int> qq;
        _dist[s][s] = 0;
        qq.push_back(s);
        while (!qq.empty()) {
            int u = qq.front(); qq.pop_front();
            for (int v : _g.adj[u])
                if (_dist[s][v] < 0) { _dist[s][v] = _dist[s][u] + 1; qq.push_back(v); }
        }
        for (int t = 0; t < _g.nsw; t++) {
            assert(_dist[s][t] >= 0 && "switch graph is disconnected");
            if (_diameter < _dist[s][t]) _diameter = _dist[s][t];
            if (s != t) { total += _dist[s][t]; pairs++; }
        }
    }
    _mean_hops = pairs ? (double)total / pairs : 0;
}

void GraphTopology::enumerate(int a, int b, std::vector<std::vector<int>>& out)
{
    std::vector<int> cur{a};
    std::vector<std::pair<int, size_t>> stack{{a, 0}};
    while (!stack.empty() && (int)out.size() < _max_paths) {
        auto& [u, idx] = stack.back();
        if (u == b) { out.push_back(cur); stack.pop_back(); cur.pop_back(); continue; }
        if (idx >= _g.adj[u].size()) { stack.pop_back(); cur.pop_back(); continue; }
        int v = _g.adj[u][idx++];
        if (_dist[v][b] == _dist[u][b] - 1) {
            cur.push_back(v);
            stack.push_back({v, 0});
        }
    }
}

void GraphTopology::build_updown_order()
{
    int root = 0;
    for (int s = 1; s < _g.nsw; s++)
        if (_g.adj[s].size() > _g.adj[root].size()) root = s;

    _udepth.assign(_g.nsw, -1);
    std::deque<int> q;
    _udepth[root] = 0;
    q.push_back(root);
    while (!q.empty()) {
        int u = q.front(); q.pop_front();
        for (int v : _g.adj[u])
            if (_udepth[v] < 0) { _udepth[v] = _udepth[u] + 1; q.push_back(v); }
    }
}

bool GraphTopology::is_up(int u, int v) const
{
    if (_udepth[v] != _udepth[u]) return _udepth[v] < _udepth[u];
    return v < u;
}

void GraphTopology::enumerate_updown(int a, int b, std::vector<std::vector<int>>& out)
{
    const int N = _g.nsw;
    std::vector<std::array<int,2>> dist(N, {-1, -1});
    std::deque<std::pair<int,int>> q;
    dist[a][0] = 0;
    q.push_back({a, 0});
    while (!q.empty()) {
        auto [u, ph] = q.front(); q.pop_front();
        for (int v : _g.adj[u]) {
            int nph = is_up(u, v) ? ph : 1;
            if (ph == 1 && is_up(u, v)) continue;
            if (dist[v][nph] < 0) { dist[v][nph] = dist[u][ph] + 1; q.push_back({v, nph}); }
        }
    }
    int best = -1;
    for (int ph = 0; ph < 2; ph++)
        if (dist[b][ph] >= 0 && (best < 0 || dist[b][ph] < best)) best = dist[b][ph];
    if (best < 0) return;

    std::vector<int> cur{a};
    std::function<void(int,int,int)> dfs = [&](int u, int ph, int d) {
        if ((int)out.size() >= _max_paths) return;
        if (u == b && d == best) { out.push_back(cur); return; }
        if (d >= best) return;
        for (int v : _g.adj[u]) {
            if (ph == 1 && is_up(u, v)) continue;
            int nph = is_up(u, v) ? ph : 1;
            if (dist[v][nph] != d + 1) continue;
            cur.push_back(v);
            dfs(v, nph, d + 1);
            cur.pop_back();
            if ((int)out.size() >= _max_paths) return;
        }
    };
    dfs(a, 0, 0);
}

vector<const Route*>* GraphTopology::get_paths(int src, int dest)
{
    vector<const Route*>* paths = new vector<const Route*>();
    assert(src != dest && src < _g.nhost && dest < _g.nhost);

    int ss = _g.host_switch[src], ds = _g.host_switch[dest];

    if (ss == ds) {
        Route* r = new Route();
        r->push_back(q_host_sw[src]); r->push_back(p_host_sw[src]); add_in(r, q_host_sw[src]);
        r->push_back(q_sw_host[dest]); r->push_back(p_sw_host[dest]);
        paths->push_back(r);
        return paths;
    }

    std::vector<std::vector<int>> sw_paths;
    if (_routing == "updown") enumerate_updown(ss, ds, sw_paths);
    else                      enumerate(ss, ds, sw_paths);
    assert(!sw_paths.empty());

    for (const auto& sp : sw_paths) {
        Route* r = new Route();
        r->push_back(q_host_sw[src]); r->push_back(p_host_sw[src]); add_in(r, q_host_sw[src]);
        for (size_t i = 0; i + 1 < sp.size(); i++) {
            Queue* q = q_sw_sw[{sp[i], sp[i + 1]}];
            r->push_back(q); r->push_back(p_sw_sw[{sp[i], sp[i + 1]}]); add_in(r, q);
        }
        r->push_back(q_sw_host[dest]); r->push_back(p_sw_host[dest]);
        paths->push_back(r);
    }
    return paths;
}

void GraphTopology::load_fc_schedule()
{
#ifdef FC_SCHEDULE_FILE
#include FC_SCHEDULE_FILE
    _fc_num_steps       = _gen_fc_num_steps;
    _fc_rs_total_chunks = _gen_fc_rs_total_chunks;
    _fc_ag_total_chunks = _gen_fc_ag_total_chunks;
    _fc_rs_steps        = _gen_fc_rs_steps;
    _fc_ag_steps        = _gen_fc_ag_steps;
#ifdef FC_SCHEDULE_NHOST
    _fc_schedule_nhost = FC_SCHEDULE_NHOST;
#endif
#ifdef FC_SCHEDULE_HAS_PATHS
    _fc_rs_paths = _gen_fc_rs_paths;
    _fc_ag_paths = _gen_fc_ag_paths;
    _fc_has_paths = true;
#endif
    std::cout << "ForestColl schedule: " << _fc_num_steps << " steps, "
              << _fc_rs_total_chunks << " chunks, built for "
              << _fc_schedule_nhost << " hosts" << std::endl;
#else
    _fc_num_steps = 0;
#endif
}

const std::vector<ZCubeFCTransmission>&
GraphTopology::fc_rs_step(int step) const
{
    assert(step >= 0 && step < (int)_fc_rs_steps.size());
    return _fc_rs_steps[step];
}

const std::vector<ZCubeFCTransmission>&
GraphTopology::fc_ag_step(int step) const
{
    assert(step >= 0 && step < (int)_fc_ag_steps.size());
    return _fc_ag_steps[step];
}

const std::vector<int>& GraphTopology::fc_rs_path(int step, int idx) const
{
    static const std::vector<int> none;
    if (step < 0 || step >= (int)_fc_rs_paths.size()) return none;
    const auto &v = _fc_rs_paths[step];
    if (idx < 0 || idx >= (int)v.size()) return none;
    return v[idx];
}

const std::vector<int>& GraphTopology::fc_ag_path(int step, int idx) const
{
    static const std::vector<int> none;
    if (step < 0 || step >= (int)_fc_ag_paths.size()) return none;
    const auto &v = _fc_ag_paths[step];
    if (idx < 0 || idx >= (int)v.size()) return none;
    return v[idx];
}

Route* GraphTopology::route_for_switch_path(int src, int dst,
                                            const std::vector<int> &sw)
{
    if (src == dst || src < 0 || dst < 0 ||
        src >= _g.nhost || dst >= _g.nhost)
        return nullptr;

    const int ss = _g.host_switch[src], ds = _g.host_switch[dst];

    if (sw.empty()) {
        if (ss != ds) return nullptr;
    } else {
        if (sw.front() != ss || sw.back() != ds) return nullptr;
    }

    Route* r = new Route();
    r->push_back(q_host_sw[src]);
    r->push_back(p_host_sw[src]);
    add_in(r, q_host_sw[src]);

    for (size_t i = 0; i + 1 < sw.size(); i++) {
        auto key = std::make_pair(sw[i], sw[i + 1]);
        auto qit = q_sw_sw.find(key);
        auto pit = p_sw_sw.find(key);
        if (qit == q_sw_sw.end() || pit == p_sw_sw.end()) {
            delete r;
            return nullptr;
        }
        r->push_back(qit->second);
        r->push_back(pit->second);
        add_in(r, qit->second);
    }

    r->push_back(q_sw_host[dst]);
    r->push_back(p_sw_host[dst]);
    return r;
}

int GraphTopology::fc_num_steps()       const { return _fc_num_steps; }
int GraphTopology::fc_rs_total_chunks() const { return _fc_rs_total_chunks; }
int GraphTopology::fc_ag_total_chunks() const { return _fc_ag_total_chunks; }

static bool fc_find_int(const std::string& s, const std::string& key, int& out)
{
    size_t p = s.find(key);
    if (p == std::string::npos) return false;
    p = s.find_first_of("0123456789", p + key.size());
    if (p == std::string::npos) return false;
    out = atoi(s.c_str() + p);
    return true;
}

static bool fc_parse_nested(const std::string& s, const std::string& name,
                            std::vector<std::vector<std::vector<int>>>& out)
{
    size_t p = s.find(name);
    if (p == std::string::npos) return false;
    p = s.find('{', p);
    if (p == std::string::npos) return false;
    out.clear();
    int depth = 0;
    std::vector<std::vector<int>> step;
    std::vector<int> rec;
    for (size_t i = p; i < s.size(); i++) {
        char c = s[i];
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') {
            size_t e = s.find('\n', i);
            if (e == std::string::npos) break;
            i = e;
            continue;
        }
        if (c == '{') {
            depth++;
            if (depth == 2) step.clear();
            if (depth == 3) rec.clear();
        } else if (c == '}') {
            if (depth == 3) step.push_back(rec);
            else if (depth == 2) out.push_back(step);
            depth--;
            if (depth == 0) return true;
        } else if (c >= '0' && c <= '9') {
            char* end = nullptr;
            long v = strtol(s.c_str() + i, &end, 10);
            if (depth == 3) rec.push_back((int)v);
            i = (size_t)(end - s.c_str()) - 1;
        }
    }
    return false;
}

bool GraphTopology::load_fc_schedule_file(const std::string& path)
{
    std::ifstream in(path);
    if (!in) { std::cerr << "ERROR: cannot read schedule " << path << "\n"; return false; }
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    int nsteps = 0, rs_total = 0, ag_total = 0, nhost = -1;
    if (!fc_find_int(s, "_gen_fc_num_steps", nsteps) ||
        !fc_find_int(s, "_gen_fc_rs_total_chunks", rs_total) ||
        !fc_find_int(s, "_gen_fc_ag_total_chunks", ag_total)) {
        std::cerr << "ERROR: " << path << " lacks the _gen_fc_* constants\n";
        return false;
    }
    fc_find_int(s, "#define FC_SCHEDULE_NHOST", nhost);

    std::vector<std::vector<std::vector<int>>> rs, ag, rsp, agp;
    if (!fc_parse_nested(s, "_gen_fc_rs_steps =", rs) ||
        !fc_parse_nested(s, "_gen_fc_ag_steps =", ag)) {
        std::cerr << "ERROR: " << path << ": could not parse the step arrays\n";
        return false;
    }
    if ((int)rs.size() != nsteps || (int)ag.size() != nsteps) {
        std::cerr << "ERROR: " << path << ": " << rs.size() << " RS and " << ag.size()
                  << " AG steps parsed, header says " << nsteps << "\n";
        return false;
    }
    auto convert = [&](const std::vector<std::vector<std::vector<int>>>& in_,
                       std::vector<std::vector<ZCubeFCTransmission>>& out_) {
        out_.assign(in_.size(), {});
        for (size_t k = 0; k < in_.size(); k++)
            for (const auto& r : in_[k]) {
                if (r.size() != 3) return false;
                out_[k].push_back({r[0], r[1], r[2]});
            }
        return true;
    };
    if (!convert(rs, _fc_rs_steps) || !convert(ag, _fc_ag_steps)) {
        std::cerr << "ERROR: " << path << ": a transmission is not {sender, receiver, chunks}\n";
        return false;
    }
    _fc_num_steps = nsteps;
    _fc_rs_total_chunks = rs_total;
    _fc_ag_total_chunks = ag_total;
    _fc_schedule_nhost = nhost;
    _fc_has_paths = false;
    if (s.find("_gen_fc_rs_paths") != std::string::npos &&
        fc_parse_nested(s, "_gen_fc_rs_paths =", rsp) &&
        fc_parse_nested(s, "_gen_fc_ag_paths =", agp)) {
        _fc_rs_paths = rsp;
        _fc_ag_paths = agp;
        _fc_has_paths = true;
    }
    long ntx = 0;
    for (const auto& st : _fc_rs_steps) ntx += st.size();
    for (const auto& st : _fc_ag_steps) ntx += st.size();
    std::cout << "ForestColl schedule (read at run time from " << path << "): "
              << _fc_num_steps << " steps, " << _fc_rs_total_chunks << " chunks, "
              << ntx << " transmissions, built for " << _fc_schedule_nhost << " hosts"
              << (_fc_has_paths ? ", with paths" : "") << std::endl;
    return true;
}
