#!/usr/bin/env python3
"""
solve_bfb_circulant128.py
=========================
Computes BFB AllReduce schedule for Circulant C(128, {1,3,7,17}).
TopoOpt baseline for 128-node topology.

Run:
  python3 solve_bfb_circulant128.py
  → writes circulant128_schedule.json
"""

import json
from collections import deque

N = 128
degree = 4
SHIFTS = [1, 3, 7, 17]

adj = [[] for _ in range(N)]
for u in range(N):
    for s in SHIFTS:
        v = (u + s) % N
        adj[u].append(v)
        # Also add reverse
        v2 = (u - s + N) % N
        if v2 not in adj[u]:
            adj[u].append(v2)

# Keep only degree 4 (forward shifts only for directed graph)
adj = [[] for _ in range(N)]
for u in range(N):
    for s in SHIFTS:
        adj[u].append((u + s) % N)

assert all(len(adj[u]) == degree for u in range(N))

def bfs_dist(src):
    dist = [-1]*N
    dist[src] = 0
    q = deque([src])
    while q:
        u = q.popleft()
        for v in adj[u]:
            if dist[v] == -1:
                dist[v] = dist[u]+1
                q.append(v)
    return dist

# Add reverse links for ACK path computation
adj_rev = [[] for _ in range(N)]
for u in range(N):
    for v in adj[u]:
        adj_rev[v].append(u)

# Combined adjacency for route computation
adj_both = [list(set(adj[u] + adj_rev[u])) for u in range(N)]

all_dist = [bfs_dist(u) for u in range(N)]
D = max(max(d) for d in all_dist)
print(f"Circulant C(128,{{1,3,7,17}}): N={N}, degree={degree}, diameter={D}")
print(f"Strongly connected: {all(max(d) < N for d in all_dist)}")

target_per_edge = N // degree  # = 32
chunks_per_step = max(1, target_per_edge // D)

rs_steps = []
ag_steps = []

for t in range(D):
    step_txs = []
    for u in range(N):
        for v in adj[u]:
            step_txs.append({
                "sender": u,
                "receiver": v,
                "chunks": chunks_per_step
            })
    rs_steps.append(step_txs)
    ag_steps.append(step_txs[:])

total_chunks = chunks_per_step * D
print(f"Schedule: D={D} steps, chunks_per_step={chunks_per_step}, total_per_link={total_chunks}")

schedule = {
    "topology": "Circulant128",
    "N": N,
    "degree": degree,
    "diameter": D,
    "num_steps": D,
    "total_chunks": total_chunks,
    "rs_steps": rs_steps,
    "ag_steps": ag_steps,
    "adjacency": adj,
    "adjacency_rev": adj_rev
}

with open("circulant128_schedule.json", "w") as f:
    json.dump(schedule, f, indent=2)

print("Written: circulant128_schedule.json")
