#!/bin/bash
set -u
OCS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GAP2="$OCS/../gap2"
while pgrep -f '^/bin/bash ./sweep_fc_tree.sh' > /dev/null; do sleep 20; done
echo "===== $(date '+%F %T') regression"
if ! "$OCS/regression/regress_defaults.sh"; then echo "REGRESSION FAILED - stopping"; exit 1; fi
echo "===== $(date '+%F %T') RoCE demand re-runs"
"$OCS/run_roce_demand.sh"
echo "===== $(date '+%F %T') waiting for the 128-host tree files"
while pgrep -f '^/bin/bash ./regen_tree_xml.sh' > /dev/null; do sleep 30; done
cat "$GAP2/regen_tree_xml.log"
echo "===== $(date '+%F %T') ForestColl trees at 128 hosts"
( cd "$GAP2" && ONLY=G128 ./sweep_fc_tree.sh > sweep_fc_tree_g128.log 2>&1 )
tail -3 "$GAP2/sweep_fc_tree_g128.log"
echo "===== $(date '+%F %T') FOLLOWUP2 CHAIN DONE"
