#!/bin/bash

SIMLOCK="${SIMLOCK:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/.simlock}"
DC="${DC:-$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../FlexNetPacket/src/clos/datacenter}"
TGD="${TGD:-$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../fig16/fattree/tgs}"

df_header () {
  echo "series,workload,nodes,deg,transport,routing,arm,variant,extra,exit,wall_s,rss_kb,reconfs,drain_ps_mean,drain_ps_max,reconf_delay_ps,reconf_period_ps,pfc_stop,tasks,total_tasks,makespan_ps,pfc_peak_pkts,pfc_overshoot_pkts,pfc_overflow,egress_peak_transit_pkts,egress_overflow_local,egress_overflow_transit,egress_transit_only_overflow,verdict" > "$CSV"
}

df_run () {
  local series="$1" wl="$2" n="$3" deg="$4" tr="$5" routing="$6" arm="$7" var="$8"
  shift 8
  if [ -n "${RERUN_ONLY:-}" ] && [ "$tr:$arm" != "$RERUN_ONLY" ]; then return 0; fi
  local bin=htsim_tcp_dyn_flat queue=lossless trx="${ROCE_EXTRA:-}"
  [ "$tr" = dctcp ] && { bin=htsim_tcp_dyn_flat_dctcp; queue=ecn; trx=""; }
  local logical="-logical $arm"
  [ "$arm" = dheu ] && logical=""
  local rd="-rdelay 10"
  case " $* " in *" -rdelay "*|*" -rdelayns "*) rd="";; esac
  local tag="${series}_${wl}${n}_${tr}_${routing}_${arm}_${var}"
  local out="$RUNS/$tag.out" err="$RUNS/$tag.err" csv="$RUNS/$tag.csv"

  printf '%-60s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" \
  timeout 3600 "${DF_BIN_DIR:-$DC}/$bin" -nodes "$n" -deg "$deg" -seed 1 -speed 100000 -q 100 \
      -ssthresh 10000 -rtt 1000 -simtime 60 -queue "$queue" -routing "$routing" \
      $logical -omethod dheu $rd "$@" $trx \
      -flowfile "$TGD/100${wl}_${n}.fbuf" -ofile "$csv" > "$out" 2> "$err"
  local rc=$?
  local tl=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tl" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tl" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')
  gc () { grep -m1 "^$1 " "$csv" 2>/dev/null | awk '{print $2}'; }
  go () { grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'; }
  local rec=$(gc DynSch_reconfs_started) torn=$(gc DynSch_reconfs_torn_down) dmean=$(gc DynSch_drain_ps_mean) dmax=$(gc DynSch_drain_ps_max)
  local rdp=$(go reconf_delay_ps) rpp=$(go reconf_period_ps)
  local stop=$(grep -c 'PAUSE 1000' "$out")
  local tasks=$(grep -c 'finished one task' "$err")
  local tot=$(grep 'finished one task' "$err" | tail -1 | sed -n 's/.*ntot \([0-9]*\).*/\1/p')
  local mk=$(gc FinalFinish)
  local pk=$(gc PFC_peak_occupancy_pkts) os=$(gc PFC_peak_overshoot_pkts) of=$(gc PFC_overflow_pkts)
  local et=$(gc EGRESS_peak_transit_pkts) el=$(gc EGRESS_overflow_local_pkts)
  local etr=$(gc EGRESS_overflow_transit_pkts) eto=$(gc EGRESS_overflow_transit_only_pkts)
  local v
  for v in rec dmean dmax rdp rpp tot mk pk os of et el etr eto; do
    eval "[ -z \"\$$v\" ] && $v=NA"; done
  local verdict=OK
  if   grep -qi "Assertion" "$err";                 then verdict=ASSERT
  elif [ "$rc" -ne 0 ];                             then verdict="EXIT$rc"
  elif [ "$mk" = NA ];                              then verdict=NO_RESULT
  elif [ "$tot" != NA ] && [ "$tasks" -lt "$tot" ]; then verdict=STALLED
  elif [ -n "$torn" ] && [ "$rec" != "$torn" ];     then verdict=DRAIN_STALLED
  fi
  local extra=$(echo "$*" | tr ',' ';')
  echo "$series,$wl,$n,$deg,$tr,$routing,$arm,$var,$extra,$rc,$wall,$rss,$rec,$dmean,$dmax,$rdp,$rpp,$stop,$tasks,$tot,$mk,$pk,$os,$of,$et,$el,$etr,$eto,$verdict" >> "$CSV"
  printf 'mk=%-14s tasks=%s/%s reconfs=%-5s ovs=%-6s %ss %s\n' "$mk" "$tasks" "$tot" "$rec" "$os" "$wall" "$verdict"
}
