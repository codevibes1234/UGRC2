#!/bin/bash
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLOS="$(cd "$HERE/../../FlexNetPacket/src/clos" && pwd)"
DC="$CLOS/datacenter"
( cd "$CLOS" && make lib >/dev/null )
CORE=""
for o in eventlist tcppacket pipe queue ecnqueue tcp dctcp mtcp loggers logfile clock config \
         network qcn exoqueue randomqueue cbr cbrpacket sent_packets ndp ndppacket \
         eth_pause_packet compositequeue prioqueue cpqueue compositeprioqueue switch \
         fairpullqueue route dyn_net_sch queue_lossless queue_lossless_input \
         queue_lossless_output rocev2; do CORE="$CORE $CLOS/$o.o"; done
SHARED=""
for o in zcube_topology zcube_partial_topology fat_tree_topology graph_topology flat_topology firstfit; do
  SHARED="$SHARED $DC/$o.o"
done
for arm in roce dctcp; do
  EXTRA=""; [ "$arm" = dctcp ] && EXTRA="-DUSE_DCTCP"
  FLAGS="-Wall -Wno-deprecated -std=c++17 -O3 -I$CLOS -I$DC -I$HOME/FlexFlow/fbuf2/include $EXTRA"
  B="$HERE/build/$arm"; mkdir -p "$B" "$HERE/bin/$arm"
  for src in ffapp_rhd ffapp_a2a ffapp_mdp main_tcp_mt_mdp_bench; do
    g++ $FLAGS -c "$DC/$src.cpp" -o "$B/$src.o"
  done
  rm -f "$HERE/bin/$arm/htsim_tcp_mt_mdp_bench"
  g++ $FLAGS "$B/main_tcp_mt_mdp_bench.o" "$B/ffapp_mdp.o" "$B/ffapp_a2a.o" "$B/ffapp_rhd.o" \
      $SHARED $CORE -o "$HERE/bin/$arm/htsim_tcp_mt_mdp_bench"
  echo "built $HERE/bin/$arm/htsim_tcp_mt_mdp_bench"
done
