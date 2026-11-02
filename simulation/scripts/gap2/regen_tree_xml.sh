#!/bin/bash
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV="${VENV:-/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/fcvenv}"
for f in flatbutterfly megafly; do
  s=$(date +%s)
  "$VENV/bin/python" "$HERE/generate_graph_schedule.py" --graph "$HERE/graphs/${f}_128.graph" \
      --nic-gbps 100 --out-cpp "$HERE/schedules/${f}_128_fc_schedule.regen.cpp" \
      --out-xml "$HERE/schedules/${f}_128.xml" > "$HERE/schedules/${f}_128.regen.log" 2>&1
  rc=$?
  e=$(date +%s)
  if cmp -s "$HERE/schedules/${f}_128_fc_schedule.cpp" "$HERE/schedules/${f}_128_fc_schedule.regen.cpp"; then
    same=IDENTICAL; rm -f "$HERE/schedules/${f}_128_fc_schedule.regen.cpp"
  else
    same=DIFFERENT
  fi
  echo "$f rc=$rc $((e - s))s step-schedule-vs-stored=$same"
done
echo "REGEN DONE"
