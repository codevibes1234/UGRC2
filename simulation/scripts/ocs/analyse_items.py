#!/usr/bin/env python3
"""Tables for PFC_HEADROOM.md, OCS_SCALE.md, RECONF_DELAY.md, DECOMPOSITION.md,
PHASED_MDP.md, SEEDS.md, OCS_432.md and ../gap2/FC_EXECUTOR.md.

Usage:  python3 analyse_items.py {gap2|pfc|ecn|dctcpx|scale|delay|decomp|phased|dctcprest|seeds|n432|fcexec|fctree|rocedemand|mt2}

Reads only the CSVs the sweeps write, prints Markdown. Every number in those four
documents comes from here, so a re-run of a sweep followed by a re-run of this
regenerates the tables. Ratios follow LOGICAL_OCS.md: "A vs B" is B's makespan
divided by A's (above 1 means A is faster), averaged arithmetically over cells.
"""
import csv, sys, os, math
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
FAMS = ["jellyfish", "flatbutterfly", "dragonfly", "megafly"]
SHORT = {"jellyfish": "Jelly", "flatbutterfly": "FB", "dragonfly": "Dfly",
         "megafly": "Mfly", "dheu": "dheu"}
WLS = [("dlrm", "16"), ("vgg", "16"), ("transformer", "16"), ("candle", "16"),
       ("dlrm", "32"), ("vgg", "32"), ("transformer", "32")]


def rows(name):
    p = os.path.join(HERE, name)
    if not os.path.exists(p):
        return []
    return list(csv.DictReader(open(p)))


def num(x):
    try:
        return float(x)
    except (TypeError, ValueError):
        return float("nan")


def mean(xs):
    xs = [x for x in xs if not math.isnan(x)]
    return sum(xs) / len(xs) if xs else float("nan")


def rng(xs):
    xs = [x for x in xs if not math.isnan(x)]
    return (min(xs), max(xs)) if xs else (float("nan"), float("nan"))


def f2(x):
    return "NA" if math.isnan(x) else f"{x:.2f}"


def logical_findings(mk, label):
    """mk[(wl, n, arm)] = makespan. Prints finding 1 and 2 for one PFC model."""
    out = {}
    cells = [(w, n) for w, n in WLS if all((w, n, a) in mk for a in FAMS + ["dheu"])]
    best_static, dheu_ratio, df_best = [], [], 0
    for w, n in cells:
        st = {a: mk[(w, n, a)] for a in FAMS}
        b = min(st, key=st.get)
        df_best += (b == "dragonfly")
        dheu_ratio.append(st[b] / mk[(w, n, "dheu")])
        best_static.append(b)
    vs = {}
    for a in ["jellyfish", "flatbutterfly", "megafly"]:
        r = [mk[(w, n, a)] / mk[(w, n, "dragonfly")] for w, n in cells]
        vs[a] = (mean(r), rng(r), sum(x > 1 for x in r))
    lo, hi = rng(dheu_ratio)
    out = dict(cells=len(cells), df_best=df_best, vs=vs,
               dheu=(mean(dheu_ratio), lo, hi, sum(x > 1 for x in dheu_ratio)),
               best=best_static)
    return out


