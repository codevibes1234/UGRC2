#!/bin/bash
set -u
DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter
TG=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/fig16/fattree/tgs
G2=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/gap2
T=$(mktemp -d)
pass=0; fail=0
check () {
  if [ "$2" = "$3" ]; then printf 'PASS  %-46s %s\n' "$1" "$3"; pass=$((pass+1))
  else printf 'FAIL  %-46s expected %s got %s\n' "$1" "$2" "$3"; fail=$((fail+1)); fi
}

$DC/htsim_tcp_graph_multijob -simtime 3600.1 -topo dragonfly -flowfiles $TG/100dlrm_16.fbuf -speed 100000 \
  -ofile $T/1.txt -nodes 128 -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 -placementseed 13 -placement compact > $T/1.log 2>&1
check "mt_placement roce dragonfly compact dlrm" 66457297638 "$(grep FinalFinish $T/1.txt | awk '{print $2}')"
check "  ...PFC pauses"                           11333       "$(grep -c 'PAUSE 1000' $T/1.log)"

$DC/htsim_tcp_mt_mdp_bench -topo fattree128 -nodes 128 -routing ecmp -jobs 1 -jobnodes 32 -jobdp 2 -jobtp 4 -jobpp 2 -jobep 2 \
  -dims tp,dp,ep,pp -placement compact -placementseed 13 -speed 200000 -q 200 -ssthresh 15 -simtime 600 -ofile $T/2.txt > $T/2.log 2>&1
check "mt_mdp fattree128 alone"                  9.85451     "$(grep -m1 '^Job0_Makespan_ms ' $T/2.txt | awk '{print $2}')"
check "  ...PFC pauses"                           10024       "$(grep -c 'PAUSE 1000' $T/2.log)"

$DC/htsim_tcp_fattree_multijob -simtime 3600.1 -flowfiles $TG/100dlrm_16.fbuf -speed 100000 -ofile $T/3.txt -nodes 128 \
  -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 -placementseed 13 -placement scatter > $T/3.log 2>&1
check "test4a roce scatter alone-dlrm"           63210285157 "$(grep FinalFinish $T/3.txt | awk '{print $2}')"

$DC/htsim_tcp_dyn_flat -nodes 16 -deg 4 -seed 1 -speed 100000 -q 100 -ssthresh 10000 -rtt 1000 -simtime 60 -queue lossless \
  -routing updown -logical dragonfly -omethod dheu -rdelay 10 -flowfile $TG/100dlrm_16.fbuf -ofile $T/4.csv > $T/4.out 2>&1
check "LOGICAL_OCS dlrm16 roce dragonfly"        110617296080 "$(grep -m1 FinalFinish $T/4.csv | awk '{print $2}')"

$DC/htsim_tcp_dyn_flat -nodes 16 -deg 4 -seed 1 -speed 100000 -q 100 -ssthresh 10000 -rtt 1000 -simtime 60 -queue lossless \
  -routing updown -omethod dheu -rdelay 10 -flowfile $TG/100dlrm_16.fbuf -ofile $T/5.csv > $T/5.out 2>&1
check "LOGICAL_OCS dlrm16 roce dheu"             177166506205 "$(grep -m1 FinalFinish $T/5.csv | awk '{print $2}')"

$DC/htsim_tcp_mt_mdp_bench -topo ocs -logical flatbutterfly -deg 4 -rdelay 10 -nodes 16 -jobs 1 -jobnodes 16 \
  -jobtp 2 -jobdp 2 -jobpp 2 -jobep 2 -dims tp,dp,ep,pp -speed 100000 -q 100 -ssthresh 10000 -simtime 60 \
  -routing updown -placement compact -placementseed 13 -ofile $T/6.txt > $T/6.log 2>&1
check "OCS_MT_MDP fullmdp flatbutterfly roce"    5.62907     "$(grep -m1 '^Makespan_ms ' $T/6.txt | awk '{print $2}')"
$DC/htsim_tcp_mt_mdp_bench -topo ocs -logical jellyfish -deg 4 -rdelay 10 -nodes 16 -jobs 4 -jobnodes 4 \
  -jobtp 2 -jobdp 2 -jobpp 1 -jobep 1 -dims tp,dp,ep,pp -speed 100000 -q 100 -ssthresh 10000 -simtime 60 \
  -routing updown -placement compact -placementseed 13 -ofile $T/7.txt > $T/7.log 2>&1
