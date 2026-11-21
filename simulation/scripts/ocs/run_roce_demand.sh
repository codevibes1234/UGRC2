#!/bin/bash
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$HERE"
export RERUN_ONLY=roce:dheu ROCE_EXTRA="-rocedemand 1" OUT_PREFIX=rd_
for s in sweep_ocs.sh sweep_logical.sh generalise_ocs.sh sweep_ocs_mt_mdp.sh \
         sweep_pfc_realism.sh sweep_reconf_delay.sh sweep_decomp.sh \
         sweep_phased.sh sweep_seeds.sh sweep_ocs_scale.sh; do
  echo "===== $(date '+%F %T') $s"
  ./$s > "$HERE/rd_${s%.sh}.log" 2>&1
  echo "      rows: $(for c in rd_*.csv; do :; done; tail -n +2 -q $(ls -t rd_*.csv | head -2) 2>/dev/null | wc -l) (latest two files)"
done
echo "===== $(date '+%F %T') ROCE DEMAND RE-RUN DONE"
