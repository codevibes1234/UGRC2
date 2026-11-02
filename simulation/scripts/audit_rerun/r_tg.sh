#!/bin/bash
v=$1; topo=$2; rt=$3; bw=$4
DC=$([ "$v" = roce ] && echo /home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter || echo /tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter)
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/audit_rerun/runs/tg/$v/$topo/$rt/$bw; mkdir -p $d
timeout 3600 $DC/htsim_tcp_graph -simtime 3600.1 -topo $topo -routing $rt -flowfile /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/fig16/fattree/tgs/${bw}dlrm_128.fbuf -speed $((bw*1000)) -ofile $d/o.txt -nodes 128 -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 > $d/l.txt 2> $d/e.txt
ff=$(grep FinalFinish $d/o.txt 2>/dev/null|awk '{print $2}'); it=$(grep -c 'finished one iter' $d/e.txt)
echo "$v,$topo,$rt,$bw,${ff:-0},$it" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/audit_rerun/tg.csv; tail -5 $d/l.txt > $d/t && mv $d/t $d/l.txt; tail -5 $d/e.txt > $d/t && mv $d/t $d/e.txt
