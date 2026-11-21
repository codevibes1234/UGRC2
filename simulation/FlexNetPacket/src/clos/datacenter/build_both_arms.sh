#!/bin/bash
set -e
cd "$(dirname "$0")"

BINS="htsim_tcp_zcube_rhd_bench htsim_tcp_zcube_fcstep_bench htsim_tcp_a2a_bench htsim_tcp_mdp_bench htsim_tcp_mt_mdp_bench"

clean() {
    rm -f $BINS
    rm -f ffapp_rhd.o ffapp_a2a.o ffapp_mdp.o ffapp_fcstep_zcube.o ffapp_fctree.o \
          main_tcp_mdp_bench.o main_tcp_mt_mdp_bench.o \
          main_tcp_a2a_bench.o main_tcp_zcube_rhd_bench.o \
          main_tcp_zcube_fcstep_bench.o
}

echo "=== arm 1 of 2: DCTCP + ECN ==="
clean
./build_zcube_benches.sh -DUSE_DCTCP
mkdir -p dctcp_bin
for b in $BINS; do mv "$b" dctcp_bin/; done

echo
echo "=== arm 2 of 2: RoCEv2 + PFC ==="
clean
./build_zcube_benches.sh

echo
echo "=== both arms built ==="
ls -la $BINS | awk '{print "  roce  ", $5, $9}'
ls -la dctcp_bin/* | awk '{print "  dctcp ", $5, $9}'
