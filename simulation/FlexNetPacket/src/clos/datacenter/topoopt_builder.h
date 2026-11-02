#ifndef TOPOOPT_BUILDER_H
#define TOPOOPT_BUILDER_H

#include "ffapp.h"
#include "ocs_logical_topology.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

struct TopoOptPlan {
    OcsConn conn;
    std::map<uint64_t, std::vector<std::vector<int>>> rings;
    std::vector<int> strides;
    int d_ar = 0, d_mp = 0;
    double ar_bytes = 0, mp_bytes = 0;
    long full_groups = 0, partial_groups = 0;
};

inline std::vector<int> topoopt_max_assignment(const std::vector<std::vector<double>> &w)
{
    const int n = (int)w.size();
    const double BIG = 1e300;
    double maxw = 0;
    for (auto &r : w) for (double x : r) maxw = std::max(maxw, x);
    std::vector<double> u(n + 1, 0), v(n + 1, 0);
    std::vector<int> p(n + 1, 0), way(n + 1, 0);
    auto cost = [&](int i, int j) { return i == j ? 1e200 : maxw - w[i][j]; };
    for (int i = 1; i <= n; i++) {
        p[0] = i;
        int j0 = 0;
        std::vector<double> minv(n + 1, BIG);
        std::vector<char> used(n + 1, 0);
        do {
            used[j0] = 1;
            int i0 = p[j0], j1 = 0;
            double delta = BIG;
            for (int j = 1; j <= n; j++)
                if (!used[j]) {
                    double cur = cost(i0 - 1, j - 1) - u[i0] - v[j];
                    if (cur < minv[j]) { minv[j] = cur; way[j] = j0; }
                    if (minv[j] < delta) { delta = minv[j]; j1 = j; }
                }
            for (int j = 0; j <= n; j++)
                if (used[j]) { u[p[j]] += delta; v[j] -= delta; }
                else minv[j] -= delta;
            j0 = j1;
        } while (p[j0] != 0);
        do { int j1 = way[j0]; p[j0] = p[j1]; j0 = j1; } while (j0);
    }
    std::vector<int> match(n, -1);
    for (int j = 1; j <= n; j++) if (p[j]) match[p[j] - 1] = j - 1;
    return match;
}

inline TopoOptPlan topoopt_plan(const std::string &taskgraph, int n, int degree)
{
    TopoOptPlan plan;
    std::string buffer;
    if (!FFApplication::LoadFileRaw(taskgraph.c_str(), &buffer)) {
        std::cerr << "ERROR: topoopt: cannot read " << taskgraph << "\n";
        exit(1);
    }
    auto tg = flatbuffers::GetRoot<FlatBufTaskGraph::TaskGraph>(buffer.c_str());
    const int nn = tg->nnode(), ns = tg->nswitch();
    if (nn != n) {
        std::cerr << "ERROR: topoopt: the taskgraph has " << nn << " nodes, -nodes is " << n << "\n";
        exit(1);
    }
    std::unordered_map<uint64_t, std::pair<int, int>> nw;
    for (unsigned i = 0; i < tg->devices()->size(); i++) {
        auto d = tg->devices()->Get(i);
        if (d->type() == FlatBufTaskGraph::DeviceType_DEVICE_COMM_NW_COMM ||
            d->type() == FlatBufTaskGraph::DeviceType_DEVICE_COMM_NW_NOMINAL)
            nw[d->deviceid()] = {(int)(d->deviceproperty() / (nn + ns)), (int)(d->deviceproperty() % (nn + ns))};
    }
    std::vector<std::vector<double>> mp(n, std::vector<double>(n, 0.0));
    for (unsigned i = 0; i < tg->tasks()->size(); i++) {
        auto t = tg->tasks()->Get(i);
        if (t->type() == FlatBufTaskGraph::SimTaskType_TASK_ALLREDUCE) {
            const int g = (int)t->nexttasks()->size();
            if (g < 2) continue;
            plan.ar_bytes += 2.0 * (g - 1) * (double)t->xfersize();
            bool full = (g == n);
            for (int k = 0; full && k < g; k++) full = ((int)t->nexttasks()->Get(k) == k);
            if (full) plan.full_groups++; else plan.partial_groups++;
        } else if (t->type() == FlatBufTaskGraph::SimTaskType_TASK_NOMINAL_COMM) {
            auto it = nw.find(t->deviceid());
            if (it == nw.end()) continue;
            int a = it->second.first, b = it->second.second;
            if (a < n && b < n && a != b) { mp[a][b] += (double)t->xfersize(); plan.mp_bytes += (double)t->xfersize(); }
        }
    }
    if (plan.mp_bytes <= 0) plan.d_ar = degree;
    else if (plan.ar_bytes <= 0) plan.d_ar = 1;
    else plan.d_ar = std::min(degree - 1, std::max(1, (int)std::lround(degree * plan.ar_bytes / (plan.ar_bytes + plan.mp_bytes))));
    plan.d_mp = degree - plan.d_ar;
    std::vector<int> coprime;
    for (int p = 1; p < n; p++) if (std::gcd(p, n) == 1) coprime.push_back(p);
    for (int k = 0; k < plan.d_ar; k++) {
        double target = std::pow((double)n, (double)k / plan.d_ar);
        int best = -1;
        for (int p : coprime) {
            if (std::find(plan.strides.begin(), plan.strides.end(), p) != plan.strides.end()) continue;
            if (best < 0 || std::fabs(p - target) < std::fabs(best - target)) best = p;
        }
        if (best < 0) break;
        plan.strides.push_back(best);
    }
    plan.conn.assign((size_t)n * n, 0);
    for (int p : plan.strides)
        for (int i = 0; i < n; i++) plan.conn[(size_t)i * n + (i + p) % n]++;
    for (int p : plan.strides) plan.rings[(uint64_t)n].push_back(std::vector<int>{p});
    for (int r = 0; r < plan.d_mp; r++) {
        std::vector<int> m = topoopt_max_assignment(mp);
        for (int i = 0; i < n; i++) {
            int j = m[i];
            if (j < 0 || j == i) continue;
            plan.conn[(size_t)i * n + j]++;
            mp[i][j] /= 2.0;
        }
    }
    std::cout << "TopoOpt_ar_bytes " << plan.ar_bytes << "\nTopoOpt_mp_bytes " << plan.mp_bytes
              << "\nTopoOpt_degree_ar " << plan.d_ar << "\nTopoOpt_degree_mp " << plan.d_mp
              << "\nTopoOpt_ar_groups_full " << plan.full_groups
              << "\nTopoOpt_ar_groups_partial " << plan.partial_groups << "\nTopoOpt_strides";
    for (int p : plan.strides) std::cout << " " << p;
    std::cout << std::endl;
    return plan;
}

#endif
