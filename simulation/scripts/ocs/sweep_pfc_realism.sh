#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

VARIANTS="base delay xoff8 xoff32"
vflags () {
  case "$1" in
    base)   echo "" ;;
    delay)  echo "-pfcdelayns $2" ;;
    xoff8)  echo "-pfcdelayns $2 -pfcxoff 8 -pfcxon 5" ;;
    xoff32) echo "-pfcdelayns $2 -pfcxoff 32 -pfcxon 29" ;;
  esac
}

RUNS="$HERE/${OUT_PREFIX:-}runs_pfc_realism"; CSV="$HERE/${OUT_PREFIX:-}pfc_realism_dynflat.csv"
mkdir -p "$RUNS"
source "$HERE/lib_dynflat.sh"
df_header

echo "########## A: LOGICAL_OCS, RoCE, four PFC models ##########"
for v in $VARIANTS; do
  for spec in "dlrm 16 4" "vgg 16 4" "transformer 16 4" "candle 16 4" \
              "dlrm 32 5" "vgg 32 5" "transformer 32 5"; do
    set -- $spec
    for arm in jellyfish flatbutterfly dragonfly megafly dheu; do
      df_run A "$1" "$2" "$3" roce updown "$arm" "$v" $(vflags "$v" 1000)
    done
  done
done

echo
echo "########## B: shortest-path routing deadlock, four PFC models ##########"
for v in $VARIANTS; do
  for spec in "dlrm 16 4" "vgg 16 4" "transformer 16 4" "candle 16 4" \
              "dlrm 32 5" "vgg 32 5" "transformer 32 5"; do
    set -- $spec
    df_run B "$1" "$2" "$3" roce shortest dheu "$v" $(vflags "$v" 1000)
  done
done

RUNS="$HERE/${OUT_PREFIX:-}runs_pfc_realism_mdp"; CSV="$HERE/${OUT_PREFIX:-}pfc_realism_mdp.csv"
mkdir -p "$RUNS"
source "$HERE/lib_mdp_ocs.sh"
mdp_header

echo
echo "########## C: OCS_MT_MDP series, RoCE, four PFC models ##########"
for v in $VARIANTS; do
  for arm in jellyfish flatbutterfly dragonfly megafly dheu; do
    for pol in compact scatter; do
      for j in 1 2 4; do
        mdp_run tenancy 16 4 "$arm" roce "$pol" "$j" 4 2 2 1 1 rhd "$v" $(vflags "$v" 500)
      done
    done
    mdp_run fullmdp 16 4 "$arm" roce compact 1 16 2 2 2 2 rhd "$v" $(vflags "$v" 500)
  done
done

echo
for f in "$HERE/pfc_realism_dynflat.csv" "$HERE/pfc_realism_mdp.csv"; do
  awk -F, -v f="$(basename "$f")" 'NR>1 && $NF!="OK"{bad++} END{printf "%s: %s non-OK run(s)\n", f, bad+0}' "$f"
done
