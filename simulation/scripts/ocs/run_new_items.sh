#!/bin/bash
set -u
OCS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WAIT_FILE="${1:-}"
stamp () { echo "===== $(date '+%F %T') $*"; }
if [ "${RESUME:-0}" != 1 ]; then
until grep -q "VERIFY DONE" "$WAIT_FILE" 2>/dev/null; do sleep 30; done
stamp "START rebuild"
"$OCS/rebuild_all.sh" || { stamp "REBUILD FAILED"; exit 1; }
stamp "START regression at defaults"
"$OCS/regression/regress_defaults.sh" | tee "$OCS/regression/last_run.txt"
grep -q "0 failed" "$OCS/regression/last_run.txt" || { stamp "REGRESSION FAILED - stopping"; exit 1; }
stamp "START new item 1: DCTCP exact queue + 1 ms timer scan"; "$OCS/sweep_dctcp_exact_rest.sh"
fi
stamp "START new item 2: phased MDP";               "$OCS/sweep_phased.sh"
stamp "START new item 4: seeds";                    "$OCS/sweep_seeds.sh"
stamp "START new item 5: 432 nodes";                "$OCS/sweep_432.sh"
stamp "START new item 3: ForestColl executor";      ( cd "$OCS/../gap2" && ./sweep_fc_exec.sh )
stamp "NEW ITEMS DONE"
