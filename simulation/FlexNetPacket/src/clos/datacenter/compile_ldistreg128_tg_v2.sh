#!/usr/bin/env bash
# compile_ldistreg128_tg_v2.sh
# =============================
# Builds htsim_tcp_ldistreg128_tg for task graph workloads.
# Routes file compiled as separate translation unit for fast compilation.
set -e

CLOS=~/TopoOpt_copy1/topoopt_fresh/simulation/FlexNetPacket/src/clos
DC=${CLOS}/datacenter
FBUF=~/TopoOpt_copy1/topoopt_fresh/simulation/FlexNet/fbuf2/include
FLAGS="-Wall -std=c++17 -O3 -I${FBUF} -I${CLOS} -I${DC}"

for f in ldistreg128_routes_generated.cpp ldistreg128_schedule_generated.cpp; do
    if [ ! -f "${DC}/${f}" ]; then
        echo "ERROR: ${f} missing. Run: python3 solve_bfb_ldistreg128_v2.py"
        exit 1
    fi
done

echo "[LDistReg128-TG] Building htsim core library..."
cd "${CLOS}"
make lib -j4
cd "${DC}"

echo "[LDistReg128-TG] Compiling ldistreg128_routes_generated.cpp ..."
g++ ${FLAGS} -c ldistreg128_routes_generated.cpp -o ldistreg128_routes_tg.o

echo "[LDistReg128-TG] Compiling ldistreg128_topology.cpp ..."
g++ ${FLAGS} -c ldistreg128_topology.cpp -o ldistreg128_topology_tg.o

echo "[LDistReg128-TG] Compiling l2k44_topology.cpp ..."
g++ ${FLAGS} -c l2k44_topology.cpp -o l2k44_topology_dr128_tg.o

echo "[LDistReg128-TG] Compiling l2k44_128_routes_generated.cpp ..."
g++ ${FLAGS} -c l2k44_128_routes_generated.cpp -o l2k44_routes_dr128_tg.o

echo "[LDistReg128-TG] Compiling circulant128_topology.cpp ..."
g++ ${FLAGS} -c circulant128_topology.cpp -o circulant128_topology_dr128_tg.o

echo "[LDistReg128-TG] Compiling biring128_topology.cpp ..."
g++ ${FLAGS} -c biring128_topology.cpp -o biring128_topology_dr128_tg.o

echo "[LDistReg128-TG] Compiling circulant16_topology.cpp ..."
g++ ${FLAGS} -c circulant16_topology.cpp -o circulant16_topology_dr128_tg.o

echo "[LDistReg128-TG] Compiling lk44_topology.cpp ..."
g++ ${FLAGS} -c lk44_topology.cpp -o lk44_topology_dr128_tg.o

echo "[LDistReg128-TG] Compiling distreg32_topology.cpp ..."
g++ ${FLAGS} -c distreg32_topology.cpp -o distreg32_topology_dr128_tg.o

echo "[LDistReg128-TG] Compiling ffapp.cpp ..."
g++ ${FLAGS} -c "${CLOS}/ffapp.cpp" -o ffapp_dr128_tg.o

echo "[LDistReg128-TG] Compiling firstfit.cpp ..."
g++ ${FLAGS} -c firstfit.cpp -o firstfit_dr128_tg.o

echo "[LDistReg128-TG] Compiling main_tcp_ldistreg128_tg.cpp ..."
g++ ${FLAGS} -c main_tcp_ldistreg128_tg.cpp -o main_tcp_ldistreg128_tg.o

echo "[LDistReg128-TG] Linking htsim_tcp_ldistreg128_tg ..."
g++ ${FLAGS} \
    firstfit_dr128_tg.o \
    main_tcp_ldistreg128_tg.o \
    ldistreg128_topology_tg.o \
    ldistreg128_routes_tg.o \
    l2k44_topology_dr128_tg.o \
    l2k44_routes_dr128_tg.o \
    circulant128_topology_dr128_tg.o \
    biring128_topology_dr128_tg.o \
    circulant16_topology_dr128_tg.o \
    lk44_topology_dr128_tg.o \
    distreg32_topology_dr128_tg.o \
    ffapp_dr128_tg.o \
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
    -o htsim_tcp_ldistreg128_tg

echo ""
echo "[LDistReg128-TG] Build complete: ${DC}/htsim_tcp_ldistreg128_tg"
echo ""
echo "Usage:"
echo "  ./htsim_tcp_ldistreg128_tg -flowfile <task.fbuf> -speed 200000 -ssthresh 15 -q 200 -rto 10 -simtime 3600.1 -ofile out.txt"
