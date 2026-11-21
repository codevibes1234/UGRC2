#!/bin/bash
B=/tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter
TG=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/fig16/fattree/tgs
R=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/fattree_vs_topoopt_roce
for bw in 10 25 40 80 100 160 200 320 400 800 1600; do
  d=$R/runs/dctcp_dlrm128/$bw; mkdir -p $d
  s=$(date +%s)
  timeout 3600 $B/htsim_tcp_fattree -simtime 3600.1 -flowfile $TG/${bw}dlrm_128.fbuf -speed $((bw*1000)) -ofile $d/nwsim.txt -nodes 128 -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 > /dev/null 2> $d/stderr.txt
  rc=$?
  ff=$(grep FinalFinish $d/nwsim.txt | awk '{print $2}')
  it=$(grep -c 'finished one iter' $d/stderr.txt)
  tail -20 $d/stderr.txt > $d/t && mv $d/t $d/stderr.txt
  echo "$bw,$rc,$ff,$it,$(( $(date +%s)-s ))" >> $R/dctcp_dlrm128.csv
done
echo DONE128 >> $R/dctcp_dlrm128.csv
