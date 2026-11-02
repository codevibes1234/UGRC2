#!/bin/bash
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLOS="$(cd "$HERE/../../FlexNetPacket/src/clos" && pwd)"; DC="$CLOS/datacenter"
( cd "$CLOS" && make lib >/dev/null )
LIBO=""
for o in eventlist tcppacket pipe queue ecnqueue tcp dctcp mtcp loggers logfile clock config network \
         qcn exoqueue randomqueue cbr cbrpacket sent_packets ndp ndppacket eth_pause_packet \
         compositequeue prioqueue cpqueue compositeprioqueue switch fairpullqueue route dyn_net_sch \
         queue_lossless queue_lossless_input queue_lossless_output rocev2; do LIBO="$LIBO $CLOS/$o.o"; done
for arm in roce dctcp; do
  EXTRA=""; [ "$arm" = dctcp ] && EXTRA="-DUSE_DCTCP"
  F="-Wall -Wno-deprecated -std=c++17 -O3 -I$CLOS -I$DC -I$HOME/FlexFlow/fbuf2/include $EXTRA"
  B="$HERE/build/zcube_$arm"; O="$HERE/bin_zcube/$arm"; mkdir -p "$B" "$O"
  for src in zcube_topology firstfit ffapp_rhd ffapp_fcstep_zcube ffapp_fctree \
             main_tcp_zcube_rhd_bench main_tcp_zcube_fcstep_bench; do
    g++ $F -c "$DC/$src.cpp" -o "$B/$src.o"
  done
  rm -f "$O/htsim_tcp_zcube_rhd_bench" "$O/htsim_tcp_zcube_fcstep_bench"
  g++ $F "$B/main_tcp_zcube_rhd_bench.o" "$B/ffapp_rhd.o" "$B/zcube_topology.o" "$B/firstfit.o" $LIBO \
      -o "$O/htsim_tcp_zcube_rhd_bench"
  g++ $F "$B/main_tcp_zcube_fcstep_bench.o" "$B/ffapp_fcstep_zcube.o" "$B/ffapp_fctree.o" \
      "$B/ffapp_rhd.o" "$B/zcube_topology.o" "$B/firstfit.o" $LIBO -o "$O/htsim_tcp_zcube_fcstep_bench"
  echo "built $arm"
done
