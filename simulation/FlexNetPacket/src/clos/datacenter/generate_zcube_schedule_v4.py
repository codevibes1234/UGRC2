#!/usr/bin/env python3
"""
generate_zcube_schedule.py
==========================
Generates a ForestColl AllReduce schedule for ZCube(4,2) with 16 GPUs
(1 GPU per server, 16 servers total).

Topology: ZCube(4,2)
  - 16 GPU nodes, integer IDs 0..15
  - 4 L0 switches:  IDs 16..19
      GPU g connects to switch 16 + (g // 4)
  - 4 L1 switches:  IDs 20..23
      GPU g connects to switch 20 + (g % 4)
  - All L0-L1 switch pairs fully connected (4x4=16 pairs, both directions)
  - All links at bandwidth nic_gbps / 8  GB/s
  - No intra-server NVLink

ALL nodes are plain Python ints so that ForestColl's sorted() calls never
hit the 'str < int' TypeError (fixed from v2/v3).

Output:
  zcube_4_2_schedule.xml           -- ForestColl spanning-tree XML (debug)
  zcube_fc_schedule_generated.cpp  -- C++ static arrays for htsim

Usage:
  python3 generate_zcube_schedule.py [--nic-gbps 200] [--fixed-k K]

Requirements:
  pip install networkx --break-system-packages
  pipeline_allgather.py in the same directory (ForestColl source)
"""

import argparse
import xml.etree.ElementTree as ET
from collections import defaultdict

import networkx as nx

from pipeline_allgather import optimal_pipeline_spanning_trees, spanning_trees_to_xml


# ─────────────────────────────────────────────────────────────────────────────
# Node ID layout
# ─────────────────────────────────────────────────────────────────────────────
#
#   GPU nodes   :  0 .. 15    (g)
#   L0 switches :  16 .. 19   (16 + g//4)
#   L1 switches :  20 .. 23   (20 + g%4)
#
# All integers → sorted() works everywhere inside ForestColl.

N_GPU      = 16
L0_BASE    = 16          # L0_0=16, L0_1=17, L0_2=18, L0_3=19
L1_BASE    = 20          # L1_0=20, L1_1=21, L1_2=22, L1_3=23
N_L0       = 4
N_L1       = 4

def l0(i: int) -> int: return L0_BASE + i
def l1(i: int) -> int: return L1_BASE + i


# ─────────────────────────────────────────────────────────────────────────────
# STEP 1  Build ZCube(4,2) topology
# ─────────────────────────────────────────────────────────────────────────────

