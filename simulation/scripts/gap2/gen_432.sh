#!/bin/bash
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV="${VENV:-/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/fcvenv}"
for f in jellyfish flatbutterfly dragonfly megafly; do
  ( s=$(date +%s)
    /usr/bin/time -f "%e s %M KB" "$VENV/bin/python" -u "$HERE/generate_graph_schedule.py" \
        --graph "$HERE/graphs/${f}_432.graph" --nic-gbps 100 --fast-maxflow \
        --out-cpp "$HERE/schedules/${f}_432_fc_schedule.cpp" --out-xml "$HERE/schedules/${f}_432.xml" \
        > "$HERE/schedules/${f}_432.gen.log" 2>&1
    echo "$f rc=$? $(( $(date +%s) - s ))s $(tail -1 "$HERE/schedules/${f}_432.gen.log")" ) &
done
wait
echo "GEN 432 DONE"
