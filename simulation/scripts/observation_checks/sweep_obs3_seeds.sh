#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
G="$(cd "$HERE/.." && pwd)"
DC="$(cd "$G/../FlexNetPacket/src/clos/datacenter" && pwd)"
TG="$G/fig16/fattree/tgs"
SIMLOCK="$G/.simlock"
ONLY="${ONLY:-F1 F2 PLACE}"
SEEDS="${SEEDS:-0 1 2 3 4 5}"
want () { case " $ONLY " in *" $1 "*) return 0;; esac; return 1; }

if want F1; then
  CSV="$HERE/obs3_a2a.csv"; RUNS="$HERE/runs_obs3_a2a"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "gpus,topology,msg_bytes,speed_mbps,ecmp_seed,exit,latency_ms,wall_s" > "$CSV"
  run_a2a () {
    local g=$1 topo=$2 sz=$3 sp=$4 s=$5
    grep -q "^$g,$topo,$sz,$sp,$s," "$CSV" && return 0
    local tag="a2a_${g}_${topo}_${sz}_${sp}_s${s}" nodes=""; [ "$g" = 128 ] && nodes="-nodes 128"
    flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$DC/htsim_tcp_a2a_bench" \
      -topo "$topo" $nodes -msgsize "$sz" -speed "$sp" -ssthresh 15 -q 200 -simtime 600 -ecmpseed "$s" \
      -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.log" 2>&1
    local rc=$? lat=$(grep -m1 Latency_ms "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}')
    tail -5 "$RUNS/$tag.log" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.log"
    echo "$g,$topo,$sz,$sp,$s,$rc,${lat:-NA},$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
  }
  for s in $SEEDS; do
    for cfg in $(cut -d, -f2,3,4 "$G/alltoall_roce/results.csv" | grep -E '^(fattree|zcube),' | sort -u); do
      IFS=, read topo sz sp <<< "$cfg"; run_a2a 16 "$topo" "$sz" "$sp" "$s"
    done
    for cfg in $(cut -d, -f2,3,4 "$G/alltoall_roce/results128.csv" | sort -u); do
      IFS=, read topo sz sp <<< "$cfg"; run_a2a 128 "$topo" "$sz" "$sp" "$s"
    done
  done
  echo "F1 done"
fi

if want F2; then
  CSV="$HERE/obs3_tg.csv"; RUNS="$HERE/runs_obs3_tg"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "taskgraph,transport,ecmp_mode,ecmp_seed,exit,final_finish_ps,tasks,wall_s" > "$CSV"
  run_tg () {
    local tg=$1 tr=$2 mode=$3 s=$4
    grep -q "^$tg,$tr,$mode,$s," "$CSV" && return 0
    local bw=$(echo "$tg" | sed 's/^\([0-9]*\).*/\1/') flow=0; [ "$mode" = flow ] && flow=1
    local tag="tg_${tg}_${tr}_${mode}_s${s}"
    flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$HERE/bin/$tr/htsim_tcp_fattree" \
      -simtime 3600.1 -flowfile "$TG/$tg.fbuf" -speed $((bw * 1000)) -nodes 16 -ssthresh 10000 -rttnet 1000 \
      -rttrack 1000 -q 10000 -ecmpseed "$s" -ecmpflow "$flow" -ofile "$RUNS/$tag.txt" > /dev/null 2> "$RUNS/$tag.err"
    local rc=$? ff=$(grep -m1 FinalFinish "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}')
    local t=$(grep 'finished one task' "$RUNS/$tag.err" | tail -1 | grep -o 'nfin [0-9]* ntot [0-9]*' | awk '{print $2"/"$4}')
    tail -3 "$RUNS/$tag.err" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.err"
    echo "$tg,$tr,$mode,$s,$rc,${ff:-NA},${t:-NA},$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
  }
  for tg in 40dlrm_16 400dlrm_16 40vgg_16 400vgg_16 40transformer_16 400transformer_16 40candle_16 400candle_16; do
    for tr in roce dctcp; do
      run_tg "$tg" "$tr" pair 0
      for s in 1 2 3; do run_tg "$tg" "$tr" pair "$s"; run_tg "$tg" "$tr" flow "$s"; done
    done
  done
  echo "F2 done"
fi

if want PLACE; then
  CSV="$HERE/obs3_place.csv"; RUNS="$HERE/runs_obs3_place"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "arm,placement,jobs,placement_seed,exit,job0_ms,makespan_ms,incomplete,wall_s" > "$CSV"
  run_pl () {
    local arm=$1 pol=$2 j=$3 s=$4
    grep -q "^$arm,$pol,$j,$s," "$CSV" && return 0
    local tag="pl_${arm}_${pol}_j${j}_s${s}"
    flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$DC/htsim_tcp_mt_mdp_bench" \
      -topo ocs -deg 9 -rdelay 10 -logical "$arm" -nodes 128 -dims tp,dp,ep,pp -placement "$pol" -placementseed "$s" \
      -speed 100000 -q 100 -ssthresh 10000 -simtime 60 -routing updown -jobs "$j" -jobnodes 16 -jobtp 2 -jobdp 4 \
      -jobpp 2 -jobep 1 -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.log" 2>&1
    local rc=$?
    g () { grep -m1 "^$1 " "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}'; }
    tail -5 "$RUNS/$tag.log" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.log"
    echo "$arm,$pol,$j,$s,$rc,$(g Job0_Makespan_ms),$(g Makespan_ms),$(g Incomplete),$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
  }
  for arm in flatbutterfly dragonfly jellyfish megafly; do
    for pol in compact scatter; do for j in 1 8; do
      for s in 13 1 2 3 4; do run_pl "$arm" "$pol" "$j" "$s"; done
    done; done
  done
  echo "PLACE done"
fi
echo "OBS3 SWEEP DONE"
