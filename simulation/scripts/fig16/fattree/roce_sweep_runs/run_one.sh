#!/bin/bash
topo=$1; f=$2; r=$3
DC=/home/venkatks/Code/git_repositories/UGRC2/simulation/FlexNetPacket/src/clos/datacenter
TG=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/fig16/fattree/tgs
OUT=/home/venkatks/Code/git_repositories/UGRC2/simulation/scripts/fig16/fattree/roce_sweep_runs/$topo
base=${f%.fbuf}
tag=$(echo "$f" | sed 's/\.fbuf/_/g; s/_$//')
bw=$(echo "$base" | sed -E 's/^([0-9]+)[a-z].*/\1/')
model=$(echo "$base" | sed -E 's/^[0-9]+([a-z]+)_.*/\1/')
n=$(echo "$base" | sed -E 's/^[0-9]+[a-z]+_([0-9]+).*/\1/')
if [ "$topo" = fattree ]; then case $n in 16) nodes=16;; 32) nodes=54;; *) nodes=$n;; esac
else case $n in 16) nodes=16;; 32) nodes=36;; *) nodes=$n;; esac; fi
d=$OUT/$tag/run$r; mkdir -p $d
s=$(date +%s.%N)
timeout 3600 $DC/htsim_tcp_$topo -simtime 3600.1 -flowfile $TG/$f -speed $((bw*1000)) -ofile $d/nwsim.txt -nodes $nodes -ssthresh 10000 -rttnet 1000 -rttrack 1000 -q 10000 > $d/stdout.txt 2> $d/stderr.txt
rc=$?
e=$(date +%s.%N)
wall=$(echo "$e - $s" | bc)
ff=$(grep FinalFinish $d/nwsim.txt 2>/dev/null | awk '{print $2}')
iters=$(grep -c "finished one iter" $d/stderr.txt)
last=$(grep "finished one task" $d/stderr.txt | tail -1 | sed -E 's/.*nfin ([0-9]+) ntot ([0-9]+).*/\1\/\2/')
tail -50 $d/stderr.txt > $d/stderr.tail && mv $d/stderr.tail $d/stderr.txt
echo "$tag,$model,$n,$nodes,$bw,$r,$rc,$ff,$iters,$last,$wall" >> $OUT/results.csv
