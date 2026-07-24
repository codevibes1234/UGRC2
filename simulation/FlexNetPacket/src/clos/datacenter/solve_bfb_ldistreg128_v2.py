#!/usr/bin/env python3
"""
solve_bfb_ldistreg128.py
========================
Computes BFB AllReduce schedule for L(DistReg(4,32)) — 128-node, degree-4.

Construction (directed throughout, per paper Definition 12):
  DistReg(4,32): 32-node, degree-4 distance-regular graph
                 "Incidence graph of AG(4,4) minus parallel class"
                 Intersection array: {4,3,3,1; 1,1,3,4}
                 Hardcoded adjacency from distance-regular.org
  L(DistReg):   Line graph of directed version of DistReg(4,32)
                Each undirected edge {u,v} → two directed edges (u,v) and (v,u)
                Each directed edge becomes one node in L(DistReg)
                Two nodes are adjacent if their edges share a head-to-tail endpoint
                → 128 nodes, degree 4, diameter 5, T_B = 1.000*M/B

Properties (from paper Table 7):
  N=128, degree=4, diameter D(G)=5
  T_L = 5*alpha, T_B = 1.000*M/B
  Reverse-symmetric: G ≅ G^T

Outputs:
  ldistreg128_routes_generated.cpp
  ldistreg128_schedule_generated.cpp

Run:
  pip install scipy --break-system-packages
  python3 solve_bfb_ldistreg128.py
"""

import sys
from collections import defaultdict, deque

try:
    from scipy.optimize import linprog
except ImportError:
    sys.exit("ERROR: pip install scipy --break-system-packages")

# ---------------------------------------------------------------------------
# DistReg(4,32) — hardcoded adjacency from distance-regular.org
# "Incidence graph of AG(4,4) minus parallel class"
# Intersection array: {4,3,3,1; 1,1,3,4}
# ---------------------------------------------------------------------------

N_DR  = 32
DEG   = 4
LDISTREG128_D = 5  # diameter of L(DistReg(4,32)), per paper Table 7

ADJ_DR = [
    [16, 17, 18, 19],  #  0
    [20, 21, 22, 23],  #  1
    [24, 25, 26, 27],  #  2
    [28, 29, 30, 31],  #  3
    [16, 20, 24, 28],  #  4
    [17, 21, 25, 29],  #  5
    [18, 22, 26, 30],  #  6
    [19, 23, 27, 31],  #  7
    [16, 22, 27, 29],  #  8
    [19, 21, 24, 30],  #  9
    [17, 23, 26, 28],  # 10
    [18, 20, 25, 31],  # 11
    [16, 23, 25, 30],  # 12
    [18, 21, 27, 28],  # 13
    [19, 20, 26, 29],  # 14
    [17, 22, 24, 31],  # 15
    [ 0,  4,  8, 12],  # 16
    [ 0,  5, 10, 15],  # 17
    [ 0,  6, 11, 13],  # 18
    [ 0,  7,  9, 14],  # 19
    [ 1,  4, 11, 14],  # 20
    [ 1,  5,  9, 13],  # 21
    [ 1,  6,  8, 15],  # 22
    [ 1,  7, 10, 12],  # 23
    [ 2,  4,  9, 15],  # 24
    [ 2,  5, 11, 12],  # 25
    [ 2,  6, 10, 14],  # 26
    [ 2,  7,  8, 13],  # 27
    [ 3,  4, 10, 13],  # 28
    [ 3,  5,  8, 14],  # 29
    [ 3,  6,  9, 12],  # 30
    [ 3,  7, 11, 15],  # 31
]

# ---------------------------------------------------------------------------
# Build L(DistReg(4,32)) — directed line graph
# ---------------------------------------------------------------------------

