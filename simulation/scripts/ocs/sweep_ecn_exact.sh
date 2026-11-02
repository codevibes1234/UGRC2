#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

RUNS="$HERE/runs_ecn_exact"; CSV="$HERE/ecn_exact_dynflat.csv"
mkdir -p "$RUNS"
source "$HERE/lib_dynflat.sh"
df_header
WORKLOADS=("dlrm 16 4" "vgg 16 4" "transformer 16 4" "candle 16 4"
           "dlrm 32 5" "vgg 32 5" "transformer 32 5")

echo "########## E1: LOGICAL_OCS, DCTCP, exact 100-packet ECN queue ##########"
for spec in "${WORKLOADS[@]}"; do
  set -- $spec
  for arm in jellyfish flatbutterfly dragonfly megafly dheu; do
    df_run E1 "$1" "$2" "$3" dctcp updown "$arm" ecnexact -ecnqexact
  done
done

echo
echo "########## E2: delay sweep, DCTCP, exact ECN queue, period 400 us ##########"
for spec in "${WORKLOADS[@]}"; do
  set -- $spec
  for d in 10 100 1000 10000 100000 1000000 10000000; do
    df_run E2 "$1" "$2" "$3" dctcp updown dheu "d${d}_p400000_ecnexact" \
        -rdelayns "$d" -rperiodns 400000 -ecnqexact
  done
done

RUNS="$HERE/runs_ecn_exact_mdp"; CSV="$HERE/ecn_exact_mdp.csv"
mkdir -p "$RUNS"
source "$HERE/lib_mdp_ocs.sh"
mdp_header
echo
echo "########## E3: OCS_MT_MDP, DCTCP, exact ECN queue ##########"
for arm in jellyfish flatbutterfly dragonfly megafly dheu; do
  for pol in compact scatter; do
    for j in 1 2 4; do
      mdp_run tenancy 16 4 "$arm" dctcp "$pol" "$j" 4 2 2 1 1 rhd ecnexact -ecnqexact
    done
  done
  mdp_run fullmdp 16 4 "$arm" dctcp compact 1 16 2 2 2 2 rhd ecnexact -ecnqexact
done

echo
for f in "$HERE/ecn_exact_dynflat.csv" "$HERE/ecn_exact_mdp.csv"; do
  awk -F, -v f="$(basename "$f")" 'NR>1 && $NF!="OK"{bad++} END{printf "%s: %s non-OK run(s)\n", f, bad+0}' "$f"
done
