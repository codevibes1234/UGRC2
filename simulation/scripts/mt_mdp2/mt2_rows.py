#!/usr/bin/env python3
import hashlib
import re
import sys

out_path, log_path, prefix = sys.argv[1], sys.argv[2], sys.argv[3]
kv = {}
try:
    for line in open(out_path):
        p = line.split(None, 1)
        if len(p) == 2:
            kv.setdefault(p[0], p[1].strip())
except OSError:
    pass

jobs = int(kv.get("Jobs", "0") or 0)
owner = {}
for j in range(jobs):
    for n in kv.get(f"Job{j}_Nodes", "").split(","):
        if n:
            owner[int(n)] = j

rto = [0] * max(jobs, 1)
total = 0
pat = re.compile(r" RTO .* SRC (\d+) DST (\d+)")
try:
    for line in open(log_path, errors="replace"):
        if " RTO " not in line:
            continue
        m = pat.search(line)
        if not m:
            continue
        total += 1
        j = owner.get(int(m.group(1)))
        if j is not None:
            rto[j] += 1
except OSError:
    pass

if jobs == 0:
    print(f"{prefix},NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,{total}")
for j in range(jobs):
    nodes = kv.get(f"Job{j}_Nodes", "")
    md5 = hashlib.md5(nodes.encode()).hexdigest()[:8] if nodes else "NA"
    g = lambda k: kv.get(f"Job{j}_{k}", "NA")
    print(f"{prefix},{j},{g('Decomp')},{md5},{g('Makespan_ms')},{g('Class_tp_ms')},"
          f"{g('Class_dp_ms')},{g('Class_ep_ms')},{g('Class_pp_ms')},{g('Incomplete')},"
          f"{rto[j]},{total}")
