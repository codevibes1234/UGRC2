#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
RUNS="$HERE/runs_fc_exec"; CSV="$HERE/fc_exec.csv"
VENV="${VENV:-/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/fcvenv}"
MSG=268435456
mkdir -p "$RUNS"

echo "########## step 0: generic binaries ##########"
for arm in roce dctcp; do
  "$HERE/build_fc_bench.sh" generic 0 "$arm" > "$RUNS/build_generic_$arm.log" 2>&1 \
    && echo "built generic/$arm" || { echo "BUILD FAILED generic/$arm"; exit 1; }
done

echo "########## step 1: 128-host schedules ##########"
for fam in jellyfish dragonfly; do
  sc="$HERE/schedules/${fam}_128_fc_schedule.cpp"
  if [ -f "$sc" ]; then echo "have $sc"; continue; fi
  echo "generating $fam/128 (start $(date +%T))"
  /usr/bin/time -f "%e s %M kB" -o "$RUNS/gen_${fam}_128.time" \
    "$VENV/bin/python" "$HERE/generate_graph_schedule.py" \
      --graph "$HERE/graphs/${fam}_128.graph" --out-cpp "$sc" \
      --out-xml "$HERE/schedules/${fam}_128.xml" > "$RUNS/gen_${fam}_128.log" 2>&1 \
    && echo "  done: $(cat "$RUNS/gen_${fam}_128.time")" \
    || echo "  GENERATION FAILED (see $RUNS/gen_${fam}_128.log)"
done

echo "family,nodes,transport,algo,exec,fcpaths,rtxscan_ms,exit,wall_s,rss_kb,latency_ms,algbw_gbs,theoretical_gbs,pfc_stop,fc_routed,fallback_routed,pfc_peak_pkts,pfc_overshoot_pkts,pfc_overflow,verdict" > "$CSV"

run_one () {
  local fam="$1" n="$2" tr="$3" algo="$4" ex="$5" paths="$6" scan="${7:-1000}"
  local suffix=""; [ "$tr" = dctcp ] && suffix="_dctcp"
  local bin="$DC/htsim_tcp_graph_fc_bench_generic${suffix}"
  local queue=lossless; [ "$tr" = dctcp ] && queue=ecn
  local sc="$HERE/schedules/${fam}_${n}_fc_schedule.cpp"
  local tag="${fam}_${n}_${tr}_${algo}_${ex}_scan${scan}"; [ "$paths" = 1 ] && tag="${tag}_paths"
  local out="$RUNS/$tag.txt" log="$RUNS/$tag.log"
  local extra=""
  [ "$algo" = forestcoll ] && extra="-fcschedule $sc -fcexec $ex"
  [ "$paths" = 1 ] && extra="$extra -fcpaths"
  extra="$extra -rtxscanms $scan"
  printf '%-48s ' "$tag"
  if [ "$algo" = forestcoll ] && [ ! -f "$sc" ]; then
    echo "no schedule"; echo "$fam,$n,$tr,$algo,$ex,$paths,$scan,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NO_SCHEDULE" >> "$CSV"; return
  fi
  /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" \
  timeout 7200 "$bin" -topo "$fam" -nodes "$n" -msgsize "$MSG" -speed 100000 -q 200 -rtt 1000 \
      -ssthresh 10000 -simtime 600 -queue "$queue" -routing updown -algo "$algo" $extra \
      -ofile "$out" > "$log" 2>&1
  local rc=$?
  local tl=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tl" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tl" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')
  gt () { local v=$(grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'); echo "${v:-NA}"; }
  local lat=$(gt Latency_ms) bw=$(gt AlgBW_GBps)
  local th=$(grep -m1 'theoretical algbw' "$sc" 2>/dev/null | sed 's/.*= //; s/ GB.s//'); th=${th:-NA}
  [ "$algo" = rhd ] && th=NA
  local stop=$(grep -c 'PAUSE 1000' "$log")
  local fcr=$(grep -m1 'flows routed on ForestColl paths' "$log" | sed 's/.*paths: \([0-9]*\),.*/\1/')
  local fbr=$(grep -m1 'flows routed on ForestColl paths' "$log" | sed 's/.*default routing: \([0-9]*\).*/\1/')
  local verdict=OK
  if   grep -qi "Assertion" "$log"; then verdict=ASSERT
  elif grep -q "did not finish" "$log"; then verdict=DID_NOT_FINISH
  elif [ "$rc" -ne 0 ]; then verdict="EXIT$rc"
  elif [ "$lat" = NA ]; then verdict=NO_RESULT; fi
  echo "$fam,$n,$tr,$algo,$ex,$paths,$scan,$rc,$wall,$rss,$lat,$bw,$th,$stop,${fcr:-NA},${fbr:-NA},$(gt PFC_peak_occupancy_pkts),$(gt PFC_peak_overshoot_pkts),$(gt PFC_overflow_pkts),$verdict" >> "$CSV"
  printf 'lat=%-11s algbw=%-9s %ss %s\n' "$lat" "$bw" "$wall" "$verdict"
}

echo "########## step 2: 32 hosts ##########"
for fam in jellyfish flatbutterfly dragonfly megafly; do
  run_one "$fam" 32 roce forestcoll step 0
  run_one "$fam" 32 roce forestcoll dataflow 0
  run_one "$fam" 32 roce forestcoll dataflow 1
  for scan in 1000 1; do
    run_one "$fam" 32 dctcp rhd none 0 "$scan"
    run_one "$fam" 32 dctcp forestcoll step 0 "$scan"
    run_one "$fam" 32 dctcp forestcoll dataflow 0 "$scan"
    run_one "$fam" 32 dctcp forestcoll dataflow 1 "$scan"
  done
done

echo "########## step 3: 128 hosts ##########"
for fam in jellyfish flatbutterfly dragonfly megafly; do
  run_one "$fam" 128 roce rhd none 0
  run_one "$fam" 128 roce forestcoll step 0
  run_one "$fam" 128 roce forestcoll dataflow 0
  for scan in 1000 1; do
    run_one "$fam" 128 dctcp rhd none 0 "$scan"
    run_one "$fam" 128 dctcp forestcoll step 0 "$scan"
    run_one "$fam" 128 dctcp forestcoll dataflow 0 "$scan"
  done
done

echo
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
