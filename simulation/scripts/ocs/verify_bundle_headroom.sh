#!/bin/bash
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
TGD="$HERE/../fig16/fattree/tgs"
OUT="$HERE/runs_bundle_headroom"; mkdir -p "$OUT"
printf '%-12s %-10s %-6s %10s %10s %12s %12s %10s\n' workload arm model overshoot link_gbps ovf_fixed ovf_scaled makespan_ms
for spec in "dlrm 16 4" "vgg 16 4" "candle 16 4" "vgg 32 5"; do
  set -- $spec
  for arm in dragonfly dheu; do
    for model in base delay; do
      fl=""; [ "$model" = delay ] && fl="-pfcdelayns 1000"
      lg="-logical $arm"; [ "$arm" = dheu ] && lg=""
      c="$OUT/${1}${2}_${arm}_${model}.csv"
      "$DC/htsim_tcp_dyn_flat" -nodes "$2" -deg "$3" -seed 1 -speed 100000 -q 100 -ssthresh 10000 \
          -rtt 1000 -simtime 60 -queue lossless -routing updown $lg -omethod dheu -rdelay 10 $fl \
          -flowfile "$TGD/100${1}_${2}.fbuf" -ofile "$c" > "$OUT/${1}${2}_${arm}_${model}.out" 2>/dev/null
      g () { grep -m1 "^$1 " "$c" | awk '{print $2}'; }
      printf '%-12s %-10s %-6s %10.2f %10s %12s %12s %10.2f\n' "$1$2" "$arm" "$model" \
          "$(g PFC_peak_overshoot_pkts)" "$(g PFC_peak_overshoot_link_gbps)" \
          "$(g PFC_overflow_pkts)" "$(g PFC_overflow_speed_scaled_headroom_pkts)" \
          "$(awk '/^FinalFinish /{print $2/1e9}' "$c")"
    done
  done
done
