#!/bin/bash
set -u
v=$1; topo=$2; pol=$3; mix=$4; jobs=$5

R=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mt_placement
DCROOT=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter
if [ "$v" = roce ]; then BIN=$DCROOT/htsim_tcp_graph_multijob
                    else BIN=$DCROOT/dctcp_bin/htsim_tcp_graph_multijob; fi

d=$R/runs/$v/$topo/$pol/$mix; mkdir -p "$d"
s=$(date +%s.%N)
timeout 3600 "$BIN" -simtime 3600.1 -topo "$topo" -flowfiles "$jobs" -speed 100000 \
    -ofile "$d/out.txt" -nodes 128 -ssthresh 10000 -rttnet 1000 -rttrack 1000 \
    -q 10000 -placementseed 13 -placement "$pol" > "$d/log.txt" 2> "$d/err.txt"
rc=$?
e=$(date +%s.%N)
wall=$(echo "$e - $s" | bc)

pa=$(grep -c 'PAUSE 1000' "$d/log.txt"); pr=$(grep -c 'PAUSE 0' "$d/log.txt")
rto=$(grep -c ' RTO ' "$d/log.txt")

while read -r name ps; do
  job=$(echo "$name" | sed 's/FinalFinish_//')
  nl=$(grep -m1 "^Nodes_.*${job} " "$d/out.txt" | cut -d' ' -f2-)
  n_n=$(echo "$nl" | tr ',' '\n' | grep -c .)
  n_md5=$(echo "$nl" | md5sum | cut -c1-8)
  echo "$v,$topo,$pol,$mix,$job,$rc,$ps,$pa,$pr,$rto,$n_n,$n_md5,$wall" >> "$R/results.csv"
done < <(grep FinalFinish "$d/out.txt" | sed 's|/home.*/tgs/||')

tail -20 "$d/log.txt" > "$d/t" && mv "$d/t" "$d/log.txt"
tail -20 "$d/err.txt" > "$d/t" && mv "$d/t" "$d/err.txt"
