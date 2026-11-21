#!/bin/bash
v=$1; topo=$2; mix=$3; jobs=$4
if [ "$v" = roce ]; then DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter; else DC=/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter; fi
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/topofamilies_roce/runs_mt/$v/$topo/$mix; mkdir -p $d
timeout 3600 $DC/htsim_tcp_graph_multijob -simtime 3600.1 -topo $topo -flowfiles "$jobs" -speed 100000 -ofile $d/out.txt -nodes 128 -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 -placementseed 13 > $d/log.txt 2> $d/err.txt
rc=$?; pa=$(grep -c 'PAUSE 1000' $d/log.txt); pr=$(grep -c 'PAUSE 0' $d/log.txt)
while read -r name ps; do echo "$v,$topo,$mix,$(echo $name | sed 's/FinalFinish_//'),$rc,$ps,$pa,$pr" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/topofamilies_roce/results_mt.csv; done < <(grep FinalFinish $d/out.txt | sed 's|/home.*/tgs/||')
tail -15 $d/log.txt > $d/t && mv $d/t $d/log.txt; tail -15 $d/err.txt > $d/t && mv $d/t $d/err.txt
