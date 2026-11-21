#!/bin/bash
v=$1; topo=$2; rt=$3; dims=$4; tag=$5
DC=$([ "$v" = roce ] && echo /home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter || echo /tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/baseline/simulation/FlexNetPacket/src/clos/datacenter)
d=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/audit_rerun/runs/mdp/$v/$topo/$rt/$tag; mkdir -p $d
timeout 3600 $DC/htsim_tcp_mdp_bench -topo $topo -nodes 128 -routing $rt -dp 2 -tp 4 -pp 4 -ep 4 -dims $dims -speed 200000 -q 200 -ssthresh 15 -simtime 600 -ofile $d/o.txt > $d/l.txt 2>&1
g(){ grep "^$1 " $d/o.txt 2>/dev/null|awk '{print $2}'; }
echo "$v,$topo,$rt,$tag,$(g Class_tp_ms),$(g Class_dp_ms),$(g Class_ep_ms),$(g Class_pp_ms),$(g Makespan_ms),$(g Incomplete)" >> /home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/audit_rerun/mdp.csv; tail -5 $d/l.txt > $d/t && mv $d/t $d/l.txt
