#!/bin/bash
set -uo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
TG="${TG:-$HERE/../fig16/fattree/tgs/100dlrm_16.fbuf}"
RUNS="$HERE/runs"
CSV="$HERE/reconf_verify.csv"

mkdir -p "$RUNS"

COMMON="-nodes 16 -deg 4 -seed 1 -speed 100000 -q 100 -ssthresh 10000 -rtt 1000 -simtime 20"

echo "arm,omethod,rdelay,exit,started,torn_down,completed,immediate,drain_max_ps,drain_mean_ps,buffered,stranded,final_finish_ps,verdict" > "$CSV"

run_one () {
  local arm="$1" bin="$2" omethod="$3" rdelay="$4" extra="${5:-}"
  local queue=ecn; [ "$arm" = roce ] && queue=lossless
  local tag="${arm}_${omethod}_rd${rdelay}"
  local out="$RUNS/$tag.out" err="$RUNS/$tag.err"

  printf '%-34s ' "$tag"
  DYN_SCH_DEBUG=1 timeout 1800 "$DC/$bin" $COMMON \
      -omethod "$omethod" -rdelay "$rdelay" -queue "$queue" -routing updown $extra \
      -flowfile "$TG" -ofile "$RUNS/$tag.csv" > "$out" 2> "$err"
  local rc=$?

  local started=$(grep -m1 "^DynSch_reconfs_started "    "$out" | awk '{print $2}')
  local torn=$(   grep -m1 "^DynSch_reconfs_torn_down "  "$out" | awk '{print $2}')
  local done_=$(  grep -m1 "^DynSch_reconfs_completed "  "$out" | awk '{print $2}')
  local imm=$(    grep -m1 "^DynSch_drains_immediate "   "$out" | awk '{print $2}')
  local dmax=$(   grep -m1 "^DynSch_drain_ps_max "       "$out" | awk '{print $2}')
  local dmean=$(  grep -m1 "^DynSch_drain_ps_mean "      "$out" | awk '{print $2}')
  local buf=$(    grep -m1 "^DynSch_pkts_buffered_during_reconf " "$out" | awk '{print $2}')
  local str=$(    grep -m1 "^DynSch_stranded_on_dead_link "       "$out" | awk '{print $2}')
  local fin=$(    grep -m1 "^FinalFinish "    "$RUNS/$tag.csv" | awk '{print $2}')
  started=${started:-NA}; torn=${torn:-NA}; done_=${done_:-NA}; imm=${imm:-NA}
  dmax=${dmax:-NA}; dmean=${dmean:-NA}; buf=${buf:-NA}; str=${str:-NA}; fin=${fin:-NA}

  local verdict="OK"
  if grep -qi "Assertion" "$err"; then verdict="ASSERT"
  elif [ "$rc" -ne 0 ];               then verdict="EXIT$rc"
  elif [ "$started" = "NA" ];         then verdict="NO_STATS"
  elif [ "$started" != "$torn" ];     then verdict="DRAIN_STALLED"
  elif [ "$str" != "0" ];             then verdict="STRANDED"
  fi


  echo "$arm,$omethod,$rdelay,$rc,$started,$torn,$done_,$imm,$dmax,$dmean,$buf,$str,$fin,$verdict" >> "$CSV"
  printf 'rc=%-3s started=%-4s drained=%-4s immediate=%-4s stranded=%-3s %s\n' \
         "$rc" "$started" "$torn" "$imm" "$str" "$verdict"
}

echo "=== DCTCP arm: the pause path is live here ==="
for om in dheu sipring sipocs; do
  for rd in 1 10 100 1000; do
    run_one dctcp htsim_tcp_dyn_flat_dctcp "$om" "$rd"
  done
done

echo
echo "=== RoCE arm (the pause path now reaches RoCEv2Src) ==="
for rd in 10 100; do
  run_one roce htsim_tcp_dyn_flat dheu "$rd"
done

echo
echo "=== summary ($CSV) ==="
column -s, -t "$CSV"
echo
awk -F, 'NR>1 && $14!="OK" && $14 !~ /^EXPECTED_/ {bad++} END{printf "%s unexpected failure(s)\n", bad+0}' "$CSV"
