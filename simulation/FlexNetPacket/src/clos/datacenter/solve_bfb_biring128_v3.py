#!/usr/bin/env python3
"""
solve_bfb_biring128_v3.py
=========================
Computes BFB AllReduce schedule for BiRing(2,8) □ UniRing(1,4)².

Graph properties:
  N=128, degree=4, diameter=10, T_B=0.992·M/B

Node index: u = a*16 + b*4 + c
  a ∈ {0..7}  (BiRing dimension,  bidirectional: +a and -a)
  b ∈ {0..3}  (UniRing₁ dimension, one directed port: +b only)
  c ∈ {0..3}  (UniRing₂ dimension, one directed port: +c only)

Key corrections over v2:
  - LP uses directed adjacency (forward edges only) for distances
  - G^T built by reversing directed edges (NOT same as G — graph is not symmetric)
  - Route computation uses bidirectional adjacency for ACK paths
  - emit_routes uses static array format for fast compilation
  - load_biring128_routes() extern function emitted for topology.cpp

Run:
  pip install scipy --break-system-packages
  python3 solve_bfb_biring128_v3.py
"""

import sys
from collections import defaultdict, deque

try:
    from scipy.optimize import linprog
except ImportError:
    sys.exit("ERROR: pip install scipy --break-system-packages")

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

N                = 128
DEG              = 4
EXPECTED_DIAMETER = 10
TB_PAPER         = 0.992

# ---------------------------------------------------------------------------
# Graph construction
# ---------------------------------------------------------------------------

def node(a, b, c):
    return a * 16 + b * 4 + c

def build_biring128():
    """
    Returns:
      adj_G  — directed forward adjacency (degree 4, used for BFB LP on G)
      adj_GT — directed transpose adjacency (used for BFB LP on G^T)
      adj_B  — bidirectional adjacency (used for route computation only)
    """
    adj_G = [[] for _ in range(N)]
    for a in range(8):
        for b in range(4):
            for c in range(4):
                u = node(a, b, c)
                adj_G[u].append(node((a + 1) % 8, b,          c))   # BiRing +a
                adj_G[u].append(node((a - 1) % 8, b,          c))   # BiRing -a
                adj_G[u].append(node(a, (b + 1) % 4,          c))   # UniRing₁ +b
                adj_G[u].append(node(a,  b,         (c + 1) % 4))   # UniRing₂ +c

    for u in range(N):
        assert len(adj_G[u]) == DEG, \
            f"node {u} has degree {len(adj_G[u])}, expected {DEG}"

    # G^T: reverse all directed edges
    adj_GT = [[] for _ in range(N)]
    for u in range(N):
        for v in adj_G[u]:
            adj_GT[v].append(u)

    for u in range(N):
        assert len(adj_GT[u]) == DEG, \
            f"G^T node {u} has in-degree {len(adj_GT[u])}, expected {DEG}"

    # Bidirectional: union of G and G^T, for route computation
    adj_B_sets = [set(adj_G[u]) for u in range(N)]
    for u in range(N):
        for v in adj_G[u]:
            adj_B_sets[v].add(u)
    adj_B = [sorted(s) for s in adj_B_sets]

    print(f"  adj_G built:  N={N}, degree={DEG}")
    print(f"  adj_GT built: N={N}, degree={DEG}")
    print(f"  adj_B built:  max_degree="
          f"{max(len(adj_B[u]) for u in range(N))}")

    return adj_G, adj_GT, adj_B

# ---------------------------------------------------------------------------
# BFS utilities
# ---------------------------------------------------------------------------

def bfs_dist(adj, src):
    dist = [-1] * N
    dist[src] = 0
    q = deque([src])
    while q:
        u = q.popleft()
        for v in adj[u]:
            if dist[v] == -1:
                dist[v] = dist[u] + 1
                q.append(v)
    assert all(d >= 0 for d in dist), \
        f"Graph not connected from {src}: {dist.count(-1)} unreachable nodes"
    return dist

def all_dists(adj):
    return [bfs_dist(adj, u) for u in range(N)]

def bfs_single_path(adj, src):
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

# ---------------------------------------------------------------------------
# BFB AllGather LP solver
# Uses directed adjacency and directed distances — same formulation as
# solve_bfb_l2k44_128_v3.py and solve_bfb_ldistreg128_v2.py
# ---------------------------------------------------------------------------

CANDIDATE_P = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096]

