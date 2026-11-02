#!/bin/bash
R=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mt_mdp
n=0; total=$(wc -l < "$R/todo.txt")
while IFS= read -r line; do
  [ -z "$line" ] && continue
  n=$((n+1))
  echo "[$n/$total] $line"
  eval "$line" || echo "  !! nonzero exit"
done < "$R/todo.txt"
echo MT_MDP_SWEEP_DONE
