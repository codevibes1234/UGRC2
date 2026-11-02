#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

RUNS="$HERE/runs_dctcp_exact_dynflat"; CSV="$HERE/dctcp_exact_dynflat.csv"
mkdir -p "$RUNS"
source "$HERE/lib_dynflat.sh"
df_header
WORKLOADS=("dlrm 16 4" "vgg 16 4" "transformer 16 4" "candle 16 4"
           "dlrm 32 5" "vgg 32 5" "transformer 32 5")
echo "########## X1/X2: LOGICAL_OCS DCTCP, 1 ms timer scan ##########"
for spec in "${WORKLOADS[@]}"; do
  set -- $spec
  for arm in jellyfish flatbutterfly dragonfly megafly dheu; do
    df_run X1 "$1" "$2" "$3" dctcp updown "$arm" scan1 -rtxscanms 1
    df_run X2 "$1" "$2" "$3" dctcp updown "$arm" scan1_ecnexact -rtxscanms 1 -ecnqexact
  done
done
echo "########## X3: delay sweep, DCTCP, exact queue, 1 ms scan ##########"
for spec in "${WORKLOADS[@]}"; do
  set -- $spec
  for d in 10 100 1000 10000 100000 1000000 10000000; do
    df_run X3 "$1" "$2" "$3" dctcp updown dheu "d${d}_p400000_scan1_ecnexact" \
        -rdelayns "$d" -rperiodns 400000 -rtxscanms 1 -ecnqexact
  done
done

RUNS="$HERE/runs_dctcp_exact"; CSV="$HERE/dctcp_exact_mdp.csv"
mkdir -p "$RUNS"
MDP_LABEL_COL=variant
source "$HERE/lib_mdp_ocs.sh"
mdp_header
ARMS="jellyfish flatbutterfly dragonfly megafly dheu"
X=-ecnqexact

echo "########## DECOMPOSITION, DCTCP, exact queue ##########"
for deg in 4 6; do
  for dec in "2 2 2 2" "4 2 2 1" "4 4 1 1" "8 2 1 1" "16 1 1 1"; do
    set -- $dec
    for ar in rhd ring; do for arm in $ARMS; do
      mdp_run "F16d$deg" 16 "$deg" "$arm" dctcp compact 1 16 "$1" "$2" "$3" "$4" "$ar" ecnexact $X
    done; done
  done
done
for dec in "3 3 3 1" "9 3 1 1" "3 9 1 1" "27 1 1 1" "3 3 1 3"; do
  set -- $dec
  for arm in $ARMS; do
    mdp_run F27d6 27 6 "$arm" dctcp compact 1 27 "$1" "$2" "$3" "$4" ring ecnexact $X
  done
done
for deg in 4 6; do
  for dec in "3 2 2 1" "2 3 2 1" "3 4 1 1" "4 3 1 1" "12 1 1 1"; do
    set -- $dec
    for ar in ring auto; do for arm in $ARMS; do
      mdp_run "J12d$deg" 16 "$deg" "$arm" dctcp compact 1 12 "$1" "$2" "$3" "$4" "$ar" ecnexact $X
    done; done
  done
done

echo "########## OCS_SCALE, DCTCP, exact queue ##########"
for fab in "128 9 4 4 4 2" "128 12 4 4 4 2" "256 12 4 4 4 4"; do
  set -- $fab
  n=$1 deg=$2 tp=$3 dp=$4 pp=$5 ep=$6
  for ar in rhd ring; do
    [ "$n" = 128 ] && [ "$deg" = 12 ] && [ "$ar" = ring ] && continue
    for arm in $ARMS; do
      [ "$n" = 256 ] && [ "$arm" = dheu ] && continue
      mdp_run fullmdp "$n" "$deg" "$arm" dctcp compact 1 "$n" "$tp" "$dp" "$pp" "$ep" "$ar" ecnexact $X
    done
  done
done
for fab in "128 9 32 2 4 2 2" "256 12 64 4 4 2 2"; do
  set -- $fab
  n=$1 deg=$2 jn=$3 tp=$4 dp=$5 pp=$6 ep=$7
  for arm in $ARMS; do
    [ "$n" = 256 ] && [ "$arm" = dheu ] && continue
    for pol in compact scatter; do for j in 1 2 4; do
      mdp_run tenancy "$n" "$deg" "$arm" dctcp "$pol" "$j" "$jn" "$tp" "$dp" "$pp" "$ep" rhd ecnexact $X
    done; done
  done
done
echo
for f in "$HERE/dctcp_exact_dynflat.csv" "$HERE/dctcp_exact_mdp.csv"; do
  awk -F, -v f="$(basename "$f")" 'NR>1 && $NF!="OK"{bad++} END{printf "%s: %s non-OK run(s)\n", f, bad+0}' "$f"
done
