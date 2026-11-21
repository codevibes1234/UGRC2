#ifndef GRAPH_GEN_H
#define GRAPH_GEN_H

#include "graph_topology.h"
#include <algorithm>
#include <random>
#include <string>

inline SwitchGraph make_jellyfish(int nhost, int ports, int hosts_per_sw, unsigned seed = 13)
{
    SwitchGraph g;
    g.nhost = nhost;
    g.nsw = (nhost + hosts_per_sw - 1) / hosts_per_sw;
    int r = ports - hosts_per_sw;
    g.adj.assign(g.nsw, {});
    g.host_switch.resize(nhost);
    for (int h = 0; h < nhost; h++) g.host_switch[h] = h / hosts_per_sw;

    std::mt19937 gen(seed);
    auto deg = [&](int s) { return (int)g.adj[s].size(); };
    auto linked = [&](int a, int b) {
        return std::find(g.adj[a].begin(), g.adj[a].end(), b) != g.adj[a].end();
    };
    auto link = [&](int a, int b) { g.adj[a].push_back(b); g.adj[b].push_back(a); };

    std::vector<int> free_sw;
    for (int s = 0; s < g.nsw; s++) free_sw.push_back(s);
    while (free_sw.size() > 1) {
        std::shuffle(free_sw.begin(), free_sw.end(), gen);
        bool progress = false;
        for (size_t i = 0; i + 1 < free_sw.size(); i += 2) {
            int a = free_sw[i], b = free_sw[i + 1];
            if (a != b && !linked(a, b) && deg(a) < r && deg(b) < r) { link(a, b); progress = true; }
        }
        std::vector<int> still;
        for (int s : free_sw) if (deg(s) < r) still.push_back(s);
        free_sw.swap(still);
        if (!progress) break;
    }
    for (int s = 0; s < g.nsw; s++) {
        while (deg(s) + 1 < r) {
            bool done = false;
            for (int x = 0; x < g.nsw && !done; x++) {
                if (x == s) continue;
                for (int y : g.adj[x]) {
                    if (y == s || linked(s, x) || linked(s, y)) continue;
                    g.adj[x].erase(std::find(g.adj[x].begin(), g.adj[x].end(), y));
                    g.adj[y].erase(std::find(g.adj[y].begin(), g.adj[y].end(), x));
                    link(s, x); link(s, y);
                    done = true;
                    break;
                }
            }
            if (!done) break;
        }
    }
    g.name = "Jellyfish(" + std::to_string(g.nsw) + " switches, " + std::to_string(ports) +
             " ports, " + std::to_string(hosts_per_sw) + " hosts each)";
    return g;
}

inline SwitchGraph make_flattened_butterfly(int nhost, int k, int n, int hosts_per_sw)
{
    SwitchGraph g;
    g.nhost = nhost;
    g.nsw = 1;
    for (int i = 0; i < n; i++) g.nsw *= k;
    g.adj.assign(g.nsw, {});
    g.host_switch.resize(nhost);
    for (int h = 0; h < nhost; h++) g.host_switch[h] = (h / hosts_per_sw) % g.nsw;

    std::vector<int> stride(n, 1);
    for (int d = 1; d < n; d++) stride[d] = stride[d - 1] * k;
    for (int s = 0; s < g.nsw; s++)
        for (int d = 0; d < n; d++) {
            int coord = (s / stride[d]) % k;
            for (int c = 0; c < k; c++) {
                if (c == coord) continue;
                int t = s + (c - coord) * stride[d];
                if (t > s) { g.adj[s].push_back(t); g.adj[t].push_back(s); }
            }
        }
    g.name = "FlattenedButterfly(" + std::to_string(k) + "-ary " + std::to_string(n) + "-flat, " +
             std::to_string(hosts_per_sw) + " hosts per switch)";
    return g;
}

inline SwitchGraph make_dragonfly(int nhost, int ngroup, int a, int hosts_per_sw)
{
    SwitchGraph g;
    g.nhost = nhost;
    g.nsw = ngroup * a;
    g.adj.assign(g.nsw, {});
    g.host_switch.resize(nhost);
    for (int h = 0; h < nhost; h++) g.host_switch[h] = (h / hosts_per_sw) % g.nsw;

    for (int grp = 0; grp < ngroup; grp++)
        for (int i = 0; i < a; i++)
            for (int j = i + 1; j < a; j++) {
                g.adj[grp * a + i].push_back(grp * a + j);
                g.adj[grp * a + j].push_back(grp * a + i);
            }

    for (int g1 = 0; g1 < ngroup; g1++)
        for (int g2 = g1 + 1; g2 < ngroup; g2++) {
            int idx = (g2 - g1 - 1) % a;
            int s1 = g1 * a + idx, s2 = g2 * a + ((g1) % a);
            g.adj[s1].push_back(s2);
            g.adj[s2].push_back(s1);
        }
    g.name = "Dragonfly(" + std::to_string(ngroup) + " groups x " + std::to_string(a) +
             " routers, " + std::to_string(hosts_per_sw) + " hosts per router)";
    return g;
}

inline SwitchGraph make_megafly(int nhost, int ngroup, int nleaf, int nspine, int hosts_per_leaf)
{
    SwitchGraph g;
    g.nhost = nhost;
    int per_group = nleaf + nspine;
    g.nsw = ngroup * per_group;
    g.adj.assign(g.nsw, {});
    auto leaf = [&](int grp, int i) { return grp * per_group + i; };
    auto spine = [&](int grp, int i) { return grp * per_group + nleaf + i; };

    g.host_switch.resize(nhost);
    for (int h = 0; h < nhost; h++) {
        int idx = h / hosts_per_leaf;
        g.host_switch[h] = leaf(idx / nleaf % ngroup, idx % nleaf);
    }

    for (int grp = 0; grp < ngroup; grp++)
        for (int l = 0; l < nleaf; l++)
            for (int s = 0; s < nspine; s++) {
                g.adj[leaf(grp, l)].push_back(spine(grp, s));
                g.adj[spine(grp, s)].push_back(leaf(grp, l));
            }

    for (int g1 = 0; g1 < ngroup; g1++)
        for (int g2 = g1 + 1; g2 < ngroup; g2++) {
            int i = (g2 - g1 - 1) % nspine, j = g1 % nspine;
            g.adj[spine(g1, i)].push_back(spine(g2, j));
            g.adj[spine(g2, j)].push_back(spine(g1, i));
        }
    g.name = "Megafly(" + std::to_string(ngroup) + " groups x [" + std::to_string(nleaf) +
             " leaf + " + std::to_string(nspine) + " spine], " + std::to_string(hosts_per_leaf) +
             " hosts per leaf)";
    return g;
}

inline SwitchGraph make_named(const std::string &name, int nhost)
{
    if (nhost == 432) {
        if (name == "jellyfish")     return make_jellyfish(432, 12, 4, 13);
        if (name == "flatbutterfly") return make_flattened_butterfly(432, 6, 2, 12);
        if (name == "dragonfly")     return make_dragonfly(432, 27, 4, 4);
        if (name == "megafly")       return make_megafly(432, 9, 6, 3, 8);
    }
    if (name == "jellyfish")     return make_jellyfish(nhost, 12, 4, 13);
    if (name == "flatbutterfly") return make_flattened_butterfly(nhost, 4, 2, 8);
    if (name == "dragonfly")     return make_dragonfly(nhost, 8, 4, 4);
    if (name == "megafly")       return make_megafly(nhost, 4, 4, 2, 8);
    return SwitchGraph{};
}

#endif
