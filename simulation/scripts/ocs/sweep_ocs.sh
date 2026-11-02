#!/bin/bash
set -uo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
TG="${TG:-$HERE/../fig16/fattree/tgs/100dlrm_16.fbuf}"
RUNS="$HERE/${OUT_PREFIX:-}runs_sweep"
CSV="$HERE/${OUT_PREFIX:-}sweep_ocs.csv"
mkdir -p "$RUNS"

COMMON="-nodes 16 -deg 4 -seed 1 -speed 100000 -q 100 -ssthresh 10000 -rtt 1000 -simtime 20"

echo "transport,routing,omethod,rdelay,exit,wall_s,started,torn_down,immediate,real_drains,drain_max_ps,drain_mean_ps,buffered,stranded,pfc_stop,pfc_resume,tasks,makespan_ps,verdict" > "$CSV"

run_one () {
  local arm="$1" routing="$2" omethod="$3" rdelay="$4"
  if [ -n "${RERUN_ONLY:-}" ] && [ "$arm:$omethod" != "$RERUN_ONLY" ]; then return 0; fi
  local bin queue trx=""
  if [ "$arm" = dctcp ]; then bin=htsim_tcp_dyn_flat_dctcp; queue=ecn
                         else bin=htsim_tcp_dyn_flat;       queue=lossless; trx="${ROCE_EXTRA:-}"; fi
  local tag="${arm}_${routing}_${omethod}_rd${rdelay}"
  local out="$RUNS/$tag.out" err="$RUNS/$tag.err" csv="$RUNS/$tag.csv"

  printf '%-40s ' "$tag"
  local s=$(date +%s.%N)
  DYN_SCH_DEBUG=1 flock "$SIMLOCK" timeout 2400 "$DC/$bin" $COMMON \
      -omethod "$omethod" -rdelay "$rdelay" -queue "$queue" -routing "$routing" $trx \
      -flowfile "$TG" -ofile "$csv" > "$out" 2> "$err"
  local rc=$?
  local wall=$(echo "$(date +%s.%N) - $s" | bc)

  g () { grep -m1 "^$1 " "$out" | awk '{print $2}'; }
  local started=$(g DynSch_reconfs_started)   torn=$(g DynSch_reconfs_torn_down)
  local imm=$(g DynSch_drains_immediate)      dmax=$(g DynSch_drain_ps_max)
  local dmean=$(g DynSch_drain_ps_mean)       buf=$(g DynSch_pkts_buffered_during_reconf)
  local str=$(g DynSch_stranded_on_dead_link)
  local stop=$(grep -c 'PAUSE 1000' "$out") resume=$(grep -c 'PAUSE 0' "$out")
  local tasks=$(grep -c 'finished one task' "$err")
  local mk=$(grep -m1 '^FinalFinish ' "$csv" 2>/dev/null | awk '{print $2}')
  started=${started:-NA}; torn=${torn:-NA}; imm=${imm:-NA}; dmax=${dmax:-NA}
  dmean=${dmean:-NA}; buf=${buf:-NA}; str=${str:-NA}; mk=${mk:-NA}
  local real=NA
  [ "$started" != NA ] && [ "$imm" != NA ] && real=$(( started - imm ))

  local verdict=OK
  if   grep -qi "Assertion" "$err";        then verdict=ASSERT
  elif [ "$rc" -ne 0 ];                    then verdict="EXIT$rc"
  elif [ "$started" = NA ];                then verdict=NO_STATS
  elif [ "$tasks" -lt 1278 ];              then verdict=STALLED
  elif [ "$started" != "$torn" ];          then verdict=DRAIN_STALLED
  elif [ "$str" != 0 ];                    then verdict=STRANDED
  fi
  if [ "$arm" = roce ] && [ "$routing" = shortest ] && [ "$verdict" != OK ]; then
    verdict="EXPECTED_$verdict"
  fi

  echo "$arm,$routing,$omethod,$rdelay,$rc,$wall,$started,$torn,$imm,$real,$dmax,$dmean,$buf,$str,$stop,$resume,$tasks,$mk,$verdict" >> "$CSV"
  printf 'tasks=%-5s drains=%-4s/%-4s pfc=%s/%s mk=%-14s %s\n' \
         "$tasks" "$real" "$started" "$stop" "$resume" "$mk" "$verdict"
}

for arm in dctcp roce; do
  for routing in updown shortest; do
    for rd in 1 10 100 1000; do
      run_one "$arm" "$routing" dheu "$rd"
    done
  done
done

echo
echo "=== static-topology baseline (sipring retunes nothing without Gurobi) ==="
run_one dctcp updown sipring 10
run_one roce  updown sipring 10

echo
column -s, -t "$CSV"
echo
awk -F, 'NR>1 && $19!="OK" && $19 !~ /^EXPECTED_/ {bad++} END{printf "%s unexpected failure(s)\n", bad+0}' "$CSV"
