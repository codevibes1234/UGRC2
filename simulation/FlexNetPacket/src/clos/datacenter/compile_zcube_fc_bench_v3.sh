#!/bin/bash
#
# compile_zcube_fc_bench.sh
# =========================
# Standalone ForestColl AllReduce benchmark for ZCube(4,2).

set -e

# =============================================================================
# Paths
# =============================================================================

CLOS=~/research_paper/TopoOpt/simulation/FlexNetPacket/src/clos
DC=${CLOS}/datacenter
FBUF=~/research_paper/TopoOpt_copy1/TopoOpt/simulation/FlexNet/fbuf2/include

FLAGS="-Wall -std=c++17 -O3 -I${FBUF} -I${CLOS} -I${DC}"

cd "${DC}"

echo "============================================================"
echo " ZCube(4,2) ForestColl Standalone Benchmark Build"
echo "============================================================"
echo ""

# =============================================================================
# Required source files
# =============================================================================

echo "=== Checking source files ==="

# Swapped zcube_topology_bench.cpp for the actual file: zcube_topology.cpp
REQUIRED_FILES=(
    "${DC}/ffapp_forestcoll_zcube.h"
    "${DC}/ffapp_forestcoll_zcube.cpp"
    "${DC}/zcube_fc_schedule_generated.cpp"
    "${DC}/main_tcp_zcube_fc_bench.cpp"
    "${DC}/zcube_topology.cpp"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "ERROR: missing required source:"
        echo "  $file"
        exit 1
    fi
done

echo "✓ Required source files found"
echo ""

# =============================================================================
# htsim core objects
# =============================================================================

CORE_OBJ="${CLOS}/eventlist.o \
${CLOS}/tcppacket.o \
${CLOS}/pipe.o \
${CLOS}/queue.o \
${CLOS}/ecnqueue.o \
${CLOS}/tcp.o \
${CLOS}/dctcp.o \
${CLOS}/mtcp.o \
${CLOS}/loggers.o \
${CLOS}/logfile.o \
${CLOS}/clock.o \
${CLOS}/config.o \
${CLOS}/network.o \
${CLOS}/qcn.o \
${CLOS}/exoqueue.o \
${CLOS}/randomqueue.o \
${CLOS}/cbr.o \
${CLOS}/cbrpacket.o \
${CLOS}/sent_packets.o \
${CLOS}/ndp.o \
${CLOS}/ndppacket.o \
${CLOS}/eth_pause_packet.o \
${CLOS}/compositequeue.o \
${CLOS}/prioqueue.o \
${CLOS}/cpqueue.o \
${CLOS}/compositeprioqueue.o \
${CLOS}/switch.o \
${CLOS}/fairpullqueue.o \
${CLOS}/route.o \
${CLOS}/dyn_net_sch.o \
${CLOS}/queue_lossless.o \
${CLOS}/queue_lossless_input.o \
${CLOS}/queue_lossless_output.o"

# =============================================================================
# Prebuilt topology objects
# =============================================================================

PREBUILT_TOPO_OBJ="${DC}/firstfit.o"

# =============================================================================
# Check prebuilt dependencies
# =============================================================================

echo "=== Checking prebuilt dependencies ==="

MISSING=0

for obj in ${CORE_OBJ} ${PREBUILT_TOPO_OBJ}; do
    if [ ! -f "$obj" ]; then
        echo "MISSING: $obj"
        MISSING=1
    fi
done

if [ ${MISSING} -ne 0 ]; then
    echo ""
    echo "ERROR: required prebuilt objects are missing."
    echo "Please run 'make' in the parent 'clos' directory first."
    exit 1
fi

echo "✓ Required prebuilt objects found"
echo ""

# =============================================================================
# Clean ONLY this benchmark's build products
# =============================================================================

echo "=== Cleaning old ForestColl benchmark build ==="

rm -f "${DC}/ffapp_forestcoll_zcube.o"
rm -f "${DC}/main_tcp_zcube_fc_bench.o"
rm -f "${DC}/zcube_topology.o"
rm -f "${DC}/htsim_tcp_zcube_fc_bench"

echo "✓ Clean"
echo ""

# =============================================================================
# Compile Topology
# =============================================================================

echo "=== Compiling zcube_topology.cpp ==="

g++ ${FLAGS} \
    -c "${DC}/zcube_topology.cpp" \
    -o "${DC}/zcube_topology.o"

echo "✓ zcube_topology.o"
echo ""

# =============================================================================
# Compile ForestColl executor
# =============================================================================

echo "=== Compiling ffapp_forestcoll_zcube.cpp ==="

g++ ${FLAGS} \
    -c "${DC}/ffapp_forestcoll_zcube.cpp" \
    -o "${DC}/ffapp_forestcoll_zcube.o"

echo "✓ ffapp_forestcoll_zcube.o"
echo ""

# =============================================================================
# Compile benchmark main
# =============================================================================

echo "=== Compiling main_tcp_zcube_fc_bench.cpp ==="

g++ ${FLAGS} \
    -c "${DC}/main_tcp_zcube_fc_bench.cpp" \
    -o "${DC}/main_tcp_zcube_fc_bench.o"

echo "✓ main_tcp_zcube_fc_bench.o"
echo ""

# =============================================================================
# Link (Using the corrected zcube_topology.o)
# =============================================================================

echo "=== Linking ==="

g++ ${FLAGS} \
    "${DC}/main_tcp_zcube_fc_bench.o" \
    "${DC}/ffapp_forestcoll_zcube.o" \
    "${DC}/zcube_topology.o" \
    ${PREBUILT_TOPO_OBJ} \
    ${CORE_OBJ} \
    -o "${DC}/htsim_tcp_zcube_fc_bench"

echo ""
echo "============================================================"
echo "✓✓✓ BUILD SUCCESSFUL ✓✓✓"
echo "============================================================"
ls -lh "${DC}/htsim_tcp_zcube_fc_bench"

echo ""
echo "Run:"
echo ""
echo "./htsim_tcp_zcube_fc_bench \\"
echo "    -msgsize 1073741824 \\"
echo "    -simtime 5.0 \\"
echo "    -ofile fc_1gb.txt"
echo ""