def build_ldistreg128():
    # Verify DistReg(4,32) symmetry
    for u in range(N_DR):
        assert len(ADJ_DR[u]) == DEG, f"node {u} has degree {len(ADJ_DR[u])}"
        for v in ADJ_DR[u]:
            assert u in ADJ_DR[v], f"DistReg not symmetric: {u}->{v}"
    print(f"  DistReg(4,32): {N_DR} nodes, degree {DEG} ✓")

    # Build directed edge list: each undirected edge {u,v} → (u,v) and (v,u)
    # To get exactly 128 directed edges, enumerate (u,v) for each u and
    # each v in ADJ_DR[u]. Since the graph is undirected and symmetric,
    # this gives 32 * 4 = 128 directed edges.
    edges = []
    edge_index = {}
    for u in range(N_DR):
        for v in ADJ_DR[u]:
            edge_index[(u, v)] = len(edges)
            edges.append((u, v))

    N = len(edges)
    assert N == 128, f"Expected 128 directed edges, got {N}"

    # Build directed L(DistReg): node i = directed edge i
    # Edge i -> j if head(edge_i) == tail(edge_j) and i != j
    # i.e. edge (u,v) -> edge (v,w) for all w in ADJ_DR[v], w != u
    # (w != u because (v,u) would be the reverse edge, which creates
    #  a self-loop in the line graph — excluded per Definition 12)
    adj = defaultdict(list)
    for i, (u, v) in enumerate(edges):
        for w in ADJ_DR[v]:
            j = edge_index[(v, w)]
            if j != i:
                adj[i].append(j)

    # Each node should have out-degree 4
    # (v has 4 neighbours in DistReg; one of them is u (reverse), leaving 3?
    #  No — since DistReg is undirected, ADJ_DR[v] contains u.
    #  edge_index[(v,u)] is a valid directed edge, so j = edge_index[(v,u)] != i
    #  because i = edge_index[(u,v)] and (u,v) != (v,u).
    #  So all 4 out-edges of v in the directed graph become neighbours of i,
    #  giving out-degree exactly 4.)
    for i in range(N):
        assert len(adj[i]) == DEG, \
            f"L(DistReg) node {i} has out-degree {len(adj[i])}, expected {DEG}"

    print(f"  L(DistReg(4,32)): {N} nodes, degree {DEG} ✓")
    return adj, edges, N

# ---------------------------------------------------------------------------
# BFS utilities
# ---------------------------------------------------------------------------

def bfs_dist(adj, src, N):
    d = {src: 0}
    q = deque([src])
    while q:
        u = q.popleft()
        for v in adj[u]:
            if v not in d:
                d[v] = d[u] + 1
                q.append(v)
    assert len(d) == N, \
        f"Graph not connected from {src}: reached {len(d)}/{N} nodes"
    return d

def all_dists(adj, N):
    return {u: bfs_dist(adj, u, N) for u in range(N)}

def bfs_single_path(adj, src, N):
    """BFS shortest path tree from src."""
    parent = {src: None}
    q = deque([src])
    while q:
        u = q.popleft()
        for v in adj[u]:
            if v not in parent:
                parent[v] = u
                q.append(v)
    paths = {}
    for dst in range(N):
        if dst == src:
            continue
        path, cur = [], dst
        while cur is not None:
            path.append(cur)
            cur = parent.get(cur)
        path.reverse()
        if path and path[0] == src:
            paths[dst] = path
    return paths

def transpose(adj, N):
    """Reverse all edges."""
    t = defaultdict(list)
    for u in range(N):
        for v in adj[u]:
            if u not in t[v]:
                t[v].append(u)
    return t

def bidir(adj, N):
    """Make bidirectional — for route computation so ACK paths exist."""
    b = defaultdict(list)
    for u in range(N):
        for v in adj[u]:
            if v not in b[u]: b[u].append(v)
            if u not in b[v]: b[v].append(u)
    return b

# ---------------------------------------------------------------------------
# BFB AllGather LP solver — identical to solve_bfb_l2k44_128_v3.py
# ---------------------------------------------------------------------------

def solve_bfb_ag(adj, N):
    df = all_dists(adj, N)
    D  = max(max(d.values()) for d in df.values())
    print(f"  Diameter = {D}")

    in_nbr = defaultdict(list)
    for u in range(N):
        for v in adj[u]:
            in_nbr[v].append(u)

    steps     = [[] for _ in range(D)]
    all_fracs = []

    for t in range(1, D + 1):
        print(f"  Solving LP step t={t} ...")
        for u in range(N):
            origins = [v for v in range(N)
                       if v != u and df[v].get(u) == t]
            if not origins:
                continue

            elig = {v: [w for w in in_nbr[u]
                        if df[v].get(w) == t - 1]
                    for v in origins}

            vw = [(v, w) for v in origins for w in elig[v]]
            if not vw:
                continue

            nvar  = len(vw) + 1
            U_idx = nvar - 1
            idx   = {pair: i for i, pair in enumerate(vw)}

            c_obj        = [0.0] * nvar
            c_obj[U_idx] = 1.0

            A_eq, b_eq = [], []
            for v in origins:
                row = [0.0] * nvar
                for w in elig[v]:
                    row[idx[(v, w)]] = 1.0
                A_eq.append(row)
                b_eq.append(1.0)

            A_ub, b_ub = [], []
            for w in in_nbr[u]:
                row = [0.0] * nvar
                for v in origins:
                    if (v, w) in idx:
                        row[idx[(v, w)]] = 1.0
                row[U_idx] = -1.0
                A_ub.append(row)
                b_ub.append(0.0)

            bounds = [(0.0, 1.0)] * (nvar - 1) + [(0.0, None)]

            res = linprog(c_obj, A_ub=A_ub, b_ub=b_ub,
                          A_eq=A_eq, b_eq=b_eq,
                          bounds=bounds, method='highs')

            if not res.success:
                print(f"    WARNING: LP failed u={u} t={t}: {res.message}")
                for v in origins:
                    for w in elig[v]:
                        frac = 1.0 / len(elig[v])
                        steps[t - 1].append((w, u, v, frac))
                        all_fracs.append(frac)
                continue

            x = res.x
            for (v, w), i in idx.items():
                frac = x[i]
                if frac > 1e-6:
                    steps[t - 1].append((w, u, v, frac))
                    all_fracs.append(frac)

    # Find minimum P for clean discretisation (error < 1%)
    P = 4
    for candidate in [4, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256]:
        err = max((abs(round(candidate * f) - candidate * f)
                   for f in all_fracs), default=0.0)
        if err < 0.01:
            P = candidate
            break

    print(f"  Discretisation: P={P}")

    int_steps = []
    for step in steps:
        int_step = []
        for (w, u, v, frac) in step:
            chunks = max(1, round(P * frac))
            int_step.append((w, u, v, chunks))
        int_steps.append(int_step)

    return int_steps, P, D


