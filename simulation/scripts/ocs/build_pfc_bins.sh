#!/bin/bash
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLOS="$(cd "$HERE/../../FlexNetPacket/src/clos" && pwd)"
DC="$CLOS/datacenter"
NAME="${1:-bin_pfc}"
B="$HERE/build_${NAME#bin_}"; O="$HERE/$NAME"
mkdir -p "$B/roce" "$B/dctcp" "$O/roce" "$O/dctcp"
( cd "$CLOS" && make lib >/dev/null )
LIBO=""
for o in eventlist tcppacket pipe queue queue_lossless queue_lossless_input queue_lossless_output \
         ecnqueue tcp dctcp rocev2 mtcp loggers logfile clock config network qcn exoqueue \
         randomqueue cbr cbrpacket sent_packets ndp ndppacket eth_pause_packet compositequeue \
         prioqueue cpqueue compositeprioqueue switch fairpullqueue route dyn_net_sch; do
  LIBO="$LIBO $CLOS/$o.o"
done
for arm in roce dctcp; do
  EXTRA=""; [ "$arm" = dctcp ] && EXTRA="-DUSE_DCTCP"
  F="-Wall -Wno-deprecated -std=c++17 -O3 -I$CLOS -I$DC -I$HOME/FlexFlow/fbuf2/include $EXTRA"
  for src in flat_topology firstfit main_tcp_dyn_flat main_tcp_mt_mdp_bench ffapp_rhd ffapp_a2a \
             ffapp_mdp zcube_topology zcube_partial_topology fat_tree_topology graph_topology; do
    g++ $F -c "$DC/$src.cpp" -o "$B/$arm/$src.o"
  done
  g++ $F -c "$CLOS/ffapp.cpp" -o "$B/$arm/ffapp.o"
  suf=""; [ "$arm" = dctcp ] && suf="_dctcp"
  rm -f "$O/htsim_tcp_dyn_flat$suf" "$O/$arm/htsim_tcp_mt_mdp_bench"
  g++ $F "$B/$arm/firstfit.o" "$B/$arm/main_tcp_dyn_flat.o" "$B/$arm/flat_topology.o" $LIBO \
      "$B/$arm/ffapp.o" -o "$O/htsim_tcp_dyn_flat$suf"
  g++ $F "$B/$arm/main_tcp_mt_mdp_bench.o" "$B/$arm/ffapp_mdp.o" "$B/$arm/ffapp_a2a.o" \
      "$B/$arm/ffapp_rhd.o" "$B/$arm/zcube_topology.o" "$B/$arm/zcube_partial_topology.o" \
      "$B/$arm/fat_tree_topology.o" "$B/$arm/graph_topology.o" "$B/$arm/flat_topology.o" \
      "$B/$arm/firstfit.o" $LIBO -o "$O/$arm/htsim_tcp_mt_mdp_bench"
  echo "built $arm"
done
ls -la "$O"/htsim_tcp_dyn_flat* "$O"/*/htsim_tcp_mt_mdp_bench
