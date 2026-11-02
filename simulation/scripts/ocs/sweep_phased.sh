#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNS="$HERE/${OUT_PREFIX:-}runs_phased"; CSV="$HERE/${OUT_PREFIX:-}phased.csv"
mkdir -p "$RUNS"
MDP_LABEL_COL=variant
MDP_DCTCP_EXTRA=-ecnqexact
source "$HERE/lib_mdp_ocs.sh"
mdp_header

mflags () {
  case "$1" in
    conc)    echo "" ;;
    ph4c0)   echo "-phased 4" ;;
    ph8c0)   echo "-phased 8" ;;
    ph8c100) echo "-phased 8 -fwdus 100 -bwdus 200" ;;
  esac
}
ARMS="jellyfish flatbutterfly dragonfly megafly dheu"

echo "########## A: full MDP, 16 nodes ##########"
for mode in conc ph4c0 ph8c0 ph8c100; do
  for arm in $ARMS; do for tr in roce dctcp; do
    mdp_run A 16 4 "$arm" "$tr" compact 1 16 2 2 2 2 rhd "$mode" $(mflags $mode)
  done; done
done

echo "########## B: full MDP, 128 nodes ##########"
for mode in conc ph8c0 ph8c100; do
  for arm in $ARMS; do for tr in roce dctcp; do
    mdp_run B 128 9 "$arm" "$tr" compact 1 128 4 4 4 2 rhd "$mode" $(mflags $mode)
  done; done
done

echo "########## C: full MDP, 256 nodes (static families) ##########"
for mode in conc ph8c0 ph8c100; do
  for arm in jellyfish flatbutterfly dragonfly megafly; do for tr in roce dctcp; do
    mdp_run C 256 12 "$arm" "$tr" compact 1 256 4 4 4 4 rhd "$mode" $(mflags $mode)
  done; done
done

echo "########## D: tenancy, 128 nodes ##########"
for mode in conc ph8c0; do
for arm in $ARMS; do for tr in roce dctcp; do for pol in compact scatter; do
  for j in 1 2 4; do
    mdp_run D 128 9 "$arm" "$tr" "$pol" "$j" 32 2 4 2 2 rhd "$mode" $(mflags $mode)
  done
done; done; done
done

echo "########## E: tenancy, 16 nodes ##########"
for mode in conc ph4c0; do
for arm in $ARMS; do for tr in roce dctcp; do for pol in compact scatter; do
  for j in 1 2 4; do
    mdp_run E 16 4 "$arm" "$tr" "$pol" "$j" 4 2 2 1 1 rhd "$mode" $(mflags $mode)
  done
done; done; done
done

echo
awk -F, 'NR>1 && $NF!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
