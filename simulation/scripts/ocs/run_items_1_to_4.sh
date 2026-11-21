#!/bin/bash
set -u
OCS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GAP2="$OCS/../gap2"
stamp () { echo "===== $(date '+%F %T') $*"; }

if [ "${SKIP_GAP2:-0}" != 1 ]; then
stamp "START item 1a: Gap 2 re-measure, 32 hosts (jumbo-frame fix + PAUSE delay)"
( cd "$GAP2" && SKIP_BUILD=1 PFC_VARIANTS="base delay" ./sweep_gap2.sh )
stamp "START item 1a: Gap 2 re-measure, 128-host Megafly"
( cd "$GAP2" && SKIP_BUILD=1 NHOST=128 FAMILIES=megafly ALGOS="forestcoll rhd" \
    PFC_VARIANTS="base delay" ./sweep_gap2.sh )
fi
stamp "START item 1b: PFC realism (LOGICAL_OCS, deadlock, OCS_MT_MDP)"
"$OCS/sweep_pfc_realism.sh"
stamp "START item 4: decompositions"
"$OCS/sweep_decomp.sh"
stamp "START item 3: reconfiguration delay"
"$OCS/sweep_reconf_delay.sh"
stamp "START item 2: scale (128 and 256 nodes)"
"$OCS/sweep_ocs_scale.sh"
stamp "ALL DONE"
