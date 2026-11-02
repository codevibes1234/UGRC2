#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNS="$HERE/runs_432"; CSV="$HERE/ocs_432.csv"
mkdir -p "$RUNS"
MDP_LABEL_COL=variant
MDP_DCTCP_EXTRA=-ecnqexact
source "$HERE/lib_mdp_ocs.sh"
mdp_header
DEG=16
ARMS="jellyfish dragonfly megafly"

echo "########## probe ##########"
mdp_run A 432 $DEG dragonfly roce compact 1 432 4 9 4 3 auto base
rss=$(tail -1 "$CSV" | cut -d, -f14)
echo "probe RSS ${rss} kB"
if [ "$rss" = NA ] || [ "$rss" -gt 5000000 ]; then
  echo "STOP: probe RSS ${rss} kB is too close to this machine's memory"; exit 1
fi

echo "########## full MDP ##########"
for dec in "A 4 9 4 3" "B 8 6 9 1"; do
  set -- $dec
  for arm in $ARMS; do for tr in roce dctcp; do
    [ "$1" = A ] && [ "$arm" = dragonfly ] && [ "$tr" = roce ] && continue
    mdp_run "$1" 432 $DEG "$arm" "$tr" compact 1 432 "$2" "$3" "$4" "$5" auto base
  done; done
done

echo "########## tenancy, 108-node jobs ##########"
for arm in $ARMS; do for tr in roce dctcp; do for pol in compact scatter; do
  for j in 1 2 4; do
    mdp_run T 432 $DEG "$arm" "$tr" "$pol" "$j" 108 4 3 3 3 auto base
  done
done; done; done
echo
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
