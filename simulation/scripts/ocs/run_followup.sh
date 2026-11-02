#!/bin/bash
set -u
OCS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WAIT_PID="${1:?pid of the running chain}"
stamp () { echo "===== $(date '+%F %T') $*"; }
while kill -0 "$WAIT_PID" 2>/dev/null; do sleep 30; done
stamp "START rebuild"
"$OCS/rebuild_all.sh" || { stamp "REBUILD FAILED"; exit 1; }
stamp "START regression at defaults"
"$OCS/regression/regress_defaults.sh" | tee "$OCS/regression/last_run.txt"
grep -q "0 failed" "$OCS/regression/last_run.txt" || { stamp "REGRESSION FAILED - not running the ECN sweep"; exit 1; }
stamp "START ECN exact-queue sweep"
"$OCS/sweep_ecn_exact.sh"
stamp "START Gap 2: 128-host DCTCP ForestColl again, with a 600 s simulated-time cap"
G="$OCS/../gap2"; C="$G/gap2_results_128.csv"
grep -v "^megafly,128,dctcp,forestcoll," "$C" > "$C.tmp" && mv "$C.tmp" "$C"
( cd "$G" && SKIP_BUILD=1 NHOST=128 FAMILIES=megafly ALGOS=forestcoll \
    ONLY="megafly:dctcp:forestcoll" SIMTIME=600 ./sweep_gap2.sh )
stamp "FOLLOWUP DONE"
