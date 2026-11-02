#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
RUNS="$HERE/runs"; CSV="$HERE/mt2.csv"
ONLY="${ONLY:-RTO T16 T8 BLK HET T432}"
mkdir -p "$RUNS"
[ -f "$CSV" ] || echo "series,topo,nodes,deg,arm,transport,placement,label,jobs,exit,wall_s,rss_kb,pfc_stop,verdict,job_idx,job_decomp,job_nodes_md5,makespan_ms,tp_ms,dp_ms,ep_ms,pp_ms,incomplete,rto_job,rto_total" > "$CSV"
want () { case " $ONLY " in *" $1 "*) return 0;; esac; return 1; }

mt2_run () {
  local series="$1" topo="$2" n="$3" deg="$4" arm="$5" tr="$6" pol="$7" label="$8"
  shift 8
  local bin="$HERE/bin/$tr/htsim_tcp_mt_mdp_bench" fab common trx=""
  if [ "$topo" = ocs ]; then
    fab="-topo ocs -deg $deg -rdelay 10"; [ "$arm" != dheu ] && fab="$fab -logical $arm"
    common="-speed 100000 -q 100 -ssthresh 10000 -simtime 60 -routing updown"
    if [ "$tr" = roce ]; then trx="-rocedemand 1"; else trx="-ecnqexact"; fi
  else
    fab="-topo $topo"
    common="-speed 200000 -q 200 -ssthresh 15 -simtime 600 -routing ecmp"
  fi
  local tag="${series}_${topo}${n}_${arm}_${tr}_${pol}_${label}"
  local out="$RUNS/$tag.txt" log="$RUNS/$tag.log"
  printf '%-70s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" \
    timeout 5400 "$bin" $fab -nodes "$n" -dims tp,dp,ep,pp -placement "$pol" \
      -placementseed 13 $common "$@" $trx -ofile "$out" > "$log" 2>&1
  local rc=$?
  local tl=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tl" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tl" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')
  local stop=$(grep -c 'PAUSE 1000' "$log")
  local jobs=$(grep -m1 '^Jobs ' "$out" 2>/dev/null | awk '{print $2}')
  local inc=$(grep -m1 '^Incomplete ' "$out" 2>/dev/null | awk '{print $2}')
  local verdict=OK
  if   grep -qi "Assertion" "$log"; then verdict=ASSERT
  elif grep -q "ERROR" "$log";      then verdict=ERROR
  elif [ "$rc" -ne 0 ];             then verdict="EXIT$rc"
  elif [ -z "$inc" ];               then verdict=NO_RESULT
  elif [ "$inc" != 0 ];             then verdict=INCOMPLETE
  fi
  python3 "$HERE/mt2_rows.py" "$out" "$log" \
    "$series,$topo,$n,$deg,$arm,$tr,$pol,$label,${jobs:-NA},$rc,$wall,$rss,$stop,$verdict" >> "$CSV"
  printf 'job0=%-9s mk=%-9s rto=%-4s %ss %s\n' "$(grep -m1 '^Job0_Makespan_ms ' "$out" 2>/dev/null | awk '{print $2}')" \
    "$(grep -m1 '^Makespan_ms ' "$out" 2>/dev/null | awk '{print $2}')" "$(grep -c ' RTO ' "$log")" "$wall" "$verdict"
  if [ "$(stat -c %s "$log")" -gt 3000000 ]; then
    { grep -v 'PAUSE' "$log" | head -400; echo "... [PAUSE lines dropped; $stop PAUSE 1000 events] ..."; } > "$log.trim" && mv "$log.trim" "$log"
  fi
}

ARMS="jellyfish flatbutterfly dragonfly megafly dheu"

if want RTO; then
  echo "########## RTO: per-job attribution of MT_MDP_RESULTS.md section 7 ##########"
  for cfg in "jellyfish compact 32 2 4 2 2" "zcubepartial scatter 32 2 4 2 2" "fattree128 compact 24 2 2 2 3"; do
    set -- $cfg
    for j in 1 4; do
      mt2_run RTO "$1" 128 0 static dctcp "$2" "j$j" -jobs $j -jobnodes $3 -jobdp $4 -jobtp $5 -jobpp $6 -jobep $7
    done
  done
fi

if want T16; then
  echo "########## T16: 1-8 sixteen-node jobs, 128 nodes, degree 9 ##########"
  for arm in $ARMS; do for tr in roce dctcp; do for pol in compact scatter; do
    for j in 1 2 4 8; do
      mt2_run T16 ocs 128 9 "$arm" "$tr" "$pol" "j$j" -jobs $j -jobnodes 16 -jobtp 2 -jobdp 4 -jobpp 2 -jobep 1
    done
  done; done; done
fi

if want T8; then
  echo "########## T8: 1-16 eight-node jobs ##########"
  for arm in $ARMS; do for tr in roce dctcp; do for pol in compact scatter; do
    for j in 1 4 16; do
      mt2_run T8 ocs 128 9 "$arm" "$tr" "$pol" "j$j" -jobs $j -jobnodes 8 -jobtp 2 -jobdp 2 -jobpp 2 -jobep 1
    done
  done; done; done
fi

if want BLK; then
  echo "########## BLK: block placement, four 32-node jobs ##########"
  for arm in $ARMS; do for tr in roce dctcp; do
    for b in 1 2 4 8 16 32; do for j in 1 4; do
      mt2_run BLK ocs 128 9 "$arm" "$tr" block "b${b}_j$j" -blocksize $b -jobs $j -jobnodes 32 -jobtp 2 -jobdp 4 -jobpp 2 -jobep 2
    done; done
  done; done
fi

if want HET; then
  echo "########## HET: heterogeneous mixes, each type as job 0 ##########"
  A=2x4x2x2; B=1x32x1x1; C=2x2x1x8; D=2x2x8x1
  for arm in $ARMS; do for tr in roce dctcp; do for pol in compact scatter; do
    for rot in "A $A,$B,$C,$D" "B $B,$C,$D,$A" "C $C,$D,$A,$B" "D $D,$A,$B,$C"; do
      set -- $rot
      t=$1; mix=$2; me=${mix%%,*}
      mt2_run HET ocs 128 9 "$arm" "$tr" "$pol" "${t}_alone" -jobmix "$me"
      mt2_run HET ocs 128 9 "$arm" "$tr" "$pol" "${t}_homog" -jobmix "$me,$me,$me,$me"
      mt2_run HET ocs 128 9 "$arm" "$tr" "$pol" "${t}_hetero" -jobmix "$mix"
    done
  done; done; done
fi

if want T432; then
  echo "########## T432: 1 and 27 sixteen-node jobs on 432 nodes, degree 16 ##########"
  for arm in jellyfish dragonfly megafly; do for tr in roce dctcp; do for pol in compact scatter; do
    for j in 1 27; do
      mt2_run T432 ocs 432 16 "$arm" "$tr" "$pol" "j$j" -jobs $j -jobnodes 16 -jobtp 2 -jobdp 4 -jobpp 2 -jobep 1
    done
  done; done; done
fi

echo
awk -F, 'NR>1 && $14!="OK"{bad[$1" "$2" "$5" "$6" "$7" "$8]=$14} END{n=0; for(k in bad){n++; print "  non-OK: " k " " bad[k]} printf "%d non-OK run(s)\n", n}' "$CSV"
echo "MT2 SWEEP DONE ($ONLY)"
