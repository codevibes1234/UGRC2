#!/bin/bash
set -u
LABEL="${1:?usage: smoke_test.sh <label> [quick]}"
MODE="${2:-full}"

ROOT=/home/venkatks/Code/git_repositories/UGRC2
DC=$ROOT/simulation/FlexNetPacket/src/clos/datacenter
REF=$ROOT/simulation/scripts/audit_rerun/runs/mdp/roce
OUT=$ROOT/simulation/scripts/mt_mdp/smoke/$LABEL

if [ "$MODE" = quick ]; then
  CONFIGS="fattree128/ecmp/all jellyfish/ecmp/all jellyfish/updown/all megafly/ecmp/all dragonfly/ecmp/tp"
else
  CONFIGS=$(cd "$REF" && find . -name o.txt | sed 's|^\./||; s|/o\.txt$||' | sort)
fi

mkdir -p "$OUT"
pass=0; fail=0; missing=0
: > "$OUT/report.txt"

for cfg in $CONFIGS; do
  topo=${cfg%%/*}; rest=${cfg#*/}; rt=${rest%%/*}; tag=${rest#*/}
  d="$OUT/$cfg"; mkdir -p "$d"

  if [ "$tag" = all ]; then dims="tp,dp,ep,pp"; else dims="$tag"; fi

  timeout 3600 "$DC/htsim_tcp_mdp_bench" -topo "$topo" -nodes 128 -routing "$rt" \
      -dp 2 -tp 4 -pp 4 -ep 4 -dims "$dims" -speed 200000 -q 200 -ssthresh 15 \
      -simtime 600 -ofile "$d/o.txt" > "$d/l.txt" 2>&1
  rc=$?

  if [ ! -f "$REF/$cfg/o.txt" ]; then
    echo "MISSING-REF $cfg" | tee -a "$OUT/report.txt"; missing=$((missing+1)); continue
  fi
  if diff -q "$REF/$cfg/o.txt" "$d/o.txt" > /dev/null 2>&1; then
    echo "PASS $cfg (rc=$rc)" | tee -a "$OUT/report.txt"; pass=$((pass+1))
  else
    echo "FAIL $cfg (rc=$rc)" | tee -a "$OUT/report.txt"
    diff "$REF/$cfg/o.txt" "$d/o.txt" | sed 's/^/     /' | tee -a "$OUT/report.txt"
    fail=$((fail+1))
  fi
  tail -5 "$d/l.txt" > "$d/t" && mv "$d/t" "$d/l.txt"
done

echo "---- $LABEL: $pass pass, $fail fail, $missing missing-ref ----" | tee -a "$OUT/report.txt"
[ "$fail" -eq 0 ] && [ "$missing" -eq 0 ]
