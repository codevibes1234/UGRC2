#!/bin/bash
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLOS="$HERE/../../FlexNetPacket/src/clos"; DC="$CLOS/datacenter"; SB="$HERE/build_sb/roce"
G="$HERE/.."; OUT="$HERE/randpath"; mkdir -p "$OUT"
python3 - "$CLOS/ffapp.cpp" "$OUT/ffapp_randpath.cpp" <<'PY'
import sys
s = open(sys.argv[1]).read()
for a, b in [('choice = (int)ecmp_path_index(src_node, dst_node, srcpaths->size());', 'choice = rand() % srcpaths->size();'),
             ('choice = (int)ecmp_path_index(dst_node, src_node, dstpaths->size());', 'choice = rand() % dstpaths->size();')]:
    s = s.replace(a, b)
open(sys.argv[2], 'w').write(s)
PY
F="-Wall -Wno-deprecated -std=c++17 -O3 -I$CLOS -I$DC -I$HOME/FlexFlow/fbuf2/include"
LIBO=""
for o in eventlist tcppacket pipe queue queue_lossless queue_lossless_input queue_lossless_output ecnqueue tcp \
         dctcp rocev2 mtcp loggers logfile clock config network qcn exoqueue randomqueue cbr cbrpacket \
         sent_packets ndp ndppacket eth_pause_packet compositequeue prioqueue cpqueue compositeprioqueue \
         switch fairpullqueue route dyn_net_sch; do LIBO="$LIBO $CLOS/$o.o"; done
g++ $F -c "$OUT/ffapp_randpath.cpp" -o "$OUT/ffapp_randpath.o"
rm -f "$OUT/htsim_tcp_fattree_randpath"
g++ $F "$SB/firstfit.o" "$SB/main_tcp_fattree.o" "$SB/fat_tree_topology.o" $LIBO "$OUT/ffapp_randpath.o" \
    -o "$OUT/htsim_tcp_fattree_randpath"
echo "tag,final_finish_ps,wall_s,rss_kb" > "$HERE/randpath_test.csv"
for t in 10candle_16:10 25vgg_16:25 40dlrm_128:40; do
  tag=${t%%:*}; bw=${t##*:}; n=${tag##*_}
  flock "$G/.simlock" /usr/bin/time -f "%e %M" -o "$OUT/$tag.time" timeout 1800 "$OUT/htsim_tcp_fattree_randpath" \
    -simtime 3600.1 -flowfile "$G/fig16/fattree/tgs/$tag.fbuf" -speed $((bw * 1000)) -ofile "$OUT/$tag.nwsim" \
    -nodes "$n" -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 > /dev/null 2> "$OUT/$tag.err"
  echo "$tag,$(grep -m1 FinalFinish "$OUT/$tag.nwsim" | awk '{print $2}'),$(tail -1 "$OUT/$tag.time" | tr ' ' ',')" >> "$HERE/randpath_test.csv"
done
cat "$HERE/randpath_test.csv"
