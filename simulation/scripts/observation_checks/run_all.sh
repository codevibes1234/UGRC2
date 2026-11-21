#!/bin/bash
cd "$(dirname "$0")"
for s in sweep_obs5_topoopt.sh sweep_obs1_topoopt.sh sweep_obs4_rto.sh sweep_obs3_seeds.sh sweep_obs2_dcqcn.sh; do
  echo "== $s start $(date +%T)"
  ./"$s" > "${s%.sh}.log" 2>&1
  echo "== $s end $(date +%T) rc=$?"
done
echo "ALL SWEEPS DONE $(date +%T)"
