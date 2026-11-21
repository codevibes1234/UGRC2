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
RUNS="$HERE/${OUT_PREFIX:-}runs_pfc_realism_mdp"; CSV="$HERE/${OUT_PREFIX:-}pfc_realism_mdp.csv"
mkdir -p "$RUNS"
source "$HERE/lib_mdp_ocs.sh"
mdp_header
echo "C: OCS_MT_MDP series, RoCE, four PFC models"
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
awk -F, -v f="$(basename "$CSV")" 'NR>1 && $NF!="OK"{bad++} END{printf "%s: %s non-OK run(s)\n", f, bad+0}' "$CSV"
