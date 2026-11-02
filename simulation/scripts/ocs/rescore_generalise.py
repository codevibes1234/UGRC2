#!/usr/bin/env python3
"""
Recompute the generalisation sweep's table from the retained run artifacts.

Why this exists: the first run of generalise_ocs.sh had two reporting defects -
/usr/bin/time prepends "Command terminated by signal N" when a run is OOM-killed,
which split one CSV row across three lines, and the ntot extraction returned 1 for
completed runs so the "did the taskgraph finish?" check never fired. Every run's
.out/.err/.csv/.time file is kept, so the table can be rebuilt without spending
another hour of simulation.

Reads runs_gen/*.{out,err,csv,time} and writes generalise_ocs_rescored.csv.
"""
import csv
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
RUNS = os.path.join(HERE, "runs_gen")
OUT = os.path.join(HERE, "generalise_ocs_rescored.csv")

TAG = re.compile(r"^(?P<wl>[a-z]+)(?P<n>\d+)_(?P<arm>dctcp|roce)_"
                 r"(?P<routing>updown|shortest)_(?P<om>[a-z]+)_rd(?P<rd>\d+)$")


def stat_from(path, key):
    try:
        with open(path, errors="replace") as f:
            for line in f:
                if line.startswith(key + " "):
                    return line.split()[1]
    except OSError:
        pass
    return None


def count_in(path, needle):
    n = 0
    try:
        with open(path, errors="replace") as f:
            for line in f:
                if needle in line:
                    n += 1
    except OSError:
        return 0
    return n


def last_ntot(path):
    """ntot off the last task-finish line - read positionally, not by regex group 2."""
    tot = None
    try:
        with open(path, errors="replace") as f:
            for line in f:
                if "finished one task" in line:
                    m = re.search(r"ntot (\d+)", line)
                    if m:
                        tot = m.group(1)
    except OSError:
        pass
    return tot


def time_fields(path):
    """Last line of a /usr/bin/time -f '%e %M' file, tolerating a signal message."""
    wall = rss = "NA"
    killed = False
    try:
        with open(path, errors="replace") as f:
            lines = [l.strip() for l in f if l.strip()]
    except OSError:
        return wall, rss, killed
    for l in lines:
        if "terminated by signal" in l:
            killed = True
    if lines:
        parts = lines[-1].split()
        if len(parts) >= 2:
            if re.fullmatch(r"[0-9.]+", parts[0]):
                wall = parts[0]
            if re.fullmatch(r"[0-9]+", parts[1]):
                rss = parts[1]
    return wall, rss, killed


def main():
    rows = []
    for out in sorted(glob.glob(os.path.join(RUNS, "*.out"))):
        tag = os.path.basename(out)[:-4]
        m = TAG.match(tag)
        if not m:
            print(f"  skipping unparsable tag {tag}", file=sys.stderr)
            continue
        d = m.groupdict()
        err = os.path.join(RUNS, tag + ".err")
        csvf = os.path.join(RUNS, tag + ".csv")
        timef = os.path.join(RUNS, tag + ".time")

        wall, rss, killed = time_fields(timef)
        started = stat_from(csvf, "DynSch_reconfs_started")
        torn = stat_from(csvf, "DynSch_reconfs_torn_down")
        imm = stat_from(csvf, "DynSch_drains_immediate")
        stranded = stat_from(csvf, "DynSch_stranded_on_dead_link")
        mk = stat_from(csvf, "FinalFinish")
        stop = count_in(out, "PAUSE 1000")
        resume = count_in(out, "PAUSE 0")
        tasks = count_in(err, "finished one task")
        tot = last_ntot(err)

        real = "NA"
        if started and imm:
            real = str(int(started) - int(imm))

        if killed:
            verdict = "OOM_KILLED"
        elif started is None:
            verdict = "NO_STATS"
        elif tot is not None and tasks < int(tot):
            verdict = "STALLED"
        elif started != torn:
            verdict = "DRAIN_STALLED"
        elif stranded not in (None, "0"):
            verdict = "STRANDED"
        else:
            verdict = "OK"
        if d["arm"] == "roce" and d["routing"] == "shortest" and verdict != "OK":
            verdict = "EXPECTED_" + verdict

        rows.append(dict(
            workload=d["wl"], nodes=d["n"], transport=d["arm"], routing=d["routing"],
            omethod=d["om"], rdelay=d["rd"], wall_s=wall, rss_kb=rss,
            started=started or "NA", torn_down=torn or "NA", real_drains=real,
            pfc_stop=stop, pfc_resume=resume, tasks=tasks, total_tasks=tot or "NA",
            makespan_ps=mk or "NA", verdict=verdict))

    rows.sort(key=lambda r: (r["workload"], int(r["nodes"]), r["transport"],
                             r["routing"], r["omethod"], int(r["rdelay"])))
    cols = ["workload", "nodes", "transport", "routing", "omethod", "rdelay",
            "wall_s", "rss_kb", "started", "torn_down", "real_drains",
            "pfc_stop", "pfc_resume", "tasks", "total_tasks", "makespan_ps", "verdict"]
    with open(OUT, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=cols)
        w.writeheader()
        w.writerows(rows)

    bad = [r for r in rows if r["verdict"] != "OK"
           and not r["verdict"].startswith("EXPECTED_")]
    print(f"{len(rows)} runs rescored -> {OUT}")
    print(f"{len(bad)} unexpected failure(s)")
    for r in bad:
        print(f"  {r['workload']}{r['nodes']} {r['transport']} {r['routing']} "
              f"{r['omethod']} rd{r['rdelay']}: {r['verdict']} "
              f"(tasks {r['tasks']}/{r['total_tasks']}, rss {r['rss_kb']} kB)")


if __name__ == "__main__":
    main()
