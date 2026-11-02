#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNS="$HERE/${OUT_PREFIX:-}runs_ocs_scale"; CSV="$HERE/${OUT_PREFIX:-}ocs_scale.csv"
mkdir -p "$RUNS"
source "$HERE/lib_mdp_ocs.sh"
mdp_header

ARMS="jellyfish flatbutterfly dragonfly megafly dheu"

echo "########## fullmdp ##########"
for fab in "128 9 4 4 4 2" "128 12 4 4 4 2" "256 12 4 4 4 4"; do
  set -- $fab
  n=$1 deg=$2 tp=$3 dp=$4 pp=$5 ep=$6
  for ar in rhd ring; do
    [ "$n" = 128 ] && [ "$deg" = 12 ] && [ "$ar" = ring ] && continue
    for arm in $ARMS; do
      [ "$n" = 256 ] && [ "$arm" = dheu ] && continue
      for tr in roce dctcp; do
        mdp_run fullmdp "$n" "$deg" "$arm" "$tr" compact 1 "$n" "$tp" "$dp" "$pp" "$ep" "$ar" base
      done
    done
  done
done

echo
echo "########## tenancy ##########"
for fab in "128 9 32 2 4 2 2" "256 12 64 4 4 2 2"; do
  set -- $fab
  n=$1 deg=$2 jn=$3 tp=$4 dp=$5 pp=$6 ep=$7
  for arm in $ARMS; do
    [ "$n" = 256 ] && [ "$arm" = dheu ] && continue
    for tr in roce dctcp; do
      for pol in compact scatter; do
        for j in 1 2 4; do
          mdp_run tenancy "$n" "$deg" "$arm" "$tr" "$pol" "$j" "$jn" "$tp" "$dp" "$pp" "$ep" rhd base
        done
      done
    done
  done
done

echo
echo "########## 256-node dheu, full MDP, RHD (slow: see header) ##########"
for tr in roce dctcp; do
  mdp_run fullmdp 256 12 dheu "$tr" compact 1 256 4 4 4 4 rhd base
done

echo
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
