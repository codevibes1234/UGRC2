#!/bin/bash
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$HERE"
while pgrep -f '^/bin/bash ./gen_432.sh' > /dev/null; do sleep 60; done
cat gen_432.log
ONLY=G432 ./sweep_fc_tree.sh > sweep_fc_tree_g432.log 2>&1
tail -3 sweep_fc_tree_g432.log
echo "G432 CHAIN DONE"
