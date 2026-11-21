#!/bin/bash
v=$1; mix=$2; seed=$3; jobs=$4
if [ "$v" = roce ]; then DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter; else DC=/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter; fi
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/multitenant_roce/runs/$v/$mix/seed$seed; mkdir -p $d
s=$(date +%s.%N)
timeout 3600 $DC/htsim_tcp_fattree_multijob -simtime 3600.1 -flowfiles "$jobs" -speed 100000 \
  -ofile $d/out.txt -nodes 128 -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 \
  -placementseed $seed > $d/log.txt 2> $d/err.txt
rc=$?
e=$(date +%s.%N)
rto=$(grep -c ' RTO ' $d/log.txt)
while read -r name ps; do
  job=$(echo "$name" | sed 's/FinalFinish_//')
  echo "$v,$mix,$seed,$job,$rc,$ps,$rto,$(echo "$e - $s" | bc)" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/multitenant_roce/results.csv
done < <(grep FinalFinish $d/out.txt | sed 's|/home.*/tgs/||')
tail -20 $d/err.txt > $d/t && mv $d/t $d/err.txt
tail -20 $d/log.txt > $d/t && mv $d/t $d/log.txt
