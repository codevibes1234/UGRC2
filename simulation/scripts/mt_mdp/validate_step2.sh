#!/bin/bash
set -u
STAGE="${1:-all}"
ROOT=/home/venkatks/Code/git_repositories/UGRC2
DC=$ROOT/simulation/FlexNetPacket/src/clos/datacenter
BIN=$DC/htsim_tcp_mt_mdp_bench
REF=$ROOT/simulation/scripts/audit_rerun/runs/mdp/roce
OUT=$ROOT/simulation/scripts/mt_mdp/validate
mkdir -p "$OUT"

pass=0; fail=0
ok()   { echo "  PASS  $1"; pass=$((pass+1)); }
bad()  { echo "  FAIL  $1"; fail=$((fail+1)); }

COMMON="-nodes 128 -speed 200000 -q 200 -ssthresh 15 -simtime 600"

get() { grep -m1 "^$2 " "$1" 2>/dev/null | awk '{print $2}'; }

if [ "$STAGE" = args ] || [ "$STAGE" = all ]; then
echo "=== stage args: negative cases ==="
d=$OUT/args; mkdir -p "$d"

reject() {
  local what="$1"; shift
  if "$BIN" $COMMON -topo fattree128 -ofile "$d/o.txt" "$@" > "$d/l.txt" 2>&1; then
    bad "$what (was ACCEPTED)"
  else
    rc=$?
    if [ "$rc" -eq 1 ] && grep -q ERROR "$d/l.txt"; then
      ok "$what -> $(grep -m1 ERROR "$d/l.txt" | cut -c1-90)"
    else
      bad "$what (exit $rc, no ERROR line)"
    fi
  fi
}

reject "decomposition that does not multiply to -jobnodes" \
       -jobs 1 -jobnodes 32 -jobdp 2 -jobtp 4 -jobpp 2 -jobep 3 -dims tp
reject "more jobs than the cluster can hold" \
       -jobs 5 -jobnodes 32 -jobdp 2 -jobtp 4 -jobpp 2 -jobep 2 -dims tp
reject "unknown dimension name" \
       -jobs 1 -jobnodes 32 -jobdp 2 -jobtp 4 -jobpp 2 -jobep 2 -dims all
reject "unknown placement policy" \
       -jobs 1 -jobnodes 32 -jobdp 2 -jobtp 4 -jobpp 2 -jobep 2 -dims tp -placement random
reject "zero jobs" \
       -jobs 0 -jobnodes 32 -jobdp 2 -jobtp 4 -jobpp 2 -jobep 2 -dims tp
fi

if [ "$STAGE" = j1 ] || [ "$STAGE" = all ]; then
echo "=== stage j1: equivalence with the single-job paragraph-3 sweep ==="
CONFIGS=$(cd "$REF" && find . -name o.txt | sed 's|^\./||; s|/o\.txt$||' | sort)
for cfg in $CONFIGS; do
  topo=${cfg%%/*}; rest=${cfg#*/}; rt=${rest%%/*}; tag=${rest#*/}
  d=$OUT/j1/$cfg; mkdir -p "$d"
  if [ "$tag" = all ]; then dims="tp,dp,ep,pp"; else dims="$tag"; fi

  timeout 3600 "$BIN" $COMMON -topo "$topo" -routing "$rt" \
      -jobs 1 -jobnodes 128 -jobdp 2 -jobtp 4 -jobpp 4 -jobep 4 \
      -dims "$dims" -placement compact -placementseed 13 \
      -ofile "$d/o.txt" > "$d/l.txt" 2>&1

  diffs=""
  for dim in tp dp ep pp; do
    r=$(get "$REF/$cfg/o.txt" "Class_${dim}_ms"); [ -z "$r" ] && continue
    n=$(get "$d/o.txt" "Job0_Class_${dim}_ms")
    [ "$r" != "$n" ] && diffs="$diffs Class_${dim}_ms($r vs $n)"
    rt_=$(get "$REF/$cfg/o.txt" "Tasks_${dim}"); n_=$(get "$d/o.txt" "Job0_Tasks_${dim}")
    [ "$rt_" != "$n_" ] && diffs="$diffs Tasks_${dim}($rt_ vs $n_)"
  done
  for key in Makespan_ms Incomplete; do
    r=$(get "$REF/$cfg/o.txt" "$key"); n=$(get "$d/o.txt" "$key")
    [ "$r" != "$n" ] && diffs="$diffs $key($r vs $n)"
  done

  if [ -z "$diffs" ]; then ok "j1 $cfg"; else bad "j1 $cfg:$diffs"; fi
  tail -5 "$d/l.txt" > "$d/t" && mv "$d/t" "$d/l.txt"
