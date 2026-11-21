#!/bin/bash
R=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mt_mdp
R1=$R/run_one.sh
: > $R/todo.txt
emit() { echo "$R1 $*" >> $R/todo.txt; }

ALL6="fattree128 zcubepartial jellyfish flatbutterfly dragonfly megafly"
CYCLIC="jellyfish flatbutterfly dragonfly"

for v in roce dctcp; do for t in $ALL6; do
  emit $v $t ecmp compact 1 32 2 4 2 2 alone
  emit $v $t ecmp compact 4 32 2 4 2 2 mix4
done; done

for v in roce dctcp; do for t in fattree128 jellyfish; do
  for J in 2 3; do
    emit $v $t ecmp compact $J 32 2 4 2 2 mix$J
  done
done; done

for v in roce dctcp; do for t in $ALL6; do
  emit $v $t ecmp scatter 1 32 2 4 2 2 alone
  emit $v $t ecmp scatter 4 32 2 4 2 2 mix4
done; done

for t in $CYCLIC; do
  for pol in compact scatter; do
    emit roce $t updown $pol 1 32 2 4 2 2 alone
    emit roce $t updown $pol 4 32 2 4 2 2 mix4
  done
done

for v in roce dctcp; do for t in fattree128 zcubepartial; do
  emit $v $t ecmp compact 1 24 2 2 2 3 alone24
  emit $v $t ecmp compact 4 24 2 2 2 3 mix4_24
done; done

echo "todo.txt: $(wc -l < $R/todo.txt) runs"
awk '{print $2}' $R/todo.txt | sort | uniq -c | sed 's/^/  /'
