#!/bin/bash
v=$1; topo=$2; sz=$3; sp=$4; r=$5
if [ "$v" = roce ]; then DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter; else DC=/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter; fi
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/alltoall_roce/runs/$v/$topo/${sz}_${sp}/run$r; mkdir -p $d
s=$(date +%s.%N)
timeout 3600 $DC/htsim_tcp_a2a_bench -topo $topo -msgsize $sz -speed $sp -ssthresh 15 -q 200 -simtime 600 -ofile $d/out.txt > $d/log.txt 2>&1
rc=$?
e=$(date +%s.%N)
lat=$(grep Latency_ms $d/out.txt 2>/dev/null | awk '{print $2}')
rto=$(grep -c ' RTO ' $d/log.txt)
tail -30 $d/log.txt > $d/log.tail && mv $d/log.tail $d/log.txt
echo "$v,$topo,$sz,$sp,$r,$rc,$lat,$rto,$(echo "$e - $s" | bc)" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/alltoall_roce/results.csv
