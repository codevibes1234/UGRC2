#!/bin/bash
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$HERE"
LIST="sweep_ocs_mt_mdp.sh sweep_pfc_realism.sh sweep_decomp.sh sweep_phased.sh sweep_seeds.sh sweep_ocs_scale.sh"
echo "===== $(date '+%F %T') reproduction check (current binaries, fix off)"
./repro_check.sh > repro_check.log 2>&1
if ! python3 -I repro_compare.py > repro_compare.txt 2>&1; then
  cat repro_compare.txt
  echo "===== $(date '+%F %T') STOPPED: stored results do not reproduce"
  exit 1
fi
cat repro_compare.txt
for s in $LIST; do
  echo "===== $(date '+%F %T') A $s"
  CP_SKIP_DHEU=1 ./$s > "A_${s%.sh}.log" 2>&1
  echo "      rows: $(($(wc -l < "$(ls -t *.csv | head -1)") - 1)) in $(ls -t *.csv | head -1)"
done
for s in $LIST; do
  echo "===== $(date '+%F %T') B $s"
  RERUN_ONLY=roce:dheu ROCE_EXTRA="-rocedemand 1" OUT_PREFIX=rd_ ./$s > "B_${s%.sh}.log" 2>&1
  echo "      rows: $(($(wc -l < "$(ls -t *.csv | head -1)") - 1)) in $(ls -t *.csv | head -1)"
done
for s in sweep_432.sh sweep_dheu_scale.sh sweep_framework.sh sweep_obs3_place.sh; do
  echo "===== $(date '+%F %T') C $s"
  ./$s > "C_${s%.sh}.log" 2>&1
  echo "      rows: $(($(wc -l < "$(ls -t *.csv | head -1)") - 1)) in $(ls -t *.csv | head -1)"
done
echo "===== $(date '+%F %T') CP RE-RUN DONE"
