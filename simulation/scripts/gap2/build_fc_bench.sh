#!/bin/bash
set -euo pipefail

if [ $# -lt 2 ]; then
  echo "usage: $0 <jellyfish|flatbutterfly|dragonfly|megafly> <nhost> [roce|dctcp] [schedule.cpp]" >&2
  exit 1
fi
FAMILY="$1"; NHOST="$2"; ARM="${3:-roce}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCHED="${4:-$HERE/schedules/${FAMILY}_${NHOST}_fc_schedule.cpp}"

CLOS="$(cd "$HERE/../../FlexNetPacket/src/clos" && pwd)"
DC="$CLOS/datacenter"
SUFFIX=""; EXTRA=""
if [ "$ARM" = dctcp ]; then SUFFIX="_dctcp"; EXTRA="-DUSE_DCTCP"; fi
OUT="$DC/htsim_tcp_graph_fc_bench_${FAMILY}_${NHOST}${SUFFIX}"
OBJ="$HERE/build/${FAMILY}_${NHOST}${SUFFIX}"
GENERIC=0
if [ "$FAMILY" = generic ]; then
  GENERIC=1
  OUT="$DC/htsim_tcp_graph_fc_bench_generic${SUFFIX}"
  OBJ="$HERE/build/generic${SUFFIX}"
fi
mkdir -p "$OBJ"

if [ "$GENERIC" = 1 ]; then
  echo "schedule: none compiled in (generic build; pass -fcschedule <file> at run time)"
elif [ ! -f "$SCHED" ]; then
  echo "ERROR: no schedule at $SCHED" >&2
  echo "       generate one first:" >&2
  echo "         $DC/dump_switch_graph $FAMILY $NHOST $HERE/graphs/${FAMILY}_${NHOST}.graph" >&2
  echo "         <venv>/bin/python $HERE/generate_graph_schedule.py \\" >&2
  echo "             --graph $HERE/graphs/${FAMILY}_${NHOST}.graph --out-cpp $SCHED" >&2
  exit 1
fi
if [ "$GENERIC" = 0 ] && ! grep -q 'FC_SCHEDULE_NHOST' "$SCHED"; then
  echo "ERROR: $SCHED defines no FC_SCHEDULE_NHOST, so the benchmark cannot check" >&2
  echo "       it against -nodes. Regenerate it with the current generator." >&2
  exit 1
fi
[ "$GENERIC" = 0 ] && echo "schedule: $(grep -m1 'FC_SCHEDULE_NHOST' "$SCHED")  ($(basename "$SCHED"))"

echo "=== library objects (make lib tracks header dependencies now) ==="
( cd "$CLOS" && make lib >/dev/null )

CORE=""
for o in eventlist tcppacket pipe queue ecnqueue tcp dctcp mtcp loggers logfile \
         clock config network qcn exoqueue randomqueue cbr cbrpacket sent_packets \
         ndp ndppacket eth_pause_packet compositequeue prioqueue cpqueue \
         compositeprioqueue switch fairpullqueue route dyn_net_sch \
         queue_lossless queue_lossless_input queue_lossless_output; do
  CORE="$CORE $CLOS/$o.o"
done
[ -f "$CLOS/rocev2.o" ] && CORE="$CORE $CLOS/rocev2.o"

FF_HOME="${FF_HOME:-$HOME/FlexFlow}"
FLAGS="-Wall -Wno-deprecated -std=c++17 -O3 -I$CLOS -I$DC -I$FF_HOME/fbuf2/include $EXTRA"

echo "=== compiling ($ARM) ==="
cd "$DC"
if [ "$GENERIC" = 1 ]; then
  g++ $FLAGS -c graph_topology.cpp -o "$OBJ/graph_topology.o"
else
  g++ $FLAGS -I"$(dirname "$SCHED")" -DFC_SCHEDULE_FILE="\"$(basename "$SCHED")\"" \
      -c graph_topology.cpp -o "$OBJ/graph_topology.o"
fi
g++ $FLAGS -c ffapp_rhd.cpp            -o "$OBJ/ffapp_rhd.o"
g++ $FLAGS -c ffapp_fcstep_zcube.cpp   -o "$OBJ/ffapp_fcstep_zcube.o"
g++ $FLAGS -c ffapp_fctree.cpp         -o "$OBJ/ffapp_fctree.o"
g++ $FLAGS -c main_tcp_graph_fc_bench.cpp -o "$OBJ/main.o"
g++ $FLAGS -c firstfit.cpp             -o "$OBJ/firstfit.o"

echo "=== linking ==="
rm -f "$OUT"
g++ $FLAGS "$OBJ/main.o" "$OBJ/ffapp_fcstep_zcube.o" "$OBJ/ffapp_fctree.o" "$OBJ/ffapp_rhd.o" \
    "$OBJ/graph_topology.o" "$OBJ/firstfit.o" $CORE -o "$OUT"

echo
echo "built: $OUT"