def build_zcube_topology(nic_gbps: float = 200.0) -> tuple[nx.DiGraph, list[int]]:
    """
    Build the ZCube(4,2) directed graph for ForestColl.

    All nodes are plain ints:
      GPU g      -> node g          (0..15)
      L0 switch  -> node 16 + g//4  (16..19)
      L1 switch  -> node 20 + g%4   (20..23)

    Edges (all bidirectional, capacity = nic_gbps/8 GB/s):
      GPU g  <->  L0 switch  16 + g//4
      GPU g  <->  L1 switch  20 + g%4
      L0 i   <->  L1 j        for all i in 0..3, j in 0..3

    Total directed edges:
      GPU-switch : 16 GPUs x 2 NICs x 2 directions = 64
      L0-L1      : 4 x 4 x 2 directions            = 32
      Total                                         = 96
    """
    bw = nic_gbps / 8.0      # GB/s

    G = nx.DiGraph()
    compute_nodes = list(range(N_GPU))

    # Add all nodes explicitly
    G.add_nodes_from(compute_nodes)
    G.add_nodes_from(range(L0_BASE, L0_BASE + N_L0))  # 16..19
    G.add_nodes_from(range(L1_BASE, L1_BASE + N_L1))  # 20..23

    # GPU <-> L0 and GPU <-> L1
    for g in range(N_GPU):
        sw0 = l0(g // 4)
        sw1 = l1(g % 4)
        for sw in (sw0, sw1):
            G.add_edge(g,  sw, capacity=bw)
            G.add_edge(sw, g,  capacity=bw)

    # L0 <-> L1 full bipartite
    for i in range(N_L0):
        for j in range(N_L1):
            G.add_edge(l0(i), l1(j), capacity=bw)
            G.add_edge(l1(j), l0(i), capacity=bw)

    # Sanity checks
    for g in compute_nodes:
        sw_nbrs = [n for n in G.neighbors(g) if n >= L0_BASE]
        assert len(sw_nbrs) == 2, \
            f"GPU {g} has {len(sw_nbrs)} switch neighbours, expected 2"

    n_edge = G.number_of_edges()
    assert n_edge == 96, f"Expected 96 directed edges, got {n_edge}"

    print(f"  Topology: {N_GPU} GPUs + {N_L0+N_L1} switches = "
          f"{G.number_of_nodes()} nodes, {n_edge} directed edges")
    print(f"  Link bandwidth: {bw} GB/s  ({nic_gbps} Gb/s)")
    print(f"  GPU nodes: 0..{N_GPU-1}")
    print(f"  L0 switch nodes: {L0_BASE}..{L0_BASE+N_L0-1}  "
          f"(L0_i = 16+i, connects GPUs in group i)")
    print(f"  L1 switch nodes: {L1_BASE}..{L1_BASE+N_L1-1}  "
          f"(L1_j = 20+j, connects GPUs on rail j)")

    return G, compute_nodes


# ─────────────────────────────────────────────────────────────────────────────
# STEP 2  Run ForestColl
# ─────────────────────────────────────────────────────────────────────────────

def run_forestcoll(
    G: nx.DiGraph,
    compute_nodes: list[int],
    fixed_k: int | None = None,
) -> tuple[float, int, dict, dict]:
    """
    Run ForestColl's optimal_pipeline_spanning_trees on G.

    fixed_k : if not None, use ForestColl's fixed-k binary search.
              None (default) → unconstrained optimal k.
    """
    if fixed_k is not None:
        print(f"  Running ForestColl with fixed_K={fixed_k} ...")
    else:
        print("  Running ForestColl (unconstrained k) ...")
    print("  (May take several minutes for 16 GPU + 8 switch nodes.)")

    (U, k), (Ts, Cs) = optimal_pipeline_spanning_trees(
        G,
        compute_nodes=compute_nodes,
        fixed_K=fixed_k,
    )

    N      = len(compute_nodes)
    algbw  = N * k / U
    bw_per_gpu = 200.0 / 8 * 2   # 2 NICs
    print(f"  U={U:.6f}  k={k}  theoretical algbw = {algbw:.4f} GB/s")
    print(f"  Per-GPU total NIC bandwidth = {bw_per_gpu:.1f} GB/s")
    print(f"  Trees: {len(Ts)}  (expect {N}×{k} = {N*k})")

    return U, k, Ts, Cs


# ─────────────────────────────────────────────────────────────────────────────
# STEP 3  Parse XML → C++
# ─────────────────────────────────────────────────────────────────────────────

def _bfs_depths(tree_el: ET.Element) -> dict[str, int]:
    """BFS depth of every src node from the tree root (root = depth 0)."""
    root_name = tree_el.attrib["root"]
    children: dict[str, list[str]] = defaultdict(list)
    for send in tree_el.findall("send"):
        children[send.attrib["src"]].append(send.attrib["dst"])

    depth = {root_name: 0}
    queue = [root_name]
    while queue:
        node = queue.pop(0)
        for child in children[node]:
            if child not in depth:
                depth[child] = depth[node] + 1
                queue.append(child)
    return depth


def xml_to_cpp(xml_string: str, output_path: str) -> tuple[int, int]:
    """
    Parse ForestColl XML and write C++ static arrays.

    AG (AllGather)  : root sends down at step = sender's BFS depth.
    RS (ReduceScatter): leaves send up at step = tree_height - sender's depth.
      Direction is reversed: AG (src->dst) becomes RS (dst->src).

    num_steps = global max_height + 1   (depth levels 0 .. max_height).
    """
    root_el        = ET.fromstring(xml_string)
    nchunkspernode = int(root_el.attrib["nchunkspernode"])
    N              = N_GPU
    total_chunks   = N * nchunkspernode

    max_height = 0
    ag_steps: dict[int, list] = defaultdict(list)
    rs_steps: dict[int, list] = defaultdict(list)

    trees = root_el.findall("tree")
    print(f"  Parsing {len(trees)} trees ...")

    for tree in trees:
        height  = int(tree.attrib["height"])
        nchunks = int(tree.attrib["nchunks"])
        depths  = _bfs_depths(tree)
        max_height = max(max_height, height)

        for send in tree.findall("send"):
            src_name = send.attrib["src"]
            dst_name = send.attrib["dst"]
            # Nodes are integers stored as strings in XML attributes
            src_id   = int(src_name)
            dst_id   = int(dst_name)
            d        = depths.get(src_name, 0)

            ag_steps[d].append((src_id, dst_id, nchunks))
            rs_steps[height - d].append((dst_id, src_id, nchunks))

    num_steps = max_height + 1   # depth levels 0 .. max_height inclusive

    print(f"  max_height={max_height}  num_steps={num_steps}  "
          f"total_chunks={total_chunks}")
    print(f"  AG sends/step: { {s: len(v) for s, v in sorted(ag_steps.items())} }")
    print(f"  RS sends/step: { {s: len(v) for s, v in sorted(rs_steps.items())} }")

    lines = [
        "// AUTO-GENERATED by generate_zcube_schedule.py — DO NOT EDIT",
        "//",
        "// ForestColl AllGather + ReduceScatter schedule for ZCube(4,2)",
        f"// N={N}  nchunkspernode={nchunkspernode}  "
        f"total_chunks={total_chunks}  num_steps={num_steps}",
        "//",
        "// GPU IDs 0..15  (1 GPU per server, linear indexing)",
        "//",
        "// AG: root sends down the tree.  step d = nodes at depth d transmit.",
        "// RS: leaves aggregate up.       step 0 = deepest leaves send.",
        "//",
        "// #included inside ZCubeTopology::load_fc_schedule().",
        "",
        f"static const int _gen_fc_num_steps       = {num_steps};",
        f"static const int _gen_fc_rs_total_chunks = {total_chunks};",
        f"static const int _gen_fc_ag_total_chunks = {total_chunks};",
        "",
    ]

    for phase, steps in [("rs", rs_steps), ("ag", ag_steps)]:
        lines.append(
            f"static const std::vector<std::vector<ZCubeFCTransmission>>"
            f" _gen_fc_{phase}_steps = {{"
        )
        for step in range(num_steps):
            txs = steps.get(step, [])
            lines.append(f"    // {phase.upper()} step {step}: {len(txs)} transmissions")
            lines.append("    {")
            for snd, rcv, nc in txs:
                lines.append(
                    f"        {{ {snd:2d}, {rcv:2d}, {nc} }},"
                    f"  // GPU{snd} -> GPU{rcv}  ({nc} chunk(s))"
                )
            lines.append("    },")
        lines += ["};", ""]

    with open(output_path, "w") as f:
        f.write("\n".join(lines))

    print(f"  Written: {output_path}")
    return num_steps, total_chunks


# ─────────────────────────────────────────────────────────────────────────────
# MAIN
# ─────────────────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate ForestColl schedule for ZCube(4,2)"
    )
    parser.add_argument(
        "--nic-gbps", type=float, default=200.0,
        help="Per-NIC bandwidth in Gb/s (default: 200)"
    )
    parser.add_argument(
        "--fixed-k", type=int, default=None,
        help="Fix trees-per-root (default: ForestColl decides). "
             "Use only if optimal k is impractically large."
    )
    args = parser.parse_args()

    sep = "=" * 60
    print(sep)
    print("ZCube(4,2) ForestColl Schedule Generator")
    print(sep)

    print("\n[1/3] Building ZCube(4,2) topology ...")
    G, compute_nodes = build_zcube_topology(nic_gbps=args.nic_gbps)

    print("\n[2/3] Running ForestColl ...")
    U, k, Ts, Cs = run_forestcoll(G, compute_nodes, fixed_k=args.fixed_k)

    xml_string = spanning_trees_to_xml(Ts, Cs, k)
    with open("zcube_4_2_schedule.xml", "w") as f:
        f.write(xml_string)
    print("  Saved: zcube_4_2_schedule.xml")

    print("\n[3/3] Converting to C++ static arrays ...")
    num_steps, total_chunks = xml_to_cpp(
        xml_string, "zcube_fc_schedule_generated.cpp"
    )

    N = len(compute_nodes)
    print(f"""
{sep}
Done.
{sep}
  U            = {U:.6f}
  k            = {k}   (trees per root)
  total trees  = {N * k}
  num_steps    = {num_steps}
  total_chunks = {total_chunks}
  algbw        = {N * k / U:.4f} GB/s
{sep}
""")


if __name__ == "__main__":
    main()
