#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export MDP_BIN_DIR="$HERE/bin_v3" ROCE_EXTRA="-rocedemand 1"
ONLY="${ONLY:-B D G}"
want () { case " $ONLY " in *" $1 "*) return 0;; esac; return 1; }
mflags () {
  case "$1" in
    conc)    echo "" ;;
    ph8c0)   echo "-phased 8" ;;
    ph8c100) echo "-phased 8 -fwdus 100 -bwdus 200" ;;
    f1b8c0)  echo "-iter1f1b 8 -layers 4 -dpbuckets 4" ;;
    f1b8c25) echo "-iter1f1b 8 -layers 4 -dpbuckets 4 -layerfwdus 25 -layerbwdus 50" ;;
  esac
}
RUNS="$HERE/runs_framework"; CSV="$HERE/framework.csv"
mkdir -p "$RUNS"
MDP_LABEL_COL=variant
MDP_DCTCP_EXTRA=-ecnqexact
source "$HERE/lib_mdp_ocs.sh"
[ -f "$CSV" ] || mdp_header
ARMS="jellyfish flatbutterfly dragonfly megafly dheu"

if want A; then
  echo "########## A: one 16-node job ##########"
  for mode in conc ph8c0 ph8c100 f1b8c0 f1b8c25; do for arm in $ARMS; do for tr in roce dctcp; do
    mdp_run A 16 4 "$arm" "$tr" compact 1 16 2 2 2 2 rhd "$mode" $(mflags $mode)
  done; done; done
fi
if want B; then
  echo "########## B: one 128-node job ##########"
  for mode in f1b8c0 f1b8c25; do for arm in $ARMS; do for tr in roce dctcp; do
    mdp_run B 128 9 "$arm" "$tr" compact 1 128 4 4 4 2 rhd "$mode" $(mflags $mode)
  done; done; done
fi
if want D; then
  echo "########## D: tenancy, 128 nodes ##########"
  for mode in f1b8c25; do for arm in $ARMS; do for tr in roce dctcp; do for pol in compact scatter; do
    for j in 1 2 4; do
      mdp_run D 128 9 "$arm" "$tr" "$pol" "$j" 32 2 4 2 2 rhd "$mode" $(mflags $mode)
    done
  done; done; done; done
fi
if want G; then
  echo "########## G: switch fabrics, one 128-node job ##########"
  GCSV="$HERE/framework_switch.csv"
  [ -f "$GCSV" ] || echo "topo,transport,mode,exit,wall_s,job0_ms,tp_ms,dp_ms,ep_ms,pp_ms,incomplete,verdict" > "$GCSV"
  for topo in fattree128 jellyfish flatbutterfly dragonfly megafly; do
    rt=updown; [ "$topo" = fattree128 ] && rt=ecmp
    for mode in conc ph8c100 f1b8c25; do for tr in roce dctcp; do
      bin="$HERE/bin_v3/$tr/htsim_tcp_mt_mdp_bench"; tag="G_${topo}_${tr}_${mode}"
      out="$RUNS/$tag.txt"; log="$RUNS/$tag.log"
      printf '%-50s ' "$tag"
      flock "$SIMLOCK" /usr/bin/time -f "%e" -o "$RUNS/$tag.time" timeout 3600 "$bin" -topo "$topo" -nodes 128 \
        -routing "$rt" -jobs 1 -jobnodes 128 -jobtp 4 -jobdp 2 -jobpp 4 -jobep 4 -dims tp,dp,ep,pp \
        -placement compact -placementseed 13 -speed 200000 -q 200 -ssthresh 15 -simtime 600 \
        $(mflags $mode) -ofile "$out" > "$log" 2>&1
      rc=$?
      g () { local v=$(grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'); echo "${v:-NA}"; }
      inc=$(g Incomplete); v=OK
      [ "$rc" -ne 0 ] && v="EXIT$rc"; [ "$inc" = NA ] && v=NO_RESULT; [ "$inc" != NA ] && [ "$inc" != 0 ] && v=INCOMPLETE
      echo "$topo,$tr,$mode,$rc,$(tail -1 "$RUNS/$tag.time"),$(g Job0_Makespan_ms),$(g Job0_Class_tp_ms),$(g Job0_Class_dp_ms),$(g Job0_Class_ep_ms),$(g Job0_Class_pp_ms),$inc,$v" >> "$GCSV"
      echo "job0=$(g Job0_Makespan_ms) $v"
      if [ "$(stat -c %s "$log")" -gt 2000000 ]; then { head -40 "$log"; echo ...; tail -40 "$log"; } > "$log.t" && mv "$log.t" "$log"; fi
    done; done
  done
fi
echo "FRAMEWORK SWEEP DONE ($ONLY)"
