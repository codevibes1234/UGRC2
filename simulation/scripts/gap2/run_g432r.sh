#!/bin/bash
cd "$(dirname "$0")"
until grep -q 'BUILDS DONE' /tmp/claude-1000/-home-venkatks-Code-git-repositories-UGRC2/82ef024e-f9a0-4e83-b3ff-0720f1f18bcd/scratchpad/rebuild_now.log 2>/dev/null; do sleep 15; done
ONLY=G432R ./sweep_fc_tree.sh > sweep_fc_tree_g432r.log 2>&1
