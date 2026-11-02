#!/usr/bin/env python3
"""
generate_graph_schedule.py
==========================
Generate a ForestColl AllGather + ReduceScatter schedule for any of the switch-graph
topology families (Jellyfish, Flattened Butterfly, Dragonfly, Megafly).

This is the topology-general version of
clos/datacenter/generate_zcube_schedule_v4.py, whose only topology-specific part was a
60-line hand-built ZCube(4,2) graph.

The topology is NOT rebuilt here. It is read from a dump produced by
clos/datacenter/dump_switch_graph, i.e. by the same code the simulator uses. That matters:
a ForestColl schedule is computed for a SPECIFIC graph, and if the generator's graph differs
from the simulator's, the run still completes - it just routes collective traffic over links
the planner never saw, with nothing anywhere reporting an error. Jellyfish cannot be
reproduced outside C++ at all, because make_jellyfish() draws from std::mt19937 through
std::shuffle and that selection order is implementation-defined in libstdc++.

Bandwidth model, matching GraphTopology (clos/datacenter/graph_topology.cpp:27):

    _linkspeed = SPEED / host_ports

applied uniformly to every host-switch and switch-switch link. So all edges get the same
capacity here too.

Node id layout in the ForestColl graph (all plain ints, because ForestColl sorts node ids):

    host   h            ->  node h              (0 .. nhost-1)
    switch s            ->  node nhost + s      (nhost .. nhost+nsw-1)

Usage:
    generate_graph_schedule.py --graph graphs/dragonfly_128.graph \\
        --nic-gbps 100 --out-cpp schedules/dragonfly_128_fc.cpp

Requires networkx and clos/datacenter/pipeline_allgather.py (ForestColl).
"""

import argparse
import os
import sys
import xml.etree.ElementTree as ET
from collections import defaultdict

import networkx as nx

_HERE = os.path.dirname(os.path.abspath(__file__))
_DATACENTER = os.path.normpath(
    os.path.join(_HERE, "..", "..", "FlexNetPacket", "src", "clos", "datacenter"))
sys.path.insert(0, _DATACENTER)

from pipeline_allgather import optimal_pipeline_spanning_trees, spanning_trees_to_xml



def load_graph_dump(path):
    """Parse a dump_switch_graph file. Returns a dict of its fields."""
    meta = {"name": "", "nhost": None, "nsw": None, "host_ports": 1}
    host_switch = {}
    links = []

    with open(path) as f:
        for lineno, raw in enumerate(f, 1):
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            key = parts[0]
            if key == "name":
                meta["name"] = " ".join(parts[1:])
            elif key in ("nhost", "nsw", "host_ports"):
                meta[key] = int(parts[1])
            elif key == "host":
                host_switch[int(parts[1])] = int(parts[2])
            elif key == "link":
                links.append((int(parts[1]), int(parts[2])))
            else:
                raise ValueError(f"{path}:{lineno}: unknown key {key!r}")

    for field in ("nhost", "nsw"):
        if meta[field] is None:
            raise ValueError(f"{path}: missing {field}")
    missing = [h for h in range(meta["nhost"]) if h not in host_switch]
    if missing:
        raise ValueError(f"{path}: no switch for host(s) {missing[:5]}")

    meta["host_switch"] = [host_switch[h] for h in range(meta["nhost"])]
    meta["links"] = links
    return meta


def build_forestcoll_graph(meta, nic_gbps):
    """Turn a dumped switch graph into the directed capacitated graph ForestColl wants."""
    nhost = meta["nhost"]
    nsw = meta["nsw"]
    host_ports = meta["host_ports"] or 1
    bw = (nic_gbps / host_ports) / 8.0

    G = nx.DiGraph()
    compute_nodes = list(range(nhost))
    G.add_nodes_from(compute_nodes)
    G.add_nodes_from(range(nhost, nhost + nsw))

    for h, s in enumerate(meta["host_switch"]):
        sw = nhost + s
        G.add_edge(h, sw, capacity=bw)
        G.add_edge(sw, h, capacity=bw)

    for a, b in meta["links"]:
        G.add_edge(nhost + a, nhost + b, capacity=bw)
        G.add_edge(nhost + b, nhost + a, capacity=bw)

    expected = 2 * nhost + 2 * len(meta["links"])
    got = G.number_of_edges()
    assert got == expected, f"expected {expected} directed edges, got {got}"

    print(f"  {meta['name']}")
    print(f"  {nhost} hosts + {nsw} switches = {G.number_of_nodes()} nodes, "
          f"{got} directed edges")
    print(f"  uniform link bandwidth {bw} GB/s ({nic_gbps} Gb/s / {host_ports} port(s))")
    return G, compute_nodes



