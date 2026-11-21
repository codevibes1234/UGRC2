import csv, sys
from collections import defaultdict

t = defaultdict(dict)
for r in csv.DictReader(open(sys.argv[1])):
    t[(r["arm"], r["placement"])][(r["jobs"], r["placement_seed"])] = float(r["job0_ms"])
for (arm, pol), d in sorted(t.items()):
    iso = [d[("8", s)] / d[("1", s)] for s in ["13", "1", "2", "3", "4"]]
    print(f"{arm:14s} {pol:8s} " + " ".join(f"{x:.2f}" for x in iso) + f"   {min(iso):.2f}-{max(iso):.2f}")
