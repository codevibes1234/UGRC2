#!/bin/bash
set -uo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
TGD="$HERE/../fig16/fattree/tgs"
RUNS="$HERE/${OUT_PREFIX:-}runs_logical"; CSV="$HERE/${OUT_PREFIX:-}logical_ocs.csv"
mkdir -p "$RUNS"

echo "workload,nodes,degree,transport,arm,exit,wall_s,rss_kb,links,deg_min,deg_max,deg_mean,diameter,mean_hops,reconfs,pfc_stop,pfc_resume,tasks,total_tasks,makespan_ps,verdict" > "$CSV"

run_one () {
  local wl="$1" n="$2" deg="$3" arm="$4" armtype="$5"
  if [ -n "${RERUN_ONLY:-}" ] && [ "$6:$arm" != "$RERUN_ONLY" ]; then return 0; fi
  local bin=htsim_tcp_dyn_flat queue=lossless trx="${ROCE_EXTRA:-}"
  [ "$6" = dctcp ] && { bin=htsim_tcp_dyn_flat_dctcp; queue=ecn; trx=""; }
  local tr="$6"
  local tag="${wl}${n}_${tr}_${arm}"
  local out="$RUNS/$tag.out" err="$RUNS/$tag.err" csv="$RUNS/$tag.csv"

  local extra="-logical $arm -omethod dheu -rdelay 10"
  [ "$armtype" = dynamic ] && extra="-omethod dheu -rdelay 10"

  printf '%-34s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" \
  timeout 3600 "$DC/$bin" -nodes "$n" -deg "$deg" -seed 1 -speed 100000 -q 100 \
      -ssthresh 10000 -rtt 1000 -simtime 60 -queue "$queue" -routing updown \
      $extra $trx -flowfile "$TGD/100${wl}_${n}.fbuf" -ofile "$csv" > "$out" 2> "$err"
  local rc=$?
  local tl=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tl" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tl" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')

  g () { grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'; }
  local links=$(g OCS_links) dmin=$(g OCS_degree_min) dmax=$(g OCS_degree_max)
  local dmean=$(g OCS_degree_mean) diam=$(g OCS_diameter) mh=$(g OCS_mean_hops)
  local rec=$(grep -m1 '^DynSch_reconfs_started ' "$csv" 2>/dev/null | awk '{print $2}')
  local stop=$(grep -c 'PAUSE 1000' "$out") resume=$(grep -c 'PAUSE 0' "$out")
  local tasks=$(grep -c 'finished one task' "$err")
  local tot=$(grep 'finished one task' "$err" | tail -1 | sed -n 's/.*ntot \([0-9]*\).*/\1/p')
  local mk=$(grep -m1 '^FinalFinish ' "$csv" 2>/dev/null | awk '{print $2}')
  for v in links dmin dmax dmean diam mh rec tot mk; do
    eval "[ -z \"\$$v\" ] && $v=NA"; done

  local verdict=OK
  if   grep -qi "Assertion" "$err";                 then verdict=ASSERT
  elif [ "$rc" -ne 0 ];                             then verdict="EXIT$rc"
  elif [ "$mk" = NA ];                              then verdict=NO_RESULT
  elif [ "$tot" != NA ] && [ "$tasks" -lt "$tot" ]; then verdict=STALLED
  fi

  echo "$wl,$n,$deg,$tr,$arm,$rc,$wall,$rss,$links,$dmin,$dmax,$dmean,$diam,$mh,$rec,$stop,$resume,$tasks,$tot,$mk,$verdict" >> "$CSV"
  printf 'links=%-4s diam=%-3s tasks=%-5s/%-5s mk=%-14s %ss %s\n' \
         "$links" "$diam" "$tasks" "$tot" "$mk" "$wall" "$verdict"
}

ARMS="jellyfish flatbutterfly dragonfly megafly"

echo "########## 16 nodes, degree 4 ##########"
for wl in dlrm vgg transformer candle; do
  for tr in roce dctcp; do
    for a in $ARMS; do run_one "$wl" 16 4 "$a" static "$tr"; done
    run_one "$wl" 16 4 dheu dynamic "$tr"
  done
done

echo
echo "########## 32 nodes, degree 5 (candle skipped: 6.25 GB) ##########"
for wl in dlrm vgg transformer; do
  for tr in roce dctcp; do
    for a in $ARMS; do run_one "$wl" 32 5 "$a" static "$tr"; done
    run_one "$wl" 32 5 dheu dynamic "$tr"
  done
done

echo
column -s, -t "$CSV"
echo
awk -F, 'NR>1 && $21!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
