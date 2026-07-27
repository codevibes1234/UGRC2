#!/usr/bin/env bash
# compile_circulant128_bench.sh
# For TopoOpt Baseline (Circulant128 + FFMultiRingAllreduceTask, 4 disjoint rings)
set -e
CLOS=~/research_paper/TopoOpt/simulation/FlexNetPacket/src/clos
DC=${CLOS}/datacenter
FBUF=~/research_paper/TopoOpt/simulation/FlexNet/fbuf2/include
# Changed -O3 to -O0 for ultra-fast compilation testing.
# IMPORTANT: Change this back to -O3 before running your final paper experiments!
FLAGS="-Wall -std=c++17 -O0 -I${FBUF} -I${CLOS} -I${DC}"
# Commented out the core library build to save massive amounts of time.
# Uncomment these 4 lines ONLY if you modify core files like tcp.cpp or ffapp.cpp.
# echo "[Circulant128-Bench] Building htsim core library..."
# cd "${CLOS}"
# make lib -j4
# cd "${DC}"
echo "[Circulant128-Bench] Compiling circulant128_topology.cpp ..."
g++ ${FLAGS} -c circulant128_topology.cpp -o circulant128_topology.o
echo "[Circulant128-Bench] Compiling flat_topology.cpp ..."
g++ ${FLAGS} -c flat_topology.cpp -o flat_topology.o
echo "[Circulant128-Bench] Compiling firstfit.cpp ..."
g++ ${FLAGS} -c firstfit.cpp -o firstfit.o
echo "[Circulant128-Bench] Compiling ffapp_multiring_tg.cpp ..."
g++ ${FLAGS} -c ffapp_multiring_tg.cpp -o ffapp_multiring_tg.o
echo "[Circulant128-Bench] Compiling main_tcp_circulant128_bench.cpp ..."
g++ ${FLAGS} -c main_tcp_circulant128_bench.cpp -o main_tcp_circulant128_bench.o
echo "[Circulant128-Bench] Linking htsim_tcp_circulant128_bench ..."
g++ ${FLAGS} \
    firstfit.o \
    main_tcp_circulant128_bench.o \
    circulant128_topology.o \
    flat_topology.o \
    ffapp_multiring_tg.o \
    "${CLOS}/ffapp.o"         \
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
    -o htsim_tcp_circulant128_bench
echo ""
echo "[Circulant128-Bench] Build complete: ${DC}/htsim_tcp_circulant128_bench"
