#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SEEDS="${SEEDS:-2 3 4 5}"

RUNS="$HERE/${OUT_PREFIX:-}runs_seeds"; CSV="$HERE/${OUT_PREFIX:-}seeds_dynflat.csv"
mkdir -p "$RUNS"
source "$HERE/lib_dynflat.sh"
df_header
echo "########## S1: LOGICAL_OCS, 16 nodes ##########"
for s in $SEEDS; do
  for wl in dlrm vgg transformer candle; do
    for tr in roce dctcp; do for arm in jellyfish flatbutterfly dragonfly megafly dheu; do
      df_run S1 "$wl" 16 4 "$tr" updown "$arm" "seed$s" -seed "$s" -logicalseed "$s"
    done; done
  done
done

RUNS="$HERE/${OUT_PREFIX:-}runs_seeds_mdp"; CSV="$HERE/${OUT_PREFIX:-}seeds_mdp.csv"
mkdir -p "$RUNS"
MDP_LABEL_COL=variant
source "$HERE/lib_mdp_ocs.sh"
mdp_header
ARMS="jellyfish flatbutterfly dragonfly megafly dheu"
echo "########## S2: OCS_MT_MDP, 16 nodes ##########"
for s in $SEEDS; do
  for arm in $ARMS; do for tr in roce dctcp; do
    for pol in compact scatter; do for j in 1 2 4; do
      mdp_run S2tenancy 16 4 "$arm" "$tr" "$pol" "$j" 4 2 2 1 1 rhd "seed$s" -placementseed "$s" -logicalseed "$s"
    done; done
    mdp_run S2fullmdp 16 4 "$arm" "$tr" compact 1 16 2 2 2 2 rhd "seed$s" -placementseed "$s" -logicalseed "$s"
  done; done
done
echo "########## S3: OCS_SCALE, 128 nodes, static families ##########"
for s in $SEEDS; do
  for arm in jellyfish flatbutterfly dragonfly megafly; do for tr in roce dctcp; do
    mdp_run S3fullmdp 128 9 "$arm" "$tr" compact 1 128 4 4 4 2 rhd "seed$s" -placementseed "$s" -logicalseed "$s"
    for pol in compact scatter; do for j in 1 4; do
      mdp_run S3tenancy 128 9 "$arm" "$tr" "$pol" "$j" 32 2 4 2 2 rhd "seed$s" -placementseed "$s" -logicalseed "$s"
    done; done
  done; done
done
echo
for f in "$HERE/seeds_dynflat.csv" "$HERE/seeds_mdp.csv"; do
  awk -F, -v f="$(basename "$f")" 'NR>1 && $NF!="OK"{bad++} END{printf "%s: %s non-OK run(s)\n", f, bad+0}' "$f"
done