def run_forestcoll(G, compute_nodes, fixed_k=None):
    if fixed_k is not None:
        print(f"  running ForestColl with fixed_K={fixed_k} ...")
    else:
        print("  running ForestColl (unconstrained k) ...")
    print("  (iterative max-flow; can take a long time on large graphs)")

    (U, k), (Ts, Cs) = optimal_pipeline_spanning_trees(
        G, compute_nodes=compute_nodes, fixed_K=fixed_k)

    N = len(compute_nodes)
    algbw = N * k / U
    print(f"  U={U:.6f}  k={k}  theoretical algbw={algbw:.4f} GB/s")
    print(f"  trees: {len(Ts)}  (expect {N} x {k} = {N * k})")
    return U, k, Ts, Cs, algbw



def _bfs_depths(tree_el):
    """BFS depth of every node from the tree root (root = depth 0)."""
    root_name = tree_el.attrib["root"]
    children = defaultdict(list)
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


def xml_to_cpp(xml_string, out_path, nhost, label, struct_name, algbw):
    """
    Emit the same C++ arrays the ZCube generator does, so the existing executor
    (ffapp_forestcoll_zcube.{h,cpp}) can consume them unchanged.

    AG (AllGather)    : root sends down; step = sender's BFS depth.
    RS (ReduceScatter): leaves aggregate up; step = tree height - sender's depth,
                        and the direction is reversed.
    """
    root_el = ET.fromstring(xml_string)
    nchunkspernode = int(root_el.attrib["nchunkspernode"])
    total_chunks = nhost * nchunkspernode

    max_height = 0
    ag_steps = defaultdict(list)
    rs_steps = defaultdict(list)

    trees = root_el.findall("tree")
    print(f"  parsing {len(trees)} trees ...")

    for tree in trees:
        height = int(tree.attrib["height"])
        nchunks = int(tree.attrib["nchunks"])
        depths = _bfs_depths(tree)
        max_height = max(max_height, height)

        for send in tree.findall("send"):
            src_id = int(send.attrib["src"])
            dst_id = int(send.attrib["dst"])
            d = depths.get(send.attrib["src"], 0)
            if src_id >= nhost or dst_id >= nhost:
                continue

            raw = send.attrib.get("path", "")
            nodes = [int(x) for x in raw.split(",") if x != ""] if raw else []
            fwd_sw = [n - nhost for n in nodes[1:-1] if n >= nhost]
            if len(fwd_sw) != len(nodes) - 2:
                fwd_sw = None

            ag_steps[d].append((src_id, dst_id, nchunks, fwd_sw))
            rs_steps[height - d].append(
                (dst_id, src_id, nchunks,
                 list(reversed(fwd_sw)) if fwd_sw is not None else None))

    num_steps = max_height + 1

    print(f"  max_height={max_height}  num_steps={num_steps}  total_chunks={total_chunks}")
    print(f"  AG sends/step: { {s: len(v) for s, v in sorted(ag_steps.items())} }")
    print(f"  RS sends/step: { {s: len(v) for s, v in sorted(rs_steps.items())} }")

    lines = [
        f"// AUTO-GENERATED by generate_graph_schedule.py - DO NOT EDIT",
        "//",
        f"// ForestColl AllGather + ReduceScatter schedule for {label}",
        f"// N={nhost}  nchunkspernode={nchunkspernode}  "
        f"total_chunks={total_chunks}  num_steps={num_steps}",
        f"// theoretical algbw = {algbw:.4f} GB/s",
        "//",
        f"// Host (GPU) IDs 0..{nhost - 1}, one GPU per server, linear indexing.",
        "//",
        "// AG: root sends down the tree.  step d = nodes at depth d transmit.",
        "// RS: leaves aggregate up.       step 0 = deepest leaves send.",
        "",
        f"#define FC_SCHEDULE_NHOST {nhost}",
        "",
        f"static const int _gen_fc_num_steps       = {num_steps};",
        f"static const int _gen_fc_rs_total_chunks = {total_chunks};",
        f"static const int _gen_fc_ag_total_chunks = {total_chunks};",
        "",
    ]

    width = max(2, len(str(nhost - 1)))
    n_pathless = sum(1 for steps in (rs_steps, ag_steps)
                     for v in steps.values() for t in v if t[3] is None)

    for phase, steps in (("rs", rs_steps), ("ag", ag_steps)):
        lines.append(
            f"static const std::vector<std::vector<{struct_name}>>"
            f" _gen_fc_{phase}_steps = {{")
        for step in range(num_steps):
            txs = steps.get(step, [])
            lines.append(f"    // {phase.upper()} step {step}: {len(txs)} transmissions")
            lines.append("    {")
            for snd, rcv, nc, _ in txs:
                lines.append(
                    f"        {{ {snd:{width}d}, {rcv:{width}d}, {nc} }},"
                    f"  // GPU{snd} -> GPU{rcv}  ({nc} chunk(s))")
            lines.append("    },")
        lines += ["};", ""]

    lines.append("// Physical switch path of each transmission above, same indexing.")
    lines.append("// An empty inner list means ForestColl gave no usable path for that")
    lines.append("// transmission and the simulator should fall back to its own routing.")
    lines.append("#define FC_SCHEDULE_HAS_PATHS 1")
    lines.append("")
    for phase, steps in (("rs", rs_steps), ("ag", ag_steps)):
        lines.append("static const std::vector<std::vector<std::vector<int>>>"
                     f" _gen_fc_{phase}_paths = {{")
        for step in range(num_steps):
            txs = steps.get(step, [])
            lines.append("    {")
            for snd, rcv, nc, sw in txs:
                body = "" if not sw else ", ".join(str(x) for x in sw)
                lines.append(f"        {{ {body} }},")
            lines.append("    },")
        lines += ["};", ""]
    print(f"  transmissions without a usable path: {n_pathless}")

    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    with open(out_path, "w") as f:
        f.write("\n".join(lines))
    print(f"  written: {out_path}")
    return num_steps, total_chunks


