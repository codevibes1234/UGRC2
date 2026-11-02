#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNS="$HERE/runs_dheu_scale"; CSV="$HERE/dheu_scale.csv"
mkdir -p "$RUNS"
export MDP_BIN_DIR="$HERE/bin_v2" ROCE_EXTRA="-rocedemand 1"
MDP_LABEL_COL=variant
MDP_DCTCP_EXTRA=-ecnqexact
source "$HERE/lib_mdp_ocs.sh"
mdp_header
echo "########## 256 nodes ##########"
for tr in roce dctcp; do mdp_run S256 256 12 dheu "$tr" compact 1 256 4 4 4 4 rhd demand; done
echo "########## 432 nodes, full MDP ##########"
for dec in "A 4 9 4 3" "B 8 6 9 1"; do
  set -- $dec
  for tr in roce dctcp; do mdp_run "$1" 432 16 dheu "$tr" compact 1 432 "$2" "$3" "$4" "$5" auto demand; done
done
echo "########## 432 nodes, tenancy (108-node jobs) ##########"
for tr in roce dctcp; do for pol in compact scatter; do for j in 1 2 4; do
  mdp_run T 432 16 dheu "$tr" "$pol" "$j" 108 4 3 3 3 auto demand
done; done; done
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
echo "DHEU SCALE SWEEP DONE"
