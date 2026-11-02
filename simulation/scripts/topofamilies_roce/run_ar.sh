#!/bin/bash
v=$1; topo=$2; sz=$3; sp=$4
if [ "$v" = roce ]; then DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter; else DC=/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter; fi
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/topofamilies_roce/runs_ar/$v/$topo/$sz; mkdir -p $d
timeout 3600 $DC/htsim_tcp_a2a_bench -topo $topo -nodes 128 -collective allreduce -msgsize $sz -speed $sp -ssthresh 15 -q 200 -simtime 600 -ofile $d/out.txt > $d/log.txt 2>&1
rc=$?; lat=$(grep Latency_ms $d/out.txt 2>/dev/null | awk '{print $2}')
pa=$(grep -c 'PAUSE 1000' $d/log.txt); pr=$(grep -c 'PAUSE 0' $d/log.txt); rto=$(grep -c ' RTO ' $d/log.txt)
tail -20 $d/log.txt > $d/t && mv $d/t $d/log.txt
echo "$v,$topo,$sz,$sp,$rc,$lat,$pa,$pr,$rto" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/topofamilies_roce/results_ar.csv