def derive_rs_from_ag(ag_steps):
    """RS on G = reverse of AG on G^T."""
    rs = []
    for t in range(len(ag_steps) - 1, -1, -1):
        rs.append([(rcv, snd, orig, chunks)
                   for (snd, rcv, orig, chunks) in ag_steps[t]])
    return rs

# ---------------------------------------------------------------------------
# C++ emitters — static array format for fast compilation
# ---------------------------------------------------------------------------

def emit_routes(routes, N):
    lines = [
        "// ldistreg128_routes_generated.cpp",
        "// Auto-generated by solve_bfb_ldistreg128.py — DO NOT EDIT",
        "//",
        "// Routes stored as flat static array for fast compilation.",
        "// Format: src, dst, hop0, hop1, ..., -1 (path sentinel)",
        "// Table ends with -2.",
        "//",
        "// Call load_ldistreg128_routes(_routes) from load_bfb_routes().",
        "",
        "#include <vector>",
        "#include <unordered_map>",
        "#include <cstdint>",
        "",
        "static const int _ldistreg128_route_table[] = {",
    ]

    for src in range(N):
        for dst, path in routes[src].items():
            hops = ", ".join(str(h) for h in path)
            lines.append(f"    {src}, {dst}, {hops}, -1,")

    lines += [
        "    -2  // end of table",
        "};",
        "",
        "void load_ldistreg128_routes(",
        "    std::unordered_map<uint64_t, std::vector<std::vector<int>>>& _routes)",
        "{",
        f"    constexpr int N = {N};",
        "    const int* p = _ldistreg128_route_table;",
        "    while (*p != -2) {",
        "        int src = *p++;",
        "        int dst = *p++;",
        "        std::vector<int> path;",
        "        while (*p != -1) path.push_back(*p++);",
        "        ++p;  // skip -1 sentinel",
        "        _routes[(uint64_t)src * N + dst].push_back(std::move(path));",
        "    }",
        "}",
        "",
    ]
    return "\n".join(lines) + "\n"


