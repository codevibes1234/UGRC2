#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export DF_BIN_DIR="$HERE/bin_pfc" MDP_BIN_DIR="$HERE/bin_pfc" ROCE_EXTRA="-rocedemand 1"
ONLY="${ONLY:-P M}"
want () { case " $ONLY " in *" $1 "*) return 0;; esac; return 1; }
DCX="-ecnqexact -rtxscanms 1"

if want P; then
  RUNS="$HERE/runs_pfc_classes"; CSV="$HERE/pfc_classes_dynflat.csv"
  mkdir -p "$RUNS"
  source "$HERE/lib_dynflat.sh"
  df_header
  for cell in "dlrm 16 4" "vgg 16 4" "transformer 16 4" "candle 16 4" "dlrm 32 5" "vgg 32 5" "transformer 32 5"; do
    set -- $cell; wl=$1; n=$2; deg=$3
    for arm in jellyfish flatbutterfly dragonfly megafly; do
      df_run P "$wl" "$n" "$deg" roce updown   "$arm" k1
      df_run P "$wl" "$n" "$deg" roce shortest "$arm" k1
      for k in 2 3 4 8; do df_run P "$wl" "$n" "$deg" roce shortest "$arm" "k$k" -pfcclasses $k; done
      df_run P "$wl" "$n" "$deg" roce updown   "$arm" k8 -pfcclasses 8
      df_run P "$wl" "$n" "$deg" dctcp updown   "$arm" corrected $DCX
      df_run P "$wl" "$n" "$deg" dctcp shortest "$arm" corrected $DCX
    done
    df_run P "$wl" "$n" "$deg" roce updown   dheu k1
    df_run P "$wl" "$n" "$deg" roce shortest dheu k8 -pfcclasses 8
    df_run P "$wl" "$n" "$deg" dctcp updown   dheu corrected $DCX
    df_run P "$wl" "$n" "$deg" dctcp shortest dheu corrected $DCX
  done
fi

if want M; then
  RUNS="$HERE/runs_pfc_classes_mdp"; CSV="$HERE/pfc_classes_mdp.csv"
  mkdir -p "$RUNS"
  MDP_LABEL_COL=variant
  MDP_DCTCP_EXTRA=-ecnqexact
  source "$HERE/lib_mdp_ocs.sh"
  mdp_header
  for fab in "16 4 2 2 2 2" "128 9 4 4 4 2"; do
    set -- $fab; n=$1; deg=$2; tp=$3; dp=$4; pp=$5; ep=$6
    for arm in jellyfish flatbutterfly dragonfly megafly; do
      mdp_run M "$n" "$deg" "$arm" roce compact 1 "$n" $tp $dp $pp $ep rhd updown_k1
      mdp_run M "$n" "$deg" "$arm" roce compact 1 "$n" $tp $dp $pp $ep rhd shortest_k1 -routing shortest
      mdp_run M "$n" "$deg" "$arm" roce compact 1 "$n" $tp $dp $pp $ep rhd shortest_k4 -routing shortest -pfcclasses 4
      mdp_run M "$n" "$deg" "$arm" roce compact 1 "$n" $tp $dp $pp $ep rhd shortest_k8 -routing shortest -pfcclasses 8
      mdp_run M "$n" "$deg" "$arm" dctcp compact 1 "$n" $tp $dp $pp $ep rhd updown
      mdp_run M "$n" "$deg" "$arm" dctcp compact 1 "$n" $tp $dp $pp $ep rhd shortest -routing shortest
    done
  done
fi
echo "PFC CLASSES SWEEP DONE ($ONLY)"
