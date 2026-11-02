#!/bin/bash
v=$1; topo=$2; dims=$3; tag=$4
if [ "$v" = roce ]; then DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter; else DC=/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter; fi
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mdp_roce/runs/$v/$topo/$tag; mkdir -p $d
timeout 3600 $DC/htsim_tcp_mdp_bench -topo $topo -nodes 128 -dp 2 -tp 4 -pp 4 -ep 4 \
  -dims $dims -speed 200000 -q 200 -ssthresh 15 -simtime 600 -ofile $d/out.txt > $d/log.txt 2>&1
rc=$?
get(){ grep "^$1 " $d/out.txt 2>/dev/null | awk '{print $2}'; }
pa=$(grep -c 'PAUSE 1000' $d/log.txt); pr=$(grep -c 'PAUSE 0' $d/log.txt); rto=$(grep -c ' RTO ' $d/log.txt)
echo "$v,$topo,$tag,$rc,$(get Class_tp_ms),$(get Class_dp_ms),$(get Class_ep_ms),$(get Class_pp_ms),$(get Makespan_ms),$(get Incomplete),$pa,$pr,$rto" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mdp_roce/results.csv
tail -15 $d/log.txt > $d/t && mv $d/t $d/log.txt
