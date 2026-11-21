import csv, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
OCS = os.path.join(HERE, "..", "ocs")
SOURCES = ["decomp.csv", "rd_decomp.csv", "phased.csv", "ocs_scale.csv", "seeds_mdp.csv",
           "pfc_realism_mdp.csv", "framework.csv", "dheu_scale.csv", "ocs_432.csv"]


def key(r):
    lab = r.get("variant", r.get("pfc_variant", ""))
    return (r["series"], r["nodes"], r["deg"], r["arm"], r["transport"], r["placement"], r["jobs"],
            r["jobnodes"], r["decomp"], r["allreduce"], lab)


def main():
    new = list(csv.DictReader(open(os.path.join(HERE, "repro.csv"))))
    bad = 0
    for i, r in enumerate(new):
        src = SOURCES[i]
        old = [o for o in csv.DictReader(open(os.path.join(OCS, src))) if key(o) == key(r)]
        if not old:
            print(f"NO STORED ROW  {src} {key(r)}")
            bad += 1
            continue
        o = old[-1]
        same = o["job0_ms"] == r["job0_ms"] and o["makespan_ms"] == r["makespan_ms"]
        bad += 0 if same else 1
        print(f"{'match   ' if same else 'MISMATCH'} {src:22s} job0 stored {o['job0_ms']:>10s} now {r['job0_ms']:>10s}  "
              f"makespan stored {o['makespan_ms']:>10s} now {r['makespan_ms']:>10s}  {r['verdict']}")
    print("ALL MATCH" if bad == 0 else f"{bad} MISMATCH(ES)")
    sys.exit(1 if bad else 0)


main()
