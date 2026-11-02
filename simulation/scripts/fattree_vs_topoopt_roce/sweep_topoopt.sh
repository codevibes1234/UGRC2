#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
BIN="$HERE/../ocs/bin_v4"; TG="$HERE/../fig16/fattree/tgs"
RUNS="$HERE/runs_topoopt"; CSV="$HERE/topoopt.csv"
mkdir -p "$RUNS"
[ -f "$CSV" ] || echo "model,nodes,degree,port_gbps,server_gbps,transport,exit,wall_s,rss_kb,final_finish_ps,tasks_done,total_tasks,strides,degree_ar,degree_mp,diameter,pfc_saturated,pfc_overflow,verdict" > "$CSV"

run () {
  local model=$1 n=$2 d=$3 B=$4 tr=$5
  local b=$((B / d)) f="$TG/${B}${model}_${n}.fbuf"
  [ -f "$f" ] || return 0
  grep -q "^$model,$n,$d,$b,$B,$tr," "$CSV" && return 0
  local bin="$BIN/htsim_tcp_dyn_flat" extra="-queue lossless -pfcclasses 8"
  [ "$tr" = dctcp ] && { bin="$BIN/htsim_tcp_dyn_flat_dctcp"; extra="-queue ecn -ecnqexact -rtxscanms 1"; }
  local tag="${model}${n}_d${d}_b${b}_${tr}"
  local out="$RUNS/$tag.out" err="$RUNS/$tag.err" csv="$RUNS/$tag.csv"
  printf '%-34s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" timeout 7200 "$bin" -nodes "$n" -deg "$d" \
    -seed 1 -speed $((b * 1000)) -q 10000 -ssthresh 10000 -rtt 1000 -simtime 3600.1 -routing shortest \
    -logical topoopt -omethod dheu -rdelay 10 $extra -flowfile "$f" -ofile "$csv" > "$out" 2> "$err"
  local rc=$? tl; tl=$(tail -1 "$RUNS/$tag.time")
  local ff=$(grep -m1 '^FinalFinish ' "$csv" 2>/dev/null | awk '{print $2}')
  local done_=$(grep -c 'finished one task' "$err")
  local tot=$(grep 'finished one task' "$err" | tail -1 | sed -n 's/.*ntot \([0-9]*\).*/\1/p')
  local st=$(grep -m1 '^TopoOpt_strides' "$out" | cut -d' ' -f2- | tr ' ' '/')
  local dar=$(grep -m1 '^TopoOpt_degree_ar' "$out" | awk '{print $2}') dmp=$(grep -m1 '^TopoOpt_degree_mp' "$out" | awk '{print $2}')
  local dia=$(grep -m1 '^OCS_diameter' "$out" | awk '{print $2}')
  local sat=$(grep -m1 '^PFC_class_saturated' "$csv" 2>/dev/null | awk '{print $2}')
  local ovf=$(grep -m1 '^PFC_overflow_pkts' "$csv" 2>/dev/null | awk '{print $2}')
  local v=OK
  if [ "$rc" -ne 0 ]; then v="EXIT$rc"; elif [ -z "$ff" ]; then v=NO_RESULT; elif [ -n "$tot" ] && [ "$done_" -lt "$tot" ]; then v=STALLED; fi
  echo "$model,$n,$d,$b,$B,$tr,$rc,$(echo $tl | awk '{print $1}'),$(echo $tl | awk '{print $2}'),${ff:-NA},$done_,${tot:-NA},${st:-NA},${dar:-NA},${dmp:-NA},${dia:-NA},${sat:-NA},${ovf:-NA},$v" >> "$CSV"
  echo "ff=${ff:-NA} tasks=$done_/${tot:-NA} $v"
  for x in "$out" "$err"; do
    if [ "$(stat -c %s "$x")" -gt 2000000 ]; then { head -60 "$x"; echo "...trimmed..."; tail -60 "$x"; } > "$x.t" && mv "$x.t" "$x"; fi
  done
}

BWS="${BWS:-40 100 200 400 1600}"
for d in 4 8; do
  for B in $BWS; do
    [ $((B % d)) -ne 0 ] && continue
    for tr in roce dctcp; do run dlrm 128 "$d" "$B" "$tr"; done
  done
done
for cell in "dlrm 16" "vgg 16" "transformer 16" "candle 16" "dlrm 32" "vgg 32" "transformer 32"; do
  set -- $cell
  for B in $BWS; do for tr in roce dctcp; do run "$1" "$2" 4 "$B" "$tr"; done; done
done
echo "TOPOOPT SWEEP DONE"
