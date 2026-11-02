#!/bin/bash
R=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mdp_roce
while IFS= read -r line; do [ -z "$line" ] && continue; eval "$line"; done < "$R/todo.txt"
echo MDP_DONE
