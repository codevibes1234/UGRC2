import sys, os
sys.path.insert(0, "/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter")
import importlib.util
spec = importlib.util.spec_from_file_location(
    "ggs", "/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/gap2/generate_graph_schedule.py")
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
import networkx as nx

G_DIR = "/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/gap2/graphs"

def build(meta, host_gbps, fabric_gbps):
    nhost, nsw = meta["nhost"], meta["nsw"]
    hb, fb = host_gbps/8.0, fabric_gbps/8.0
    G = nx.DiGraph(); cn = list(range(nhost))
    G.add_nodes_from(cn); G.add_nodes_from(range(nhost, nhost+nsw))
    for h, s in enumerate(meta["host_switch"]):
        G.add_edge(h, nhost+s, capacity=hb); G.add_edge(nhost+s, h, capacity=hb)
    for a, b in meta["links"]:
        G.add_edge(nhost+a, nhost+b, capacity=fb); G.add_edge(nhost+b, nhost+a, capacity=fb)
    return G, cn

for ratio in (0.1, 0.05):
    print(f"\n=== switch-switch links at {ratio:g}x the host link rate ===")
    for fam in ("jellyfish", "flatbutterfly", "dragonfly", "megafly"):
        meta = m.load_graph_dump(f"{G_DIR}/{fam}_32.graph")
        G, cn = build(meta, 100.0, 100.0*ratio)
        try:
            (U, k), (Ts, Cs) = m.optimal_pipeline_spanning_trees(G, compute_nodes=cn, fixed_K=None)
            print(f"  {fam:<16} U={U:<10.4f} k={k:<3} predicted algBW = {len(cn)*k/U:8.4f} GB/s")
        except Exception as e:
            print(f"  {fam:<16} FAILED: {type(e).__name__}: {e}")
