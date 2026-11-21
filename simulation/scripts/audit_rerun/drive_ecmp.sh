#!/bin/bash
A=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/audit_rerun
while IFS= read -r l; do [ -z "$l" ] && continue; eval "$l"; done < "$A/todo_ecmp.txt"
echo ECMP_DONE
