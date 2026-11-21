#ifndef OCS_LOGICAL_TOPOLOGY_H
#define OCS_LOGICAL_TOPOLOGY_H

#include <algorithm>
#include <deque>
#include <iostream>
#include <random>
#include <string>
#include <vector>

typedef std::vector<size_t> OcsConn;

inline int ocs_degree_of(const OcsConn &c, int n, int i)
{
    int d = 0;
    for (int j = 0; j < n; j++)
        if (j != i) d += (int)c[(size_t)i * n + j];
    return d;
}

inline bool ocs_linked(const OcsConn &c, int n, int i, int j)
{
    return c[(size_t)i * n + j] > 0;
}

inline bool ocs_link(OcsConn &c, int n, int i, int j, int degree)
{
    if (i == j) return false;
    if (ocs_linked(c, n, i, j)) return false;
    if (ocs_degree_of(c, n, i) >= degree) return false;
    if (ocs_degree_of(c, n, j) >= degree) return false;
    c[(size_t)i * n + j] = 1;
    c[(size_t)j * n + i] = 1;
    return true;
}

inline OcsConn ocs_flattened_butterfly(int n, int degree, std::string &how)
{
    int best_k = -1, best_nd = -1, best_deg = -1;
    for (int k = 2; k <= n; k++) {
        long p = 1; int nd = 0;
        while (p < n) { p *= k; nd++; }
        if (p != n) continue;
        int deg = nd * (k - 1);
        if (deg <= degree && deg > best_deg) { best_deg = deg; best_k = k; best_nd = nd; }
    }
    if (best_k < 0) {
        std::cerr << "ERROR: no k-ary n-flat on " << n << " nodes fits degree "
                  << degree << ". Flattened Butterfly needs n = k^m and degree "
                     "m(k-1); try 16 nodes with -deg 4 (2-ary 4-flat) or -deg 6 "
                     "(4-ary 2-flat).\n";
        exit(1);
    }
    how = std::to_string(best_k) + "-ary " + std::to_string(best_nd) +
          "-flat, degree " + std::to_string(best_deg);

    OcsConn c((size_t)n * n, 0);
    std::vector<int> stride(best_nd, 1);
    for (int d = 1; d < best_nd; d++) stride[d] = stride[d - 1] * best_k;
    for (int s = 0; s < n; s++)
        for (int d = 0; d < best_nd; d++) {
            int coord = (s / stride[d]) % best_k;
            for (int v = 0; v < best_k; v++) {
                if (v == coord) continue;
                int t = s + (v - coord) * stride[d];
                if (t > s) ocs_link(c, n, s, t, degree);
            }
        }
    return c;
}

inline OcsConn ocs_dragonfly(int n, int degree, std::string &how)
{
    int best_a = -1;
    for (int a = 2; a <= degree + 1 && a <= n; a++) {
        if (n % a) continue;
        int ngroup = n / a, h = degree - (a - 1);
        if (h < 0) continue;
        if (ngroup > 1 && (long)a * h < ngroup - 1) continue;
        best_a = a;
    }
    if (best_a < 0) {
        std::cerr << "ERROR: no Dragonfly on " << n << " nodes fits degree "
                  << degree << " (needs a group size a dividing n with "
                     "a*(degree-a+1) >= n/a - 1).\n";
        exit(1);
    }
    int a = best_a, ngroup = n / a, h = degree - (a - 1);
    how = std::to_string(ngroup) + " groups x " + std::to_string(a) +
          " nodes, " + std::to_string(h) + " global port(s) per node";

    OcsConn c((size_t)n * n, 0);
    for (int g = 0; g < ngroup; g++)
        for (int i = 0; i < a; i++)
            for (int j = i + 1; j < a; j++)
                ocs_link(c, n, g * a + i, g * a + j, degree);

    for (int g1 = 0; g1 < ngroup; g1++)
        for (int g2 = g1 + 1; g2 < ngroup; g2++) {
            bool placed = false;
            for (int i = 0; i < a && !placed; i++)
                for (int j = 0; j < a && !placed; j++) {
                    int u = g1 * a + (i + g2) % a, v = g2 * a + (j + g1) % a;
                    placed = ocs_link(c, n, u, v, degree);
                }
            if (!placed)
                std::cerr << "WARNING: Dragonfly could not join group " << g1
                          << " to group " << g2 << " within the degree budget\n";
        }

    bool added = true;
    while (added) {
        added = false;
        for (int g1 = 0; g1 < ngroup; g1++)
            for (int g2 = g1 + 1; g2 < ngroup; g2++)
                for (int i = 0; i < a; i++)
                    for (int j = 0; j < a; j++)
                        if (ocs_link(c, n, g1 * a + i, g2 * a + j, degree)) {
                            added = true;
                            i = a; j = a;
                        }
    }
    return c;
}

