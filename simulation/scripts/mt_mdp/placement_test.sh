#!/bin/bash
set -e
DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter
cd "$DC"

CORE="../eventlist.o ../tcppacket.o ../pipe.o ../queue.o ../ecnqueue.o ../tcp.o ../dctcp.o ../mtcp.o ../loggers.o ../logfile.o ../clock.o ../config.o ../network.o ../qcn.o ../exoqueue.o ../randomqueue.o ../cbr.o ../cbrpacket.o ../sent_packets.o ../ndp.o ../ndppacket.o ../eth_pause_packet.o ../compositequeue.o ../prioqueue.o ../cpqueue.o ../compositeprioqueue.o ../switch.o ../fairpullqueue.o ../route.o ../dyn_net_sch.o ../queue_lossless.o ../queue_lossless_input.o ../queue_lossless_output.o"
[ -f ../rocev2.o ] && CORE="$CORE ../rocev2.o"
FLAGS="-Wall -Wno-deprecated -std=c++17 -O2 -I.. -I. -I$HOME/FlexFlow/fbuf2/include"

rm -f test_mdp_placement test_mdp_placement.o
g++ $FLAGS -c test_mdp_placement.cpp -o test_mdp_placement.o
g++ $FLAGS test_mdp_placement.o ffapp_mdp.o ffapp_a2a.o ffapp_rhd.o \
    fat_tree_topology.o firstfit.o $CORE -o test_mdp_placement

echo "=== positive cases ==="
./test_mdp_placement
pos=$?

echo "=== negative case: a wrong-sized placement must be rejected ==="
if ./test_mdp_placement badsize > /tmp/badsize.$$ 2>&1; then
    echo "  FAIL  a 64-entry placement for 128 ranks was accepted"
    rm -f /tmp/badsize.$$
    exit 1
else
    rc=$?
    if [ "$rc" -eq 1 ]; then
        echo "  PASS  rejected with exit 1: $(grep -m1 ERROR /tmp/badsize.$$)"
    else
        echo "  FAIL  wrong exit code $rc (wanted 1)"; cat /tmp/badsize.$$; rm -f /tmp/badsize.$$; exit 1
    fi
fi
rm -f /tmp/badsize.$$
exit $pos
