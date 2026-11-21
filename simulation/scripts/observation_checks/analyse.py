import csv, os, sys
from collections import defaultdict
from statistics import mean

H = os.path.dirname(os.path.abspath(__file__))
G = os.path.dirname(H)


def rows(p):
    p = p if os.path.isabs(p) else os.path.join(H, p)
    return list(csv.DictReader(open(p))) if os.path.exists(p) else []


def num(x):
    try:
        return float(x)
    except (TypeError, ValueError):
        return float("nan")


def ok(x):
    return x == x


def f2(x):
    return f"{x:.2f}" if ok(x) else "-"


def rng(v):
    v = [x for x in v if ok(x)]
    return f"{min(v):.2f}-{max(v):.2f}" if v else "-"


def mb(b):
    return int(b) // 1048576


def obs5():
    print("## Observation 5: VGG on TopoOpt under RoCE\n")
    print("### Diagnosis runs (VGG-16, 100 Gb/s per server, 4 x 25 Gb/s interfaces), iteration ms\n")
    names = [("base", "RoCE as stored"), ("w64", "window 64 credits"), ("wbig", "unlimited window"),
             ("dcqcn", "DCQCN, no window"), ("prio", "credit priority within a PFC class (first version)"),
             ("prio_w64", "first version + window 64"), ("prio_w256", "first version + window 256"),
             ("k1short", "1 PFC class"), ("k1prio", "1 PFC class + credit priority + window 256"),
             ("fwdack", "credits returned along the sender's own forward queue (diagnostic)"),
             ("k8prio", "credit priority across PFC classes (the fix)"), ("k8prio_w64", "the fix + window 64"),
             ("dcqcnw_cp", "the fix + DCQCN with RoCE's window"), ("dctcp", "DCTCP")]
    print("| variant | ms |\n| --- | --- |")
    for t, lab in names:
        p = os.path.join(H, "runs_obs5", (t if t.startswith(("k8prio", "k1prio", "fwdack")) else "vgg100_" + t) + ".csv")
        v = float("nan")
        if os.path.exists(p):
            for l in open(p):
                if l.startswith("FinalFinish "):
                    v = int(l.split()[1]) / 1e9
                    break
        print(f"| {lab} | {f2(v)} |")
    F = {}
    for r in rows(os.path.join(G, "audit_rerun", "sb_taskgraph.csv")):
        if r["exit"] == "0" and r["final_finish_ps"] not in ("", "NA"):
            F[(r["tag"], r["transport"])] = num(r["final_finish_ps"]) / 1e9
    old = {}
    for r in rows(os.path.join(G, "fattree_vs_topoopt_roce", "topoopt.csv")):
        if r["verdict"] == "OK":
            old[(r["model"], r["nodes"], r["server_gbps"], r["transport"])] = num(r["final_finish_ps"]) / 1e9
    print("\n### TopoOpt (d = 4) against Fat-Tree at equal per-server bandwidth, RoCE, with and without the fix\n")
    print("| taskgraph | Gb/s per server | Fat-Tree RoCE | TopoOpt RoCE before | before / Fat-Tree | TopoOpt RoCE with fix | fix / Fat-Tree | TopoOpt DCTCP / Fat-Tree DCTCP (stored) | pauses | class saturated |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    out = []
    for r in rows("obs5_topoopt.csv"):
        tag = f"{r['server_gbps']}{r['model']}_{r['nodes']}"
        fr = F.get((tag, "roce"), float("nan")); fd = F.get((tag, "dctcp"), float("nan"))
        new = num(r["final_finish_ps"]) / 1e9 if r["exit"] == "0" else float("nan")
        ob = old.get((r["model"], r["nodes"], r["server_gbps"], "roce"), float("nan"))
        od = old.get((r["model"], r["nodes"], r["server_gbps"], "dctcp"), float("nan"))
        out.append((r["model"], new / fr))
        print(f"| {r['model']} {r['nodes']} | {r['server_gbps']} | {f2(fr)} | {f2(ob)} | {f2(ob / fr)} | {f2(new)} | {f2(new / fr)} | {f2(od / fd)} | {r['pfc_pause_sent']} | {r['pfc_class_saturated']} |")
    return out


def obs1():
    print("\n## Observation 1: reconfiguration against TopoOpt's fixed topology\n")
    R = rows("obs1_topoopt.csv")
    W = [("dlrm", "16"), ("vgg", "16"), ("transformer", "16"), ("candle", "16"), ("dlrm", "32"), ("vgg", "32"), ("transformer", "32")]
    by = defaultdict(dict)
    for r in R:
        if r["verdict"] != "OK":
            continue
        by[(r["workload"], r["nodes"])][(r["series"], r["routing"], r["arm"], r["variant"])] = num(r["makespan_ps"]) / 1e9
    delays = [10, 100, 1000, 10000, 100000, 1000000, 10000000]
    dl = {10: "10 ns", 100: "100 ns", 1000: "1 us", 10000: "10 us", 100000: "100 us", 1000000: "1 ms", 10000000: "10 ms"}
    print("All RoCE with credit priority; reconfiguration with RoCE demand visible; ms.\n")
    print("| workload | TopoOpt fixed | best generic family (routing) | " + " | ".join(f"reconf. {dl[d]}" for d in delays) + " |")
    print("| --- | --- | --- | " + " | ".join("---" for _ in delays) + " |")
    wins_t = defaultdict(int); wins_g = defaultdict(int); n = 0
    for w in W:
        c = by.get(w, {})
        t = c.get(("T", "shortest", "topoopt", "k8_cp"), float("nan"))
        fams = {k: v for k, v in c.items() if k[0] == "S"}
        bk = min(fams, key=fams.get) if fams else None
        g = fams[bk] if bk else float("nan")
        cells = []
        n += 1
        for d in delays:
            v = c.get(("D", "updown", "dheu", f"d{d}_cp"), float("nan"))
            if ok(v) and ok(t) and v < t: wins_t[d] += 1
            if ok(v) and ok(g) and v < g: wins_g[d] += 1
            cells.append(f"{f2(v)} ({f2(t / v)}x)")
        lab = f"{bk[2]} {'K8' if bk[1] == 'shortest' else 'up*/down*'}" if bk else "-"
        print(f"| {w[0]} {w[1]} | {f2(t)} | {f2(g)} ({lab}) | " + " | ".join(cells) + " |")
    print("\n(x = TopoOpt fixed / reconfigurable; above 1 the reconfigurable fabric is faster)\n")
    print("| delay | workloads where reconfiguration beats TopoOpt fixed | ... beats the best generic family |\n| --- | --- | --- |")
    for d in delays:
        print(f"| {dl[d]} | {wins_t[d]} of {n} | {wins_g[d]} of {n} |")
    print("\n### The same comparison, routing matched\n")
    print("Each reconfigurable variant against the fixed families with the same routing, and against TopoOpt fixed (shortest paths, 8 PFC classes).\n")
    print("| delay | reconf. up*/down* beats best up*/down* family | reconf. 8 classes beats best 8-class family | reconf. 8 classes beats TopoOpt fixed | reconf. 8 classes: classes saturated in |")
    print("| --- | --- | --- | --- | --- |")
    sat = defaultdict(int)
    for r in R:
        if r["series"] == "D" and r["variant"].endswith("_k8_cp"):
            p2 = os.path.join(H, "runs_obs1", f"D_{r['workload']}{r['nodes']}_roce_shortest_dheu_{r['variant']}.csv")
            if os.path.exists(p2):
                for l in open(p2):
                    if l.startswith("PFC_class_saturated ") and l.split()[1] != "0":
                        sat[r["variant"]] += 1
    for d in delays:
        a1 = a2 = a3 = n1 = n2 = n3 = 0
        for w in W:
            c = by.get(w, {})
            ud = [v for k, v in c.items() if k[0] == "S" and k[1] == "updown"]
            k8 = [v for k, v in c.items() if k[0] == "S" and k[1] == "shortest"]
            t = c.get(("T", "shortest", "topoopt", "k8_cp"))
            x = c.get(("D", "updown", "dheu", f"d{d}_cp")); y = c.get(("D", "shortest", "dheu", f"d{d}_k8_cp"))
            if x is not None and ud: n1 += 1; a1 += x < min(ud)
            if y is not None and k8: n2 += 1; a2 += y < min(k8)
            if y is not None and t is not None: n3 += 1; a3 += y < t
        print(f"| {dl[d]} | {a1} of {n1} | {a2} of {n2} | {a3} of {n3} | {sat.get(f'd{d}_k8_cp', 0)} |")
    print("\n### Table 5's phased row against Figure 6 (lossless runs with credit priority)\n")
    S = rows(os.path.join(G, "ocs_merged", "phased.csv")); D = rows(os.path.join(G, "ocs_merged", "rd_phased.csv"))
    key = lambda r: (r["series"], r["nodes"], r["deg"], r["placement"], r["jobs"], r["jobnodes"], r["decomp"], r["allreduce"], r["variant"])
    best = {}
    for r in S:
        if r["transport"] == "roce" and r["arm"] != "dheu" and r["verdict"] == "OK":
            v = num(r["job0_ms"]); k = key(r)
            if k not in best or v < best[k][0]:
                best[k] = (v, r["arm"])
    grp = defaultdict(lambda: [0, 0])
    for r in D:
        k = key(r)
        if r["transport"] != "roce" or r["verdict"] != "OK" or k not in best:
            continue
        mode = "all-at-once" if r["variant"] == "conc" else "GPipe"
        g = {"A": "A: one 16-node pipelined job", "B": "B: one 128-node pipelined job", "D": "D: 32-node jobs on 128 nodes, 1-4 tenants",
             "E": "E: 4-node jobs without a pipeline, 1-4 tenants"}[r["series"]]
        grp[(g, mode)][1] += 1
        if best[k][0] / num(r["job0_ms"]) > 1:
            grp[(g, mode)][0] += 1
    print("| series | model | reconfiguration wins |\n| --- | --- | --- |")
    for (g, m), (w, t) in sorted(grp.items()):
        print(f"| {g} | {m} | {w} of {t} |")
    return wins_t, wins_g, n


def f1_clauses(lat):
    za = [lat[(16, "zcube", s, p)] / lat[(16, "fattree", s, p)] for (g, t, s, p) in lat if g == 16 and t == "fattree" and (16, "zcube", s, p) in lat]
    zb = [lat[(16, "bcube", s, p)] / lat[(16, "fattree", s, p)] for (g, t, s, p) in lat if g == 16 and t == "fattree" and (16, "bcube", s, p) in lat]
    zc = [lat[(128, "zcubepartial", s, p)] / lat[(128, "fattree128", s, p)] for (g, t, s, p) in lat if g == 128 and t == "fattree128" and (128, "zcubepartial", s, p) in lat]
    return za, zb, zc


def clause_line(lab, za, zb, zc):
    def c(v, below):
        v = [x for x in v if ok(x)]
        return f"{rng(v)} ({sum(1 for x in v if (x < 1) == below)} of {len(v)})" if v else "-"
    print(f"| {lab} | {c(za, True)} | {c(zb, False)} | {c(zc, True)} |")


def obs4():
    print("\n## Observation 4: the 10 ms retransmission timeout\n")
    A = rows("obs4_a2a.csv"); FC = rows("obs4_fc.csv"); M = rows("obs4_mdp.csv")
    roce = {}
    for r in rows("obs2_a2a.csv"):
        if r["transport"] == "roce" and r["exit"] == "0":
            roce[(int(r["gpus"]), r["topology"], r["msg_bytes"], r["speed_mbps"])] = num(r["latency_ms"])
    print("### F1 All-to-All under DCTCP: the thesis's three clauses at each minimum timeout\n")
    print("Ratio ranges, with in brackets the points that keep the clause's direction "
          "((a) ZCube faster than Fat-Tree(4); (b) BCube slower; (c) ZCube-Partial faster than Fat-Tree(8)).\n")
    print("| transport / min RTO | (a) ZCube / Fat-Tree(4) | (b) BCube / Fat-Tree(4) | (c) ZCube-Partial / Fat-Tree(8) |\n| --- | --- | --- | --- |")
    clause_line("RoCE (lossless, no CC)", *f1_clauses(roce))
    res = {}
    thesis = {}
    for rto in ["10000", "1000", "200"]:
        lat = {(int(r["gpus"]), r["topology"], r["msg_bytes"], r["speed_mbps"]): num(r["latency_ms"]) for r in A if r["min_rto_us"] == rto and r["exit"] == "0"}
        clause_line(f"DCTCP, {int(rto) / 1000:g} ms", *f1_clauses(lat))
        rr = [lat[k] / roce[k] for k in lat if k in roce and ok(roce[k])]
        ev = [num(r["rto_events"]) for r in A if r["min_rto_us"] == rto]
        res[rto] = (rr, sum(ev))
        za, zb, zc = f1_clauses(lat)
        big = [lat[(16, "bcube", s, p)] / lat[(16, "fattree", s, p)] for (g, t, s, p) in lat if g == 16 and t == "fattree" and int(s) >= 67108864 and (16, "bcube", s, p) in lat]
        nrto = sum(1 for r in A if r["min_rto_us"] == rto and r["exit"] == "0" and num(r["rto_events"]) > 0)
        thesis[rto] = (sum(1 for x in za if abs(x - 1) <= 0.05), len(za), rng(big), sum(1 for x in zc if x > 1), len(zc), nrto, sum(1 for r in A if r["min_rto_us"] == rto and r["exit"] == "0"))
    print("\n### F1 against the thesis's own DCTCP claims\n")
    print("| min RTO | (a) ZCube within 5% of Fat-Tree(4) | (b) BCube / Fat-Tree(4) from 64 MB | (c) Fat-Tree(8) faster than ZCube-Partial | runs with at least one RTO |\n| --- | --- | --- | --- | --- |")
    for rto, (a5, na, bb, c8, nc, nr, nt) in thesis.items():
        print(f"| {int(rto) / 1000:g} ms | {a5} of {na} | {bb} | {c8} of {nc} | {nr} of {nt} |")
    print("\n| min RTO | DCTCP / RoCE over all 107 All-to-All points (min-max, mean) | DCTCP slower in | RTO events |\n| --- | --- | --- | --- |")
    for rto, (rr, ev) in res.items():
        if rr:
            print(f"| {int(rto) / 1000:g} ms | {rng(rr)}, {mean(rr):.2f} | {sum(1 for x in rr if x > 1)} of {len(rr)} | {int(ev)} |")
    print("\n### F3 ForestColl (step schedule) against RHD on ZCube(4,2), ForestColl / RHD\n")
    L = {}
    for r in FC:
        if r["exit"] == "0":
            L[(r["transport"], r["algo"], r["msg_bytes"], r["speed_mbps"], r["min_rto_us"])] = num(r["latency_ms"])
    sizes = sorted({int(r["msg_bytes"]) for r in FC})
    print("| setting | " + " | ".join(f"{mb(s)} MB" for s in sizes) + " |\n| --- | " + " | ".join("---" for _ in sizes) + " |")
    for sp in ["200000", "400000", "800000"]:
        for tr, rto, lab in [("roce", "0", "RoCE"), ("dctcp", "10000", "DCTCP 10 ms"), ("dctcp", "1000", "DCTCP 1 ms"), ("dctcp", "200", "DCTCP 0.2 ms")]:
            cells = []
            for s in sizes:
                a = L.get((tr, "fcstep", str(s), sp, rto)); b = L.get((tr, "rhd", str(s), sp, rto))
                cells.append(f2(a / b) if a and b else "-")
            print(f"| {int(sp) // 1000} G {lab} | " + " | ".join(cells) + " |")
    collapse = {}
    for rto in ["10000", "1000", "200"]:
        v = []
        for s in sizes:
            for sp in ["200000", "400000", "800000"]:
                a = L.get(("dctcp", "fcstep", str(s), sp, rto)); b = L.get(("roce", "fcstep", str(s), sp, "0"))
                if a and b and s >= 16777216:
                    v.append(a / b)
        collapse[rto] = v
    print("\n| min RTO | ForestColl DCTCP / ForestColl RoCE at 16 MB and above (min-max, mean) |\n| --- | --- |")
    for rto, v in collapse.items():
        if v:
            print(f"| {int(rto) / 1000:g} ms | {rng(v)}, {mean(v):.2f} |")
    print("\n### The 54 MDP cells: DCTCP / RoCE on the reconfigurable fabric\n")
    key = lambda r: (r["series"], r["nodes"], r["deg"], r["placement"], r["jobs"], r["jobnodes"], r["decomp"], r["allreduce"])
    RD = {}
    for f in ["rd_decomp.csv", "rd_ocs_scale.csv"]:
        for r in rows(os.path.join(G, "ocs", f)):
            if r["arm"] == "dheu" and r["transport"] == "roce" and r["verdict"] == "OK":
                RD[key(r)] = num(r["job0_ms"])
    md = {}
    RC = {key(r): num(r["job0_ms"]) for r in rows("obs2_mdp.csv") if r["variant"] == "roce_cp" and r["verdict"] == "OK"}
    print("| min RTO | cells | DCTCP / RoCE, original runs without credit priority (min-max, mean) | RoCE faster in | DCTCP / RoCE with credit priority, as in the report (min-max, mean) | RoCE faster in |\n| --- | --- | --- | --- | --- | --- |")
    for rto in ["10000", "1000", "200"]:
        v = [num(r["job0_ms"]) / RD[key(r)] for r in M if r["variant"] == f"ecnexact_rto{rto}" and r["verdict"] == "OK" and key(r) in RD]
        w = [num(r["job0_ms"]) / RC[key(r)] for r in M if r["variant"] == f"ecnexact_rto{rto}" and r["verdict"] == "OK" and key(r) in RC]
        md[rto] = v
        if v:
            print(f"| {int(rto) / 1000:g} ms | {len(v)} | {rng(v)}, {mean(v):.2f} | {sum(1 for x in v if x > 1)} | {rng(w)}, {mean(w):.2f} | {sum(1 for x in w if x > 1)} |")
    return res, collapse, md


def obs3():
    print("\n## Observation 3: seeds\n")
    A = rows("obs3_a2a.csv")
    lat = defaultdict(dict)
    for r in A:
        if r["exit"] == "0":
            lat[(int(r["gpus"]), r["topology"], r["msg_bytes"], r["speed_mbps"])][r["ecmp_seed"]] = num(r["latency_ms"])
    print("### All-to-All under RoCE: spread over ECMP hash seeds 0-5\n")
    print("| topology | points | max/min over seeds per point (largest) | mean |\n| --- | --- | --- | --- |")
    for g, t in [(16, "fattree"), (16, "zcube"), (128, "fattree128"), (128, "zcubepartial")]:
        sp = [max(v.values()) / min(v.values()) for k, v in lat.items() if k[0] == g and k[1] == t and len(v) > 1]
        if sp:
            print(f"| {t} ({g} GPUs) | {len(sp)} | {max(sp):.3f} | {mean(sp):.3f} |")
    print("\n### Clause F1(c) per seed: ZCube-Partial / Fat-Tree(8), 128 GPUs, RoCE\n")
    seeds = sorted({r["ecmp_seed"] for r in A})
    pts = sorted({(k[2], k[3]) for k in lat if k[0] == 128})
    print("| point | " + " | ".join(f"seed {s}" for s in seeds) + " | spread |\n| --- | " + " | ".join("---" for _ in seeds) + " | --- |")
    flips = 0
    for sz, sp in pts:
        a = lat.get((128, "zcubepartial", sz, sp), {}); b = lat.get((128, "fattree128", sz, sp), {})
        rr = [a[s] / b[s] for s in seeds if s in a and s in b]
        if rr and min(rr) < 1 < max(rr): flips += 1
        print(f"| {mb(sz)} MB {int(sp) // 1000} G | " + " | ".join(f2(a[s] / b[s]) if s in a and s in b else "-" for s in seeds) + f" | {rng(rr)} |")
    za_all = []
    for s in seeds:
        sub = {k: v[s] for k, v in lat.items() if s in v}
        za, zb, zc = f1_clauses(sub)
        za_all.append((s, za, zc))
    print("\n| seed | (a) ZCube / Fat-Tree(4) at 16 GPUs | (c) ZCube-Partial / Fat-Tree(8) at 128 GPUs |\n| --- | --- | --- |")
    for s, za, zc in za_all:
        print(f"| {s} | {rng(za)} ({sum(1 for x in za if x < 1)} of {len(za)} below 1) | {rng(zc)} ({sum(1 for x in zc if x < 1)} of {len(zc)} below 1) |")
    print("\n| clause | points where the direction is the same at all seeds | ratio of the seed means (min-max) | points below 1 on the seed means |\n| --- | --- | --- | --- |")
    for lab, g, x, y in [("(a) ZCube / Fat-Tree(4)", 16, "zcube", "fattree"), ("(c) ZCube-Partial / Fat-Tree(8)", 128, "zcubepartial", "fattree128")]:
        st = n = below = 0; rm = []
        for sz, sp in sorted({(k[2], k[3]) for k in lat if k[0] == g}):
            a = lat.get((g, x, sz, sp), {}); b = lat.get((g, y, sz, sp), {})
            rr = [a[s] / b[s] for s in seeds if s in a and s in b]
            if not rr:
                continue
            n += 1; st += max(rr) < 1 or min(rr) > 1
            m = mean(a[s] for s in seeds if s in a and s in b) / mean(b[s] for s in seeds if s in a and s in b)
            rm.append(m); below += m < 1
        print(f"| {lab} | {st} of {n} | {rng(rm)} | {below} of {n} |")
    T = rows("obs3_tg.csv")
    print("\n### Fat-Tree taskgraphs (16 GPUs): spread over ECMP seeds and per-flow hashing, ms\n")
    print("| taskgraph | transport | per host pair, seeds 0-3 | per flow, seeds 1-3 | max/min over all 7 |\n| --- | --- | --- | --- | --- |")
    tg = defaultdict(dict)
    for r in T:
        if r["exit"] == "0" and r["final_finish_ps"] not in ("", "NA"):
            tg[(r["taskgraph"], r["transport"])][(r["ecmp_mode"], r["ecmp_seed"])] = num(r["final_finish_ps"]) / 1e9
    spreads = []
    for (t, tr), v in sorted(tg.items()):
        p = [v[k] for k in v if k[0] == "pair"]; f = [v[k] for k in v if k[0] == "flow"]
        allv = list(v.values()); spreads.append((t, tr, max(allv) / min(allv)))
        print(f"| {t} | {tr} | {rng(p)} | {rng(f)} | {max(allv) / min(allv):.3f} |")
    print("\n| taskgraph | RoCE / DCTCP over the 7 matched ECMP settings (min-max) | direction stable? |\n| --- | --- | --- |")
    for t in sorted({k[0] for k in tg}):
        a = tg.get((t, "roce"), {}); b = tg.get((t, "dctcp"), {})
        rr = [a[k] / b[k] for k in a if k in b]
        if rr:
            print(f"| {t} | {rng(rr)} | {'yes' if min(rr) > 1 or max(rr) < 1 else 'no, crosses 1'} |")
    P = rows(os.path.join(G, "ocs_cp", "obs3_place.csv"))
    print("\n### Multi-tenancy (16-node jobs, 128-node OCS, RoCE with credit priority): job 0 with 8 tenants / alone, per placement seed\n")
    pl = defaultdict(dict)
    for r in P:
        if r["exit"] == "0":
            pl[(r["arm"], r["placement"])][(r["jobs"], r["placement_seed"])] = num(r["job0_ms"])
    print("| family | placement | isolation per seed (13 = the stored seed, 1-4) | min-max |\n| --- | --- | --- | --- |")
    for (arm, pol), v in sorted(pl.items()):
        iso = []
        for s in ["13", "1", "2", "3", "4"]:
            if ("8", s) in v and ("1", s) in v:
                iso.append(v[("8", s)] / v[("1", s)])
        print(f"| {arm} | {pol} | {', '.join(f2(x) for x in iso)} | {rng(iso)} |")
    return spreads, flips, len(pts)


def obs2():
    print("\n## Observation 2: lossy DCTCP against lossless DCQCN\n")
    TR = [("dctcp", "lossy: DCTCP, 10 ms RTO"), ("dctcp200", "lossy: DCTCP, 0.2 ms RTO"), ("roce", "lossless, no CC: RoCE + PFC (fixed window)"),
          ("dcqcn", "lossless + DCQCN, no window"), ("dcqcnw", "lossless + DCQCN, RoCE's window")]
    A = rows("obs2_a2a.csv"); A4 = rows("obs4_a2a.csv")
    lat = defaultdict(dict)
    for r in A:
        if r["exit"] == "0":
            lat[r["transport"]][(int(r["gpus"]), r["topology"], r["msg_bytes"], r["speed_mbps"])] = num(r["latency_ms"])
    for r in A4:
        if r["exit"] == "0" and r["min_rto_us"] in ("10000", "200"):
            lat["dctcp" if r["min_rto_us"] == "10000" else "dctcp200"][(int(r["gpus"]), r["topology"], r["msg_bytes"], r["speed_mbps"])] = num(r["latency_ms"])
    print("### F1 All-to-All clauses per setup\n")
    print("Ratio ranges, with in brackets the points where (a) ZCube is faster than Fat-Tree(4), (b) BCube is slower, (c) ZCube-Partial is faster than Fat-Tree(8).\n")
    print("| setup | (a) ZCube / Fat-Tree(4) | (b) BCube / Fat-Tree(4) | (c) ZCube-Partial / Fat-Tree(8) |\n| --- | --- | --- | --- |")
    for tr, lab in TR:
        clause_line(lab, *f1_clauses(lat.get(tr, {})))
    print("\n| pair, over the 107 All-to-All points | first / second (min-max, mean) | second faster in |\n| --- | --- | --- |")
    out = {}
    for x, y, lab in [("dctcp", "dcqcn", "DCTCP 10 ms / DCQCN no window"), ("dctcp200", "dcqcn", "DCTCP 0.2 ms / DCQCN no window"),
                      ("dctcp", "dcqcnw", "DCTCP 10 ms / DCQCN with window"), ("dctcp200", "dcqcnw", "DCTCP 0.2 ms / DCQCN with window"),
                      ("roce", "dcqcn", "RoCE / DCQCN no window"), ("roce", "dcqcnw", "RoCE / DCQCN with window")]:
        d = lat.get(x, {}); q = lat.get(y, {})
        v = [d[k] / q[k] for k in d if k in q]
        out[(x, y)] = v
        if v:
            print(f"| {lab} | {rng(v)}, {mean(v):.2f} | {sum(1 for t in v if t > 1)} of {len(v)} |")
    FC = rows("obs2_fc.csv"); F4 = rows("obs4_fc.csv")
    L = {}
    for r in FC:
        if r["exit"] == "0":
            L[(r["transport"], r["algo"], r["msg_bytes"], r["speed_mbps"])] = num(r["latency_ms"])
    for r in F4:
        if r["exit"] == "0" and r["min_rto_us"] in ("0", "10000", "200"):
            L[(r["transport"] + ("200" if r["min_rto_us"] == "200" else ""), r["algo"], r["msg_bytes"], r["speed_mbps"])] = num(r["latency_ms"])
    sizes = sorted({int(r["msg_bytes"]) for r in FC})
    print("\n### F3 ForestColl / RHD on ZCube(4,2)\n")
    print("| speed | setup | " + " | ".join(f"{mb(x)} MB" for x in sizes) + " |\n| --- | --- | " + " | ".join("---" for _ in sizes) + " |")
    for sp in ["200000", "400000", "800000"]:
        for tr, lab in TR:
            print(f"| {int(sp) // 1000} G | {lab} | " + " | ".join(f2(L[(tr, 'fcstep', str(x), sp)] / L[(tr, 'rhd', str(x), sp)]) if (tr, 'fcstep', str(x), sp) in L and (tr, 'rhd', str(x), sp) in L else "-" for x in sizes) + " |")
    print("\n| variant | ForestColl / ForestColl under RoCE (min-max, mean) | RHD / RHD under RoCE (min-max, mean) |\n| --- | --- | --- |")
    for tr, lab in TR[3:]:
        v = [L[(tr, "fcstep", str(x), sp)] / L[("roce", "fcstep", str(x), sp)] for x in sizes for sp in ["200000", "400000", "800000"] if (tr, "fcstep", str(x), sp) in L and ("roce", "fcstep", str(x), sp) in L]
        w = [L[(tr, "rhd", str(x), sp)] / L[("roce", "rhd", str(x), sp)] for x in sizes for sp in ["200000", "400000", "800000"] if (tr, "rhd", str(x), sp) in L and ("roce", "rhd", str(x), sp) in L]
        if v:
            print(f"| {lab} | {rng(v)}, {mean(v):.2f} | {rng(w)}, {mean(w):.2f} |")
    T = rows("obs2_tg.csv")
    tg = defaultdict(dict)
    for r in T:
        if r["exit"] == "0" and r["final_finish_ps"] not in ("", "NA"):
            tg[r["taskgraph"]][r["transport"]] = num(r["final_finish_ps"]) / 1e9
    print("\n### F2 Fat-Tree taskgraphs (16 GPUs), iteration ms\n")
    print("| taskgraph | DCTCP (lossy, 10 ms) | RoCE + PFC | DCQCN, no window | DCQCN, RoCE's window | RoCE / DCTCP | DCQCN no window / DCTCP | DCQCN with window / DCTCP |\n| --- | --- | --- | --- | --- | --- | --- | --- |")
    r1 = []; r2 = []
    nan = float("nan")
    for t in sorted(tg, key=lambda x: (x.split("_")[0].lstrip("0123456789"), int("".join(c for c in x.split("_")[0] if c.isdigit())))):
        v = tg[t]; c = v.get("dctcp", nan)
        r1.append(v.get("roce", nan) / c); r2.append(v.get("dcqcnw", nan) / c)
        print(f"| {t} | {f2(c)} | {f2(v.get('roce', nan))} | {f2(v.get('dcqcn', nan))} | {f2(v.get('dcqcnw', nan))} | {f2(v.get('roce', nan) / c)} | {f2(v.get('dcqcn', nan) / c)} | {f2(v.get('dcqcnw', nan) / c)} |")
    O = rows("obs2_ocs.csv")
    oc = defaultdict(dict)
    for r in O:
        if r["exit"] == "0" and r["makespan_ps"] not in ("", "NA"):
            oc[(r["workload"], r["nodes"], r["arm"])][r["transport"]] = num(r["makespan_ps"]) / 1e9
    print("\n### OCS fabric, the seven reconfiguration workloads at 100 Gb/s, ms (lossless arms with credit priority)\n")
    print("| workload | fabric | DCTCP | RoCE + PFC | DCQCN, no window | DCQCN, RoCE's window | DCQCN with window / DCTCP | DCQCN with window / RoCE |\n| --- | --- | --- | --- | --- | --- | --- | --- |")
    r3 = []
    for k in sorted(oc):
        v = oc[k]; r3.append(v.get("dcqcnw", nan) / v.get("dctcp", nan))
        arm = "reconfigurable (10 ns)" if k[2] == "dheu" else "fixed " + k[2]
        print(f"| {k[0]} {k[1]} | {arm} | {f2(v.get('dctcp', nan))} | {f2(v.get('roce', nan))} | {f2(v.get('dcqcn', nan))} | {f2(v.get('dcqcnw', nan))} | {f2(r3[-1])} | {f2(v.get('dcqcnw', nan) / v.get('roce', nan))} |")
    key = lambda r: (r["series"], r["nodes"], r["deg"], r["placement"], r["jobs"], r["jobnodes"], r["decomp"], r["allreduce"])
    M = rows("obs2_mdp.csv"); M4 = rows("obs4_mdp.csv")
    ref = {lab: {key(r): num(r["job0_ms"]) for r in M if r["variant"] == lab and r["verdict"] == "OK"} for lab in ["roce_cp", "dcqcn_cp", "dcqcnw_cp"]}
    names = {"roce_cp": "RoCE + PFC", "dcqcn_cp": "DCQCN, no window", "dcqcnw_cp": "DCQCN, RoCE's window"}
    print("\n### The 54 MDP cells on the reconfigurable fabric, DCTCP / lossless (lossless with credit priority)\n")
    print("| DCTCP min RTO | lossless variant | cells | DCTCP / lossless (min-max, mean) | lossless faster in |\n| --- | --- | --- | --- | --- |")
    md = {}
    for rto in ["10000", "1000", "200"]:
        dc = {key(r): num(r["job0_ms"]) for r in M4 if r["variant"] == f"ecnexact_rto{rto}" and r["verdict"] == "OK"}
        for lab in ["roce_cp", "dcqcn_cp", "dcqcnw_cp"]:
            v = [dc[k] / ref[lab][k] for k in ref[lab] if k in dc]
            md[(rto, lab)] = v
            if v:
                print(f"| {int(rto) / 1000:g} ms | {names[lab]} | {len(v)} | {rng(v)}, {mean(v):.2f} | {sum(1 for x in v if x > 1)} |")
    print("\n| variant | DCQCN / RoCE over the MDP cells (min-max, mean) | DCQCN faster in |\n| --- | --- | --- |")
    for lab in ["dcqcn_cp", "dcqcnw_cp"]:
        v = [ref[lab][k] / ref["roce_cp"][k] for k in ref[lab] if k in ref["roce_cp"]]
        if v:
            print(f"| {names[lab]} | {rng(v)}, {mean(v):.2f} | {sum(1 for x in v if x < 1)} of {len(v)} |")
    return out, r1, r2, r3, md

if __name__ == "__main__":
    which = sys.argv[1:] or ["5", "1", "4", "3", "2"]
    for w in which:
        {"5": obs5, "1": obs1, "4": obs4, "3": obs3, "2": obs2}[w]()
