#!/bin/bash
set -e
G="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLOS="$G/../FlexNetPacket/src/clos"
echo "### make lib";                 ( cd "$CLOS" && make lib >/dev/null 2>&1 )
echo "### OCS dyn_flat arms";        ( cd "$G/ocs" && ./build_ocs.sh >/dev/null 2>&1 )
echo "### zcube/mdp/mt_mdp arms";    ( cd "$CLOS/datacenter" && ./build_both_arms.sh >/dev/null 2>&1 )
echo "### graph multijob arms";      ( cd "$CLOS/datacenter" && ./build_multijob_arms.sh >/dev/null 2>&1 )
echo "### fattree multijob arms";    ( cd "$CLOS/datacenter" && ./build_fattree_multijob_arms.sh >/dev/null 2>&1 )
for f in jellyfish flatbutterfly dragonfly megafly; do for a in roce dctcp; do
  echo "### graph fc bench $f/32/$a"; ( cd "$G/gap2" && ./build_fc_bench.sh $f 32 $a >/dev/null 2>&1 )
done; done
for a in roce dctcp; do
  echo "### graph fc bench megafly/128/$a"; ( cd "$G/gap2" && ./build_fc_bench.sh megafly 128 $a >/dev/null 2>&1 )
done
for a in roce dctcp; do
  echo "### graph fc bench generic/$a"; ( cd "$G/gap2" && ./build_fc_bench.sh generic 0 $a >/dev/null 2>&1 )
done
echo "ALL BUILT"
