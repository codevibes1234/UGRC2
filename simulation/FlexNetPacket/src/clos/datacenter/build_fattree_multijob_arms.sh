#!/bin/bash
set -e
cd "$(dirname "$0")"
CLOS=..

CLOSABS=$(cd "$CLOS" && pwd)
CLOSINC="-I$CLOSABS/datacenter -I$CLOSABS -I$HOME/FlexFlow/fbuf2/include/"

build() {
    echo "=== $2 ==="
    rm -f htsim_tcp_fattree_multijob main_tcp_multijob_fattree.o "$CLOS/ffapp.o"
    ( cd "$CLOS" && make CFLAGS="-Wall $CLOSINC -std=c++17 -O3 $1" ffapp.o )
    make CFLAGS="-Wall -std=c++17 -O3 $1" htsim_tcp_fattree_multijob
}

build "-DUSE_DCTCP" "arm 1 of 2: DCTCP + ECN"
mkdir -p dctcp_bin
mv htsim_tcp_fattree_multijob dctcp_bin/

build "" "arm 2 of 2: RoCEv2 + PFC"

echo
echo "=== both arms built ==="
ls -la htsim_tcp_fattree_multijob dctcp_bin/htsim_tcp_fattree_multijob | awk '{print "  ", $5, $9}'
