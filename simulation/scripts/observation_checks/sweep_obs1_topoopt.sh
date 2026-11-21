#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
G="$(cd "$HERE/.." && pwd)"
RUNS="$HERE/runs_obs1"; CSV="$HERE/obs1_topoopt.csv"
mkdir -p "$RUNS"
source "$G/ocs/lib_dynflat.sh"
[ -f "$CSV" ] || df_header
WORKLOADS=("dlrm 16 4" "vgg 16 4" "transformer 16 4" "candle 16 4" "dlrm 32 5" "vgg 32 5" "transformer 32 5")
CP="-creditprio 1"
have () { grep -q "^$1,$2,$3,$4,roce,$5,$6,$7," "$CSV"; }
for spec in "${WORKLOADS[@]}"; do
  set -- $spec
  have T "$1" "$2" "$3" shortest topoopt k8_cp || df_run T "$1" "$2" "$3" roce shortest topoopt k8_cp -pfcclasses 8 $CP
  for arm in jellyfish flatbutterfly dragonfly megafly; do
    have S "$1" "$2" "$3" updown "$arm" cp || df_run S "$1" "$2" "$3" roce updown "$arm" cp $CP
    have S "$1" "$2" "$3" shortest "$arm" k8_cp || df_run S "$1" "$2" "$3" roce shortest "$arm" k8_cp -pfcclasses 8 $CP
  done
  for d in 10 100 1000 10000 100000 1000000 10000000; do
    have D "$1" "$2" "$3" updown dheu "d${d}_cp" || df_run D "$1" "$2" "$3" roce updown dheu "d${d}_cp" -rdelayns "$d" -rperiodns 400000 -rocedemand 1 $CP
    have D "$1" "$2" "$3" shortest dheu "d${d}_k8_cp" || df_run D "$1" "$2" "$3" roce shortest dheu "d${d}_k8_cp" -rdelayns "$d" -rperiodns 400000 -rocedemand 1 -pfcclasses 8 $CP
  done
done
echo "OBS1 SWEEP DONE"
