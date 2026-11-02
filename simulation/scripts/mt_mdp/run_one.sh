#!/bin/bash
set -u
v=$1; topo=$2; rt=$3; pol=$4; J=$5; S=$6; jdp=$7; jtp=$8; jpp=$9; jep=${10}; tag=${11}

R=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mt_mdp
DCROOT=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter
if [ "$v" = roce ]; then BIN=$DCROOT/htsim_tcp_mt_mdp_bench
                    else BIN=$DCROOT/dctcp_bin/htsim_tcp_mt_mdp_bench; fi

d=$R/runs/$v/$topo/$rt/$pol/$tag; mkdir -p "$d"

timeout 3600 "$BIN" -topo "$topo" -nodes 128 -routing "$rt" \
    -jobs "$J" -jobnodes "$S" -jobdp "$jdp" -jobtp "$jtp" -jobpp "$jpp" -jobep "$jep" \
    -dims tp,dp,ep,pp -placement "$pol" -placementseed 13 \
    -speed 200000 -q 200 -ssthresh 15 -simtime 600 \
    -ofile "$d/o.txt" > "$d/l.txt" 2>&1
rc=$?

g() { grep -m1 "^$1 " "$d/o.txt" 2>/dev/null | awk '{print $2}'; }

pa=$(grep -c 'PAUSE 1000' "$d/l.txt"); pr=$(grep -c 'PAUSE 0' "$d/l.txt")
rto=$(grep -c ' RTO ' "$d/l.txt")
decomp="${jdp}x${jtp}x${jpp}x${jep}"

if ! grep -q "_Class_" "$d/o.txt" 2>/dev/null; then
  echo "$v,$topo,$rt,$pol,$J,$S,$decomp,$tag,-,,,,,,,,,$pa,$pr,$rto,HARNESS" >> "$R/results.csv"
  tail -20 "$d/l.txt" > "$d/t" && mv "$d/t" "$d/l.txt"
  exit 1
fi

for j in $(seq 0 $((J-1))); do
  nl=$(g "Job${j}_Nodes")
  n_n=$(echo "$nl" | tr ',' '\n' | grep -c .)
  n_md5=$(echo "$nl" | md5sum | cut -c1-8)
  echo "$v,$topo,$rt,$pol,$J,$S,$decomp,$tag,$j,$(g Job${j}_Class_tp_ms),$(g Job${j}_Class_dp_ms),$(g Job${j}_Class_ep_ms),$(g Job${j}_Class_pp_ms),$(g Job${j}_Makespan_ms),$(g Job${j}_Incomplete),$n_n,$n_md5,$pa,$pr,$rto,$rc" >> "$R/results.csv"
done

tail -20 "$d/l.txt" > "$d/t" && mv "$d/t" "$d/l.txt"

grep -v '^FCT ' "$d/o.txt" > "$d/o2" && mv "$d/o2" "$d/o.txt"