def emit_schedule(ag_G, P_ag_G, rs_G, P_rs_G,
                  ag_GT, P_ag_GT, rs_GT, P_rs_GT, D):
    lines = [
        "// ldistreg128_schedule_generated.cpp",
        "// Auto-generated by solve_bfb_ldistreg128.py — DO NOT EDIT",
        f"// L(DistReg(4,32)): N=128, d=4, diameter={D}, T_B=1.000*M/B",
        "",
        f"_bfb_ag_total_chunks_on_G  = {P_ag_G};",
        f"_bfb_rs_total_chunks_on_G  = {P_rs_G};",
        f"_bfb_ag_total_chunks_on_GT = {P_ag_GT};",
        f"_bfb_rs_total_chunks_on_GT = {P_rs_GT};",
        "",
        f"_bfb_ag_steps_on_G.resize({len(ag_G)});",
        f"_bfb_rs_steps_on_G.resize({len(rs_G)});",
        f"_bfb_ag_steps_on_GT.resize({len(ag_GT)});",
        f"_bfb_rs_steps_on_GT.resize({len(rs_GT)});",
        "",
    ]

    def emit_array(name, steps):
        lines.append(f"// {name}")
        for t, step in enumerate(steps):
            for (snd, rcv, orig, chunks) in step:
                lines.append(
                    f"_{name}[{t}].push_back("
                    f"{{/*sender=*/{snd},/*receiver=*/{rcv},"
                    f"/*origin=*/{orig},/*chunks=*/{chunks}}});")
        lines.append("")

    emit_array("bfb_ag_steps_on_G",  ag_G)
    emit_array("bfb_rs_steps_on_G",  rs_G)
    emit_array("bfb_ag_steps_on_GT", ag_GT)
    emit_array("bfb_rs_steps_on_GT", rs_GT)

    return "\n".join(lines) + "\n"

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    print("Building L(DistReg(4,32)) ...")
    adj_G, edges, N = build_ldistreg128()

    # Verify diameter matches paper (Table 7: D=5)
    dist0 = bfs_dist(adj_G, 0, N)
    diam  = max(dist0.values())
    print(f"  Directed diameter from node 0: {diam}")
    # Full diameter check
    df = all_dists(adj_G, N)
    full_diam = max(max(d.values()) for d in df.values())
    print(f"  Full directed diameter: {full_diam}")
    assert full_diam == LDISTREG128_D, \
        f"Expected diameter {LDISTREG128_D}, got {full_diam}. " \
        f"Check base graph construction."

    # Transpose
    adj_GT = transpose(adj_G, N)
    dist0_T = bfs_dist(adj_GT, 0, N)
    diam_T  = max(dist0_T.values())
    print(f"  G^T diameter: {diam_T}")

    # Bidirectional for route computation
    adj_B = bidir(adj_G, N)

    print("\nComputing BFS routes (bidirectional for ACK paths) ...")
    routes   = {src: bfs_single_path(adj_B, src, N) for src in range(N)}
    n_routes = sum(len(v) for v in routes.values())
    assert n_routes == N * (N - 1), \
        f"Expected {N*(N-1)} routes, got {n_routes}"
    print(f"  {n_routes} paths computed ✓")

    print("\nSolving BFB AllGather LP on G ...")
    ag_G, P_ag_G, D = solve_bfb_ag(adj_G, N)
    print(f"  AG(G):  steps={len(ag_G)}, txns={[len(s) for s in ag_G]}, P={P_ag_G}")

    print("\nSolving BFB AllGather LP on G^T ...")
    ag_GT, P_ag_GT, _ = solve_bfb_ag(adj_GT, N)
    print(f"  AG(GT): steps={len(ag_GT)}, txns={[len(s) for s in ag_GT]}, P={P_ag_GT}")

    # RS derived from AG on transpose
    rs_G    = derive_rs_from_ag(ag_GT)
    P_rs_G  = P_ag_GT
    rs_GT   = derive_rs_from_ag(ag_G)
    P_rs_GT = P_ag_G

    print(f"\n  RS(G):  txns={[len(s) for s in rs_G]} (derived from AG on G^T)")
    print(f"  RS(GT): txns={[len(s) for s in rs_GT]} (derived from AG on G)")

    print("\nWriting ldistreg128_routes_generated.cpp ...")
    code = emit_routes(routes, N)
    with open("ldistreg128_routes_generated.cpp", "w") as f:
        f.write(code)
    print(f"  {len(code.splitlines())} lines written")

    print("Writing ldistreg128_schedule_generated.cpp ...")
    code = emit_schedule(ag_G,  P_ag_G,  rs_G,  P_rs_G,
                         ag_GT, P_ag_GT, rs_GT, P_rs_GT, D)
    with open("ldistreg128_schedule_generated.cpp", "w") as f:
        f.write(code)
    print(f"  {len(code.splitlines())} lines written")

    # Theoretical bandwidth check
    M          = 1073741824   # 1 GB
    B          = 200e9 / 8    # 200 Gbps in bytes/s
    T_bw_paper = 1.000 * M / B
    print(f"\n=== Theoretical predictions (M=1GB, B=200Gbps) ===")
    print(f"  T_B paper (L(DistReg(4,32))) = {T_bw_paper*1e3:.3f} ms")

    shard      = M / N
    step_loads = []
    for t, step in enumerate(rs_G):
        unique_links = {}
        for (snd, rcv, orig, chunks) in step:
            k = (snd, rcv)
            unique_links[k] = unique_links.get(k, 0) + chunks
        max_chunks = max(unique_links.values()) if unique_links else 0
        P          = P_rs_G
        max_bytes  = max_chunks * shard / P
        step_time  = max_bytes / B
        step_loads.append(step_time * 1e3)
        print(f"  RS step {t}: max_link_load={max_chunks}/{P} "
              f"= {max_bytes/1e6:.2f} MB → {step_time*1e3:.3f} ms")

    rs_bound = sum(step_loads)
    print(f"  RS bound            = {rs_bound:.3f} ms")
    print(f"  Total AllReduce BW  = {2*rs_bound:.3f} ms")
    print(f"  T_B* paper optimal  = {T_bw_paper*1e3:.3f} ms")
    print(f"\nDone.")


if __name__ == "__main__":
    main()