def item_pfc():
    A = [r for r in rows("pfc_realism_dynflat.csv") if r["series"] == "A"]
    B = [r for r in rows("pfc_realism_dynflat.csv") if r["series"] == "B"]
    C = rows("pfc_realism_mdp.csv")
    variants = ["base", "delay", "xoff8", "xoff32"]

    print("### A. LOGICAL_OCS findings under each PFC model (RoCE)\n")
    print("| PFC model | cells | Dragonfly best | Dfly vs Jelly | Dfly vs FB | Dfly vs Mfly | dheu vs best static | dheu wins |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- |")
    per = {}
    for v in variants:
        mk = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"])
              for r in A if r["variant"] == v and r["verdict"] == "OK"}
        per[v] = mk
        f = logical_findings(mk, v)
        if not f["cells"]:
            continue
        def c(a):
            m, (lo, hi), w = f["vs"][a]
            return f"{m:.2f}x ({lo:.2f}-{hi:.2f}), {w}/{f['cells']}"
        m, lo, hi, w = f["dheu"]
        print(f"| {v} | {f['cells']} | {f['df_best']}/{f['cells']} | {c('jellyfish')} | "
              f"{c('flatbutterfly')} | {c('megafly')} | {m:.2f}x ({lo:.2f}-{hi:.2f}) | {w}/{f['cells']} |")

    print("\n### A2. Makespan change against the historical model, per arm\n")
    print("| PFC model | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- |" + " --- |" * 5)
    for v in variants[1:]:
        cells = []
        for a in FAMS + ["dheu"]:
            ch = [per[v][(w, n, a)] / per["base"][(w, n, a)] - 1
                  for w, n in WLS if (w, n, a) in per[v] and (w, n, a) in per["base"]]
            if ch:
                lo, hi = rng(ch)
                cells.append(f"{100*mean(ch):+.1f}% ({100*lo:+.1f} to {100*hi:+.1f})")
            else:
                cells.append("NA")
        print(f"| {v} | " + " | ".join(cells) + " |")

    print("\n### A3. Does each model hold its own headroom? (all 35 RoCE runs per model)\n")
    print("| PFC model | XOFF | headroom | peak overshoot (pkts) | ingress overflows | peak egress transit (pkts) | transit-only egress overflows |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    xo = {"base": 15, "delay": 15, "xoff8": 8, "xoff32": 32}
    for v in variants:
        rs = [r for r in A if r["variant"] == v]
        if not rs:
            continue
        print(f"| {v} | {xo[v]} | 5 | {max(num(r['pfc_overshoot_pkts']) for r in rs):.2f} | "
              f"{sum(int(num(r['pfc_overflow'])) for r in rs if not math.isnan(num(r['pfc_overflow'])))} | "
              f"{max(num(r['egress_peak_transit_pkts']) for r in rs):.1f} | "
              f"{sum(int(num(r['egress_transit_only_overflow'])) for r in rs if not math.isnan(num(r['egress_transit_only_overflow'])))} |")

    print("\n### A4. Where the 'LOSSLESS not working' warnings come from (dheu, base model)\n")
    print("| workload | ingress overflows | egress overflows, host-originated | egress overflows, transit | of which transit alone over the limit | peak egress transit (pkts of 200) |")
    print("| --- | --- | --- | --- | --- | --- |")
    for w, n in WLS:
        for r in A:
            if r["variant"] == "base" and r["arm"] == "dheu" and r["workload"] == w and r["nodes"] == n:
                print(f"| {w}{n} | {r['pfc_overflow']} | {r['egress_overflow_local']} | "
                      f"{r['egress_overflow_transit']} | {r['egress_transit_only_overflow']} | "
                      f"{num(r['egress_peak_transit_pkts']):.1f} |")

    print("\n### B. Shortest-path routing + dheu under PFC: does it still deadlock?\n")
    print("| PFC model | " + " | ".join(f"{w}{n}" for w, n in WLS) + " | deadlocked |")
    print("| --- |" + " --- |" * (len(WLS) + 1))
    for v in variants:
        cells, dl = [], 0
        for w, n in WLS:
            r = [x for x in B if x["variant"] == v and x["workload"] == w and x["nodes"] == n]
            if not r:
                cells.append("-"); continue
            r = r[0]
            if r["verdict"] == "STALLED":
                cells.append(f"**deadlock** ({r['tasks']}/{r['total_tasks']})"); dl += 1
            else:
                cells.append(f"{r['verdict']} {num(r['makespan_ps'])/1e9:.1f} ms")
        print(f"| {v} | " + " | ".join(cells) + f" | {dl}/{len(WLS)} |")

    print("\n### C. OCS_MT_MDP findings under each PFC model (RoCE)\n")
    print("Interference: job 0's makespan with 2 and 4 tenants divided by job 0 alone.\n")
    print("| PFC model | placement | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- | --- |" + " --- |" * 5)
    for v in variants:
        for pol in ["compact", "scatter"]:
            cells = []
            for a in FAMS + ["dheu"]:
                g = {r["jobs"]: num(r["job0_ms"]) for r in C if r["series"] == "tenancy"
                     and r["pfc_variant"] == v and r["placement"] == pol and r["arm"] == a}
                if "1" in g and "2" in g and "4" in g:
                    cells.append(f"{g['2']/g['1']:.2f} / {g['4']/g['1']:.2f}")
                else:
                    cells.append("NA")
            print(f"| {v} | {pol} | " + " | ".join(cells) + " |")
    print("\nFull MDP (16-node TP2 DP2 PP2 EP2 job), makespan ms and the hypercube's lead:\n")
    print("| PFC model | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " | FB lead over the static families | FB lead over dheu |")
    print("| --- |" + " --- |" * 7)
    for v in variants:
        g = {r["arm"]: num(r["makespan_ms"]) for r in C if r["series"] == "fullmdp" and r["pfc_variant"] == v}
        if len(g) < 5:
            continue
        others = [g[a] / g["flatbutterfly"] for a in FAMS if a != "flatbutterfly"]
        print(f"| {v} | " + " | ".join(f"{g[a]:.2f}" for a in FAMS + ["dheu"]) +
              f" | {min(others):.2f}-{max(others):.2f}x | {g['dheu']/g['flatbutterfly']:.2f}x |")


def item_ecn():
    old = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"])
           for r in rows("logical_ocs.csv") if r["transport"] == "dctcp" and r["verdict"] == "OK"}
    new = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"])
           for r in rows("ecn_exact_dynflat.csv") if r["series"] == "E1" and r["verdict"] == "OK"}
    print("### E1. LOGICAL_OCS DCTCP half, historical 8.1 GB queue vs exact 900 kB queue\n")
    print("| finding | historical queue | exact queue |")
    print("| --- | --- | --- |")
    fo, fn = logical_findings(old, "old"), logical_findings(new, "new")
    if fn["cells"]:
        print(f"| Dragonfly best static | {fo['df_best']}/{fo['cells']} | {fn['df_best']}/{fn['cells']} |")
        for a in ["jellyfish", "flatbutterfly", "megafly"]:
            print(f"| Dragonfly vs {SHORT[a]} | {fo['vs'][a][0]:.2f}x | {fn['vs'][a][0]:.2f}x |")
        print(f"| dheu vs best static | {fo['dheu'][0]:.2f}x ({fo['dheu'][1]:.2f}-{fo['dheu'][2]:.2f}), wins {fo['dheu'][3]}/{fo['cells']} "
              f"| {fn['dheu'][0]:.2f}x ({fn['dheu'][1]:.2f}-{fn['dheu'][2]:.2f}), wins {fn['dheu'][3]}/{fn['cells']} |")
    print("\nPer cell, makespan ms (historical -> exact):\n")
    print("| workload | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- |" + " --- |" * 5)
    for w, n in WLS:
        cells = []
        for a in FAMS + ["dheu"]:
            o, nn = old.get((w, n, a), float("nan")), new.get((w, n, a), float("nan"))
            cells.append(f"{o/1e9:.1f} -> {nn/1e9:.1f}")
        print(f"| {w}{n} | " + " | ".join(cells) + " |")
    e3 = rows("ecn_exact_mdp.csv")
    old3 = rows("ocs_mt_mdp.csv")
    if e3:
        print("\n### E3. OCS_MT_MDP DCTCP, interference ratios (2 / 4 tenants), historical -> exact\n")
        print("| placement | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
        print("| --- |" + " --- |" * 5)
        for pol in ["compact", "scatter"]:
            cells = []
            for a in FAMS + ["dheu"]:
                def ratio(rs, key_arm, key_tr):
                    g = {r["jobs"]: num(r["job0_ms"]) for r in rs if r["series"] == "tenancy"
                         and r[key_tr] == "dctcp" and r["placement"] == pol and r[key_arm] == a}
                    return (g["2"]/g["1"], g["4"]/g["1"]) if all(k in g for k in "124") else (float("nan"),)*2
                o = ratio(old3, "arm", "transport"); nn = ratio(e3, "arm", "transport")
                cells.append(f"{f2(o[0])}/{f2(o[1])} -> {f2(nn[0])}/{f2(nn[1])}")
            print(f"| {pol} | " + " | ".join(cells) + " |")
        print("\nFull MDP makespan ms, historical -> exact:\n")
        print("| " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
        print("|" + " --- |" * 5)
        go = {r["arm"]: num(r["makespan_ms"]) for r in old3 if r["series"] == "fullmdp" and r["transport"] == "dctcp"}
        gn = {r["arm"]: num(r["makespan_ms"]) for r in e3 if r["series"] == "fullmdp"}
        print("| " + " | ".join(f"{go.get(a, float('nan')):.2f} -> {gn.get(a, float('nan')):.2f}" for a in FAMS + ["dheu"]) + " |")


def item_scale():
    R = [r for r in rows("ocs_scale.csv")]
    print("### Fabrics as built\n")
    print("| nodes | degree | arm | construction | links | diameter | mean hops |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    seen = set()
    for r in R:
        k = (r["nodes"], r["deg"], r["arm"])
        if k in seen or r["arm"] == "dheu":
            continue
        seen.add(k)
        print(f"| {r['nodes']} | {r['deg']} | {r['arm']} | {r['construction']} | {r['links']} | {r['diameter']} | {num(r['mean_hops']):.2f} |")
    print("\n### Full MDP, makespan ms (per-class TP / DP in brackets)\n")
    print("| nodes/deg | AllReduce | transport | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " | best static | FB lead over next static |")
    print("| --- | --- | --- |" + " --- |" * 7)
    for nd in [("128", "9"), ("128", "12"), ("256", "12")]:
        for ar in ["rhd", "ring"]:
            for tr in ["roce", "dctcp"]:
                g = {r["arm"]: r for r in R if r["series"] == "fullmdp" and (r["nodes"], r["deg"]) == nd
                     and r["allreduce"] == ar and r["transport"] == tr}
                if not g:
                    continue
                def cell(a):
                    if a not in g or g[a]["verdict"] != "OK":
                        return g[a]["verdict"] if a in g else "-"
                    x = g[a]
                    return f"{num(x['makespan_ms']):.2f} ({num(x['job0_tp_ms']):.2f} / {num(x['job0_dp_ms']):.2f})"
                st = {a: num(g[a]["makespan_ms"]) for a in FAMS if a in g and g[a]["verdict"] == "OK"}
                best = min(st, key=st.get) if st else "-"
                nxt = min(v for a, v in st.items() if a != "flatbutterfly") if len(st) > 1 else float("nan")
                lead = nxt / st["flatbutterfly"] if "flatbutterfly" in st else float("nan")
                print(f"| {nd[0]}/{nd[1]} | {ar} | {tr} | " + " | ".join(cell(a) for a in FAMS + ["dheu"]) +
                      f" | {SHORT.get(best, best)} | {f2(lead)}x |")
    print("\n### Tenancy: job 0 with 2 / 4 tenants divided by job 0 alone\n")
    print("| nodes/deg | transport | placement | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- | --- | --- |" + " --- |" * 5)
    for nd in [("128", "9"), ("256", "12")]:
        for tr in ["roce", "dctcp"]:
            for pol in ["compact", "scatter"]:
                cells = []
                for a in FAMS + ["dheu"]:
                    g = {r["jobs"]: num(r["job0_ms"]) for r in R if r["series"] == "tenancy"
                         and (r["nodes"], r["deg"]) == nd and r["transport"] == tr
                         and r["placement"] == pol and r["arm"] == a and r["verdict"] == "OK"}
                    cells.append(f"{g['2']/g['1']:.2f} / {g['4']/g['1']:.2f}" if all(k in g for k in "124") else "-")
                print(f"| {nd[0]}/{nd[1]} | {tr} | {pol} | " + " | ".join(cells) + " |")
    print("\n### Tenancy: job 0 alone, makespan ms\n")
    print("| nodes/deg | transport | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- | --- |" + " --- |" * 5)
    for nd in [("128", "9"), ("256", "12")]:
        for tr in ["roce", "dctcp"]:
            cells = []
            for a in FAMS + ["dheu"]:
                g = [num(r["job0_ms"]) for r in R if r["series"] == "tenancy" and (r["nodes"], r["deg"]) == nd
                     and r["transport"] == tr and r["placement"] == "compact" and r["arm"] == a and r["jobs"] == "1"]
                cells.append(f"{g[0]:.2f}" if g else "-")
            print(f"| {nd[0]}/{nd[1]} | {tr} | " + " | ".join(cells) + " |")
    bad = [r for r in R if r["verdict"] != "OK"]
    print(f"\nNon-OK runs: {len(bad)}" + "".join(f"\n- {r['series']} {r['nodes']}/{r['deg']} {r['arm']} {r['transport']} {r['placement']} j{r['jobs']}: {r['verdict']}" for r in bad))


def item_delay():
    R = rows("reconf_delay.csv")
    L = rows("logical_ocs.csv")
    best = {}
    for w, n in WLS:
        for tr in ["roce", "dctcp"]:
            st = {r["arm"]: num(r["makespan_ps"]) for r in L if r["workload"] == w and r["nodes"] == n
                  and r["transport"] == tr and r["arm"] in FAMS and r["verdict"] == "OK"}
            if st:
                b = min(st, key=st.get)
                best[(w, n, tr)] = (b, st[b])
    delays = ["10", "100", "1000", "10000", "100000", "1000000", "10000000"]
    lab = {"10": "10 ns", "100": "100 ns", "1000": "1 us", "10000": "10 us", "100000": "100 us",
           "1000000": "1 ms", "10000000": "10 ms"}
    for tr in ["roce", "dctcp"]:
        print(f"\n### D. {tr}: dheu speed-up over the best static family, live period 400 us\n")
        print("(above 1 = reconfiguring wins; best static family in brackets)\n")
        print("| workload | " + " | ".join(lab[d] for d in delays) + " |")
        print("| --- |" + " --- |" * len(delays))
        col = defaultdict(list)
        for w, n in WLS:
            if (w, n, tr) not in best:
                continue
            b, bm = best[(w, n, tr)]
            cells = []
            for d in delays:
                r = [x for x in R if x["series"] == "D" and x["workload"] == w and x["nodes"] == n
                     and x["transport"] == tr and x["variant"] == f"d{d}_p400000"]
                if r and r[0]["verdict"] == "OK":
                    s = bm / num(r[0]["makespan_ps"]); col[d].append(s)
                    cells.append(f"{s:.2f}")
                else:
                    cells.append(r[0]["verdict"] if r else "-")
            print(f"| {w}{n} ({SHORT[b]}) | " + " | ".join(cells) + " |")
        print("| **mean** | " + " | ".join(f"**{mean(col[d]):.2f}**" for d in delays) + " |")
        print("| wins | " + " | ".join(f"{sum(x > 1 for x in col[d])}/{len(col[d])}" for d in delays) + " |")
    print("\n### D2. Where the time goes (dheu, 400 us period): reconfigurations x delay as a share of the makespan, and mean drain\n")
    print("| transport | delay | mean reconfigurations | mean drain (us) | delay share of makespan | drain share of makespan |")
    print("| --- | --- | --- | --- | --- | --- |")
    for tr in ["roce", "dctcp"]:
        for d in delays:
            rs = [x for x in R if x["series"] == "D" and x["transport"] == tr and x["variant"] == f"d{d}_p400000" and x["verdict"] == "OK"]
            if not rs:
                continue
            rec = mean([num(x["reconfs"]) for x in rs])
            dr = mean([num(x["drain_ps_mean"]) / 1e6 for x in rs])
            sh = mean([num(x["reconfs"]) * num(x["reconf_delay_ps"]) / num(x["makespan_ps"]) for x in rs])
            dsh = mean([num(x["reconfs"]) * num(x["drain_ps_mean"]) / num(x["makespan_ps"]) for x in rs])
            print(f"| {tr} | {lab[d]} | {rec:.0f} | {dr:.1f} | {100*sh:.1f}% | {100*dsh:.1f}% |")
    print("\n### P. Period sweep at 10 ns delay: dheu speed-up over best static\n")
    periods = [("100000", "100 us"), ("400000", "400 us"), ("1600000", "1.6 ms"), ("6400000", "6.4 ms")]
    print("| transport | workload | " + " | ".join(p[1] for p in periods) + " |")
    print("| --- | --- |" + " --- |" * len(periods))
    for tr in ["roce", "dctcp"]:
        col = defaultdict(list)
        for w, n in WLS:
            if (w, n, tr) not in best:
                continue
            b, bm = best[(w, n, tr)]
            cells = []
            for p, _ in periods:
                ser, var = ("D", "d10_p400000") if p == "400000" else ("P", f"d10_p{p}")
                r = [x for x in R if x["series"] == ser and x["workload"] == w and x["nodes"] == n
                     and x["transport"] == tr and x["variant"] == var]
                if r and r[0]["verdict"] == "OK":
                    s = bm / num(r[0]["makespan_ps"]); col[p].append(s); cells.append(f"{s:.2f}")
                else:
                    cells.append(r[0]["verdict"] if r else "-")
            print(f"| {tr} | {w}{n} | " + " | ".join(cells) + " |")
        print(f"| {tr} | **mean** | " + " | ".join(f"**{mean(col[p]):.2f}**" for p, _ in periods) + " |")
    ref = {(r["workload"], r["nodes"], r["transport"]): r["makespan_ps"] for r in L if r["arm"] == "dheu"}
    same = [(x["workload"], x["nodes"], x["transport"], x["makespan_ps"], ref.get((x["workload"], x["nodes"], x["transport"])))
            for x in R if x["series"] == "D" and x["variant"] == "d100000_p400000"]
    eq = sum(1 for s in same if s[3] == s[4])
    print(f"\nHistorical point (100 us delay, 400 us period) against logical_ocs.csv's dheu row "
          f"(identical in {eq}/{len(same)} cells; the first reconfiguration now comes one period in, "
          f"not at min(delay, 1 ms)):\n")
    print("| workload | transport | this sweep / stored |")
    print("| --- | --- | --- |")
    for s in same:
        print(f"| {s[0]}{s[1]} | {s[2]} | {num(s[3])/num(s[4]):.4f} |")


def item_decomp():
    R = rows("decomp.csv")
    def table(series, ars, decs):
        for ar in ars:
            for tr in ["roce", "dctcp"]:
                print(f"\n**{series}, {ar} AllReduce, {tr}** - makespan ms, and in brackets TP-class / DP-class ms\n")
                print("| TP,DP,PP,EP | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " | best static |")
                print("| --- |" + " --- |" * 6)
                for d in decs:
                    g = {r["arm"]: r for r in R if r["series"] == series and r["allreduce"] == ar
                         and r["transport"] == tr and r["decomp"] == d}
                    if not g:
                        continue
                    def cell(a):
                        if a not in g: return "-"
                        x = g[a]
                        if x["verdict"] != "OK": return x["verdict"]
                        return f"{num(x['makespan_ms']):.2f} ({f2(num(x['job0_tp_ms']))} / {f2(num(x['job0_dp_ms']))})"
                    st = {a: num(g[a]["makespan_ms"]) for a in FAMS if a in g and g[a]["verdict"] == "OK"}
                    b = min(st, key=st.get) if st else "-"
                    ties = [a for a in st if abs(st[a] - st[b]) / st[b] < 0.005] if st else []
                    lbl = "=".join(SHORT[a] for a in ties) if len(ties) > 1 else SHORT.get(b, b)
                    nice = d.replace("tp", "").replace(".dp", ",").replace(".pp", ",").replace(".ep", ",")
                    print(f"| {nice} | " + " | ".join(cell(a) for a in FAMS + ["dheu"]) + f" | {lbl} |")
    print("### Fabrics as built\n")
    print("| nodes | degree | arm | construction | links | diameter |")
    print("| --- | --- | --- | --- | --- | --- |")
    seen = set()
    for r in R:
        k = (r["nodes"], r["deg"], r["arm"])
        if k in seen or r["arm"] == "dheu": continue
        seen.add(k)
        print(f"| {r['nodes']} | {r['deg']} | {r['arm']} | {r['construction']} | {r['links']} | {r['diameter']} |")
    p2 = ["tp2.dp2.pp2.ep2", "tp4.dp2.pp2.ep1", "tp4.dp4.pp1.ep1", "tp8.dp2.pp1.ep1", "tp16.dp1.pp1.ep1"]
    table("F16d4", ["rhd", "ring"], p2)
    table("F16d6", ["rhd", "ring"], p2)
    table("F27d6", ["ring"], ["tp3.dp3.pp3.ep1", "tp9.dp3.pp1.ep1", "tp3.dp9.pp1.ep1", "tp27.dp1.pp1.ep1", "tp3.dp3.pp1.ep3"])
    j = ["tp3.dp2.pp2.ep1", "tp2.dp3.pp2.ep1", "tp3.dp4.pp1.ep1", "tp4.dp3.pp1.ep1", "tp12.dp1.pp1.ep1"]
    table("J12d4", ["ring", "auto"], j)
    table("J12d6", ["ring", "auto"], j)
    bad = [r for r in R if r["verdict"] != "OK"]
    print(f"\nNon-OK runs: {len(bad)}")
    decomp_regret(R)


def decomp_regret(R):
    """How each static family fares across ALL decompositions: in every cell
    (series x AllReduce x transport x decomposition) divide its makespan by the
    best static family's; report wins, the mean of that ratio and its worst case.
    A family whose worst case is close to 1 is robust to the job's decomposition."""
    cells = defaultdict(dict)
    for r in R:
        if r["arm"] in FAMS and r["verdict"] == "OK":
            cells[(r["series"], r["allreduce"], r["transport"], r["decomp"])][r["arm"]] = num(r["makespan_ms"])
    def summary(keyfilter, label):
        ratio = defaultdict(list); wins = defaultdict(int); n = 0
        for k, g in cells.items():
            if not keyfilter(k) or len(g) < 4:
                continue
            n += 1
            b = min(g.values())
            for a, v in g.items():
                ratio[a].append(v / b)
                if v / b < 1.005: wins[a] += 1
        if not n:
            return
        cells_txt = []
        for a in FAMS:
            xs = ratio[a]
            cells_txt.append(f"{wins[a]}/{n} wins, mean {mean(xs):.2f}x, worst {max(xs):.2f}x")
        print(f"| {label} | " + " | ".join(cells_txt) + " |")
    print("\n### Robustness across decompositions: makespan / best static family in the same cell\n")
    print("(a win is being within 0.5% of the best; both transports pooled)\n")
    print("| cells | " + " | ".join(SHORT[a] for a in FAMS) + " |")
    print("| --- |" + " --- |" * 4)
    summary(lambda k: k[0].startswith("F16") and k[1] == "rhd", "16 nodes, RHD (all groups power-of-two)")
    summary(lambda k: k[0].startswith("F16") and k[1] == "ring", "16 nodes, ring")
    summary(lambda k: k[0] == "F27d6", "27 nodes, ring (3-ary)")
    summary(lambda k: k[0].startswith("J12") and k[1] == "ring", "12-node job on 16 nodes, ring")
    summary(lambda k: k[0].startswith("J12") and k[1] == "auto", "12-node job on 16 nodes, auto")
    summary(lambda k: not (k[0].startswith("F16") and k[1] == "rhd"), "everything except the aligned case (row 1)")
    summary(lambda k: True, "all cells")



def item_phased():
    R = rows("phased.csv")
    modes = ["conc", "ph4c0", "ph8c0", "ph8c100"]
    print("### Full MDP: makespan ms by iteration mode\n")
    for series, title in [("A", "16 nodes / 4 ports, TP2 DP2 PP2 EP2"),
                          ("B", "128 nodes / 9 ports, TP4 DP4 PP4 EP2"),
                          ("C", "256 nodes / 12 ports, TP4 DP4 PP4 EP4")]:
        for tr in ["roce", "dctcp"]:
            g = defaultdict(dict)
            for r in R:
                if r["series"] == series and r["transport"] == tr and r["verdict"] == "OK":
                    g[r["variant"]][r["arm"]] = num(r["makespan_ms"])
            if not g:
                continue
            print(f"**{series}: {title}, {tr}**\n")
            print("| mode | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " | ranking (static) | FB lead over next static |")
            print("| --- |" + " --- |" * 7)
            for m in modes:
                if m not in g: continue
                st = {a: g[m][a] for a in FAMS if a in g[m]}
                order = " < ".join(SHORT[a] for a in sorted(st, key=st.get))
                lead = (min(v for a, v in st.items() if a != "flatbutterfly") / st["flatbutterfly"]) if "flatbutterfly" in st and len(st) > 1 else float("nan")
                print(f"| {m} | " + " | ".join(f2(g[m].get(a, float('nan'))) for a in FAMS + ["dheu"]) + f" | {order} | {f2(lead)}x |")
            print()
    print("### Tenancy, phased: job 0 with 2 / 4 tenants divided by job 0 alone (concurrent in brackets)\n")
    print("| series | transport | placement | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- | --- | --- |" + " --- |" * 5)
    for series, n in [("D", "128"), ("E", "16")]:
        for tr in ["roce", "dctcp"]:
            for pol in ["compact", "scatter"]:
                cells = []
                for a in FAMS + ["dheu"]:
                    sel = lambda mode: {r["jobs"]: num(r["job0_ms"]) for r in R if r["series"] == series
                                        and r["transport"] == tr and r["placement"] == pol and r["arm"] == a
                                        and r["variant"] == mode and r["verdict"] == "OK"}
                    g = sel("ph8c0" if series == "D" else "ph4c0")
                    c = sel("conc")
                    ph = f"{g['2']/g['1']:.2f}/{g['4']/g['1']:.2f}" if all(k in g for k in "124") else "-"
                    co = f"{c['2']/c['1']:.2f}/{c['4']/c['1']:.2f}" if all(k in c for k in "124") else "-"
                    cells.append(f"{ph} (conc {co})")
                print(f"| {series} ({n} nodes) | {tr} | {pol} | " + " | ".join(cells) + " |")
    bad = [r for r in R if r["verdict"] != "OK"]
    print(f"\nNon-OK runs: {len(bad)}" + "".join(f"\n- {r['series']} {r['arm']} {r['transport']} {r['variant']} j{r['jobs']}: {r['verdict']}" for r in bad))


def item_dctcp_rest():
    """DECOMPOSITION and OCS_SCALE DCTCP cells: historical queue vs exact queue."""
    new = rows("dctcp_exact_mdp.csv")
    old = [r for r in rows("decomp.csv") + rows("ocs_scale.csv") if r["transport"] == "dctcp"]
    key = lambda r: (r["series"], r["nodes"], r["deg"], r["arm"], r["placement"], r["jobs"], r["decomp"], r["allreduce"])
    o = {key(r): num(r["makespan_ms"]) for r in old if r["verdict"] == "OK"}
    ch = defaultdict(list)
    for r in new:
        k = key(r)
        if r["verdict"] == "OK" and k in o:
            ch[(r["series"][:3], r["arm"])].append(num(r["makespan_ms"]) / o[k])
    print("### Makespan with the exact queue / with the historical queue (DCTCP)\n")
    print("| series | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- |" + " --- |" * 5)
    for ser in sorted(set(k[0] for k in ch)):
        cells = []
        for a in FAMS + ["dheu"]:
            xs = ch.get((ser, a), [])
            cells.append(f"{mean(xs):.2f} ({min(xs):.2f}-{max(xs):.2f})" if xs else "-")
        print(f"| {ser} | " + " | ".join(cells) + " |")
    cells_o, cells_n = defaultdict(dict), defaultdict(dict)
    for r in old:
        if r["verdict"] == "OK": cells_o[key(r)[:3] + key(r)[4:]][r["arm"]] = num(r["makespan_ms"])
    for r in new:
        if r["verdict"] == "OK": cells_n[key(r)[:3] + key(r)[4:]][r["arm"]] = num(r["makespan_ms"])
    same = diff = 0; flips = []
    dheu_o, dheu_n = [], []
    for k, g in cells_n.items():
        if k not in cells_o: continue
        go = cells_o[k]
        sto = {a: go[a] for a in FAMS if a in go}; stn = {a: g[a] for a in FAMS if a in g}
        if len(sto) == 4 and len(stn) == 4:
            bo, bn = min(sto, key=sto.get), min(stn, key=stn.get)
            if bo == bn or abs(stn[bo] - stn[bn]) / stn[bn] < 0.005: same += 1
            else: diff += 1; flips.append((k, bo, bn))
            if "dheu" in go and "dheu" in g:
                dheu_o.append(min(sto.values()) / go["dheu"]); dheu_n.append(min(stn.values()) / g["dheu"])
    print(f"\nBest static family unchanged in {same} of {same+diff} cells.")
    for k, bo, bn in flips[:20]:
        print(f"- {k}: {SHORT[bo]} -> {SHORT[bn]}")
    if dheu_o:
        print(f"\ndheu vs best static (DCTCP), same cells: historical {mean(dheu_o):.2f}x (wins {sum(x>1 for x in dheu_o)}/{len(dheu_o)}), "
              f"exact {mean(dheu_n):.2f}x (wins {sum(x>1 for x in dheu_n)}/{len(dheu_n)})")
    dec_roce = [r for r in rows("decomp.csv") if r["transport"] == "roce"]
    dec_new = [r for r in new if r["series"][:1] in ("F", "J")]
    print("\n### DECOMPOSITION robustness, DCTCP half under the corrected model")
    decomp_regret(dec_roce + dec_new)
    print("\n### ... DCTCP cells only, corrected model")
    decomp_regret(dec_new)
    print("\n### OCS_SCALE full MDP, DCTCP, corrected model: makespan ms\n")
    print("| nodes/deg | AllReduce | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- | --- |" + " --- |" * 5)
    for nd in [("128", "9"), ("128", "12"), ("256", "12")]:
        for ar in ["rhd", "ring"]:
            g = {r["arm"]: num(r["makespan_ms"]) for r in new if r["series"] == "fullmdp" and (r["nodes"], r["deg"]) == nd and r["allreduce"] == ar and r["verdict"] == "OK"}
            if g:
                print(f"| {nd[0]}/{nd[1]} | {ar} | " + " | ".join(f2(g.get(a, float('nan'))) for a in FAMS + ["dheu"]) + " |")
    print("\n### OCS_SCALE tenancy, DCTCP, corrected model: job 0 with 2 / 4 tenants / alone\n")
    print("| nodes/deg | placement | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- | --- |" + " --- |" * 5)
    for nd in [("128", "9"), ("256", "12")]:
        for pol in ["compact", "scatter"]:
            cells = []
            for a in FAMS + ["dheu"]:
                g = {r["jobs"]: num(r["job0_ms"]) for r in new if r["series"] == "tenancy" and (r["nodes"], r["deg"]) == nd
                     and r["placement"] == pol and r["arm"] == a and r["verdict"] == "OK"}
                cells.append(f"{g['2']/g['1']:.2f} / {g['4']/g['1']:.2f}" if all(k in g for k in "124") else "-")
            print(f"| {nd[0]}/{nd[1]} | {pol} | " + " | ".join(cells) + " |")


def item_seeds():
    S1 = rows("seeds_dynflat.csv")
    L = rows("logical_ocs.csv")
    print("### S1. LOGICAL_OCS at 16 nodes across seeds (seed 1 = the stored rows)\n")
    print("| transport | seed | Dragonfly best static | Dragonfly within 1% of best | dheu vs best static (mean) | dheu wins |")
    print("| --- | --- | --- | --- | --- | --- |")
    per_seed = {}
    for tr in ["roce", "dctcp"]:
        for sd in ["seed1", "seed2", "seed3", "seed4", "seed5"]:
            if sd == "seed1":
                mk = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"]) for r in L
                      if r["transport"] == tr and r["nodes"] == "16" and r["verdict"] == "OK"}
            else:
                mk = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"]) for r in S1
                      if r["transport"] == tr and r["variant"] == sd and r["verdict"] == "OK"}
            if not mk: continue
            f = logical_findings(mk, sd)
            per_seed[(tr, sd)] = f
            near = 0
            for w, n in WLS:
                st = {a: mk[(w, n, a)] for a in FAMS if (w, n, a) in mk}
                if len(st) == 4 and st["dragonfly"] <= 1.01 * min(st.values()): near += 1
            print(f"| {tr} | {sd} | {f['df_best']}/{f['cells']} | {near}/{f['cells']} | {f['dheu'][0]:.2f}x ({f['dheu'][1]:.2f}-{f['dheu'][2]:.2f}) | {f['dheu'][3]}/{f['cells']} |")
    M = rows("seeds_mdp.csv")
    base = rows("pfc_realism_mdp.csv")
    print("\n### S2. OCS_MT_MDP at 16 nodes: interference with 4 tenants (job 0 with 4 / alone)\n")
    print("| transport | placement | arm | seed 13 (stored) | seeds 2-5 |")
    print("| --- | --- | --- | --- | --- |")
    for tr in ["roce", "dctcp"]:
        for pol in ["compact", "scatter"]:
            for a in FAMS + ["dheu"]:
                def ratio(rs, flt):
                    g = {r["jobs"]: num(r["job0_ms"]) for r in rs if flt(r)}
                    return g["4"] / g["1"] if "1" in g and "4" in g else float("nan")
                b = ratio(base, lambda r: r["series"] == "tenancy" and r["transport"] == tr and r["placement"] == pol and r["arm"] == a and r["pfc_variant"] == "base") if tr == "roce" else \
                    ratio(rows("ocs_mt_mdp.csv"), lambda r: r["series"] == "tenancy" and r["transport"] == tr and r["placement"] == pol and r["arm"] == a)
                xs = [ratio(M, lambda r, s=s: r["series"] == "S2tenancy" and r["transport"] == tr and r["placement"] == pol and r["arm"] == a and r["variant"] == s)
                      for s in ["seed2", "seed3", "seed4", "seed5"]]
                print(f"| {tr} | {pol} | {SHORT[a]} | {f2(b)} | " + ", ".join(f2(x) for x in xs) + " |")
    print("\n### S2/S3. Full MDP across seeds: makespan ms (min-max over seeds 2-5) and the winner\n")
    print("| series | transport | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " | winner in every seed? |")
    print("| --- | --- |" + " --- |" * 6)
    for ser in ["S2fullmdp", "S3fullmdp"]:
        for tr in ["roce", "dctcp"]:
            g = defaultdict(dict)
            for r in M:
                if r["series"] == ser and r["transport"] == tr and r["verdict"] == "OK":
                    g[r["variant"]][r["arm"]] = num(r["makespan_ms"])
            if not g: continue
            cells = []
            for a in FAMS + ["dheu"]:
                xs = [g[s][a] for s in g if a in g[s]]
                cells.append(f"{min(xs):.2f}-{max(xs):.2f}" if xs else "-")
            winners = set(min({a: v for a, v in g[s].items() if a in FAMS}, key=lambda a: g[s][a]) for s in g)
            print(f"| {ser} | {tr} | " + " | ".join(cells) + f" | {'yes, ' + SHORT[winners.pop()] if len(winners) == 1 else 'no: ' + ', '.join(SHORT[w] for w in winners)} |")
    print("\n### S3. 128 nodes, 4 tenants / alone, per seed\n")
    print("| transport | placement | " + " | ".join(SHORT[a] for a in FAMS) + " |")
    print("| --- | --- |" + " --- |" * 4)
    for tr in ["roce", "dctcp"]:
        for pol in ["compact", "scatter"]:
            cells = []
            for a in FAMS:
                xs = []
                for s in ["seed2", "seed3", "seed4", "seed5"]:
                    g = {r["jobs"]: num(r["job0_ms"]) for r in M if r["series"] == "S3tenancy" and r["transport"] == tr
                         and r["placement"] == pol and r["arm"] == a and r["variant"] == s and r["verdict"] == "OK"}
                    if "1" in g and "4" in g: xs.append(g["4"] / g["1"])
                cells.append(", ".join(f"{x:.2f}" for x in xs) if xs else "-")
            print(f"| {tr} | {pol} | " + " | ".join(cells) + " |")
    bad = [r for r in S1 + M if r["verdict"] != "OK"]
    print(f"\nNon-OK runs: {len(bad)}")


def item_432():
    R = rows("ocs_432.csv")
    fam3 = ["jellyfish", "dragonfly", "megafly"]
    print("### Fabrics as built (432 nodes, 16 ports)\n")
    print("| arm | construction | links | diameter | mean hops |")
    print("| --- | --- | --- | --- | --- |")
    seen = set()
    for r in R:
        if r["arm"] in seen: continue
        seen.add(r["arm"])
        print(f"| {r['arm']} | {r['construction']} | {r['links']} | {r['diameter']} | {f2(num(r['mean_hops']))} |")
    print("\n### Full MDP, makespan ms (TP-class / DP-class)\n")
    print("| decomposition | transport | " + " | ".join(SHORT[a] for a in fam3) + " | wall s / RSS GB (max) |")
    print("| --- | --- |" + " --- |" * 4)
    for ser, d in [("A", "TP4 DP9 PP4 EP3"), ("B", "TP8 DP6 PP9 EP1")]:
        for tr in ["roce", "dctcp"]:
            g = {r["arm"]: r for r in R if r["series"] == ser and r["transport"] == tr}
            if not g: continue
            cells = [(f"{num(g[a]['makespan_ms']):.2f} ({f2(num(g[a]['job0_tp_ms']))} / {f2(num(g[a]['job0_dp_ms']))})" if g[a]["verdict"] == "OK" else g[a]["verdict"]) if a in g else "-" for a in fam3]
            w = max(num(x["wall_s"]) for x in g.values()); m = max(num(x["rss_kb"]) for x in g.values()) / 1e6
            print(f"| {d} | {tr} | " + " | ".join(cells) + f" | {w:.0f} / {m:.2f} |")
    print("\n### Tenancy, 108-node jobs: job 0 with 2 / 4 tenants divided by job 0 alone (alone ms in brackets)\n")
    print("| transport | placement | " + " | ".join(SHORT[a] for a in fam3) + " |")
    print("| --- | --- |" + " --- |" * 3)
    for tr in ["roce", "dctcp"]:
        for pol in ["compact", "scatter"]:
            cells = []
            for a in fam3:
                g = {r["jobs"]: num(r["job0_ms"]) for r in R if r["series"] == "T" and r["transport"] == tr
                     and r["placement"] == pol and r["arm"] == a and r["verdict"] == "OK"}
                cells.append(f"{g['2']/g['1']:.2f} / {g['4']/g['1']:.2f} ({g['1']:.1f})" if all(k in g for k in "124") else "-")
            print(f"| {tr} | {pol} | " + " | ".join(cells) + " |")
    bad = [r for r in R if r["verdict"] != "OK"]
    print(f"\nNon-OK runs: {len(bad)}")


def item_fcexec():
    p = os.path.join(HERE, "..", "gap2", "fc_exec.csv")
    R = list(csv.DictReader(open(p))) if os.path.exists(p) else []
    old = {}
    for f in ["gap2_results.csv", "gap2_results_128.csv"]:
        q = os.path.join(HERE, "..", "gap2", f)
        if os.path.exists(q):
            for r in csv.DictReader(open(q)):
                if r.get("pfc_variant", "base") == "base":
                    old[(r["family"], r["nodes"], r["transport"], r["algo"])] = r
    def L(fam, n, tr, algo, ex, paths, scan):
        for r in R:
            if (r["family"], r["nodes"], r["transport"], r["algo"], r["exec"], r["fcpaths"], r["rtxscan_ms"]) == (fam, n, tr, algo, ex, paths, scan):
                return num(r["latency_ms"]) if r["verdict"] == "OK" else r["verdict"]
        return float("nan")
    fmt = lambda x: x if isinstance(x, str) else ("-" if math.isnan(x) else f"{x:,.1f}")
    print("### 32 hosts, latency ms (256 MB AllReduce)\n")
    print("| family | transport, timer scan | RHD | ForestColl step, compiled-in schedule | step, parsed schedule | dataflow | dataflow, ForestColl paths |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    for fam in ["jellyfish", "flatbutterfly", "dragonfly", "megafly"]:
        for tr, scan in [("roce", "1000"), ("dctcp", "1000"), ("dctcp", "1")]:
            comp = num(old[(fam, "32", tr, "forestcoll")]["latency_ms"]) if (fam, "32", tr, "forestcoll") in old and scan == "1000" else float("nan")
            rhd = L(fam, "32", tr, "rhd", "none", "0", scan)
            if tr == "roce":
                rhd = num(old[(fam, "32", "roce", "rhd")]["latency_ms"]) if (fam, "32", "roce", "rhd") in old else float("nan")
            print(f"| {fam} | {tr}, {scan} ms | {fmt(rhd)} | {fmt(comp)} | {fmt(L(fam,'32',tr,'forestcoll','step','0',scan))} | "
                  f"{fmt(L(fam,'32',tr,'forestcoll','dataflow','0',scan))} | {fmt(L(fam,'32',tr,'forestcoll','dataflow','1',scan))} |")
    print("\n### 128 hosts, all four families, latency ms\n")
    print("| family | transport, timer scan | RHD | ForestColl step | ForestColl dataflow | best | dataflow / step |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    for fam in ["jellyfish", "flatbutterfly", "dragonfly", "megafly"]:
        for tr, scan in [("roce", "1000"), ("dctcp", "1000"), ("dctcp", "1")]:
            rh, st, df = L(fam, "128", tr, "rhd", "none", "0", scan), L(fam, "128", tr, "forestcoll", "step", "0", scan), L(fam, "128", tr, "forestcoll", "dataflow", "0", scan)
            opts = {k: v for k, v in {"RHD": rh, "FC step": st, "FC dataflow": df}.items() if not isinstance(v, str) and not math.isnan(v)}
            best = min(opts, key=opts.get) if opts else "-"
            ratio = f2(df / st) if not isinstance(df, str) and not isinstance(st, str) else "-"
            print(f"| {fam} | {tr}, {scan} ms | {fmt(rh)} | {fmt(st)} | {fmt(df)} | {best} | {ratio} |")
    bad = [r for r in R if r["verdict"] != "OK"]
    print(f"\nNon-OK runs: {len(bad)}" + "".join(f"\n- {r['family']} {r['nodes']} {r['transport']} scan {r['rtxscan_ms']} ms, {r['algo']} {r['exec']}: {r['verdict']} within 600 s" for r in bad))



def item_fctree(section=None):
    p = os.path.join(HERE, "..", "gap2", "fc_tree.csv")
    R = list(csv.DictReader(open(p))) if os.path.exists(p) else []
    D = {}
    for r in R:
        v = num(r["latency_ms"]) if r["verdict"] == "OK" else r["verdict"]
        D[(r["part"], r["topology"], r["nodes"], r["speed_mbps"], r["transport"],
           int(r["msg_bytes"]) // 1048576, r["algo"])] = v
    L = lambda part, topo, n, sp, tr, mb, algo: D.get((part, topo, n, sp, tr, mb, algo), float("nan"))
    isnum = lambda x: not isinstance(x, str) and not math.isnan(x)
    fmt = lambda x: x if isinstance(x, str) else ("-" if math.isnan(x) else (f"{x:,.3f}" if x < 10 else f"{x:,.1f}"))
    rat = lambda a, b: f2(a / b) if isnum(a) and isnum(b) else "-"
    want = lambda name: section is None or section == name
    if want("Z"):
        for sp in ["200000", "400000", "800000"]:
            print(f"\n**ZCube(4,2), {int(sp)//1000} Gb/s per server, latency ms**\n")
            print("| MB | transport | RHD | FC step | trees (thesis) | pipelined | trees, FC paths | pipelined, FC paths | pipelined/RHD | pipelined+paths/RHD |")
            print("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
            for mb in [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]:
                for tr in ["roce", "dctcp"]:
                    a = [L("Z", "zcube", "16", sp, tr, mb, x) for x in ("rhd", "step", "tree1", "treep")]
                    a += [L("ZP", "zcube", "16", sp, tr, mb, x) for x in ("tree1p", "treepp")]
                    print(f"| {mb} | {tr} | " + " | ".join(fmt(x) for x in a) + f" | {rat(a[3], a[0])} | {rat(a[5], a[0])} |")
    if want("S"):
        print("\n| topology | transport | phase barrier | " + " | ".join(f"{p} piece(s)" for p in ["1", "4", "16", "64", "256"]) + " |")
        print("| --- | --- | --- | --- | --- | --- | --- | --- |")
        for topo in ["zcube", "dragonfly", "jellyfish"]:
            for tr in ["roce", "dctcp"]:
                for b in ["1", "0"]:
                    vals = []
                    for pc in ["1", "4", "16", "64", "256"]:
                        v = [num(r["latency_ms"]) for r in R if r["part"] == "S" and r["topology"] == topo and r["transport"] == tr and r["pieces"] == pc and r["barrier"] == b and r["verdict"] == "OK"]
                        vals.append(v[0] if v else float("nan"))
                    print(f"| {topo} | {tr} | {'yes' if b == '1' else 'no'} | " + " | ".join(fmt(x) for x in vals) + " |")
    if want("G"):
        for part, ppart, n, sizes in [("G32", "G32P", "32", [1, 4, 16, 64, 256]), ("G128", "G128", "128", [16, 256])]:
            print(f"\n**The four families at {n} hosts, latency ms**\n")
            print("| family | MB | transport | RHD | FC step | trees (thesis) | pipelined | pipelined, FC paths | best FC / RHD | best |")
            print("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
            for fam in FAMS:
                for mb in sizes:
                    for tr in ["roce", "dctcp"]:
                        a = [L(part, fam, n, "100000", tr, mb, x) for x in ("rhd", "step", "tree1", "treep")] + [L(ppart, fam, n, "100000", tr, mb, "treepp")]
                        if not any(isnum(x) or isinstance(x, str) for x in a):
                            continue
                        names = ["RHD", "FC step", "FC trees", "FC pipelined", "FC pipelined+paths"]
                        opts = {k: v for k, v in zip(names, a) if isnum(v)}
                        best = min(opts, key=opts.get) if opts else "-"
                        fcs = [v for k, v in opts.items() if k != "RHD"]
                        bfc = min(fcs) if fcs else float("nan")
                        print(f"| {fam} | {mb} | {tr} | " + " | ".join(fmt(x) for x in a) + f" | {rat(bfc, a[0])} | {best} |")
    if want("G432"):
        print("\n**The four families at 432 hosts (256 MB), latency ms. Baseline: ring AllReduce (RHD needs a power of two)**\n")
        print("| family | transport | ring | FC trees (thesis) | FC trees pipelined (2 pieces) | best FC / ring |")
        print("| --- | --- | --- | --- | --- | --- |")
        for fam in FAMS:
            for tr in ["roce", "dctcp"]:
                a = [L("G432", fam, "432", "100000", tr, 256, x) for x in ("ring", "tree1", "treep")]
                if not any(isnum(x) or isinstance(x, str) for x in a):
                    continue
                fcs = [x for x in a[1:] if isnum(x)]
                print(f"| {fam} | {tr} | " + " | ".join(fmt(x) for x in a) + f" | {rat(min(fcs), a[0]) if fcs else '-'} |")
    if want("SUM"):
        print("\n| setting | MB | cells | best ForestColl faster than RHD | best ForestColl / RHD (min-max) |")
        print("| --- | --- | --- | --- | --- |")
        for label, parts, topos, n, sps, sizes in [
                ("ZCube 16, 200/400/800 Gb/s", ("Z", "ZP"), ["zcube"], "16", ["200000", "400000", "800000"], [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]),
                ("families, 32 hosts", ("G32", "G32P"), FAMS, "32", ["100000"], [1, 4, 16, 64, 256]),
                ("families, 128 hosts", ("G128", "G128"), FAMS, "128", ["100000"], [16, 256])]:
            for mb in sizes:
                rs = []
                for topo in topos:
                    for sp in sps:
                        for tr in ["roce", "dctcp"]:
                            rh = L(parts[0], topo, n, sp, tr, mb, "rhd")
                            fc = [L(parts[0], topo, n, sp, tr, mb, x) for x in ("step", "tree1", "treep")] + \
                                 [L(parts[1], topo, n, sp, tr, mb, x) for x in ("tree1p", "treepp")]
                            fc = [x for x in fc if isnum(x)]
                            if isnum(rh) and fc:
                                rs.append(min(fc) / rh)
                if rs:
                    print(f"| {label} | {mb} | {len(rs)} | {sum(1 for x in rs if x < 1)} | {min(rs):.2f}-{max(rs):.2f} |")
    if section is None:
        bad = [r for r in R if r["verdict"] != "OK"]
        print(f"\nNon-OK runs: {len(bad)}" + "".join(f"\n- {r['part']} {r['topology']} {r['nodes']} {r['transport']} {int(r['msg_bytes'])//1048576} MB {r['algo']}: {r['verdict']}" for r in bad))


RD_EXPERIMENTS = [
    ("ROCE_OCS 5d (sweep_ocs)", "sweep_ocs.csv", "rd_sweep_ocs.csv",
     ["transport", "routing", "omethod", "rdelay"], "makespan_ps", 1e-9, "omethod", ["sipring"]),
    ("ROCE_OCS 5f (generalise_ocs)", "generalise_ocs.csv", "rd_generalise_ocs.csv",
     ["workload", "nodes", "transport", "routing", "omethod", "rdelay"], "makespan_ps", 1e-9, "omethod", ["sipring"]),
    ("LOGICAL_OCS", "logical_ocs.csv", "rd_logical_ocs.csv",
     ["workload", "nodes", "degree", "transport", "arm"], "makespan_ps", 1e-9, "arm", FAMS),
    ("OCS_MT_MDP", "ocs_mt_mdp.csv", "rd_ocs_mt_mdp.csv",
     ["series", "arm", "transport", "placement", "jobs", "jobnodes", "dims"], "job0_ms", 1.0, "arm", FAMS),
    ("PFC_HEADROOM, taskgraphs", "pfc_realism_dynflat.csv", "rd_pfc_realism_dynflat.csv",
     ["series", "workload", "nodes", "deg", "transport", "routing", "arm", "variant", "extra"], "makespan_ps", 1e-9, "arm", FAMS),
    ("PFC_HEADROOM, MDP", "pfc_realism_mdp.csv", "rd_pfc_realism_mdp.csv",
     ["series", "nodes", "deg", "arm", "transport", "placement", "jobs", "jobnodes", "decomp", "allreduce", "pfc_variant"], "job0_ms", 1.0, "arm", FAMS),
    ("RECONF_DELAY", "reconf_delay.csv", "rd_reconf_delay.csv",
     ["series", "workload", "nodes", "deg", "transport", "routing", "arm", "variant", "extra"], "makespan_ps", 1e-9, "arm", FAMS),
    ("DECOMPOSITION", "decomp.csv", "rd_decomp.csv",
     ["series", "nodes", "deg", "arm", "transport", "placement", "jobs", "jobnodes", "decomp", "allreduce", "pfc_variant"], "job0_ms", 1.0, "arm", FAMS),
    ("PHASED_MDP", "phased.csv", "rd_phased.csv",
     ["series", "nodes", "deg", "arm", "transport", "placement", "jobs", "jobnodes", "decomp", "allreduce", "variant"], "job0_ms", 1.0, "arm", FAMS),
    ("SEEDS, taskgraphs", "seeds_dynflat.csv", "rd_seeds_dynflat.csv",
     ["series", "workload", "nodes", "deg", "transport", "routing", "arm", "variant", "extra"], "makespan_ps", 1e-9, "arm", FAMS),
    ("SEEDS, MDP", "seeds_mdp.csv", "rd_seeds_mdp.csv",
     ["series", "nodes", "deg", "arm", "transport", "placement", "jobs", "jobnodes", "decomp", "allreduce", "variant"], "job0_ms", 1.0, "arm", FAMS),
    ("OCS_SCALE", "ocs_scale.csv", "rd_ocs_scale.csv",
     ["series", "nodes", "deg", "arm", "transport", "placement", "jobs", "jobnodes", "decomp", "allreduce", "pfc_variant"], "job0_ms", 1.0, "arm", FAMS),
]


def rd_cells(exp):
    label, orig, rerun, keys, val, scale, armcol, statics = exp
    O, N = rows(orig), rows(rerun)
    ok = lambda r: r.get("verdict", "OK") in ("OK", "EXPECTED_STALLED") and num(r[val]) == num(r[val])
    blind = {tuple(r[k] for k in keys): r for r in O if r["transport"] == "roce" and r[armcol] == "dheu"}
    out = []
    for r in N:
        k = tuple(r[k] for k in keys)
        b = blind.get(k)
        i = keys.index(armcol)
        st = {}
        for a in statics:
            kk = list(k); kk[i] = a; kk = tuple(kk)
            for q in O:
                if tuple(q[c] for c in keys) == kk and q.get("verdict", "OK") == "OK":
                    st[a] = num(q[val]) * scale
        best = min(st, key=st.get) if st else None
        out.append(dict(key=k, blind=num(b[val]) * scale if b is not None else float("nan"),
                        blind_v=b.get("verdict", "OK") if b is not None else "-",
                        demand=num(r[val]) * scale, demand_v=r.get("verdict", "OK"),
                        best=best, best_ms=st[best] if best else float("nan"), row=r, brow=b))
    return out


def item_rocedemand():
    print("## Blind against demand-aware RoCE dheu, cell by cell\n")
    summary = []
    for exp in RD_EXPERIMENTS:
        label, orig, rerun, keys, val, scale, armcol, statics = exp
        C = rd_cells(exp)
        if not C:
            print(f"### {label}: no re-run rows yet\n")
            continue
        unit = "ms"
        print(f"### {label} (`{rerun}`, {len(C)} cells)\n")
        varying = [k for j, k in enumerate(keys) if len(set(c["key"][j] for c in C)) > 1]
        print("| " + " | ".join(varying) + " | blind | demand | demand/blind | best static | best static / demand |")
        print("| " + " | ".join("---" for _ in varying) + " | --- | --- | --- | --- | --- |")
        wins_b = wins_d = n = 0
        ratios = []
        for c in C:
            vals = [c["key"][keys.index(k)] for k in varying]
            b, d, s = c["blind"], c["demand"], c["best_ms"]
            dv = "" if c["demand_v"] == "OK" else f" ({c['demand_v']})"
            bv = "" if c["blind_v"] in ("OK", "-") else f" ({c['blind_v']})"
            print("| " + " | ".join(vals) + f" | {f2(b)}{bv} | {f2(d)}{dv} | {f2(d / b) if b == b and b else 'NA'} | "
                  + (f"{f2(s)} ({SHORT.get(c['best'], c['best'])})" if c["best"] else "-") + f" | {f2(s / d) if c['best'] and d else '-'} |")
            if c["best"] and c["demand_v"] == "OK" and not math.isnan(d):
                n += 1
                wins_d += d < s
                wins_b += (b == b) and b < s and c["blind_v"] == "OK"
                ratios.append(s / d)
        print()
        if n:
            summary.append((label, n, wins_b, wins_d, min(ratios), max(ratios), mean(ratios)))
    print("## Summary: does reconfiguring beat the best static family under RoCE?\n")
    print("| experiment | cells with a static comparison | blind dheu wins | demand-aware dheu wins | best static / demand-aware dheu (min-max, mean) |")
    print("| --- | --- | --- | --- | --- |")
    for label, n, wb, wd, lo, hi, mu in summary:
        print(f"| {label} | {n} | {wb} | {wd} | {lo:.2f}-{hi:.2f}, {mu:.2f} |")
    print("\n## Against the corrected DCTCP model\n")
    X = {(r["workload"], r["nodes"]): num(r["makespan_ps"]) / 1e9 for r in rows("dctcp_exact_dynflat.csv")
         if r["series"] == "X2" and r["arm"] == "dheu"}
    XS = defaultdict(dict)
    for r in rows("dctcp_exact_dynflat.csv"):
        if r["series"] == "X2" and r["arm"] != "dheu" and r["verdict"] == "OK":
            XS[(r["workload"], r["nodes"])][r["arm"]] = num(r["makespan_ps"]) / 1e9
    L = {(c["key"][0], c["key"][1]): c for c in rd_cells(RD_EXPERIMENTS[2])}
    if L:
        print("LOGICAL_OCS workloads, ms. DCTCP = exact queue, 1 ms timer scan (PFC_HEADROOM.md section 6).\n")
        print("| workload | RoCE dheu (demand) | DCTCP dheu | RoCE / DCTCP | RoCE best static | DCTCP best static |")
        print("| --- | --- | --- | --- | --- | --- |")
        for (wl, n) in [(a, b) for a, b in WLS]:
            c = L.get((wl, n))
            if not c:
                continue
            dx = X.get((wl, n), float("nan"))
            xs = XS.get((wl, n), {})
            xb = min(xs, key=xs.get) if xs else None
            print(f"| {wl} {n} | {f2(c['demand'])} | {f2(dx)} | {f2(c['demand'] / dx)} | {f2(c['best_ms'])} ({SHORT.get(c['best'])}) | "
                  + (f"{f2(xs[xb])} ({SHORT[xb]})" if xb else "-") + " |")
    XM = {}
    for r in rows("dctcp_exact_mdp.csv"):
        if r["arm"] == "dheu" and r["verdict"] == "OK":
            XM[(r["series"], r["nodes"], r["deg"], r["placement"], r["jobs"], r["jobnodes"], r["decomp"], r["allreduce"])] = num(r["job0_ms"])
    xs = []
    for exp in (RD_EXPERIMENTS[7], RD_EXPERIMENTS[11]):
        for c in rd_cells(exp):
            r = c["row"]
            k = (r["series"], r["nodes"], r["deg"], r["placement"], r["jobs"], r["jobnodes"], r["decomp"], r["allreduce"])
            if k in XM and c["demand_v"] == "OK":
                xs.append((exp[0], k, c["demand"], XM[k], c["best_ms"]))
    if xs:
        print("\nMDP cells with a corrected-DCTCP dheu run (DECOMPOSITION, OCS_SCALE), job 0 ms:\n")
        print("| experiment | cell | RoCE dheu (demand) | DCTCP dheu | DCTCP / RoCE | RoCE best static |")
        print("| --- | --- | --- | --- | --- | --- |")
        for lab, k, d, x, b in xs:
            print(f"| {lab} | {k[0]} n{k[1]} d{k[2]} {k[3]} j{k[4]} {k[6]} {k[7]} | {f2(d)} | {f2(x)} | {f2(x / d)} | {f2(b)} |")
        rs = [x / d for _, _, d, x, _ in xs]
        print(f"\nDCTCP / RoCE over {len(rs)} cells: {min(rs):.2f}-{max(rs):.2f}, mean {mean(rs):.2f}; RoCE faster in {sum(1 for x in rs if x > 1)}.")
    D = [c for c in rd_cells(RD_EXPERIMENTS[6])]
    X3 = {}
    for r in rows("dctcp_exact_dynflat.csv"):
        if r["series"] == "X3":
            X3[(r["workload"], r["nodes"], r["variant"].split("_scan1")[0])] = (num(r["makespan_ps"]) / 1e9, r["verdict"])
    LS = defaultdict(dict)
    for r in rows("logical_ocs.csv"):
        if r["transport"] == "roce" and r["arm"] in FAMS and r["verdict"] == "OK":
            LS[(r["workload"], r["nodes"])][r["arm"]] = num(r["makespan_ps"]) / 1e9
    if D:
        print("\nRECONF_DELAY: circuit-down time swept at a 400 us period (series D) and the period swept at 10 ns (series P), ms. "
              "Best static = the best of the four families as fixed circuits under RoCE (LOGICAL_OCS); DCTCP = exact queue, 1 ms timer scan.\n")
        print("| workload | variant | RoCE dheu blind | RoCE dheu demand | DCTCP dheu | RoCE best static | demand-aware dheu beats it? |")
        print("| --- | --- | --- | --- | --- | --- | --- |")
        wins = defaultdict(lambda: [0, 0])
        for c in D:
            wl, n, var = c["key"][1], c["key"][2], c["key"][7]
            st = LS.get((wl, n), {})
            bs = min(st.values()) if st else float("nan")
            x = X3.get((wl, n, var))
            dx = f"{x[0]:.2f}" + ("" if x[1] == "OK" else f" ({x[1]})") if x else "-"
            win = c["demand"] < bs
            wins[var][0] += win; wins[var][1] += 1
            print(f"| {wl} {n} | {var} | {f2(c['blind'])} | {f2(c['demand'])} | {dx} | {f2(bs)} | {'yes' if win else 'no'} ({f2(bs / c['demand'])}x) |")
        print("\n| variant | workloads where demand-aware RoCE dheu beats the best static family |")
        print("| --- | --- |")
        for var, (w, n) in wins.items():
            print(f"| {var} | {w}/{n} |")


def item_dheuscale():
    N = rows("dheu_scale.csv")
    S, X, F = rows("ocs_scale.csv"), rows("dctcp_exact_mdp.csv"), rows("ocs_432.csv")
    get = lambda R, **kw: [r for r in R if all(r.get(k) == v for k, v in kw.items())]
    ms = lambda r, col="job0_ms": num(r[col]) if r and r["verdict"] == "OK" else float("nan")
    print("### 256 nodes, one full-MDP job (TP4 DP4 PP4 EP4, RHD), degree 12, ms\n")
    print("| arm | RoCE | DCTCP (exact queue) |")
    print("| --- | --- | --- |")
    for a in FAMS:
        r = get(S, series="fullmdp", nodes="256", arm=a, transport="roce", allreduce="rhd")
        d = get(X, series="fullmdp", nodes="256", arm=a, transport="dctcp", allreduce="rhd")
        print(f"| {a} | {f2(ms(r[0]) if r else float('nan'))} | {f2(ms(d[0]) if d else float('nan'))} |")
    b = get(S, series="fullmdp", nodes="256", arm="dheu", transport="roce")
    print(f"| dheu, RoCE demand invisible (OCS_SCALE.md) | {f2(ms(b[0]) if b else float('nan'))} | - |")
    nr = get(N, series="S256", transport="roce"); nd = get(N, series="S256", transport="dctcp")
    print(f"| **dheu, demand visible (this sweep)** | **{f2(ms(nr[0]) if nr else float('nan'))}** | **{f2(ms(nd[0]) if nd else float('nan'))}** |")
    print("\n### 432 nodes, one full-MDP job, degree 16, ms (static families from OCS_432.md)\n")
    print("| decomposition | transport | Jellyfish | Dragonfly | Megafly | dheu (demand visible) | best static / dheu |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    for ser, dec in [("A", "tp4.dp9.pp4.ep3"), ("B", "tp8.dp6.pp9.ep1")]:
        for tr in ["roce", "dctcp"]:
            st = {a: ms(get(F, series=ser, arm=a, transport=tr)[0]) for a in ["jellyfish", "dragonfly", "megafly"] if get(F, series=ser, arm=a, transport=tr)}
            dh = get(N, series=ser, transport=tr)
            d = ms(dh[0]) if dh else float("nan")
            bs = min(v for v in st.values() if not math.isnan(v)) if st else float("nan")
            print(f"| {ser} {dec} | {tr} | " + " | ".join(f2(st.get(a, float('nan'))) for a in ["jellyfish", "dragonfly", "megafly"]) + f" | {f2(d)} | {f2(bs / d)} |")
    print("\n### 432 nodes, 1/2/4 jobs of 108 nodes (TP4 DP3 PP3 EP3): job 0's time, ms, and isolation (4 jobs / alone)\n")
    print("| transport | placement | arm | alone | 2 jobs | 4 jobs | 4 jobs / alone |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    for tr in ["roce", "dctcp"]:
        for pol in ["compact", "scatter"]:
            for a in ["jellyfish", "dragonfly", "megafly", "dheu"]:
                R = N if a == "dheu" else F
                v = []
                for j in ["1", "2", "4"]:
                    r = get(R, series="T", arm=a, transport=tr, placement=pol, jobs=j)
                    v.append(ms(r[0]) if r else float("nan"))
                print(f"| {tr} | {pol} | {a} | " + " | ".join(f2(x) for x in v) + f" | {f2(v[2] / v[0])} |")
    print("\n### Wall time of a demand-driven run, before and after the live-link drain census\n")
    print("| run | before | after |")
    print("| --- | --- | --- |")
    for tr in ["roce", "dctcp"]:
        b = get(S, series="fullmdp", nodes="256", arm="dheu", transport=tr)
        a = get(N, series="S256", transport=tr)
        print(f"| 256 nodes, full MDP, {tr} | {num(b[0]['wall_s']):.0f} s | {num(a[0]['wall_s']):.1f} s |" if b and a else "")


def _sat(path):
    try:
        for line in open(path):
            if line.startswith("PFC_class_saturated "):
                return int(line.split()[1])
    except OSError:
        pass
    return None


def item_pfcclasses():
    R = rows("pfc_classes_dynflat.csv")
    runs = os.path.join(HERE, "runs_pfc_classes")
    V = {}
    for r in R:
        tag = f"P_{r['workload']}{r['nodes']}_{r['transport']}_{r['routing']}_{r['arm']}_{r['variant']}"
        V[(r["workload"], r["nodes"], r["transport"], r["routing"], r["arm"], r["variant"])] = (
            num(r["makespan_ps"]) / 1e9, r["verdict"], _sat(os.path.join(runs, tag + ".csv")))
    def cell(k):
        if k not in V:
            return "-"
        ms, v, sat = V[k]
        if v != "OK":
            return "**stall**" if v == "STALLED" else v
        return f"{ms:.1f}" + (f" (sat {sat})" if sat else "")
    print("### Taskgraphs, makespan ms. RoCE unless marked; 'stall' = deadlocked (tasks left unfinished); "
          "'sat n' = n packets exceeded the class count (not guaranteed deadlock-free)\n")
    print("| workload | family | up*/down* K1 | shortest K1 | shortest K2 | shortest K3 | shortest K4 | shortest K8 | up*/down* K8 | DCTCP up*/down* | DCTCP shortest | RoCE tax: ud K1 / sp K8 | DCTCP tax: ud / sp |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    taxes = defaultdict(list); dtaxes = defaultdict(list); kmin = defaultdict(list)
    complete = defaultdict(lambda: [0, 0])
    for wl, n in WLS:
        for a in FAMS + ["dheu"]:
            ks = [(wl, n, "roce", "updown", a, "k1"), (wl, n, "roce", "shortest", a, "k1")] + \
                 [(wl, n, "roce", "shortest", a, f"k{k}") for k in (2, 3, 4, 8)] + \
                 [(wl, n, "roce", "updown", a, "k8"), (wl, n, "dctcp", "updown", a, "corrected"), (wl, n, "dctcp", "shortest", a, "corrected")]
            if not any(k in V for k in ks):
                continue
            ud1, sp8 = V.get(ks[0]), V.get(ks[5])
            tax = ud1[0] / sp8[0] if ud1 and sp8 and ud1[1] == "OK" and sp8[1] == "OK" else float("nan")
            du, ds = V.get(ks[7]), V.get(ks[8])
            dtax = du[0] / ds[0] if du and ds and du[1] == "OK" and ds[1] == "OK" else float("nan")
            if not math.isnan(tax): taxes[a].append(tax)
            if not math.isnan(dtax): dtaxes[a].append(dtax)
            for k in (2, 3, 4, 8):
                x = V.get((wl, n, "roce", "shortest", a, f"k{k}"))
                if x and x[1] == "OK" and not x[2]:
                    kmin[a].append(k); break
            if sp8:
                complete[a][0] += sp8[1] == "OK"; complete[a][1] += 1
            print(f"| {wl} {n} | {SHORT.get(a, a)} | " + " | ".join(cell(k) for k in ks) + f" | {f2(tax)} | {f2(dtax)} |")
    print("\n### Summary per family\n")
    print("| family | shortest K8 completes | smallest K that completes with no saturation | RoCE tax ud K1 / sp K8 (range, mean) | DCTCP tax ud / sp (range, mean) |")
    print("| --- | --- | --- | --- | --- |")
    for a in FAMS + ["dheu"]:
        if a not in complete:
            continue
        t, d = taxes[a], dtaxes[a]
        ks = sorted(set(kmin[a]))
        print(f"| {a} | {complete[a][0]}/{complete[a][1]} | {', '.join(map(str, ks)) or '-'} (per workload: {kmin[a]}) | "
              + (f"{min(t):.2f}-{max(t):.2f}, {mean(t):.2f}" if t else "-") + " | "
              + (f"{min(d):.2f}-{max(d):.2f}, {mean(d):.2f}" if d else "-") + " |")
    G = rows("pfc_classes_graph.csv")
    if G:
        print("\n### Switch-graph fabrics (GraphTopology, 128 hosts, 200 Gb/s), MDP benchmark, job 0 ms\n")
        print("| family | case | RoCE up*/down* K1 | RoCE ECMP K1 | RoCE ECMP K8 | DCTCP up*/down* | DCTCP ECMP | RoCE ud K1 / ECMP K8 |")
        print("| --- | --- | --- | --- | --- | --- | --- | --- |")
        for a in FAMS:
            for cs in ["full", "ten4"]:
                def g(tr, rt, k):
                    x = [r for r in G if r["topo"] == a and r["case"] == cs and r["transport"] == tr and r["routing"] == rt and r["classes"] == k]
                    if not x: return float("nan"), "-"
                    return (num(x[0]["job0_ms"]), "OK") if x[0]["verdict"] == "OK" else (float("nan"), f"**{x[0]['verdict'].lower()}**")
                c = [g("roce", "updown", "1"), g("roce", "ecmp", "1"), g("roce", "ecmp", "8"), g("dctcp", "updown", "1"), g("dctcp", "ecmp", "1")]
                txt = [f"{v:.2f}" if st == "OK" else st for v, st in c]
                print(f"| {a} | {'one 128-node job' if cs == 'full' else 'four 32-node jobs'} | " + " | ".join(txt) + f" | {f2(c[0][0] / c[2][0])} |")
    M = rows("pfc_classes_mdp.csv")
    if M:
        print("\n### One full-MDP job, job 0 ms\n")
        print("| nodes | family | RoCE ud K1 | RoCE sp K1 | RoCE sp K4 | RoCE sp K8 | DCTCP ud | DCTCP sp |")
        print("| --- | --- | --- | --- | --- | --- | --- | --- |")
        for n in ["16", "128"]:
            for a in FAMS:
                vals = []
                for tr, var in [("roce", "updown_k1"), ("roce", "shortest_k1"), ("roce", "shortest_k4"), ("roce", "shortest_k8"), ("dctcp", "updown"), ("dctcp", "shortest")]:
                    x = [r for r in M if r["nodes"] == n and r["arm"] == a and r["transport"] == tr and r["variant"] == var]
                    vals.append("-" if not x else (f"{num(x[0]['job0_ms']):.2f}" if x[0]["verdict"] == "OK" else f"**{x[0]['verdict'].lower()}**"))
                print(f"| {n} | {a} | " + " | ".join(vals) + " |")


def item_mt2():
    P = os.path.join(HERE, "..", "mt_mdp2", "mt2.csv")
    R = list(csv.DictReader(open(P))) if os.path.exists(P) else []
    def J(series, **kw):
        return [r for r in R if r["series"] == series and all(r.get(k) == v for k, v in kw.items())]
    def ms(r):
        return num(r["makespan_ms"]) if r and r["verdict"] == "OK" else float("nan")
    ARMS2 = FAMS + ["dheu"]
    print("### RTO: the three 'faster with neighbours' DCTCP cases, per-job retransmission timeouts\n")
    print("| fabric | placement | job | alone: job 0 ms, its RTOs | in the 4-job mix: job 0 ms, its RTOs | mix RTOs, other jobs | ratio |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    for topo, pol in [("jellyfish", "compact"), ("zcubepartial", "scatter"), ("fattree128", "compact")]:
        a = J("RTO", topo=topo, placement=pol, label="j1", job_idx="0")
        m = J("RTO", topo=topo, placement=pol, label="j4")
        if not a or not m:
            continue
        m0 = [r for r in m if r["job_idx"] == "0"][0]
        others = sum(int(r["rto_job"]) for r in m if r["job_idx"] != "0")
        print(f"| {topo} | {pol} | {a[0]['job_decomp'] if a[0]['job_decomp'] != 'NA' else ''} | {ms(a[0]):.2f} ms, {a[0]['rto_job']} | "
              f"{ms(m0):.2f} ms, {m0['rto_job']} | {others} | {ms(m0) / ms(a[0]):.2f} |")
    for ser, js, jn in [("T16", ["1", "2", "4", "8"], 16), ("T8", ["1", "4", "16"], 8)]:
        if not J(ser):
            continue
        print(f"\n### {ser}: job 0's time with 1-{js[-1]} identical {jn}-node jobs on 128 nodes (9 ports), ms; isolation = most tenants / alone\n")
        print("| transport | placement | arm | " + " | ".join(f"{j} job(s)" for j in js) + " | isolation | slowest job in the fullest mix |")
        print("| --- | --- | --- | " + " | ".join("---" for _ in js) + " | --- | --- |")
        for tr in ["roce", "dctcp"]:
            for pol in ["compact", "scatter"]:
                for a in ARMS2:
                    v = []
                    for j in js:
                        r0 = J(ser, arm=a, transport=tr, placement=pol, label=f"j{j}", job_idx="0")
                        v.append(ms(r0[0]) if r0 else float("nan"))
                    allj = [ms(r) for r in J(ser, arm=a, transport=tr, placement=pol, label=f"j{js[-1]}")]
                    worst = max([x for x in allj if not math.isnan(x)], default=float("nan"))
                    print(f"| {tr} | {pol} | {a} | " + " | ".join(f2(x) for x in v) + f" | {f2(v[-1] / v[0])} | {f2(worst)} |")
    if J("BLK"):
        print("\n### BLK: four 32-node jobs placed in aligned blocks of B nodes (B = 32: contiguous at a random offset; B = 1: uniform scatter); job 0 ms alone / with 3 neighbours\n")
        bs = ["32", "16", "8", "4", "2", "1"]
        print("| transport | arm | " + " | ".join(f"B={b}" for b in bs) + " |")
        print("| --- | --- | " + " | ".join("---" for _ in bs) + " |")
        for tr in ["roce", "dctcp"]:
            for a in ARMS2:
                cells = []
                for b in bs:
                    a1 = J("BLK", arm=a, transport=tr, label=f"b{b}_j1", job_idx="0")
                    a4 = J("BLK", arm=a, transport=tr, label=f"b{b}_j4", job_idx="0")
                    x1 = ms(a1[0]) if a1 else float("nan"); x4 = ms(a4[0]) if a4 else float("nan")
                    cells.append(f"{f2(x1)} / {f2(x4)} ({f2(x4 / x1)}x)")
                print(f"| {tr} | {a} | " + " | ".join(cells) + " |")
    if J("HET"):
        print("\n### HET: each job type as job 0 - alone, with three copies of itself, with the three other types; ms\n")
        names = {"A": "A tp2.dp4.pp2.ep2", "B": "B dp32", "C": "C tp2.dp2.ep8", "D": "D tp2.dp2.pp8"}
        print("| transport | placement | arm | job 0 type | alone | homogeneous mix | heterogeneous mix | homog / alone | hetero / alone |")
        print("| --- | --- | --- | --- | --- | --- | --- | --- | --- |")
        summ = defaultdict(list)
        for tr in ["roce", "dctcp"]:
            for pol in ["compact", "scatter"]:
                for a in ARMS2:
                    for t in "ABCD":
                        x = [J("HET", arm=a, transport=tr, placement=pol, label=f"{t}_{k}", job_idx="0") for k in ("alone", "homog", "hetero")]
                        v = [ms(y[0]) if y else float("nan") for y in x]
                        if all(math.isnan(z) for z in v):
                            continue
                        summ[(tr, pol)].append((v[1] / v[0], v[2] / v[0]))
                        print(f"| {tr} | {pol} | {a} | {names[t]} | " + " | ".join(f2(z) for z in v) + f" | {f2(v[1] / v[0])} | {f2(v[2] / v[0])} |")
        print("\n| transport | placement | cells | homog / alone (mean) | hetero / alone (mean) | cells where the heterogeneous mix hurts more |")
        print("| --- | --- | --- | --- | --- | --- |")
        for k, L in summ.items():
            L = [x for x in L if not math.isnan(x[0]) and not math.isnan(x[1])]
            if L:
                print(f"| {k[0]} | {k[1]} | {len(L)} | {mean([x[0] for x in L]):.2f} | {mean([x[1] for x in L]):.2f} | {sum(1 for x in L if x[1] > x[0])} |")
    if J("T432"):
        print("\n### T432: 1 and 27 sixteen-node jobs on 432 nodes (16 ports); job 0 ms, and the spread over all 27 jobs\n")
        print("| transport | placement | arm | alone | 27 jobs: job 0 | isolation | 27 jobs: fastest-slowest job |")
        print("| --- | --- | --- | --- | --- | --- | --- |")
        for tr in ["roce", "dctcp"]:
            for pol in ["compact", "scatter"]:
                for a in ["jellyfish", "dragonfly", "megafly"]:
                    a1 = J("T432", arm=a, transport=tr, placement=pol, label="j1", job_idx="0")
                    m = J("T432", arm=a, transport=tr, placement=pol, label="j27")
                    m0 = [r for r in m if r["job_idx"] == "0"]
                    x1 = ms(a1[0]) if a1 else float("nan"); x27 = ms(m0[0]) if m0 else float("nan")
                    allj = [ms(r) for r in m if not math.isnan(ms(r))]
                    rngs = f"{min(allj):.2f}-{max(allj):.2f}" if allj else "-"
                    print(f"| {tr} | {pol} | {a} | {f2(x1)} | {f2(x27)} | {f2(x27 / x1)} | {rngs} |")
    bad = sorted(set((r["series"], r["arm"], r["transport"], r["placement"], r["label"], r["verdict"]) for r in R if r["verdict"] != "OK"))
    print(f"\nNon-OK runs: {len(bad)}" + "".join(f"\n- {' '.join(b)}" for b in bad))


def item_singlebuild():
    A = os.path.join(HERE, "..", "audit_rerun")
    NEW = list(csv.DictReader(open(os.path.join(A, "sb_alltoall.csv")))) if os.path.exists(os.path.join(A, "sb_alltoall.csv")) else []
    old = {}
    for f in ["alltoall_summary.csv", "alltoall128_summary.csv"]:
        q = os.path.join(HERE, "..", "alltoall_roce", f)
        if os.path.exists(q):
            for r in csv.DictReader(open(q)):
                old[(r["gpus"], r["transport"], r["topology"], str(int(float(r["buffer_mb"]) * 1048576)), str(int(float(r["speed_gbps"]) * 1000)))] = num(r["latency_ms"])
    new = {(r["gpus"], r["transport"], r["topology"], r["msg_bytes"], r["speed_mbps"]): (num(r["latency_ms"]), r["rto"]) for r in NEW}
    if new:
        print("### F1, All-to-All (finding 1): stored against re-measured, ms\n")
        print("| GPUs | topology | MB | Gb/s | RoCE stored | RoCE now | DCTCP stored | DCTCP now (RTOs) |")
        print("| --- | --- | --- | --- | --- | --- | --- | --- |")
        keys = sorted(set((k[0], k[2], k[3], k[4]) for k in new), key=lambda k: (int(k[0]), k[1], int(k[2]), int(k[3])))
        ch = defaultdict(list)
        for g, t, m, sp in keys:
            ro, rn = old.get((g, "roce", t, m, sp), float("nan")), new.get((g, "roce", t, m, sp), (float("nan"), ""))
            do, dn = old.get((g, "dctcp", t, m, sp), float("nan")), new.get((g, "dctcp", t, m, sp), (float("nan"), ""))
            for tr, o, n_ in [("roce", ro, rn[0]), ("dctcp", do, dn[0])]:
                if o == o and n_ == n_ and o > 0:
                    ch[tr].append(n_ / o - 1)
            print(f"| {g} | {t} | {int(m)//1048576} | {int(sp)//1000} | {ro:.4f} | {rn[0]:.4f} | {do:.4f} | {dn[0]:.4f} ({dn[1]}) |")
        print("\n| transport | cells | mean change | mean absolute change | largest |")
        print("| --- | --- | --- | --- | --- |")
        for tr, L in ch.items():
            print(f"| {tr} | {len(L)} | {100*mean(L):+.1f}% | {100*mean([abs(x) for x in L]):.1f}% | {100*max(L, key=abs):+.1f}% |")
        def lat(g, tr, t, m, sp):
            return new.get((g, tr, t, str(m), str(sp)), (float("nan"),))[0]
        print("\n**Finding 1's clauses, re-measured (RoCE / DCTCP):**\n")
        for tr in ["roce", "dctcp"]:
            zr = [lat("16", tr, "zcube", m, sp) / lat("16", tr, "fattree", m, sp) for g_, t_, m, sp in [(k[0], k[1], k[2], k[3]) for k in keys if k[0] == "16" and k[1] == "fattree"]]
            br = [lat("16", tr, "bcube", m, sp) / lat("16", tr, "fattree", m, sp) for g_, t_, m, sp in [(k[0], k[1], k[2], k[3]) for k in keys if k[0] == "16" and k[1] == "fattree"]]
            pr = [lat("128", tr, "zcubepartial", m, sp) / lat("128", tr, "fattree128", m, sp) for g_, t_, m, sp in [(k[0], k[1], k[2], k[3]) for k in keys if k[0] == "128" and k[1] == "fattree128"]]
            zr = [x for x in zr if x == x]; br = [x for x in br if x == x]; pr = [x for x in pr if x == x]
            rg = lambda L: f"{min(L):.2f}-{max(L):.2f}" if L else "-"
            ahead = f" (Fat-Tree ahead by up to {100*(max(pr)-1):.0f}%)" if pr else ""
            print(f"- {tr}: (a) ZCube(4,2) / Fat-Tree(4) at 16 GPUs {rg(zr)}; (b) BCube(4,1) / Fat-Tree(4) {rg(br)}; "
                  f"(c) ZCube-Partial / Fat-Tree(8) at 128 GPUs {rg(pr)}{ahead}")
    T = os.path.join(A, "sb_taskgraph.csv")
    if os.path.exists(T):
        NT = list(csv.DictReader(open(T)))
        M = {r["tag"]: r for r in rows(os.path.join("..", "fig16", "fattree", "roce_results_fattree_merged.csv"))}
        print("\n### F2, the 99 distinct Fat-Tree taskgraphs: iteration time, ms\n")
        print("| taskgraph | stored RoCE (build) | RoCE now | DCTCP now | RoCE / DCTCP |")
        print("| --- | --- | --- | --- | --- |")
        by = defaultdict(dict)
        for r in NT:
            done, tot = (r["tasks_done"].split("/") + ["NA"])[:2]
            ok = r["exit"] == "0" and done == tot and r["final_finish_ps"] not in ("", "NA")
            by[r["tag"]][r["transport"]] = (num(r["final_finish_ps"]) / 1e9 if ok else float("nan"), r["tasks_done"], r["exit"])
        same = changed = 0
        ratios = []
        for tag in sorted(by, key=lambda t: (int(t.split("_")[1]), t.split("_")[0].rstrip("0123456789") if False else ''.join(c for c in t.split("_")[0] if c.isalpha()), int(''.join(c for c in t.split("_")[0] if c.isdigit())))):
            rr, dd = by[tag].get("roce"), by[tag].get("dctcp")
            st = M.get(tag)
            sv = (num(st["ff_mean_ms"]) if st and st["status"] == "ok" else float("nan")) if st else float("nan")
            sb = ("fixes 1-4" if st and st["build"].startswith("fix1-4") else "fixes 1-2") if st else "-"
            fmtv = lambda x: f"{x[0]:.1f}" if x and x[0] == x[0] else ((f"killed (memory) {x[1]}" if x[2] == "137" else f"stall {x[1]}") if x else "-")
            ratio = rr[0] / dd[0] if rr and dd and rr[0] == rr[0] and dd[0] == dd[0] else float("nan")
            if ratio == ratio: ratios.append(ratio)
            if sv == sv and rr and rr[0] == rr[0]:
                if abs(rr[0] - sv) < 1e-3: same += 1
                else: changed += 1
            stv = f"{sv:.1f}" if sv == sv else (st["status"] if st else "-")
            print(f"| {tag} | {stv} ({sb}) | {fmtv(rr)} | {fmtv(dd)} | {f2(ratio)} |")
        print(f"\nRoCE rows identical to the stored value: {same}; changed: {changed}. RoCE / DCTCP over {len(ratios)} taskgraphs where both completed: "
              + (f"{min(ratios):.2f}-{max(ratios):.2f}, mean {mean(ratios):.2f}" if ratios else "-"))


def item_framework():
    R = rows("framework.csv")
    MODES = ["conc", "ph8c0", "ph8c100", "f1b8c0", "f1b8c25"]
    NAMES = {"conc": "concurrent", "ph8c0": "GPipe 8 mb", "ph8c100": "GPipe 8 mb + compute",
             "f1b8c0": "1F1B 8 mb", "f1b8c25": "1F1B 8 mb + compute"}
    PH, RPH = rows("phased.csv"), rows("rd_phased.csv")
    def get(ser, arm, tr, mode, pol="compact", jobs="1"):
        x = [r for r in R if r["series"] == ser and r["arm"] == arm and r["transport"] == tr and r["variant"] == mode
             and r["placement"] == pol and r["jobs"] == jobs]
        if not x and ser in ("B", "D") and mode in ("conc", "ph8c0", "ph8c100"):
            src = RPH if (arm == "dheu" and tr == "roce") else PH
            x = [r for r in src if r["series"] == ser and r["arm"] == arm and r["transport"] == tr and r["variant"] == mode
                 and r["placement"] == pol and r["jobs"] == jobs]
        if not x: return float("nan"), "-"
        return (num(x[0]["job0_ms"]), "OK") if x[0]["verdict"] == "OK" else (float("nan"), x[0]["verdict"])
    for ser, label in [("A", "one 16-node job (TP2 DP2 PP2 EP2), 4 ports"), ("B", "one 128-node job (TP4 DP4 PP4 EP2), 9 ports")]:
        if not [r for r in R if r["series"] == ser] and not [r for r in PH if r["series"] == ser]:
            continue
        print(f"\n### {ser}: {label}; iteration time, ms\n")
        print("| transport | arm | " + " | ".join(NAMES[m] for m in MODES) + " |")
        print("| --- | --- | " + " | ".join("---" for _ in MODES) + " |")
        for tr in ["roce", "dctcp"]:
            for a in FAMS + ["dheu"]:
                cells = []
                for m in MODES:
                    v, st = get(ser, a, tr, m)
                    cells.append(f2(v) if st == "OK" else st)
                print(f"| {tr} | {a} | " + " | ".join(cells) + " |")
        print(f"\n**{ser}: best static family, and the demand-driven fabric against it, per mode**\n")
        print("| transport | " + " | ".join(NAMES[m] for m in MODES) + " |")
        print("| --- | " + " | ".join("---" for _ in MODES) + " |")
        for tr in ["roce", "dctcp"]:
            cells = []
            for m in MODES:
                st = {a: get(ser, a, tr, m)[0] for a in FAMS}
                st = {a: v for a, v in st.items() if not math.isnan(v)}
                d = get(ser, "dheu", tr, m)[0]
                if not st:
                    cells.append("-"); continue
                order = sorted(st, key=st.get)
                cells.append(f"{' > '.join(SHORT[a] for a in order)}; dheu {f2(st[order[0]] / d)}x")
            print(f"| {tr} | " + " | ".join(cells) + " |")
    D = [r for r in R if r["series"] == "D"] or [r for r in PH if r["series"] == "D"]
    if D:
        print("\n### D: tenancy on 128 nodes - job 0 with three neighbours / alone, per iteration mode\n")
        print("| transport | placement | arm | concurrent | GPipe 8 mb | 1F1B 8 mb + compute |")
        print("| --- | --- | --- | --- | --- | --- |")
        for tr in ["roce", "dctcp"]:
            for pol in ["compact", "scatter"]:
                for a in FAMS + ["dheu"]:
                    cells = []
                    for m in ["conc", "ph8c0", "f1b8c25"]:
                        x1 = get("D", a, tr, m, pol, "1")[0]; x4 = get("D", a, tr, m, pol, "4")[0]
                        cells.append(f"{f2(x4 / x1)} ({f2(x1)})")
                    print(f"| {tr} | {pol} | {a} | " + " | ".join(cells) + " |")
    G = rows("framework_switch.csv")
    if G:
        print("\n### G: switch fabrics (Fat-Tree and the four switch-graph families, 128 hosts, 200 Gb/s); job 0 ms\n")
        print("| transport | fabric | concurrent | GPipe 8 mb + compute | 1F1B 8 mb + compute |")
        print("| --- | --- | --- | --- | --- |")
        for tr in ["roce", "dctcp"]:
            for t in ["fattree128"] + FAMS:
                cells = []
                for m in ["conc", "ph8c100", "f1b8c25"]:
                    x = [r for r in G if r["topo"] == t and r["transport"] == tr and r["mode"] == m]
                    cells.append("-" if not x else (f2(num(x[0]["job0_ms"])) if x[0]["verdict"] == "OK" else x[0]["verdict"]))
                print(f"| {tr} | {t} | " + " | ".join(cells) + " |")
    bad = [r for r in R if r["verdict"] != "OK"]
    print(f"\nNon-OK OCS runs: {len(bad)}" + "".join(f"\n- {r['series']} {r['arm']} {r['transport']} {r['placement']} j{r['jobs']} {r['variant']}: {r['verdict']}" for r in bad))


def item_topoopt():
    T = rows(os.path.join("..", "fattree_vs_topoopt_roce", "topoopt.csv"))
    F = {}
    for r in rows(os.path.join("..", "audit_rerun", "sb_taskgraph.csv")):
        done, tot = (r["tasks_done"].split("/") + ["NA"])[:2]
        ok = r["exit"] == "0" and done == tot and r["final_finish_ps"] not in ("", "NA")
        F[(r["tag"], r["transport"])] = num(r["final_finish_ps"]) / 1e9 if ok else float("nan")
    print("### Fat-Tree (d x b per server) against reconstructed TopoOpt (d interfaces of b), iteration ms\n")
    print("| taskgraph | d | Gb/s per server | Fat-Tree RoCE | TopoOpt RoCE | TopoOpt / Fat-Tree (RoCE) | Fat-Tree DCTCP | TopoOpt DCTCP | TopoOpt / Fat-Tree (DCTCP) | TopoOpt strides (AR) / MP ports / diameter |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    agg = defaultdict(list)
    keys = sorted(set((r["model"], r["nodes"], r["degree"], r["server_gbps"]) for r in T), key=lambda k: (int(k[1]), k[0], int(k[2]), int(k[3])))
    for model, n, d, B in keys:
        tag = f"{B}{model}_{n}"
        def topo(tr):
            x = [r for r in T if (r["model"], r["nodes"], r["degree"], r["server_gbps"], r["transport"]) == (model, n, d, B, tr)]
            if not x: return float("nan"), "-", x
            return (num(x[0]["final_finish_ps"]) / 1e9 if x[0]["verdict"] == "OK" else float("nan")), x[0]["verdict"], x
        tr_, vr, xr = topo("roce"); td, vd, xd = topo("dctcp")
        fr, fd = F.get((tag, "roce"), float("nan")), F.get((tag, "dctcp"), float("nan"))
        fmt = lambda v, st="OK": f"{v:.1f}" if not math.isnan(v) else (st if st != "OK" else "-")
        info = f"{xr[0]['strides']} / {xr[0]['degree_mp']} / {xr[0]['diameter']}" if xr else "-"
        mk = ""
        if xr and xr[0]["pfc_overflow"] not in ("", "NA", "0"): mk += " o"
        if xr and xr[0]["pfc_saturated"] not in ("", "NA", "0"): mk += " s"
        print(f"| {model} {n} | {d} | {B} | {fmt(fr)} | {fmt(tr_, vr)}{mk} | {f2(tr_ / fr)} | {fmt(fd)} | {fmt(td, vd)} | {f2(td / fd)} | {info} |")
        if not math.isnan(tr_ / fr) and " o" not in mk: agg[(model, n, d, "roce")].append(tr_ / fr)
        if not math.isnan(td / fd): agg[(model, n, d, "dctcp")].append(td / fd)
    print("\n| taskgraph | d | transport | bandwidths | TopoOpt / Fat-Tree (min-max, mean; RoCE runs marked o excluded) |")
    print("| --- | --- | --- | --- | --- |")
    for k, L in sorted(agg.items(), key=lambda kv: (int(kv[0][1]), kv[0][0], int(kv[0][2]), kv[0][3])):
        print(f"| {k[0]} {k[1]} | {k[2]} | {k[3]} | {len(L)} | {min(L):.2f}-{max(L):.2f}, {mean(L):.2f} |")


def item_dctcp_x():
    """LOGICAL_OCS and RECONF_DELAY under DCTCP with the 1 ms timer scan (X1), and
    with the 1 ms scan plus the exact 900 kB OCS queue (X2, X3)."""
    X = rows("dctcp_exact_dynflat.csv")
    old = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"])
           for r in rows("logical_ocs.csv") if r["transport"] == "dctcp" and r["verdict"] == "OK"}
    x1 = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"]) for r in X if r["series"] == "X1" and r["verdict"] == "OK"}
    x2 = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"]) for r in X if r["series"] == "X2" and r["verdict"] == "OK"}
    ident = sum(1 for k in x1 if k in old and x1[k] == old[k])
    print(f"### X1. 1 ms timer scan, historical queue: identical to the stored row in {ident} of {len(x1)} cells")
    diffs = [(k, x1[k] / old[k]) for k in x1 if k in old and x1[k] != old[k]]
    if diffs:
        print("\nThe others (this / stored): " + ", ".join(f"{k[0]}{k[1]} {SHORT[k[2]]} {v:.3f}" for k, v in diffs))
    print("\n### X2. LOGICAL_OCS, DCTCP: historical model vs corrected model (exact queue + 1 ms scan)\n")
    print("| finding | historical (8.1 GB queue, 1 s scan) | corrected (900 kB queue, 1 ms scan) |")
    print("| --- | --- | --- |")
    fo, fn = logical_findings(old, "old"), logical_findings(x2, "new")
    print(f"| Dragonfly best static | {fo['df_best']}/{fo['cells']} | {fn['df_best']}/{fn['cells']} |")
    print(f"| best static family per cell | {', '.join(SHORT[b] for b in fo['best'])} | {', '.join(SHORT[b] for b in fn['best'])} |")
    for a in ["jellyfish", "flatbutterfly", "megafly"]:
        print(f"| Dragonfly vs {SHORT[a]} | {fo['vs'][a][0]:.2f}x ({fo['vs'][a][2]}/{fo['cells']} wins) | {fn['vs'][a][0]:.2f}x ({fn['vs'][a][2]}/{fn['cells']} wins) |")
    print(f"| dheu vs best static | {fo['dheu'][0]:.2f}x ({fo['dheu'][1]:.2f}-{fo['dheu'][2]:.2f}), wins {fo['dheu'][3]}/{fo['cells']} "
          f"| {fn['dheu'][0]:.2f}x ({fn['dheu'][1]:.2f}-{fn['dheu'][2]:.2f}), wins {fn['dheu'][3]}/{fn['cells']} |")
    print("\nPer cell, makespan ms (historical -> corrected):\n")
    print("| workload | " + " | ".join(SHORT[a] for a in FAMS + ["dheu"]) + " |")
    print("| --- |" + " --- |" * 5)
    for w, n in WLS:
        print(f"| {w}{n} | " + " | ".join(f"{old.get((w,n,a), float('nan'))/1e9:.1f} -> {x2.get((w,n,a), float('nan'))/1e9:.1f}" for a in FAMS + ["dheu"]) + " |")
    roce = {(r["workload"], r["nodes"], r["arm"]): num(r["makespan_ps"])
            for r in rows("logical_ocs.csv") if r["transport"] == "roce" and r["verdict"] == "OK"}
    rr = [roce[k] / x2[k] for k in x2 if k in roce and k[2] in FAMS]
    ro = [roce[k] / old[k] for k in old if k in roce and k[2] in FAMS]
    rd = [roce[k] / x2[k] for k in x2 if k in roce and k[2] == "dheu"]
    rdo = [roce[k] / old[k] for k in old if k in roce and k[2] == "dheu"]
    print(f"\nRoCE makespan / DCTCP makespan, dheu: historical {mean(rdo):.2f}x (range {rng(rdo)[0]:.2f}-{rng(rdo)[1]:.2f}), "
          f"corrected {mean(rd):.2f}x (range {rng(rd)[0]:.2f}-{rng(rd)[1]:.2f}); RoCE faster in {sum(x < 1 for x in rd)}/{len(rd)} corrected cells.")
    print(f"\nRoCE makespan / DCTCP makespan, static families, all cells: historical {mean(ro):.2f}x "
          f"(range {rng(ro)[0]:.2f}-{rng(ro)[1]:.2f}), corrected {mean(rr):.2f}x (range {rng(rr)[0]:.2f}-{rng(rr)[1]:.2f}); "
          f"RoCE faster in {sum(x < 1 for x in rr)}/{len(rr)} corrected cells.")
    delays = ["10", "100", "1000", "10000", "100000", "1000000", "10000000"]
    lab = {"10": "10 ns", "100": "100 ns", "1000": "1 us", "10000": "10 us", "100000": "100 us", "1000000": "1 ms", "10000000": "10 ms"}
    best = {}
    for w, n in WLS:
        st = {a: x2[(w, n, a)] for a in FAMS if (w, n, a) in x2}
        if st: best[(w, n)] = (min(st, key=st.get), min(st.values()))
    print("\n### X3. Delay sweep, DCTCP, corrected model: dheu speed-up over the best static family (corrected), 400 us period\n")
    print("| workload | " + " | ".join(lab[d] for d in delays) + " |")
    print("| --- |" + " --- |" * len(delays))
    col = defaultdict(list)
    for w, n in WLS:
        if (w, n) not in best: continue
        b, bm = best[(w, n)]
        cells = []
        for d in delays:
            r = [x for x in X if x["series"] == "X3" and x["workload"] == w and x["nodes"] == n and x["variant"].startswith(f"d{d}_")]
            if r and r[0]["verdict"] in ("OK", "DRAIN_STALLED") and r[0]["tasks"] == r[0]["total_tasks"]:
                v = bm / num(r[0]["makespan_ps"]); col[d].append(v)
                cells.append(f"{v:.2f}" + ("*" if r[0]["verdict"] != "OK" else ""))
            else:
                cells.append(r[0]["verdict"] if r else "-")
        print(f"| {w}{n} ({SHORT[b]}) | " + " | ".join(cells) + " |")
    print("| **mean** | " + " | ".join(f"**{mean(col[d]):.2f}**" for d in delays) + " |")
    print("| wins | " + " | ".join(f"{sum(x > 1 for x in col[d])}/{len(col[d])}" for d in delays) + " |")
    print("\n\\* the run completed every task, but the scheduler had started one more reconfiguration whose drain the end of the run cut off (DRAIN_STALLED).")

def item_gap2():
    g2 = os.path.join(HERE, "..", "gap2")
    def load(n):
        p = os.path.join(g2, n)
        return list(csv.DictReader(open(p))) if os.path.exists(p) else []
    for size, old_f, new_f in [("32", "gap2_results_pre_mtu_fix.csv", "gap2_results.csv"),
                               ("128", "gap2_results_128_pre_mtu_fix.csv", "gap2_results_128.csv")]:
        old = {(r["family"], r["transport"], r["algo"]): r for r in load(old_f)}
        print(f"\n**{size} hosts** - AllReduce latency (ms), 1,500-byte packets (before) and 9,000-byte (after)\n")
        print("| family | transport | algorithm | before | after | after / before | after, 1 us PAUSE delay | PFC overshoot (pkts) base / delay | ingress overflows |")
        print("| --- | --- | --- | --- | --- | --- | --- | --- | --- |")
        new = load(new_f)
        keys = []
        for r in new:
            k = (r["family"], r["transport"], r["algo"])
            if k not in keys: keys.append(k)
        for k in keys:
            b = [r for r in new if (r["family"], r["transport"], r["algo"]) == k and r.get("pfc_variant", "base") == "base"]
            d = [r for r in new if (r["family"], r["transport"], r["algo"]) == k and r.get("pfc_variant") == "delay"]
            o = old.get(k)
            bm = num(b[0]["latency_ms"]) if b else float("nan")
            om = num(o["latency_ms"]) if o else float("nan")
            dm = num(d[0]["latency_ms"]) if d else float("nan")
            ov = f"{f2(num(b[0]['pfc_overshoot_pkts']))} / {f2(num(d[0]['pfc_overshoot_pkts']))}" if b and d else "-"
            of = "/".join(x["pfc_overflow"] for x in b + d) if k[1] == "roce" else "-"
            verdict = "" if not b or b[0]["verdict"] == "OK" else f" ({b[0]['verdict']})"
            fmt = lambda x: "-" if math.isnan(x) else f"{x:,.2f}"
            print(f"| {k[0]} | {k[1]} | {k[2]} | {fmt(om)} | {fmt(bm)}{verdict} | {f2(bm/om)} | {fmt(dm)} | {ov} | {of} |")


if __name__ == "__main__":
    if len(sys.argv) > 2 and sys.argv[1] == "fctree":
        item_fctree(sys.argv[2]); sys.exit(0)
    {"dctcpx": item_dctcp_x, "gap2": item_gap2, "pfc": item_pfc, "phased": item_phased, "dctcprest": item_dctcp_rest,
     "seeds": item_seeds, "n432": item_432, "fcexec": item_fcexec, "fctree": item_fctree, "rocedemand": item_rocedemand, "dheuscale": item_dheuscale, "pfcclasses": item_pfcclasses, "mt2": item_mt2, "singlebuild": item_singlebuild, "framework": item_framework, "topoopt": item_topoopt, "ecn": item_ecn, "scale": item_scale,
     "delay": item_delay, "decomp": item_decomp}[sys.argv[1]]()
