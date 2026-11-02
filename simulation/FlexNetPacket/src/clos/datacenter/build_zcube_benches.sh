#!/bin/bash
set -e
cd "$(dirname "$0")"
EXTRA="$*"
CORE="../eventlist.o ../tcppacket.o ../pipe.o ../queue.o ../ecnqueue.o ../tcp.o ../dctcp.o ../mtcp.o ../loggers.o ../logfile.o ../clock.o ../config.o ../network.o ../qcn.o ../exoqueue.o ../randomqueue.o ../cbr.o ../cbrpacket.o ../sent_packets.o ../ndp.o ../ndppacket.o ../eth_pause_packet.o ../compositequeue.o ../prioqueue.o ../cpqueue.o ../compositeprioqueue.o ../switch.o ../fairpullqueue.o ../route.o ../dyn_net_sch.o ../queue_lossless.o ../queue_lossless_input.o ../queue_lossless_output.o"
[ -f ../rocev2.o ] && CORE="$CORE ../rocev2.o"
FLAGS="-Wall -Wno-deprecated -std=c++17 -O3 -I.. -I. -I$HOME/FlexFlow/fbuf2/include $EXTRA"
g++ $FLAGS -c zcube_topology.cpp -o zcube_topology.o
g++ $FLAGS -c firstfit.cpp -o firstfit.o
g++ $FLAGS -c ffapp_rhd.cpp -o ffapp_rhd.o
g++ $FLAGS -c ffapp_fcstep_zcube.cpp -o ffapp_fcstep_zcube.o
g++ $FLAGS -c ffapp_fctree.cpp -o ffapp_fctree.o
g++ $FLAGS -c ffapp_a2a.cpp -o ffapp_a2a.o
g++ $FLAGS -c fat_tree_topology.cpp -o fat_tree_topology.o
g++ $FLAGS -c bcube_topology.cpp -o bcube_topology.o
g++ $FLAGS -c zcube_partial_topology.cpp -o zcube_partial_topology.o
g++ $FLAGS -c graph_topology.cpp -o graph_topology.o
g++ $FLAGS -c flat_topology.cpp -o flat_topology.o
g++ $FLAGS -c ffapp_mdp.cpp -o ffapp_mdp.o
g++ $FLAGS -c main_tcp_mdp_bench.cpp -o main_tcp_mdp_bench.o
g++ $FLAGS -c main_tcp_a2a_bench.cpp -o main_tcp_a2a_bench.o
g++ $FLAGS -c main_tcp_zcube_rhd_bench.cpp -o main_tcp_zcube_rhd_bench.o
g++ $FLAGS -c main_tcp_zcube_fcstep_bench.cpp -o main_tcp_zcube_fcstep_bench.o
g++ $FLAGS main_tcp_zcube_rhd_bench.o ffapp_rhd.o zcube_topology.o firstfit.o $CORE -o htsim_tcp_zcube_rhd_bench
g++ $FLAGS main_tcp_zcube_fcstep_bench.o ffapp_fcstep_zcube.o ffapp_fctree.o ffapp_rhd.o zcube_topology.o firstfit.o $CORE -o htsim_tcp_zcube_fcstep_bench
g++ $FLAGS main_tcp_a2a_bench.o ffapp_a2a.o ffapp_rhd.o zcube_topology.o fat_tree_topology.o bcube_topology.o zcube_partial_topology.o graph_topology.o firstfit.o $CORE -o htsim_tcp_a2a_bench
g++ $FLAGS main_tcp_mdp_bench.o ffapp_mdp.o ffapp_a2a.o ffapp_rhd.o zcube_topology.o zcube_partial_topology.o fat_tree_topology.o graph_topology.o firstfit.o $CORE -o htsim_tcp_mdp_bench

g++ $FLAGS -c main_tcp_mt_mdp_bench.cpp -o main_tcp_mt_mdp_bench.o
g++ $FLAGS main_tcp_mt_mdp_bench.o ffapp_mdp.o ffapp_a2a.o ffapp_rhd.o zcube_topology.o zcube_partial_topology.o fat_tree_topology.o graph_topology.o flat_topology.o firstfit.o $CORE -o htsim_tcp_mt_mdp_bench

ls -la htsim_tcp_zcube_rhd_bench htsim_tcp_zcube_fcstep_bench htsim_tcp_a2a_bench htsim_tcp_mdp_bench htsim_tcp_mt_mdp_bench
