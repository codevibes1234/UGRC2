#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
CSV="$HERE/obs3_place.csv"; RUNS="$HERE/runs_obs3_place"; mkdir -p "$RUNS"
[ -f "$CSV" ] || echo "arm,placement,jobs,placement_seed,exit,job0_ms,makespan_ms,incomplete,wall_s" > "$CSV"
run_pl () {
  local arm=$1 pol=$2 j=$3 s=$4
  grep -q "^$arm,$pol,$j,$s," "$CSV" && return 0
  local tag="pl_${arm}_${pol}_j${j}_s${s}"
  flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$DC/htsim_tcp_mt_mdp_bench" \
    -topo ocs -deg 9 -rdelay 10 -logical "$arm" -nodes 128 -dims tp,dp,ep,pp -placement "$pol" -placementseed "$s" \
    -speed 100000 -q 100 -ssthresh 10000 -simtime 60 -routing updown -jobs "$j" -jobnodes 16 -jobtp 2 -jobdp 4 \
    -jobpp 2 -jobep 1 ${CP_EXTRA--creditprio 1} -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.log" 2>&1
  local rc=$?
  g () { grep -m1 "^$1 " "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}'; }
  tail -5 "$RUNS/$tag.log" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.log"
  echo "$arm,$pol,$j,$s,$rc,$(g Job0_Makespan_ms),$(g Makespan_ms),$(g Incomplete),$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
  printf '%-40s job0=%s rc=%s\n' "$tag" "$(g Job0_Makespan_ms)" "$rc"
}
for arm in flatbutterfly dragonfly jellyfish megafly; do
  for pol in compact scatter; do for j in 1 8; do
    for s in 13 1 2 3 4; do run_pl "$arm" "$pol" "$j" "$s"; done
  done; done
done
echo "PLACE done"
