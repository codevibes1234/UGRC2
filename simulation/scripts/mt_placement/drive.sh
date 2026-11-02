#!/bin/bash
R=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mt_placement
n=0; total=$(wc -l < "$R/todo.txt")
while IFS= read -r line; do
  [ -z "$line" ] && continue
  n=$((n+1)); echo "[$n/$total] $(echo "$line" | awk '{print $2,$3,$4,$5}')  $(date +%H:%M:%S)"
  eval "$line" || echo "  !! nonzero exit"
done < "$R/todo.txt"
echo MT_PLACEMENT_SWEEP_DONE
