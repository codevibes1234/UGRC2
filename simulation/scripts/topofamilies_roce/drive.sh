#!/bin/bash
T=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/topofamilies_roce
for f in todo_ar todo_tg todo_mt; do
  while IFS= read -r line; do
    [ -z "$line" ] && continue
    eval "$line"
  done < "$T/$f.txt"
  echo "${f}_DONE"
done
echo ALL_DONE
