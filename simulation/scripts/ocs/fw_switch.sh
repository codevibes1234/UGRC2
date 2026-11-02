#!/bin/bash
cd "$(dirname "$0")"
until grep -q '########## B:' sweep_framework.log; do sleep 10; done
P=$(pgrep -f '^/bin/bash ./sweep_framework.sh')
for p in $P; do pkill -P $p; kill $p; done
sleep 2; pkill -f 'bin_v3/roce/htsim_tcp_mt_mdp_bench .*-nodes 128' ; pkill -f 'bin_v3/dctcp/htsim_tcp_mt_mdp_bench .*-nodes 128'
./sweep_framework_rest.sh > sweep_framework_rest.log 2>&1
