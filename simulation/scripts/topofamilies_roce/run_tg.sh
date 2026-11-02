#!/bin/bash
v=$1; topo=$2; bw=$3
if [ "$v" = roce ]; then DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter; else DC=/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter; fi
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/topofamilies_roce/runs_tg/$v/$topo/$bw; mkdir -p $d
timeout 3600 $DC/htsim_tcp_graph -simtime 3600.1 -topo $topo -flowfile /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/fig16/fattree/tgs/${bw}dlrm_128.fbuf -speed $((bw*1000)) -ofile $d/out.txt -nodes 128 -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 > $d/log.txt 2> $d/err.txt
rc=$?; ff=$(grep FinalFinish $d/out.txt 2>/dev/null | awk '{print $2}'); it=$(grep -c 'finished one iter' $d/err.txt)
pa=$(grep -c 'PAUSE 1000' $d/log.txt); pr=$(grep -c 'PAUSE 0' $d/log.txt)
tail -15 $d/log.txt > $d/t && mv $d/t $d/log.txt; tail -15 $d/err.txt > $d/t && mv $d/t $d/err.txt
echo "$v,$topo,$bw,$rc,$ff,$it,$pa,$pr" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/topofamilies_roce/results_tg.csv