done
fi

if [ "$STAGE" = multi ] || [ "$STAGE" = all ]; then
echo "=== stage multi: multi-job behaviour ==="
JOB4="-jobs 4 -jobnodes 32 -jobdp 2 -jobtp 4 -jobpp 2 -jobep 2 -dims tp,dp,ep,pp"
JOB1="-jobs 1 -jobnodes 32 -jobdp 2 -jobtp 4 -jobpp 2 -jobep 2 -dims tp,dp,ep,pp"

for pol in compact scatter; do
  d=$OUT/multi/$pol; mkdir -p "$d"
  timeout 3600 "$BIN" $COMMON -topo fattree128 $JOB4 \
      -placement $pol -placementseed 13 -ofile "$d/mix.txt" > "$d/mix.log" 2>&1
  timeout 3600 "$BIN" $COMMON -topo fattree128 $JOB1 \
      -placement $pol -placementseed 13 -ofile "$d/alone.txt" > "$d/alone.log" 2>&1

  [ "$(get "$d/mix.txt" Jobs)" = 4 ] && ok "$pol: 4 jobs reported" || bad "$pol: job count"

  [ "$(get "$d/mix.txt" Incomplete)" = 0 ] && ok "$pol: mix completed, no stalls" \
      || bad "$pol: mix had $(get "$d/mix.txt" Incomplete) incomplete tasks"

  n_distinct=$(grep "^Job._Nodes " "$d/mix.txt" | cut -d' ' -f2- | tr ',' '\n' | sort -n | uniq | wc -l)
  n_total=$(grep "^Job._Nodes " "$d/mix.txt" | cut -d' ' -f2- | tr ',' '\n' | wc -l)
  [ "$n_distinct" = 128 ] && [ "$n_total" = 128 ] \
      && ok "$pol: 4x32 jobs hold 128 distinct nodes" \
      || bad "$pol: $n_total node slots, $n_distinct distinct"

  a=$(get "$d/alone.txt" Job0_Nodes); m=$(get "$d/mix.txt" Job0_Nodes)
  [ "$a" = "$m" ] && ok "$pol: job 0 placement identical alone and in the mix" \
      || bad "$pol: job 0 moved between runs (alone=$a mix=$m)"

  am=$(get "$d/alone.txt" Job0_Makespan_ms); mm=$(get "$d/mix.txt" Job0_Makespan_ms)
  if awk -v a="$am" -v m="$mm" 'BEGIN{exit !(m >= a)}'; then
    if [ "$am" = "$mm" ]; then
      ok "$pol: job 0 unaffected by neighbours ($am ms, link-disjoint)"
    else
      ok "$pol: job 0 slower in the mix ($am -> $mm ms, $(awk -v a="$am" -v m="$mm" 'BEGIN{printf "%.2fx", m/a}'))"
    fi
  else
    bad "$pol: job 0 FASTER in the mix ($am -> $mm ms) - impossible, indicates a bug"
  fi
done

cm=$(get "$OUT/multi/compact/mix.txt" Job0_Nodes)
sc=$(get "$OUT/multi/scatter/mix.txt" Job0_Nodes)
[ "$cm" = "$(seq -s, 0 31)" ] && ok "compact: job 0 got nodes 0..31" || bad "compact: job 0 got $cm"
[ "$sc" != "$cm" ] && ok "scatter: job 0 placement differs from compact" || bad "scatter: identical to compact"
fi

echo "---- validate_step2 ($STAGE): $pass pass, $fail fail ----"
[ "$fail" -eq 0 ]
