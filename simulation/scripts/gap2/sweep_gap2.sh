#!/bin/bash
set -uo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
MSG="${MSG:-268435456}"
NHOST="${NHOST:-32}"
if [ "$NHOST" = 32 ]; then RUNS="$HERE/runs_gap2"; CSV="$HERE/gap2_results.csv"
else RUNS="$HERE/runs_gap2_$NHOST"; CSV="$HERE/gap2_results_$NHOST.csv"; fi
PFC_VARIANTS="${PFC_VARIANTS:-base}"
ALGOS="${ALGOS:-forestcoll fcpaths rhd}"
mkdir -p "$RUNS"

[ -n "${ONLY:-}" ] || echo "family,nodes,transport,algo,exit,wall_s,rss_kb,latency_ms,algbw_gbs,theoretical_gbs,pfc_stop,pfc_resume,fc_routed,fallback_routed,pfc_variant,pfc_peak_pkts,pfc_overshoot_pkts,pfc_overflow,egress_peak_transit_pkts,egress_transit_only_overflow,verdict" > "$CSV"

FAMILIES="${FAMILIES:-jellyfish flatbutterfly dragonfly megafly}"

[ "${SKIP_BUILD:-0}" = 1 ] || {
echo "########## building 8 binaries ##########"
for fam in $FAMILIES; do
  for arm in roce dctcp; do
    printf '%-28s ' "$fam/$arm"
    if "$HERE/build_fc_bench.sh" "$fam" "$NHOST" "$arm" > "$RUNS/build_${fam}_${arm}.log" 2>&1; then
      echo "built"
    else
      echo "BUILD FAILED (see $RUNS/build_${fam}_${arm}.log)"
    fi
  done
done
}

run_one () {
  local fam="$1" arm="$2" algo="$3" pv="${4:-base}"
  local pfcflags=""
  [ "$pv" = delay ] && pfcflags="-pfcdelayns 1000"
  local algoflag="$algo" extraflag=""
  if [ "$algo" = fcpaths ]; then algoflag=forestcoll; extraflag="-fcpaths"; fi
  local suffix=""; [ "$arm" = dctcp ] && suffix="_dctcp"
  local bin="$DC/htsim_tcp_graph_fc_bench_${fam}_${NHOST}${suffix}"
  local queue=lossless; [ "$arm" = dctcp ] && queue=ecn
  local tag="${fam}_${arm}_${algo}"
  [ "$pv" != base ] && tag="${tag}_${pv}"
  local out="$RUNS/$tag.txt" log="$RUNS/$tag.log"

  printf '%-34s ' "$tag"
  if [ ! -x "$bin" ]; then
    echo "no binary"
    echo "$fam,$NHOST,$arm,$algo,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,$pv,NA,NA,NA,NA,NA,NO_BINARY" >> "$CSV"
    return
  fi

  /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" \
  timeout 3600 "$bin" -topo "$fam" -nodes "$NHOST" -msgsize "$MSG" \
      -speed 100000 -q 200 -rtt 1000 -ssthresh 10000 -simtime "${SIMTIME:-60}" \
      -queue "$queue" -routing updown -algo "$algoflag" $extraflag $pfcflags \
      -ofile "$out" > "$log" 2>&1
  local rc=$?
  local tline=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tline" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tline" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')
  local lat=$(grep -m1 '^Latency_ms '  "$out" 2>/dev/null | awk '{print $2}')
  local bw=$(grep -m1  '^AlgBW_GBps '  "$out" 2>/dev/null | awk '{print $2}')
  local th=$(grep -m1 'theoretical algbw' \
             "$HERE/schedules/${fam}_${NHOST}_fc_schedule.cpp" 2>/dev/null \
             | sed 's/.*= //; s/ GB.s//')
  local stop=$(grep -c 'PAUSE 1000' "$log" 2>/dev/null)
  local resume=$(grep -c 'PAUSE 0' "$log" 2>/dev/null)
  local fcr=$(grep -m1 'flows routed on ForestColl paths' "$log" 2>/dev/null \
              | sed 's/.*paths: \([0-9]*\),.*/\1/')
  local fbr=$(grep -m1 'flows routed on ForestColl paths' "$log" 2>/dev/null \
              | sed 's/.*default routing: \([0-9]*\).*/\1/')
  fcr=${fcr:-NA}; fbr=${fbr:-NA}
  gt () { local v=$(grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'); echo "${v:-NA}"; }
  local pk=$(gt PFC_peak_occupancy_pkts) os=$(gt PFC_peak_overshoot_pkts) of=$(gt PFC_overflow_pkts)
  local et=$(gt EGRESS_peak_transit_pkts) eto=$(gt EGRESS_overflow_transit_only_pkts)
  lat=${lat:-NA}; bw=${bw:-NA}; th=${th:-NA}

  local verdict=OK
  if   grep -qi "Assertion" "$log";                   then verdict=ASSERT
  elif grep -qi "did not finish" "$log";              then verdict=DID_NOT_FINISH
  elif [ "$rc" -ne 0 ];                               then verdict="EXIT$rc"
  elif [ "$lat" = NA ];                               then verdict=NO_RESULT
  fi
  [ "$algo" = rhd ] && th=NA

  echo "$fam,$NHOST,$arm,$algo,$rc,$wall,$rss,$lat,$bw,$th,$stop,$resume,$fcr,$fbr,$pv,$pk,$os,$of,$et,$eto,$verdict" >> "$CSV"
  printf 'lat=%-10s algbw=%-9s pfc=%s/%s fcroute=%s/%s %ss %s\n' \
         "$lat" "$bw" "$stop" "$resume" "$fcr" "$fbr" "$wall" "$verdict"
}

echo
echo "########## runs ##########"
for fam in $FAMILIES; do
  for arm in roce dctcp; do
    for algo in $ALGOS; do
      if [ -n "${ONLY:-}" ]; then
        case " $ONLY " in *" $fam:$arm:$algo "*) ;; *) continue;; esac
      fi
      if [ "$arm" = roce ]; then
        for pv in $PFC_VARIANTS; do run_one "$fam" "$arm" "$algo" "$pv"; done
      else
        run_one "$fam" "$arm" "$algo" base
      fi
    done
  done
done

echo
column -s, -t "$CSV"
echo
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