check "OCS_MT_MDP tenancy jellyfish 4 jobs"      12.6757     "$(grep -m1 '^Job0_Makespan_ms ' $T/7.txt | awk '{print $2}')"

$DC/htsim_tcp_graph_fc_bench_jellyfish_32 -topo jellyfish -nodes 32 -msgsize 268435456 -speed 100000 -q 200 -rtt 1000 \
  -ssthresh 10000 -simtime 60 -queue lossless -routing updown -algo forestcoll -ofile $T/8.txt > $T/8.log 2>&1
check "gap2 jellyfish roce forestcoll"           392.493     "$(grep -m1 '^Latency_ms ' $T/8.txt | awk '{print $2}')"


$DC/htsim_tcp_dyn_flat -nodes 16 -deg 4 -seed 1 -speed 100000 -q 100 -ssthresh 10000 -rtt 1000 -simtime 60 -queue lossless \
  -routing updown -omethod dheu -rdelay 10 -rocedemand 1 -flowfile $TG/100dlrm_16.fbuf -ofile $T/9.csv > $T/9.out 2>&1
check "ROCE_DEMAND dlrm16 roce dheu -rocedemand 1"  79160059011 "$(grep -m1 FinalFinish $T/9.csv | awk '{print $2}')"

$DC/htsim_tcp_dyn_flat -nodes 16 -deg 4 -seed 1 -speed 100000 -q 100 -ssthresh 10000 -rtt 1000 -simtime 60 -queue lossless \
  -routing shortest -logical dragonfly -omethod dheu -rdelay 10 -pfcclasses 8 -flowfile $TG/100dlrm_16.fbuf -ofile $T/10.csv > $T/10.out 2>&1
check "PFC_CLASSES dlrm16 dragonfly shortest K8"    60525397256 "$(grep -m1 FinalFinish $T/10.csv | awk '{print $2}')"

$DC/htsim_tcp_mt_mdp_bench -topo ocs -deg 9 -rdelay 10 -nodes 128 -jobs 1 -jobnodes 128 -jobtp 4 -jobdp 4 -jobpp 4 -jobep 2 \
  -dims tp,dp,ep,pp -allreduce rhd -speed 100000 -q 100 -ssthresh 10000 -simtime 60 -routing updown -placement compact \
  -placementseed 13 -rocedemand 1 -ofile $T/11.txt > $T/11.log 2>&1
check "DHEU_SCALE 128-node full MDP dheu roce"      4.85913     "$(grep -m1 '^Makespan_ms ' $T/11.txt | awk '{print $2}')"

$DC/htsim_tcp_mt_mdp_bench -topo ocs -logical jellyfish -deg 4 -rdelay 10 -nodes 16 -jobmix 2x2x1x1,2x2x1x1,2x2x1x1,2x2x1x1 \
  -dims tp,dp,ep,pp -speed 100000 -q 100 -ssthresh 10000 -simtime 60 -routing updown -placement compact -placementseed 13 \
  -ofile $T/12.txt > $T/12.log 2>&1
check "MT_MDP2 -jobmix = -jobs 4 jellyfish"         12.6757     "$(grep -m1 '^Job0_Makespan_ms ' $T/12.txt | awk '{print $2}')"

$DC/htsim_tcp_graph_fc_bench_generic -topo dragonfly -nodes 32 -msgsize 268435456 -speed 100000 -q 200 -rtt 1000 \
  -ssthresh 10000 -simtime 600 -queue lossless -routing updown -rtxscanms 1 -algo forestcoll -fcexec tree \
  -fctrees $G2/schedules/dragonfly_32.xml -fcpieces 64 -fcphasebarrier 0 -ofile $T/13.txt > $T/13.log 2>&1
check "FC_TREE dragonfly32 pipelined 64 pieces"     50.7086     "$(grep -m1 '^Latency_ms ' $T/13.txt | awk '{print $2}')"

rm -rf "$T"
echo; echo "$pass passed, $fail failed"
[ $fail -eq 0 ]