inline OcsConn ocs_megafly(int n, int degree, std::string &how)
{
    int best_ng = -1, best_leaf = -1, best_spine = -1, best_min = -1;
    for (int ng = 3; ng <= n; ng++) {
        if (n % ng) continue;
        int per = n / ng;
        for (int nspine = 1; nspine < per; nspine++) {
            int nleaf = per - nspine;
            if (nleaf < nspine) continue;
            if (nspine > degree) continue;
            int globals_needed = ng - 1;
            int spine_free = nspine * (degree - nleaf);
            if (degree - nleaf < 0 || spine_free < globals_needed) continue;
            int spine_globals = globals_needed / nspine;
            int mindeg = std::min(nspine, nleaf + spine_globals);
            if (mindeg > best_min ||
                (mindeg == best_min && (ng > best_ng ||
                                        (ng == best_ng && nleaf > best_leaf))))
                { best_min = mindeg; best_ng = ng; best_leaf = nleaf; best_spine = nspine; }
        }
    }
    if (best_ng < 0) {
        std::cerr << "ERROR: no Megafly on " << n << " nodes fits degree " << degree << ".\n";
        exit(1);
    }
    int ng = best_ng, nleaf = best_leaf, nspine = best_spine, per = nleaf + nspine;
    how = std::to_string(ng) + " groups x [" + std::to_string(nleaf) + " leaf + " +
          std::to_string(nspine) + " spine]";

    OcsConn c((size_t)n * n, 0);
    auto leaf  = [&](int g, int i) { return g * per + i; };
    auto spine = [&](int g, int i) { return g * per + nleaf + i; };

    for (int g = 0; g < ng; g++)
        for (int l = 0; l < nleaf; l++)
            for (int s = 0; s < nspine; s++)
                ocs_link(c, n, leaf(g, l), spine(g, s), degree);

    for (int g1 = 0; g1 < ng; g1++)
        for (int g2 = g1 + 1; g2 < ng; g2++) {
            bool placed = false;
            for (int i = 0; i < nspine && !placed; i++)
                for (int j = 0; j < nspine && !placed; j++)
                    placed = ocs_link(c, n, spine(g1, (i + g2) % nspine),
                                      spine(g2, (j + g1) % nspine), degree);
            if (!placed)
                std::cerr << "WARNING: Megafly could not join group " << g1
                          << " to group " << g2 << " within the degree budget\n";
        }
    return c;
}

inline OcsConn ocs_jellyfish(int n, int degree, unsigned seed, std::string &how)
{
    how = "random regular graph, degree budget " + std::to_string(degree);
    OcsConn c((size_t)n * n, 0);
    std::mt19937 gen(seed);

    std::vector<int> order(n);
    for (int i = 0; i < n; i++) order[i] = i;
    std::shuffle(order.begin(), order.end(), gen);
    for (int i = 0; i + 1 < n; i++) ocs_link(c, n, order[i], order[i + 1], degree);

    std::vector<int> free_nodes;
    for (int i = 0; i < n; i++) if (ocs_degree_of(c, n, i) < degree) free_nodes.push_back(i);
    while (free_nodes.size() > 1) {
        std::shuffle(free_nodes.begin(), free_nodes.end(), gen);
        bool progress = false;
        for (size_t i = 0; i + 1 < free_nodes.size(); i += 2)
            if (ocs_link(c, n, free_nodes[i], free_nodes[i + 1], degree)) progress = true;
        std::vector<int> still;
        for (int v : free_nodes) if (ocs_degree_of(c, n, v) < degree) still.push_back(v);
        free_nodes.swap(still);
        if (!progress) break;
    }

    for (int s = 0; s < n; s++) {
        while (ocs_degree_of(c, n, s) + 1 < degree) {
            bool done = false;
            for (int x = 0; x < n && !done; x++) {
                if (x == s) continue;
                for (int y = 0; y < n && !done; y++) {
                    if (y == s || y == x || !ocs_linked(c, n, x, y)) continue;
                    if (ocs_linked(c, n, s, x) || ocs_linked(c, n, s, y)) continue;
                    c[(size_t)x * n + y] = 0; c[(size_t)y * n + x] = 0;
                    ocs_link(c, n, s, x, degree);
                    ocs_link(c, n, s, y, degree);
                    done = true;
                }
            }
            if (!done) break;
        }
    }
    return c;
}

inline OcsConn &ocs_external_conn() { static OcsConn c; return c; }
inline std::string &ocs_external_how() { static std::string h; return h; }

inline OcsConn make_logical_conn(const std::string &family, int n, int degree,
                                 unsigned seed, std::string &how)
{
    if (family == "topoopt" && !ocs_external_conn().empty()) {
        how = ocs_external_how();
        return ocs_external_conn();
    }
    if (family == "jellyfish")     return ocs_jellyfish(n, degree, seed, how);
    if (family == "flatbutterfly") return ocs_flattened_butterfly(n, degree, how);
    if (family == "dragonfly")     return ocs_dragonfly(n, degree, how);
    if (family == "megafly")       return ocs_megafly(n, degree, how);
    how.clear();
    return OcsConn();
}

struct OcsStats { int nodes, links, deg_min, deg_max, diameter; double deg_mean, mean_hops; bool connected; };

inline OcsStats ocs_describe(const OcsConn &c, int n)
{
    OcsStats st{n, 0, 1 << 30, 0, 0, 0.0, 0.0, true};
    long degsum = 0;
    for (int i = 0; i < n; i++) {
        int d = ocs_degree_of(c, n, i);
        st.deg_min = std::min(st.deg_min, d);
        st.deg_max = std::max(st.deg_max, d);
        degsum += d;
        for (int j = i + 1; j < n; j++) if (ocs_linked(c, n, i, j)) st.links++;
    }
    st.deg_mean = n ? (double)degsum / n : 0.0;

    long hopsum = 0, pairs = 0;
    for (int s = 0; s < n; s++) {
        std::vector<int> dist(n, -1);
        std::deque<int> q{s};
        dist[s] = 0;
        while (!q.empty()) {
            int u = q.front(); q.pop_front();
            for (int v = 0; v < n; v++)
                if (v != u && dist[v] < 0 && ocs_linked(c, n, u, v)) { dist[v] = dist[u] + 1; q.push_back(v); }
        }
        for (int v = 0; v < n; v++) {
            if (v == s) continue;
            if (dist[v] < 0) { st.connected = false; continue; }
            st.diameter = std::max(st.diameter, dist[v]);
            hopsum += dist[v]; pairs++;
        }
    }
    st.mean_hops = pairs ? (double)hopsum / pairs : 0.0;
    return st;
}

#endif
