#!/bin/bash
R=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/mt_placement
TG=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/fig16/fattree/tgs
: > $R/todo.txt
D=$TG/100dlrm_16.fbuf; V=$TG/100vgg_16.fbuf
T=$TG/100transformer_16.fbuf; C=$TG/100candle_16.fbuf
for v in roce dctcp; do
  for pol in compact scatter; do
    for topo in fattree zcubepartial jellyfish flatbutterfly dragonfly megafly; do
      echo "$R/run_mt2.sh $v $topo $pol alone-dlrm $D" >> $R/todo.txt
      echo "$R/run_mt2.sh $v $topo $pol alone-vgg  $V" >> $R/todo.txt
      echo "$R/run_mt2.sh $v $topo $pol mix2 $D,$V"    >> $R/todo.txt
      echo "$R/run_mt2.sh $v $topo $pol mix4 $D,$V,$T,$C" >> $R/todo.txt
    done
  done
done
echo "todo.txt: $(wc -l < $R/todo.txt) runs"
