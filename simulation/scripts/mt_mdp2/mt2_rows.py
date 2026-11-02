#!/usr/bin/env python3
"""One CSV row per job from one htsim_tcp_mt_mdp_bench run, with per-job RTO counts.

usage: mt2_rows.py OUT LOG PREFIX
  OUT     the benchmark's -ofile (Job<j>_Nodes, Job<j>_Makespan_ms, ...)
  LOG     its stdout+stderr
  PREFIX  the leading CSV fields (already comma-joined) to put in front of each row

Per-job RTO attribution. TcpSrc::rtx_timer_hook (clos/tcp.cpp) prints one line per
retransmission timeout, ending "... SRC <a> DST <b>". Jobs never share a node
(the benchmark checks it), so the source node identifies the job. This is the
"per-flow RTO attribution" MT_MDP_RESULTS.md section 7 said was missing: until now
only the run's total was counted, so a job's own timeouts in a mix were unknown.

Columns written after PREFIX:
  job_idx,job_decomp,job_nodes_md5,makespan_ms,tp_ms,dp_ms,ep_ms,pp_ms,incomplete,
  rto_job,rto_total
"""
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
