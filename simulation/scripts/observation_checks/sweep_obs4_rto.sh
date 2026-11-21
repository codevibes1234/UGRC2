#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
G="$(cd "$HERE/.." && pwd)"
DC="$(cd "$G/../FlexNetPacket/src/clos/datacenter" && pwd)"
SIMLOCK="$G/.simlock"
ONLY="${ONLY:-F1 F3 MDP}"
RTOS="${RTOS:-10000 1000 200}"
want () { case " $ONLY " in *" $1 "*) return 0;; esac; return 1; }

if want F1; then
  CSV="$HERE/obs4_a2a.csv"; RUNS="$HERE/runs_obs4_a2a"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "gpus,topology,msg_bytes,speed_mbps,min_rto_us,exit,latency_ms,rto_events,wall_s" > "$CSV"
  run_a2a () {
    local g=$1 topo=$2 sz=$3 sp=$4 rto=$5
    grep -q "^$g,$topo,$sz,$sp,$rto," "$CSV" && return 0
    local tag="a2a_${g}_${topo}_${sz}_${sp}_rto${rto}" nodes=""; [ "$g" = 128 ] && nodes="-nodes 128"
    flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$DC/dctcp_bin/htsim_tcp_a2a_bench" \
      -topo "$topo" $nodes -msgsize "$sz" -speed "$sp" -ssthresh 15 -q 200 -simtime 600 -minrtous "$rto" \
      -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.log" 2>&1
    local rc=$? lat=$(grep -m1 Latency_ms "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}')
    local ev=$(grep -c ' RTO ' "$RUNS/$tag.log")
    tail -20 "$RUNS/$tag.log" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.log"
    echo "$g,$topo,$sz,$sp,$rto,$rc,${lat:-NA},$ev,$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
  }
  for rto in $RTOS; do
    for cfg in $(cut -d, -f2,3,4 "$G/alltoall_roce/results.csv" | grep -v '^topo' | sort -u); do
      IFS=, read topo sz sp <<< "$cfg"; run_a2a 16 "$topo" "$sz" "$sp" "$rto"
    done
    for cfg in $(cut -d, -f2,3,4 "$G/alltoall_roce/results128.csv" | grep -v '^topo' | sort -u); do
      IFS=, read topo sz sp <<< "$cfg"; run_a2a 128 "$topo" "$sz" "$sp" "$rto"
    done
  done
  echo "F1 done"
fi

if want F3; then
  CSV="$HERE/obs4_fc.csv"; RUNS="$HERE/runs_obs4_fc"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "transport,algo,msg_bytes,speed_mbps,min_rto_us,exit,latency_ms,rto_events,wall_s" > "$CSV"
  run_fc () {
    local tr=$1 alg=$2 sz=$3 sp=$4 rto=$5
    grep -q "^$tr,$alg,$sz,$sp,$rto," "$CSV" && return 0
    local bin="$DC/htsim_tcp_zcube_${alg}_bench" x=""
    [ "$tr" = dctcp ] && { bin="$DC/dctcp_bin/htsim_tcp_zcube_${alg}_bench"; x="-minrtous $rto"; }
    local tag="fc_${tr}_${alg}_${sz}_${sp}_rto${rto}"
    flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$bin" \
      -msgsize "$sz" -speed "$sp" -ssthresh 15 -q 200 -simtime 600 $x -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.log" 2>&1
    local rc=$? lat=$(grep -m1 Latency_ms "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}')
    local ev=$(grep -c ' RTO ' "$RUNS/$tag.log")
    tail -20 "$RUNS/$tag.log" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.log"
    echo "$tr,$alg,$sz,$sp,$rto,$rc,${lat:-NA},$ev,$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
  }
  SIZES="1048576 2097152 4194304 8388608 16777216 33554432 67108864 134217728 268435456 536870912 1073741824"
  for sp in 200000 400000 800000; do for sz in $SIZES; do
    for alg in rhd fcstep; do
      run_fc roce "$alg" "$sz" "$sp" 0
      for rto in $RTOS; do run_fc dctcp "$alg" "$sz" "$sp" "$rto"; done
    done
  done; done
  echo "F3 done"
fi

if want MDP; then
  source "$G/ocs/lib_mdp_ocs.sh"
  MDP_LABEL_COL=variant
  CSV="$HERE/obs4_mdp.csv"; RUNS="$HERE/runs_obs4_mdp"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || mdp_header
  X=-ecnqexact
  for rto in $RTOS; do
    export MDP_DCTCP_EXTRA="-minrtous $rto"
    lab="ecnexact_rto$rto"
    for deg in 4 6; do
      for dec in "2 2 2 2" "4 2 2 1" "4 4 1 1" "8 2 1 1" "16 1 1 1"; do
        set -- $dec
        for ar in rhd ring; do mdp_run "F16d$deg" 16 "$deg" dheu dctcp compact 1 16 "$1" "$2" "$3" "$4" "$ar" "$lab" $X; done
      done
    done
    for dec in "3 3 3 1" "9 3 1 1" "3 9 1 1" "27 1 1 1" "3 3 1 3"; do
      set -- $dec
      mdp_run F27d6 27 6 dheu dctcp compact 1 27 "$1" "$2" "$3" "$4" ring "$lab" $X
    done
    for deg in 4 6; do
      for dec in "3 2 2 1" "2 3 2 1" "3 4 1 1" "4 3 1 1" "12 1 1 1"; do
        set -- $dec
        for ar in ring auto; do mdp_run "J12d$deg" 16 "$deg" dheu dctcp compact 1 12 "$1" "$2" "$3" "$4" "$ar" "$lab" $X; done
      done
    done
    mdp_run fullmdp 128 9 dheu dctcp compact 1 128 4 4 4 2 rhd "$lab" $X
    mdp_run fullmdp 128 9 dheu dctcp compact 1 128 4 4 4 2 ring "$lab" $X
    mdp_run fullmdp 128 12 dheu dctcp compact 1 128 4 4 4 2 rhd "$lab" $X
    for pol in compact scatter; do for j in 1 2 4; do
      mdp_run tenancy 128 9 dheu dctcp "$pol" "$j" 32 2 4 2 2 rhd "$lab" $X
    done; done
  done
  echo "MDP done"
fi
echo "OBS4 SWEEP DONE"
