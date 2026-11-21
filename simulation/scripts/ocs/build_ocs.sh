#!/bin/bash
set -euo pipefail

CLOS="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../FlexNetPacket/src/clos" && pwd)"
DC="$CLOS/datacenter"
OUT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/build"
mkdir -p "$OUT"

FF_HOME="${FF_HOME:-$HOME/FlexFlow}"
INC="-I$DC/.. -I$DC -I$FF_HOME/fbuf2/include"
CFLAGS="-Wall -std=c++17 -O3"

OBJS=""
for o in eventlist tcppacket pipe queue queue_lossless queue_lossless_input \
         queue_lossless_output ecnqueue tcp dctcp rocev2 mtcp loggers logfile \
         clock config network qcn exoqueue randomqueue cbr cbrpacket \
         sent_packets ndp ndppacket eth_pause_packet compositequeue prioqueue \
         cpqueue compositeprioqueue switch fairpullqueue route dyn_net_sch; do
  OBJS="$OBJS $CLOS/$o.o"
done

echo "=== shared library objects ($CLOS) ==="
cd "$CLOS"
make lib

echo "=== arm 1: RoCEv2 (default) ==="
cd "$DC"
rm -f htsim_tcp_dyn_flat main_tcp_dyn_flat.o flat_topology.o firstfit.o
make htsim_tcp_dyn_flat

echo "=== arm 2: DCTCP (-DUSE_DCTCP) ==="
g++ $CFLAGS $INC -DUSE_DCTCP -c "$CLOS/ffapp.cpp"            -o "$OUT/ffapp_dctcp.o"
g++ $CFLAGS $INC -DUSE_DCTCP -c "$DC/main_tcp_dyn_flat.cpp"  -o "$OUT/main_tcp_dyn_flat_dctcp.o"

rm -f "$DC/htsim_tcp_dyn_flat_dctcp"
g++ $CFLAGS $INC \
    "$DC/firstfit.o" "$OUT/main_tcp_dyn_flat_dctcp.o" "$DC/flat_topology.o" \
    $OBJS "$OUT/ffapp_dctcp.o" \
    -L"$CLOS" -lhtsim \
    -o "$DC/htsim_tcp_dyn_flat_dctcp"

echo
echo "built:"
ls -la "$DC/htsim_tcp_dyn_flat" "$DC/htsim_tcp_dyn_flat_dctcp"