def solve_bfb_ag(adj, dist_all):
    """
    adj      — directed adjacency list (used to find in-neighbours)
    dist_all — all-pairs directed distances from bfs on this adj
    """
    D = max(max(d) for d in dist_all)
    print(f"  Diameter = {D}")

    # Build in-neighbours from directed adj
    in_nbr = [[] for _ in range(N)]
    for u in range(N):
        for v in adj[u]:
            in_nbr[v].append(u)

    steps     = [[] for _ in range(D)]
    all_fracs = []

    for t in range(1, D + 1):
        print(f"  Solving LP step t={t} ...")
        for u in range(N):
            origins = [v for v in range(N)
                       if v != u and dist_all[v][u] == t]
            if not origins:
                continue

            elig = {v: [w for w in in_nbr[u]
                        if dist_all[v][w] == t - 1]
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
    P = CANDIDATE_P[-1]
    for candidate in CANDIDATE_P:
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

    return int_steps, P


def derive_rs_from_ag(ag_steps):
    """RS on G = time-reverse of AG on G^T."""
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
        "// biring128_routes_generated.cpp",
        "// Auto-generated by solve_bfb_biring128_v3.py — DO NOT EDIT",
        "//",
        "// Routes stored as flat static array for fast compilation.",
        "// Format: src, dst, hop0, hop1, ..., -1 (path sentinel)",
        "// Table ends with -2.",
        "//",
        "// Call load_biring128_routes(_routes) from load_bfb_routes().",
        "",
        "#include <vector>",
        "#include <unordered_map>",
        "#include <cstdint>",
        "",
        "static const int _biring128_route_table[] = {",
    ]

    for src in range(N):
        for dst, path in routes[src].items():
            hops = ", ".join(str(h) for h in path)
            lines.append(f"    {src}, {dst}, {hops}, -1,")

    lines += [
        "    -2  // end of table",
        "};",
        "",
        "void load_biring128_routes(",
        "    std::unordered_map<uint64_t, std::vector<std::vector<int>>>& _routes)",
        "{",
        f"    constexpr int N = {N};",
        "    const int* p = _biring128_route_table;",
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
        "// biring128_schedule_generated.cpp",
        "// Auto-generated by solve_bfb_biring128_v3.py — DO NOT EDIT",
        f"// BiRing128: N=128, d=4, diameter={D}, T_B=0.992*M/B",
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
    print("Building BiRing(2,8) □ UniRing(1,4)² ...")
    adj_G, adj_GT, adj_B = build_biring128()

    # Verify directed diameter
    print("\nComputing directed distances on G ...")
    dist_G = all_dists(adj_G)
    diam_G = max(max(d) for d in dist_G)
    print(f"  Directed diameter of G  = {diam_G}")
    assert diam_G == EXPECTED_DIAMETER, \
        f"Expected diameter {EXPECTED_DIAMETER}, got {diam_G}"

    print("Computing directed distances on G^T ...")
    dist_GT = all_dists(adj_GT)
    diam_GT = max(max(d) for d in dist_GT)
    print(f"  Directed diameter of G^T = {diam_GT}")
    assert diam_GT == EXPECTED_DIAMETER, \
        f"Expected G^T diameter {EXPECTED_DIAMETER}, got {diam_GT}"

    # Routes use bidirectional adjacency
    print("\nComputing BFS routes (bidirectional for ACK paths) ...")
    routes   = {src: bfs_single_path(adj_B, src) for src in range(N)}
    n_routes = sum(len(v) for v in routes.values())
    assert n_routes == N * (N - 1), \
        f"Expected {N*(N-1)} routes, got {n_routes}"
    print(f"  {n_routes} paths computed ✓")

    # Solve LP on G using directed distances
    print(f"\nSolving BFB AllGather LP on G ({EXPECTED_DIAMETER} steps × {N} receivers)...")
    print("  Note: this may take 10-20 minutes for N=128, D=10")
    ag_G, P_ag_G = solve_bfb_ag(adj_G, dist_G)
    print(f"  AG(G):  steps={len(ag_G)}, "
          f"txns={[len(s) for s in ag_G]}, P={P_ag_G}")

    # Solve LP on G^T using G^T directed distances
    print(f"\nSolving BFB AllGather LP on G^T ...")
    ag_GT, P_ag_GT = solve_bfb_ag(adj_GT, dist_GT)
    print(f"  AG(GT): steps={len(ag_GT)}, "
          f"txns={[len(s) for s in ag_GT]}, P={P_ag_GT}")

    # Derive RS from AG on transpose
    rs_G    = derive_rs_from_ag(ag_GT)
    P_rs_G  = P_ag_GT
    rs_GT   = derive_rs_from_ag(ag_G)
    P_rs_GT = P_ag_G

    print(f"\n  RS(G):  txns={[len(s) for s in rs_G]} (derived from AG on G^T)")
    print(f"  RS(GT): txns={[len(s) for s in rs_GT]} (derived from AG on G)")

    print("\nWriting biring128_routes_generated.cpp ...")
    code = emit_routes(routes, N)
    with open("biring128_routes_generated.cpp", "w") as f:
        f.write(code)
    print(f"  {len(code.splitlines())} lines written")

    print("Writing biring128_schedule_generated.cpp ...")
    code = emit_schedule(ag_G,  P_ag_G,  rs_G,  P_rs_G,
                         ag_GT, P_ag_GT, rs_GT, P_rs_GT,
                         EXPECTED_DIAMETER)
    with open("biring128_schedule_generated.cpp", "w") as f:
        f.write(code)
    print(f"  {len(code.splitlines())} lines written")

    # Theoretical bandwidth check
    M          = 1073741824   # 1 GB
    B          = 200e9 / 8    # 200 Gbps in bytes/s
    T_bw_paper = TB_PAPER * M / B
    print(f"\n=== Theoretical predictions (M=1GB, B=200Gbps) ===")
    print(f"  T_B paper (BiRing128) = {T_bw_paper*1e3:.3f} ms")

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
