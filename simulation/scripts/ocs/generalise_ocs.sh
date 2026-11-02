#!/bin/bash
set -uo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
TGD="$HERE/../fig16/fattree/tgs"
RUNS="$HERE/${OUT_PREFIX:-}runs_gen"
CSV="$HERE/${OUT_PREFIX:-}generalise_ocs.csv"
mkdir -p "$RUNS"

echo "workload,nodes,transport,routing,omethod,rdelay,exit,wall_s,rss_kb,started,torn_down,real_drains,pfc_stop,pfc_resume,tasks,total_tasks,makespan_ps,verdict" > "$CSV"

run_one () {
  local wl="$1" n="$2" arm="$3" routing="$4" omethod="$5" rdelay="$6"
  if [ -n "${RERUN_ONLY:-}" ] && [ "$arm:$omethod" != "$RERUN_ONLY" ]; then return 0; fi
  local bin queue trx=""
  if [ "$arm" = dctcp ]; then bin=htsim_tcp_dyn_flat_dctcp; queue=ecn
                         else bin=htsim_tcp_dyn_flat;       queue=lossless; trx="${ROCE_EXTRA:-}"; fi
  local tag="${wl}${n}_${arm}_${routing}_${omethod}_rd${rdelay}"
  local out="$RUNS/$tag.out" err="$RUNS/$tag.err" csv="$RUNS/$tag.csv"

  printf '%-46s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" \
  timeout 1800 "$DC/$bin" -nodes "$n" -deg 4 -seed 1 -speed 100000 -q 100 \
      -ssthresh 10000 -rtt 1000 -simtime 20 \
      -omethod "$omethod" -rdelay "$rdelay" -queue "$queue" -routing "$routing" $trx \
      -flowfile "$TGD/100${wl}_${n}.fbuf" -ofile "$csv" > "$out" 2> "$err"
  local rc=$?
  local tline=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tline" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tline" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')

  g () { grep -m1 "^$1 " "$csv" 2>/dev/null | awk '{print $2}'; }
  local started=$(g DynSch_reconfs_started) torn=$(g DynSch_reconfs_torn_down)
  local imm=$(g DynSch_drains_immediate)
  local stop=$(grep -c 'PAUSE 1000' "$out") resume=$(grep -c 'PAUSE 0' "$out")
  local tot=$(grep 'finished one task' "$err" | tail -1 \
              | sed -n 's/.*ntot \([0-9]*\).*/\1/p')
  local tasks=$(grep -c 'finished one task' "$err")
  local mk=$(grep -m1 '^FinalFinish ' "$csv" 2>/dev/null | awk '{print $2}')
  started=${started:-NA}; torn=${torn:-NA}; imm=${imm:-NA}; mk=${mk:-NA}
  tot=${tot:-NA}; wall=${wall:-NA}; rss=${rss:-NA}
  local real=NA
  [ "$started" != NA ] && [ "$imm" != NA ] && real=$(( started - imm ))

  local verdict=OK
  if   grep -qi "Assertion" "$err";                     then verdict=ASSERT
  elif [ "$rc" -ne 0 ];                                 then verdict="EXIT$rc"
  elif [ "$started" = NA ];                             then verdict=NO_STATS
  elif [ "$tot" != NA ] && [ "$tasks" -lt "$tot" ];     then verdict=STALLED
  elif [ "$started" != "$torn" ];                       then verdict=DRAIN_STALLED
  fi
  if [ "$arm" = roce ] && [ "$routing" = shortest ] && [ "$verdict" != OK ]; then
    verdict="EXPECTED_$verdict"
  fi

  echo "$wl,$n,$arm,$routing,$omethod,$rdelay,$rc,$wall,$rss,$started,$torn,$real,$stop,$resume,$tasks,$tot,$mk,$verdict" >> "$CSV"
  printf 'tasks=%-5s/%-5s drains=%-4s/%-4s mk=%-15s %ss %s\n' \
         "$tasks" "$tot" "$real" "$started" "$mk" "$wall" "$verdict"
}

do_cell () {
  local wl="$1" n="$2"
  for arm in dctcp roce; do
    run_one "$wl" "$n" "$arm" updown dheu    1
    run_one "$wl" "$n" "$arm" updown dheu    10
    run_one "$wl" "$n" "$arm" updown dheu    1000
    if [ "$n" -lt 32 ]; then
      run_one "$wl" "$n" "$arm" updown sipring 10
    else
      echo "  (skipping ${wl}${n}_${arm}_sipring: needs 5-7 GB on a 7 GB machine)"
    fi
  done
  run_one "$wl" "$n" roce shortest dheu 10
}

echo "===== 16 nodes ====="
for wl in dlrm vgg transformer candle; do do_cell "$wl" 16; done

echo
echo "===== 32 nodes (candle excluded: 6.25 GB RSS) ====="
for wl in dlrm transformer vgg; do do_cell "$wl" 32; done

echo
column -s, -t "$CSV"
echo
awk -F, 'NR>1 && $18!="OK" && $18 !~ /^EXPECTED_/ {bad++} END{printf "%s unexpected failure(s)\n", bad+0}' "$CSV"
