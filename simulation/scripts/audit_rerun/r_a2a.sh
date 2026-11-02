#!/bin/bash
v=$1; topo=$2; rt=$3; sz=$4
DC=$([ "$v" = roce ] && echo /home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter || echo /tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter)
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/audit_rerun/runs/a2a/$v/$topo/$rt/$sz; mkdir -p $d
timeout 3600 $DC/htsim_tcp_a2a_bench -topo $topo -nodes 128 -routing $rt -msgsize $sz -speed 200000 -ssthresh 15 -q 200 -simtime 600 -ofile $d/o.txt > $d/l.txt 2>&1
lat=$(grep Latency_ms $d/o.txt 2>/dev/null|awk '{print $2}'); st=$(( $(grep -c 'PAUSE 1000' $d/l.txt) - $(grep -c 'PAUSE 0' $d/l.txt) ))
echo "$v,$topo,$rt,$sz,${lat:-0},$st" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/audit_rerun/a2a.csv; tail -5 $d/l.txt > $d/t && mv $d/t $d/l.txt
