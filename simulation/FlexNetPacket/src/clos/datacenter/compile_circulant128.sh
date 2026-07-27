#!/usr/bin/env bash
# compile_circulant128_research_fixed.sh
# For research_paper folder - circulant128 only
set -e

CLOS=~/research_paper/TopoOpt/simulation/FlexNetPacket/src/clos
DC=${CLOS}/datacenter
FBUF=~/research_paper/TopoOpt/simulation/FlexNet/fbuf2/include
FLAGS="-Wall -std=c++17 -O3 -I${FBUF} -I${CLOS} -I${DC}"

for f in circulant128_routes_generated.cpp circulant128_schedule_generated.cpp; do
    if [ ! -f "${DC}/${f}" ]; then
        echo "ERROR: ${f} missing. Run: python3 solve_bfb_circulant128.py"
        exit 1
    fi
done

echo "[Circulant128] Building htsim core library..."
cd "${CLOS}"
make lib -j4
cd "${DC}"

echo "[Circulant128] Compiling circulant128_topology.cpp ..."
g++ ${FLAGS} -c circulant128_topology.cpp -o circulant128_topology_tg.o

echo "[Circulant128] Compiling ffapp_bfb_circulant_tg.cpp ..."
g++ ${FLAGS} -c ffapp_bfb_circulant_tg.cpp -o ffapp_bfb_circulant_tg.o

echo "[Circulant128] Compiling firstfit.cpp ..."
g++ ${FLAGS} -c firstfit.cpp -o firstfit_tg.o

echo "[Circulant128] Compiling main_tcp_circulant128_tg.cpp ..."
g++ ${FLAGS} -c main_tcp_circulant128_tg.cpp -o main_tcp_circulant128_tg.o

echo "[Circulant128] Linking htsim_tcp_circulant128 ..."
g++ ${FLAGS} \
    firstfit_tg.o \
    main_tcp_circulant128_tg.o \
    circulant128_topology_tg.o \
    ffapp_bfb_circulant_tg.o \
    "${CLOS}/eventlist.o"     \
    "${CLOS}/tcppacket.o"     \
    "${CLOS}/pipe.o"          \
    "${CLOS}/queue.o"         \
    "${CLOS}/ecnqueue.o"      \
    "${CLOS}/tcp.o"           \
    "${CLOS}/dctcp.o"         \
    "${CLOS}/mtcp.o"          \
    "${CLOS}/loggers.o"       \
    "${CLOS}/logfile.o"       \
    "${CLOS}/clock.o"         \
    "${CLOS}/config.o"        \
    "${CLOS}/network.o"       \
    "${CLOS}/qcn.o"           \
    "${CLOS}/exoqueue.o"      \
    "${CLOS}/randomqueue.o"   \
    "${CLOS}/cbr.o"           \
    "${CLOS}/cbrpacket.o"     \
    "${CLOS}/sent_packets.o"  \
    "${CLOS}/ndp.o"           \
    "${CLOS}/ndppacket.o"     \
    "${CLOS}/eth_pause_packet.o" \
    "${CLOS}/compositequeue.o" \
    "${CLOS}/prioqueue.o"     \
    "${CLOS}/cpqueue.o"       \
    "${CLOS}/compositeprioqueue.o" \
    "${CLOS}/switch.o"        \
    "${CLOS}/fairpullqueue.o" \
    "${CLOS}/route.o"         \
    "${CLOS}/dyn_net_sch.o"   \
    "${CLOS}/queue_lossless.o" \
    "${CLOS}/queue_lossless_input.o" \
    "${CLOS}/queue_lossless_output.o" \
    -o htsim_tcp_circulant128

echo ""
echo "[Circulant128] Build complete: ${DC}/htsim_tcp_circulant128"
