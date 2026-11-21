#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
G="$(cd "$HERE/.." && pwd)"
DC="$(cd "$G/../FlexNetPacket/src/clos/datacenter" && pwd)"
TG="$G/fig16/fattree/tgs"
SIMLOCK="$G/.simlock"
ONLY="${ONLY:-F1 F3 F2 OCS MDP}"
want () { case " $ONLY " in *" $1 "*) return 0;; esac; return 1; }

if want F1; then
  CSV="$HERE/obs2_a2a.csv"; RUNS="$HERE/runs_obs2_a2a"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "gpus,topology,msg_bytes,speed_mbps,transport,exit,latency_ms,cnps,wall_s" > "$CSV"
  run_a2a () {
    local g=$1 topo=$2 sz=$3 sp=$4 tr=$5
    grep -q "^$g,$topo,$sz,$sp,$tr," "$CSV" && return 0
    local tag="a2a_${g}_${topo}_${sz}_${sp}_${tr}" nodes="" x=""; [ "$g" = 128 ] && nodes="-nodes 128"
    [ "$tr" = dcqcn ] && x="-dcqcn 1 -quiet 1"
    [ "$tr" = dcqcnw ] && x="-dcqcn 1 -dcqcnwin 1 -quiet 1"
    flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$DC/htsim_tcp_a2a_bench" \
      -topo "$topo" $nodes -msgsize "$sz" -speed "$sp" -ssthresh 15 -q 200 -simtime 600 $x \
      -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.log" 2>&1
    local rc=$? lat=$(grep -m1 Latency_ms "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}')
    local cn=$(grep -m1 '^DCQCN_cnps ' "$RUNS/$tag.log" 2>/dev/null | awk '{print $2}')
    tail -5 "$RUNS/$tag.log" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.log"
    echo "$g,$topo,$sz,$sp,$tr,$rc,${lat:-NA},${cn:-NA},$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
  }
  for tr in roce dcqcn dcqcnw; do
    for cfg in $(cut -d, -f2,3,4 "$G/alltoall_roce/results.csv" | sort -u); do
      IFS=, read topo sz sp <<< "$cfg"; run_a2a 16 "$topo" "$sz" "$sp" "$tr"
    done
    for cfg in $(cut -d, -f2,3,4 "$G/alltoall_roce/results128.csv" | sort -u); do
      IFS=, read topo sz sp <<< "$cfg"; run_a2a 128 "$topo" "$sz" "$sp" "$tr"
    done
  done
  echo "F1 done"
fi

if want F3; then
  CSV="$HERE/obs2_fc.csv"; RUNS="$HERE/runs_obs2_fc"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "algo,msg_bytes,speed_mbps,transport,exit,latency_ms,wall_s" > "$CSV"
  SIZES="1048576 2097152 4194304 8388608 16777216 33554432 67108864 134217728 268435456 536870912 1073741824"
  for tr in dcqcn dcqcnw; do
  x="-dcqcn 1 -quiet 1"; [ "$tr" = dcqcnw ] && x="-dcqcn 1 -dcqcnwin 1 -quiet 1"
  for sp in 200000 400000 800000; do for sz in $SIZES; do for alg in rhd fcstep; do
    grep -q "^$alg,$sz,$sp,$tr," "$CSV" && continue
    tag="fc_${alg}_${sz}_${sp}_$tr"
    flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$DC/htsim_tcp_zcube_${alg}_bench" \
      -msgsize "$sz" -speed "$sp" -ssthresh 15 -q 200 -simtime 600 $x -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.log" 2>&1
    rc=$?; lat=$(grep -m1 Latency_ms "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}')
    tail -5 "$RUNS/$tag.log" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.log"
    echo "$alg,$sz,$sp,$tr,$rc,${lat:-NA},$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
  done; done; done
  done
  echo "F3 done"
fi

if want F2; then
  CSV="$HERE/obs2_tg.csv"; RUNS="$HERE/runs_obs2_tg"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "taskgraph,transport,exit,final_finish_ps,tasks,wall_s" > "$CSV"
  for tg in 40dlrm_16 100dlrm_16 400dlrm_16 40vgg_16 100vgg_16 400vgg_16 40transformer_16 100transformer_16 400transformer_16 40candle_16 100candle_16 400candle_16; do
    bw=$(echo "$tg" | sed 's/^\([0-9]*\).*/\1/')
    for tr in roce dcqcn dcqcnw dctcp; do
      grep -q "^$tg,$tr," "$CSV" && continue
      arm=roce; x=""; [ "$tr" = dctcp ] && arm=dctcp; [ "$tr" = dcqcn ] && x="-dcqcn 1 -quiet 1"; [ "$tr" = dcqcnw ] && x="-dcqcn 1 -dcqcnwin 1 -quiet 1"
      tag="tg_${tg}_${tr}"
      flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$HERE/bin/$arm/htsim_tcp_fattree" \
        -simtime 3600.1 -flowfile "$TG/$tg.fbuf" -speed $((bw * 1000)) -nodes 16 -ssthresh 10000 -rttnet 1000 \
        -rttrack 1000 -q 10000 $x -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.out" 2> "$RUNS/$tag.err"
      rc=$?; ff=$(grep -m1 FinalFinish "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}')
      t=$(grep 'finished one task' "$RUNS/$tag.err" | tail -1 | grep -o 'nfin [0-9]* ntot [0-9]*' | awk '{print $2"/"$4}')
      tail -3 "$RUNS/$tag.err" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.err"
      tail -40 "$RUNS/$tag.out" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.out"
      echo "$tg,$tr,$rc,${ff:-NA},${t:-NA},$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
    done
  done
  echo "F2 done"
fi

if want OCS; then
  CSV="$HERE/obs2_ocs.csv"; RUNS="$HERE/runs_obs2_ocs"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || echo "workload,nodes,arm,transport,exit,makespan_ps,tasks,wall_s" > "$CSV"
  for spec in "dlrm 16 4" "vgg 16 4" "transformer 16 4" "candle 16 4" "dlrm 32 5" "vgg 32 5" "transformer 32 5"; do
    set -- $spec; wl=$1; n=$2; deg=$3
    for arm in dragonfly dheu; do for tr in roce dcqcn dcqcnw dctcp; do
      grep -q "^$wl,$n,$arm,$tr," "$CSV" && continue
      bin="$DC/htsim_tcp_dyn_flat"; q="-queue lossless"; x=""; lg="-logical $arm"
      [ "$arm" = dheu ] && lg=""
      [ "$tr" = dcqcn ] && x="-dcqcn 1 -quiet 1"
      [ "$tr" = dcqcnw ] && x="-dcqcn 1 -dcqcnwin 1 -quiet 1"
      [ "$tr" != dctcp ] && x="$x -creditprio 1"
      [ "$arm" = dheu ] && [ "$tr" != dctcp ] && x="$x -rocedemand 1"
      [ "$tr" = dctcp ] && { bin="$DC/htsim_tcp_dyn_flat_dctcp"; q="-queue ecn -ecnqexact -rtxscanms 1"; }
      tag="ocs_${wl}${n}_${arm}_${tr}"
      flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$bin" -nodes "$n" -deg "$deg" -seed 1 \
        -speed 100000 -q 100 -ssthresh 10000 -rtt 1000 -simtime 60 $q -routing updown $lg -omethod dheu -rdelay 10 $x \
        -flowfile "$TG/100${wl}_${n}.fbuf" -ofile "$RUNS/$tag.csv" > "$RUNS/$tag.out" 2> "$RUNS/$tag.err"
      rc=$?; mk=$(grep -m1 FinalFinish "$RUNS/$tag.csv" 2>/dev/null | awk '{print $2}')
      t=$(grep 'finished one task' "$RUNS/$tag.err" | tail -1 | grep -o 'nfin [0-9]* ntot [0-9]*' | awk '{print $2"/"$4}')
      tail -3 "$RUNS/$tag.err" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.err"
      tail -60 "$RUNS/$tag.out" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.out"
      echo "$wl,$n,$arm,$tr,$rc,${mk:-NA},${t:-NA},$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
    done; done
  done
  echo "OCS done"
fi

if want MDP; then
  source "$G/ocs/lib_mdp_ocs.sh"
  MDP_LABEL_COL=variant
  CSV="$HERE/obs2_mdp.csv"; RUNS="$HERE/runs_obs2_mdp"; mkdir -p "$RUNS"
  [ -f "$CSV" ] || mdp_header
  for lab in ${MDP_LABS:-roce_cp dcqcn_cp dcqcnw_cp}; do
  export ROCE_EXTRA="-rocedemand 1 -creditprio 1"
  [ "$lab" = dcqcn_cp ] && export ROCE_EXTRA="-rocedemand 1 -creditprio 1 -dcqcn 1 -quiet 1"
  [ "$lab" = dcqcnw_cp ] && export ROCE_EXTRA="-rocedemand 1 -creditprio 1 -dcqcn 1 -dcqcnwin 1 -quiet 1"
  for deg in 4 6; do
    for dec in "2 2 2 2" "4 2 2 1" "4 4 1 1" "8 2 1 1" "16 1 1 1"; do
      set -- $dec
      for ar in rhd ring; do mdp_run "F16d$deg" 16 "$deg" dheu roce compact 1 16 "$1" "$2" "$3" "$4" "$ar" "$lab"; done
    done
  done
  for dec in "3 3 3 1" "9 3 1 1" "3 9 1 1" "27 1 1 1" "3 3 1 3"; do
    set -- $dec
    mdp_run F27d6 27 6 dheu roce compact 1 27 "$1" "$2" "$3" "$4" ring "$lab"
  done
  for deg in 4 6; do
    for dec in "3 2 2 1" "2 3 2 1" "3 4 1 1" "4 3 1 1" "12 1 1 1"; do
      set -- $dec
      for ar in ring auto; do mdp_run "J12d$deg" 16 "$deg" dheu roce compact 1 12 "$1" "$2" "$3" "$4" "$ar" "$lab"; done
    done
  done
  mdp_run fullmdp 128 9 dheu roce compact 1 128 4 4 4 2 rhd "$lab"
  mdp_run fullmdp 128 9 dheu roce compact 1 128 4 4 4 2 ring "$lab"
  mdp_run fullmdp 128 12 dheu roce compact 1 128 4 4 4 2 rhd "$lab"
  for pol in compact scatter; do for j in 1 2 4; do
    mdp_run tenancy 128 9 dheu roce "$pol" "$j" 32 2 4 2 2 rhd "$lab"
  done; done
  done
  echo "MDP done"
fi
echo "OBS2 SWEEP DONE"
