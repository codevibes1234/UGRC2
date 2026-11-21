#!/bin/bash

SIMLOCK="${SIMLOCK:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/.simlock}"
DC="${DC:-$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../FlexNetPacket/src/clos/datacenter}"

mdp_header () {
  echo "series,nodes,deg,arm,transport,placement,jobs,jobnodes,decomp,allreduce,${MDP_LABEL_COL:-pfc_variant},exit,wall_s,rss_kb,construction,links,diameter,mean_hops,reconfs,pfc_stop,job0_ms,job0_tp_ms,job0_dp_ms,job0_ep_ms,job0_pp_ms,makespan_ms,incomplete,pfc_peak_pkts,pfc_overshoot_pkts,pfc_overflow,egress_peak_transit_pkts,egress_transit_only_overflow,verdict" > "$CSV"
}

mdp_run () {
  local series="$1" n="$2" deg="$3" arm="$4" tr="$5" pol="$6" jobs="$7" jn="$8"
  local tp="$9" dp="${10}" pp="${11}" ep="${12}" ar="${13}" pv="${14}"
  shift 14
  if [ -n "${RERUN_ONLY:-}" ] && [ "$tr:$arm" != "$RERUN_ONLY" ]; then return 0; fi
  local bin="$DC/htsim_tcp_mt_mdp_bench"
  [ -n "${MDP_BIN_DIR:-}" ] && bin="$MDP_BIN_DIR/roce/htsim_tcp_mt_mdp_bench"
  local trx="${ROCE_EXTRA:-}"
  [ "$tr" = dctcp ] && { bin="$DC/dctcp_bin/htsim_tcp_mt_mdp_bench"; trx="${MDP_DCTCP_EXTRA:-}"; }
  [ "$tr" = dctcp ] && [ -n "${MDP_BIN_DIR:-}" ] && bin="$MDP_BIN_DIR/dctcp/htsim_tcp_mt_mdp_bench"
  local logical="-logical $arm"
  [ "$arm" = dheu ] && logical=""
  local tag="${series}_n${n}d${deg}_${arm}_${tr}_${pol}_j${jobs}_tp${tp}dp${dp}pp${pp}ep${ep}_${ar}_${pv}"
  local out="$RUNS/$tag.txt" log="$RUNS/$tag.log"

  printf '%-78s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" \
  timeout 3600 "$bin" -topo ocs $logical -deg "$deg" -rdelay 10 \
      -nodes "$n" -jobs "$jobs" -jobnodes "$jn" \
      -jobtp "$tp" -jobdp "$dp" -jobpp "$pp" -jobep "$ep" -dims tp,dp,ep,pp \
      -allreduce "$ar" -speed 100000 -q 100 -ssthresh 10000 -simtime 60 \
      -routing updown -placement "$pol" -placementseed 13 "$@" $trx \
      -ofile "$out" > "$log" 2>&1
  local rc=$?
  local tl=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tl" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tl" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')
  gl () { grep -m1 "^$1 " "$log" 2>/dev/null | awk '{print $2}'; }
  gt () { grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'; }
  local cons=$(grep -m1 '^OCS_construction ' "$log" 2>/dev/null | cut -d' ' -f2- | tr ',' ';')
  [ "$arm" = dheu ] && cons="demand-driven"
  local links=$(gl OCS_links) diam=$(gl OCS_diameter) mh=$(gl OCS_mean_hops)
  local rec=$(gl DynSch_reconfs_started)
  local stop=$(grep -c 'PAUSE 1000' "$log")
  local j0=$(gt Job0_Makespan_ms) mk=$(gt Makespan_ms) inc=$(gt Incomplete)
  local ctp=$(gt Job0_Class_tp_ms) cdp=$(gt Job0_Class_dp_ms)
  local cep=$(gt Job0_Class_ep_ms) cpp=$(gt Job0_Class_pp_ms)
  local pk=$(gt PFC_peak_occupancy_pkts) os=$(gt PFC_peak_overshoot_pkts) of=$(gt PFC_overflow_pkts)
  local et=$(gt EGRESS_peak_transit_pkts) eto=$(gt EGRESS_overflow_transit_only_pkts)
  local v
  for v in cons links diam mh rec j0 mk inc ctp cdp cep cpp pk os of et eto; do
    eval "[ -z \"\$$v\" ] && $v=NA"; done
  local verdict=OK
  if   grep -qi "Assertion" "$log";            then verdict=ASSERT
  elif grep -q "ERROR" "$log";                 then verdict=ERROR
  elif [ "$rc" -ne 0 ];                        then verdict="EXIT$rc"
  elif [ "$mk" = NA ];                         then verdict=NO_RESULT
  elif [ "$inc" != NA ] && [ "$inc" != 0 ];    then verdict=INCOMPLETE
  fi
  echo "$series,$n,$deg,$arm,$tr,$pol,$jobs,$jn,tp$tp.dp$dp.pp$pp.ep$ep,$ar,$pv,$rc,$wall,$rss,$cons,$links,$diam,$mh,$rec,$stop,$j0,$ctp,$cdp,$cep,$cpp,$mk,$inc,$pk,$os,$of,$et,$eto,$verdict" >> "$CSV"
  printf 'job0=%-9s mk=%-9s tp=%-8s dp=%-8s %ss %s\n' "$j0" "$mk" "$ctp" "$cdp" "$wall" "$verdict"
}
