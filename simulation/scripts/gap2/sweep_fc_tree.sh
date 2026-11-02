#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
RUNS="$HERE/runs_fc_tree"; CSV="$HERE/fc_tree.csv"
ONLY="${ONLY:-Z ZP S G32 G32P G128}"
PIECE=131072
mkdir -p "$RUNS"
[ -f "$CSV" ] || echo "part,topology,nodes,speed_mbps,transport,msg_bytes,algo,pieces,barrier,exit,wall_s,rss_kb,latency_ms,algbw_gbs,rs_ms,flows,max_in_flight,pfc_stop,verdict" > "$CSV"

want () { case " $ONLY " in *" $1 "*) return 0;; esac; return 1; }

run_one () {
  local part="$1" topo="$2" n="$3" sp="$4" tr="$5" msg="$6" algo="$7" pc="${8:-1}" bar="${9:-1}" pth="${10:-0}"
  local tag="${part}_${topo}_${n}_${sp}_${tr}_${msg}_${algo}_p${pc}_b${bar}"
  local out="$RUNS/$tag.txt" log="$RUNS/$tag.log" bin args
  local piecearg="-fcpieces $pc"; [ "$pc" = auto ] && piecearg="-fcpiecebytes $PIECE"
  local treeargs="-fcexec tree $piecearg -fcphasebarrier $bar"
  [ "$pth" = 1 ] && treeargs="$treeargs -fcpaths"
  if [ "$topo" = zcube ]; then
    local d="$HERE/bin_zcube/$tr"
    args="-msgsize $msg -speed $sp -ssthresh 15 -q 200 -simtime 600"
    case "$algo" in
      rhd)  bin="$d/htsim_tcp_zcube_rhd_bench" ;;
      step) bin="$d/htsim_tcp_zcube_fcstep_bench" ;;
      *)    bin="$d/htsim_tcp_zcube_fcstep_bench"; args="$args $treeargs -fctrees $DC/zcube_4_2_schedule.xml" ;;
    esac
  else
    local suffix="" queue=lossless
    [ "$tr" = dctcp ] && { suffix="_dctcp"; queue=ecn; }
    bin="$DC/htsim_tcp_graph_fc_bench_generic${suffix}"
    args="-topo $topo -nodes $n -msgsize $msg -speed $sp -q 200 -rtt 1000 -ssthresh 10000 -simtime 600 -queue $queue -routing updown -rtxscanms 1"
    case "$algo" in
      rhd)  args="$args -algo rhd" ;;
      ring) args="$args -algo ring" ;;
      step) args="$args -algo forestcoll -fcexec step -fcschedule $HERE/schedules/${topo}_${n}_fc_schedule.cpp" ;;
      *)    args="$args -algo forestcoll $treeargs -fctrees $HERE/schedules/${topo}_${n}.xml" ;;
    esac
  fi
  printf '%-64s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" timeout 7200 "$bin" $args -ofile "$out" > "$log" 2>&1
  local rc=$?
  local tl=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tl" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tl" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')
  gt () { local v=$(grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'); echo "${v:-NA}"; }
  local lat=$(gt Latency_ms) bw=$(gt AlgBW_GBps); [ "$bw" = NA ] && bw=$(gt AlgBW_GBs)
  local stop=$(grep -c 'PAUSE 1000' "$log")
  local verdict=OK
  if   grep -q "INTERNAL ERROR" "$log"; then verdict=INTERNAL_ERROR
  elif grep -qi "Assertion" "$log"; then verdict=ASSERT
  elif grep -q "did not finish" "$log"; then verdict=DID_NOT_FINISH
  elif [ "$rc" -ne 0 ]; then verdict="EXIT$rc"
  elif [ "$lat" = NA ] || [ "$lat" = 0 ]; then verdict=NO_RESULT; fi
  echo "$part,$topo,$n,$sp,$tr,$msg,$algo,$(gt FCTreePieces),$(gt FCTreePhaseBarrier),$rc,$wall,$rss,$lat,$bw,$(gt FCTreeRSFinish_ms),$(gt FCTreeFlows),$(gt FCTreeMaxInFlight),$stop,$verdict" >> "$CSV"
  printf 'lat=%-11s %ss %s\n' "$lat" "$wall" "$verdict"
  if [ "$(stat -c %s "$log")" -gt 2000000 ]; then
    { head -40 "$log"; echo "... [$(wc -l < "$log") lines trimmed] ..."; tail -40 "$log"; } > "$log.trim" && mv "$log.trim" "$log"
  fi
}

MB=1048576
if want Z; then
  echo "########## Z: ZCube(4,2) 16 hosts ##########"
  for sp in 200000 400000 800000; do
    for m in 1 2 4 8 16 32 64 128 256 512 1024; do
      for tr in roce dctcp; do
        run_one Z zcube 16 $sp $tr $((m * MB)) rhd
        run_one Z zcube 16 $sp $tr $((m * MB)) step
        run_one Z zcube 16 $sp $tr $((m * MB)) tree1 1 1
        run_one Z zcube 16 $sp $tr $((m * MB)) treep auto 0
      done
    done
  done
fi

if want ZP; then
  echo "########## ZP: ZCube(4,2), trees on ForestColl's own paths ##########"
  for sp in 200000 400000 800000; do
    for m in 1 2 4 8 16 32 64 128 256 512 1024; do
      for tr in roce dctcp; do
        run_one ZP zcube 16 $sp $tr $((m * MB)) tree1p 1 1 1
        run_one ZP zcube 16 $sp $tr $((m * MB)) treepp auto 0 1
      done
    done
  done
fi

if want S; then
  echo "########## S: piece-count sensitivity, 256 MB ##########"
  for tr in roce dctcp; do
    for pc in 1 4 16 64 256; do for bar in 1 0; do
      run_one S zcube 16 200000 $tr $((256 * MB)) tree $pc $bar
    done; done
    for fam in dragonfly jellyfish; do
      for pc in 1 4 16 64 256; do for bar in 1 0; do
        run_one S $fam 32 100000 $tr $((256 * MB)) tree $pc $bar
      done; done
    done
  done
fi

if want G32; then
  echo "########## G32: four families, 32 hosts ##########"
  for fam in jellyfish flatbutterfly dragonfly megafly; do
    for m in 1 4 16 64 256; do
      for tr in roce dctcp; do
        run_one G32 $fam 32 100000 $tr $((m * MB)) rhd
        run_one G32 $fam 32 100000 $tr $((m * MB)) step
        run_one G32 $fam 32 100000 $tr $((m * MB)) tree1 1 1
        run_one G32 $fam 32 100000 $tr $((m * MB)) treep auto 0
      done
    done
  done
fi

if want G32P; then
  echo "########## G32P: four families, 32 hosts, pipelined trees on ForestColl's paths ##########"
  for fam in jellyfish flatbutterfly dragonfly megafly; do
    for m in 1 4 16 64 256; do
      for tr in roce dctcp; do
        run_one G32P $fam 32 100000 $tr $((m * MB)) treepp auto 0 1
      done
    done
  done
fi

if want G128; then
  echo "########## G128: four families, 128 hosts ##########"
  for fam in jellyfish flatbutterfly dragonfly megafly; do
    if [ ! -f "$HERE/schedules/${fam}_128.xml" ]; then echo "no $fam/128 trees"; continue; fi
    for m in 16 256; do
      for tr in roce dctcp; do
        run_one G128 $fam 128 100000 $tr $((m * MB)) rhd
        run_one G128 $fam 128 100000 $tr $((m * MB)) tree1 1 1
        run_one G128 $fam 128 100000 $tr $((m * MB)) treep auto 0
        run_one G128 $fam 128 100000 $tr $((m * MB)) treepp auto 0 1
      done
    done
  done
fi

if want G432R; then
  echo "########## G432R: 432 hosts, RoCE, ring vs pipelined trees ##########"
  for fam in jellyfish megafly flatbutterfly; do
    [ -f "$HERE/schedules/${fam}_432.xml" ] || continue
    run_one G432 $fam 432 100000 roce $((256 * MB)) ring
    run_one G432 $fam 432 100000 roce $((256 * MB)) treep 2 0
  done
fi

if want G432; then
  echo "########## G432: four families, 432 hosts, 256 MB ##########"
  for fam in jellyfish flatbutterfly dragonfly megafly; do
    if [ ! -f "$HERE/schedules/${fam}_432.xml" ]; then echo "no $fam/432 trees"; continue; fi
    for tr in roce dctcp; do
      run_one G432 $fam 432 100000 $tr $((256 * MB)) ring
      run_one G432 $fam 432 100000 $tr $((256 * MB)) tree1 1 1
      run_one G432 $fam 432 100000 $tr $((256 * MB)) treep 2 0
    done
  done
fi

echo
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
echo "FC TREE SWEEP DONE ($ONLY)"
