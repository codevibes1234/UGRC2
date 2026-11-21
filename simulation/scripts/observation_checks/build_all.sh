#!/bin/bash
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
G="$(cd "$HERE/.." && pwd)"
CLOS="$(cd "$G/../FlexNetPacket/src/clos" && pwd)"; DC="$CLOS/datacenter"
( cd "$CLOS" && make lib >/dev/null 2>&1 ) && echo "lib ok"
( cd "$G/ocs" && ./build_ocs.sh >/dev/null 2>&1 ) && echo "ocs ok"
( cd "$DC" && ./build_both_arms.sh >/dev/null 2>&1 ) && echo "bench arms ok"
( cd "$DC" && ./build_multijob_arms.sh >/dev/null 2>&1 ) && echo "graph multijob ok"
( cd "$DC" && ./build_fattree_multijob_arms.sh >/dev/null 2>&1 ) && echo "fattree multijob ok"
( cd "$G/gap2" && ./build_fc_bench.sh jellyfish 32 roce >/dev/null 2>&1 ) && echo "fc jellyfish32 ok"
for a in roce dctcp; do ( cd "$G/gap2" && ./build_fc_bench.sh generic 0 $a >/dev/null 2>&1 ) && echo "fc generic $a ok"; done
LIBO=""
for o in eventlist tcppacket pipe queue queue_lossless queue_lossless_input queue_lossless_output \
         ecnqueue tcp dctcp rocev2 mtcp loggers logfile clock config network qcn exoqueue \
         randomqueue cbr cbrpacket sent_packets ndp ndppacket eth_pause_packet compositequeue \
         prioqueue cpqueue compositeprioqueue switch fairpullqueue route dyn_net_sch; do LIBO="$LIBO $CLOS/$o.o"; done
for arm in roce dctcp; do
  EXTRA=""; [ "$arm" = dctcp ] && EXTRA="-DUSE_DCTCP"
  F="-Wall -Wno-deprecated -std=c++17 -O3 -I$CLOS -I$DC -I$HOME/FlexFlow/fbuf2/include $EXTRA"
  O="$HERE/build/$arm"; B="$HERE/bin/$arm"; mkdir -p "$O" "$B"
  for src in main_tcp_fattree fat_tree_topology firstfit; do g++ $F -c "$DC/$src.cpp" -o "$O/$src.o" 2>/dev/null; done
  g++ $F -c "$CLOS/ffapp.cpp" -o "$O/ffapp.o" 2>/dev/null
  rm -f "$B/htsim_tcp_fattree"
  g++ $F "$O/firstfit.o" "$O/main_tcp_fattree.o" "$O/fat_tree_topology.o" $LIBO "$O/ffapp.o" -o "$B/htsim_tcp_fattree"
  echo "fattree $arm ok"
done
echo "BUILD DONE"
