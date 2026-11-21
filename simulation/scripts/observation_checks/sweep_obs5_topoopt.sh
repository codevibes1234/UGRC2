#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
G="$(cd "$HERE/.." && pwd)"
DC="$(cd "$G/../FlexNetPacket/src/clos/datacenter" && pwd)"
TG="$G/fig16/fattree/tgs"
SIMLOCK="$G/.simlock"
RUNS="$HERE/runs_obs5_topoopt"; CSV="$HERE/obs5_topoopt.csv"
mkdir -p "$RUNS"
[ -f "$CSV" ] || echo "model,nodes,degree,port_gbps,server_gbps,variant,exit,wall_s,final_finish_ps,tasks,pfc_pause_sent,pfc_class_saturated,egress_overflow_local" > "$CSV"
run () {
  local model=$1 n=$2 d=$3 B=$4 var=$5; shift 5
  local b=$((B / d)) f="$TG/${B}${model}_${n}.fbuf"
  [ -f "$f" ] || return 0
  grep -q "^$model,$n,$d,$b,$B,$var," "$CSV" && return 0
  local tag="${model}${n}_d${d}_b${b}_${var}"
  flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 7200 "$DC/htsim_tcp_dyn_flat" -nodes "$n" -deg "$d" \
    -seed 1 -speed $((b * 1000)) -q 10000 -ssthresh 10000 -rtt 1000 -simtime 3600.1 -routing shortest \
    -logical topoopt -omethod dheu -rdelay 10 -queue lossless -pfcclasses 8 "$@" -flowfile "$f" -ofile "$RUNS/$tag.csv" \
    > "$RUNS/$tag.out" 2> "$RUNS/$tag.err"
  local rc=$? ff=$(grep -m1 '^FinalFinish ' "$RUNS/$tag.csv" 2>/dev/null | awk '{print $2}')
  local t=$(grep 'finished one task' "$RUNS/$tag.err" | tail -1 | grep -o 'nfin [0-9]* ntot [0-9]*' | awk '{print $2"/"$4}')
  local ps=$(grep -m1 '^PFC_pause_sent ' "$RUNS/$tag.csv" 2>/dev/null | awk '{print $2}')
  local sat=$(grep -m1 '^PFC_class_saturated ' "$RUNS/$tag.csv" 2>/dev/null | awk '{print $2}')
  local ov=$(grep -m1 '^EGRESS_overflow_local_pkts ' "$RUNS/$tag.csv" 2>/dev/null | awk '{print $2}')
  for x in "$RUNS/$tag.out" "$RUNS/$tag.err"; do
    if [ "$(stat -c %s "$x")" -gt 2000000 ]; then { head -40 "$x"; tail -40 "$x"; } > "$x.t" && mv "$x.t" "$x"; fi
  done
  echo "$model,$n,$d,$b,$B,$var,$rc,$(tail -1 "$RUNS/$tag.time"),${ff:-NA},${t:-NA},${ps:-NA},${sat:-NA},${ov:-NA}" >> "$CSV"
}
for cell in "dlrm 16" "vgg 16" "transformer 16"; do
  set -- $cell
  for B in 40 80 100 200 400 1600; do run "$1" "$2" 4 "$B" creditprio -creditprio 1; done
done
run dlrm 128 4 40 creditprio -creditprio 1
run dlrm 128 4 100 creditprio -creditprio 1
echo "OBS5 SWEEP DONE"
