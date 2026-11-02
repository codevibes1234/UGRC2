#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
RUNS="$HERE/runs_pfc_classes_graph"; CSV="$HERE/pfc_classes_graph.csv"
mkdir -p "$RUNS"
echo "topo,case,transport,routing,classes,exit,wall_s,job0_ms,makespan_ms,incomplete,pfc_stop,saturated,verdict" > "$CSV"
run () {
  local topo=$1 cs=$2 tr=$3 rt=$4 k=$5; shift 5
  local bin="$HERE/bin_pfc/$tr/htsim_tcp_mt_mdp_bench" kk=""
  [ "$tr" = roce ] && kk="-pfcclasses $k"
  local tag="${topo}_${cs}_${tr}_${rt}_k$k" out="$RUNS/${topo}_${cs}_${tr}_${rt}_k$k.txt" log="$RUNS/${topo}_${cs}_${tr}_${rt}_k$k.log"
  printf '%-44s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$bin" -topo "$topo" -nodes 128 \
    -routing "$rt" "$@" -dims tp,dp,ep,pp -placement compact -placementseed 13 \
    -speed 200000 -q 200 -ssthresh 15 -simtime 600 $kk -ofile "$out" > "$log" 2>&1
  local rc=$? g; g () { grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'; }
  local inc=$(g Incomplete) sat=$(g PFC_class_saturated) v=OK
  [ "$rc" -ne 0 ] && v="EXIT$rc"; [ -z "$inc" ] && v=NO_RESULT; [ -n "$inc" ] && [ "$inc" != 0 ] && v=STALLED
  echo "$topo,$cs,$tr,$rt,$k,$rc,$(tail -1 "$RUNS/$tag.time"),$(g Job0_Makespan_ms),$(g Makespan_ms),${inc:-NA},$(grep -c 'PAUSE 1000' "$log"),${sat:-NA},$v" >> "$CSV"
  echo "job0=$(g Job0_Makespan_ms) inc=${inc:-NA} sat=${sat:-} $v"
  if [ "$(stat -c %s "$log")" -gt 2000000 ]; then { head -50 "$log"; echo "...trimmed..."; tail -50 "$log"; } > "$log.t" && mv "$log.t" "$log"; fi
}
for topo in jellyfish flatbutterfly dragonfly megafly; do
  for cs in "full -jobs 1 -jobnodes 128 -jobtp 4 -jobdp 2 -jobpp 4 -jobep 4" "ten4 -jobs 4 -jobnodes 32 -jobtp 4 -jobdp 2 -jobpp 2 -jobep 2"; do
    set -- $cs; c=$1; shift
    run "$topo" "$c" roce updown 1 "$@"
    run "$topo" "$c" roce ecmp 1 "$@"
    run "$topo" "$c" roce ecmp 8 "$@"
    run "$topo" "$c" dctcp updown 1 "$@"
    run "$topo" "$c" dctcp ecmp 1 "$@"
  done
done
echo "PFC CLASSES GRAPH SWEEP DONE"
