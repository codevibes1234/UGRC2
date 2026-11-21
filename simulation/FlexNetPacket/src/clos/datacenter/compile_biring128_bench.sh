#!/usr/bin/env bash
# compile_biring128_bench.sh
# ==========================
# Builds htsim_tcp_biring128_bench — pure AllReduce microbenchmark.
# Uses ffapp_biring128.cpp instead of ffapp.cpp.
set -e

CLOS=~/TopoOpt_copy1/topoopt_fresh/simulation/FlexNetPacket/src/clos
DC=${CLOS}/datacenter
FBUF=~/TopoOpt_copy1/topoopt_fresh/simulation/FlexNet/fbuf2/include
FLAGS="-Wall -std=c++17 -O3 -I${FBUF} -I${CLOS} -I${DC}"

# ── Check generated files exist ───────────────────────────────────────────────
for f in biring128_routes_generated.cpp biring128_schedule_generated.cpp; do
    if [ ! -f "${DC}/${f}" ]; then
        echo "ERROR: ${f} missing. Run solve_bfb_biring128_v3.py first."
        exit 1
    fi
done

# ── Build htsim core library ──────────────────────────────────────────────────
echo "[BiRing128-Bench] Building htsim core library..."
cd "${CLOS}"
make lib -j4

cd "${DC}"

# ── Compile units ─────────────────────────────────────────────────────────────
echo "[BiRing128-Bench] Compiling biring128_routes_generated.cpp ..."
g++ ${FLAGS} -c biring128_routes_generated.cpp -o biring128_bench_routes.o

echo "[BiRing128-Bench] Compiling biring128_topology.cpp ..."
g++ ${FLAGS} -c biring128_topology.cpp -o biring128_bench_topology.o

echo "[BiRing128-Bench] Compiling ffapp_biring128.cpp ..."
g++ ${FLAGS} -c ffapp_biring128.cpp -o biring128_bench_ffapp.o

echo "[BiRing128-Bench] Compiling firstfit.cpp ..."
g++ ${FLAGS} -c firstfit.cpp -o biring128_bench_firstfit.o

echo "[BiRing128-Bench] Compiling main_tcp_biring128_bench.cpp ..."
g++ ${FLAGS} -c main_tcp_biring128_bench.cpp -o biring128_bench_main.o

# ── Link ──────────────────────────────────────────────────────────────────────
echo "[BiRing128-Bench] Linking htsim_tcp_biring128_bench ..."
g++ ${FLAGS} \
    biring128_bench_firstfit.o  \
    biring128_bench_main.o      \
    biring128_bench_topology.o  \
    biring128_bench_routes.o    \
    biring128_bench_ffapp.o     \
    "${CLOS}/eventlist.o"     \
    "${CLOS}/tcppacket.o"     \
    "${CLOS}/pipe.o"          \
    "${CLOS}/queue.o"         \
    "${CLOS}/ecnqueue.o"      \
    "${CLOS}/tcp.o"           \
    "${CLOS}/rocev2.o"        \
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
    -o htsim_tcp_biring128_bench

echo ""
echo "[BiRing128-Bench] Build complete: ${DC}/htsim_tcp_biring128_bench"
echo ""
echo "Quick test:"
echo "  ./htsim_tcp_biring128_bench -arsize 4194304 -speed 200000 -ssthresh 15 -q 200 -rto 10 -simtime 3600.1 -ofile /dev/null"