def main():
    ap = argparse.ArgumentParser(
        description="ForestColl schedule generator for dumped switch graphs")
    ap.add_argument("--graph", required=True, help="file from dump_switch_graph")
    ap.add_argument("--nic-gbps", type=float, default=100.0,
                    help="per-NIC bandwidth in Gb/s; match the simulator's -speed "
                         "(-speed 100000 means 100 Gb/s). Default 100.")
    ap.add_argument("--fixed-k", type=int, default=None,
                    help="fix trees-per-root; use only if the optimal k is impractical")
    ap.add_argument("--out-cpp", required=True)
    ap.add_argument("--out-xml", default=None)
    ap.add_argument("--fast-maxflow", action="store_true",
                    help="compute ForestColl's max-flows with igraph (C) instead of networkx "
                         "(pure Python); same values, so the same schedule - see fast_maxflow.py")
    ap.add_argument("--struct-name", default="ZCubeFCTransmission",
                    help="C++ struct the arrays hold (default keeps the existing "
                         "executor's type, so schedules are drop-in)")
    args = ap.parse_args()

    sep = "=" * 70
    print(sep)
    print("ForestColl schedule generator (switch-graph families)")
    print(sep)
    if args.fast_maxflow:
        sys.path.insert(0, _HERE)
        import fast_maxflow
        fast_maxflow.install()
        print("  max-flow: igraph (fast_maxflow.py)")

    meta = load_graph_dump(args.graph)
    G, compute_nodes = build_forestcoll_graph(meta, args.nic_gbps)

    U, k, Ts, Cs, algbw = run_forestcoll(G, compute_nodes, args.fixed_k)

    xml_string = spanning_trees_to_xml(Ts, Cs, k)
    if args.out_xml:
        os.makedirs(os.path.dirname(os.path.abspath(args.out_xml)), exist_ok=True)
        with open(args.out_xml, "w") as f:
            f.write(xml_string)
        print(f"  written: {args.out_xml}")

    num_steps, total_chunks = xml_to_cpp(
        xml_string, args.out_cpp, meta["nhost"], meta["name"],
        args.struct_name, algbw)

    print(sep)
    print("Done.")
    print(f"  U            = {U:.6f}")
    print(f"  k            = {k}   (trees per root)")
    print(f"  total trees  = {len(Ts)}")
    print(f"  num_steps    = {num_steps}")
    print(f"  total_chunks = {total_chunks}")
    print(f"  algbw        = {algbw:.4f} GB/s")
    if args.fast_maxflow:
        print(f"  max-flow calls = {fast_maxflow.calls}")
    print(sep)


if __name__ == "__main__":
    main()
