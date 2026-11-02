#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNS="$HERE/${OUT_PREFIX:-}runs_decomp"; CSV="$HERE/${OUT_PREFIX:-}decomp.csv"
mkdir -p "$RUNS"
source "$HERE/lib_mdp_ocs.sh"
mdp_header

ARMS="jellyfish flatbutterfly dragonfly megafly dheu"

echo "########## F16: 16-node fabrics, power-of-two decompositions ##########"
for deg in 4 6; do
  for dec in "2 2 2 2" "4 2 2 1" "4 4 1 1" "8 2 1 1" "16 1 1 1"; do
    set -- $dec
    for ar in rhd ring; do
      for arm in $ARMS; do
        for tr in roce dctcp; do
          mdp_run "F16d$deg" 16 "$deg" "$arm" "$tr" compact 1 16 "$1" "$2" "$3" "$4" "$ar" base
        done
      done
    done
  done
done

echo
echo "########## F27: 27-node fabric, 3-ary decompositions ##########"
for dec in "3 3 3 1" "9 3 1 1" "3 9 1 1" "27 1 1 1" "3 3 1 3"; do
  set -- $dec
  for arm in $ARMS; do
    for tr in roce dctcp; do
      mdp_run F27d6 27 6 "$arm" "$tr" compact 1 27 "$1" "$2" "$3" "$4" ring base
    done
  done
done

echo
echo "########## J12: 12-node job on the 16-node fabrics ##########"
for deg in 4 6; do
  for dec in "3 2 2 1" "2 3 2 1" "3 4 1 1" "4 3 1 1" "12 1 1 1"; do
    set -- $dec
    for ar in ring auto; do
      for arm in $ARMS; do
        for tr in roce dctcp; do
          mdp_run "J12d$deg" 16 "$deg" "$arm" "$tr" compact 1 12 "$1" "$2" "$3" "$4" "$ar" base
        done
      done
    done
  done
done

echo
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
