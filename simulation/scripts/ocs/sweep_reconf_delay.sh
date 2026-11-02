#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNS="$HERE/${OUT_PREFIX:-}runs_reconf_delay"; CSV="$HERE/${OUT_PREFIX:-}reconf_delay.csv"
mkdir -p "$RUNS"
source "$HERE/lib_dynflat.sh"
df_header

WORKLOADS=("dlrm 16 4" "vgg 16 4" "transformer 16 4" "candle 16 4"
           "dlrm 32 5" "vgg 32 5" "transformer 32 5")

echo "########## D: delay sweep, live period fixed at 400 us ##########"
for spec in "${WORKLOADS[@]}"; do
  set -- $spec
  for tr in roce dctcp; do
    for d in 10 100 1000 10000 100000 1000000 10000000; do
      df_run D "$1" "$2" "$3" "$tr" updown dheu "d${d}_p400000" -rdelayns "$d" -rperiodns 400000
    done
  done
done

echo
echo "########## P: period sweep, delay fixed at 10 ns ##########"
for spec in "${WORKLOADS[@]}"; do
  set -- $spec
  for tr in roce dctcp; do
    for p in 100000 1600000 6400000; do
      df_run P "$1" "$2" "$3" "$tr" updown dheu "d10_p${p}" -rdelayns 10 -rperiodns "$p"
    done
  done
done

echo
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
