#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
CLOS="$(cd "$HERE/../../FlexNetPacket/src/clos" && pwd)"; DC="$CLOS/datacenter"
BIN="$HERE/bin_sb"; RUNS="$HERE/runs_sb"
TG="$HERE/../fig16/fattree/tgs"
ONLY="${ONLY:-BUILD F1 F2}"
want () { case " $ONLY " in *" $1 "*) return 0;; esac; return 1; }
mkdir -p "$BIN/roce" "$BIN/dctcp" "$RUNS"

if want BUILD; then
  echo "########## build (current sources, every object fresh) ##########"
  ( cd "$CLOS" && make lib >/dev/null )
  LIBO=""
  for o in eventlist tcppacket pipe queue queue_lossless queue_lossless_input queue_lossless_output \
           ecnqueue tcp dctcp rocev2 mtcp loggers logfile clock config network qcn exoqueue \
           randomqueue cbr cbrpacket sent_packets ndp ndppacket eth_pause_packet compositequeue \
           prioqueue cpqueue compositeprioqueue switch fairpullqueue route dyn_net_sch; do
    LIBO="$LIBO $CLOS/$o.o"
  done
  for arm in roce dctcp; do
    EXTRA=""; [ "$arm" = dctcp ] && EXTRA="-DUSE_DCTCP"
    F="-Wall -Wno-deprecated -std=c++17 -O3 -I$CLOS -I$DC -I$HOME/FlexFlow/fbuf2/include $EXTRA"
    O="$HERE/build_sb/$arm"; mkdir -p "$O"
    for src in main_tcp_fattree main_tcp_a2a_bench ffapp_a2a ffapp_rhd fat_tree_topology firstfit \
               zcube_topology bcube_topology zcube_partial_topology graph_topology; do
      g++ $F -c "$DC/$src.cpp" -o "$O/$src.o"
    done
    g++ $F -c "$CLOS/ffapp.cpp" -o "$O/ffapp.o"
    rm -f "$BIN/$arm/htsim_tcp_fattree" "$BIN/$arm/htsim_tcp_a2a_bench"
    g++ $F "$O/firstfit.o" "$O/main_tcp_fattree.o" "$O/fat_tree_topology.o" $LIBO "$O/ffapp.o" \
        -o "$BIN/$arm/htsim_tcp_fattree"
    g++ $F "$O/main_tcp_a2a_bench.o" "$O/ffapp_a2a.o" "$O/ffapp_rhd.o" "$O/zcube_topology.o" \
        "$O/fat_tree_topology.o" "$O/bcube_topology.o" "$O/zcube_partial_topology.o" \
        "$O/graph_topology.o" "$O/firstfit.o" $LIBO -o "$BIN/$arm/htsim_tcp_a2a_bench"
    echo "built $arm"
  done
fi

if want F1; then
  echo "########## F1: All-to-All (finding 1) ##########"
  CSV="$HERE/sb_alltoall.csv"
  [ -f "$CSV" ] || echo "gpus,transport,topology,msg_bytes,speed_mbps,exit,latency_ms,rto,wall_s" > "$CSV"
  a2a () {
    local tag="a2a_$1_$2_$3_$4_$5" nodes=""; [ "$1" = 128 ] && nodes="-nodes 128"
    flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$BIN/$2/htsim_tcp_a2a_bench" \
      -topo "$3" $nodes -msgsize "$4" -speed "$5" -ssthresh 15 -q 200 -simtime 600 \
      -ofile "$RUNS/$tag.txt" > "$RUNS/$tag.log" 2>&1
    local rc=$? lat=$(grep -m1 Latency_ms "$RUNS/$tag.txt" 2>/dev/null | awk '{print $2}')
    local rto=$(grep -c ' RTO ' "$RUNS/$tag.log")
    tail -30 "$RUNS/$tag.log" > "$RUNS/$tag.t" && mv "$RUNS/$tag.t" "$RUNS/$tag.log"
    echo "$1,$2,$3,$4,$5,$rc,${lat:-NA},$rto,$(tail -1 "$RUNS/$tag.time")" >> "$CSV"
    printf '%-60s lat=%s rto=%s\n' "$tag" "${lat:-NA}" "$rto"
  }
  A=../alltoall_roce
  for cfg in $(cut -d, -f2,3,4 "$HERE/$A/results.csv" | sort -u); do
    IFS=, read topo sz sp <<< "$cfg"
    for tr in roce dctcp; do a2a 16 "$tr" "$topo" "$sz" "$sp"; done
  done
  for cfg in $(cut -d, -f2,3,4 "$HERE/$A/results128.csv" | sort -u); do
    IFS=, read topo sz sp <<< "$cfg"
    for tr in roce dctcp; do a2a 128 "$tr" "$topo" "$sz" "$sp"; done
  done
fi

if want F2; then
  echo "########## F2: Fat-Tree taskgraphs (finding 2, ROCE_RESULTS.md) ##########"
  CSV="$HERE/sb_taskgraph.csv"
  [ -f "$CSV" ] || echo "tag,model,nodes,fattree_nodes,bw_gbps,transport,exit,final_finish_ps,tasks_done,wall_s,rss_kb" > "$CSV"
  for f in $(ls "$TG" | grep -E '^[0-9]+[a-z]+_[0-9]+\.fbuf$' | sort -t_ -k2,2n -k1,1n); do
    tag="${f%.fbuf}"; n="${tag##*_}"; bw=$(echo "$tag" | sed 's/^\([0-9]*\).*/\1/'); model=$(echo "$tag" | sed 's/^[0-9]*\([a-z]*\)_.*/\1/')
    ftn=$n; [ "$n" = 32 ] && ftn=54
    for tr in roce dctcp; do
      d="$RUNS/tg_${tag}_$tr"; mkdir -p "$d"
      flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$d/time" timeout 3600 "$BIN/$tr/htsim_tcp_fattree" \
        -simtime 3600.1 -flowfile "$TG/$f" -speed $((bw * 1000)) -ofile "$d/nwsim.txt" -nodes "$ftn" \
        -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 > /dev/null 2> "$d/stderr.txt"
      rc=$?
      ff=$(grep -m1 FinalFinish "$d/nwsim.txt" 2>/dev/null | awk '{print $2}')
      done_=$(grep -c 'finished one task' "$d/stderr.txt")
      tot=$(grep 'finished one task' "$d/stderr.txt" | tail -1 | sed -n 's/.*ntot \([0-9]*\).*/\1/p')
      tail -20 "$d/stderr.txt" > "$d/t" && mv "$d/t" "$d/stderr.txt"
      tl=$(tail -1 "$d/time")
      echo "$tag,$model,$n,$ftn,$bw,$tr,$rc,${ff:-NA},$done_/${tot:-NA},$(echo "$tl" | awk '{print $1}'),$(echo "$tl" | awk '{print $2}')" >> "$CSV"
      printf '%-30s %-6s rc=%-4s ff=%-16s tasks=%s/%s\n' "$tag" "$tr" "$rc" "${ff:-NA}" "$done_" "${tot:-NA}"
    done
  done
fi
echo "SINGLE BUILD RERUN DONE ($ONLY)"
