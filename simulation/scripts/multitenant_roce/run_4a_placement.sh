#!/bin/bash
set -uo pipefail

R="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DC="$R/../../FlexNetPacket/src/clos/datacenter"
TG="$R/../fig16/fattree/tgs"
RUNS="$R/runs_placement"; CSV="$R/results_placement.csv"
mkdir -p "$RUNS"

echo "transport,placement,mix,seed,job,rc,wall_s,rss_kb,finalfinish_ps,rto,nodes_n,nodes_md5,verdict" > "$CSV"

declare -A MIX
MIX[alone-dlrm]="$TG/100dlrm_16.fbuf"
MIX[alone-vgg]="$TG/100vgg_16.fbuf"
MIX[alone-transformer]="$TG/100transformer_16.fbuf"
MIX[alone-candle]="$TG/100candle_16.fbuf"
MIX[mix2]="$TG/100dlrm_16.fbuf,$TG/100vgg_16.fbuf"
MIX[mix4]="$TG/100dlrm_16.fbuf,$TG/100vgg_16.fbuf,$TG/100transformer_16.fbuf,$TG/100candle_16.fbuf"
ORDER="alone-dlrm alone-vgg alone-transformer alone-candle mix2 mix4"

run_one () {
  local v="$1" pol="$2" mix="$3" seed="$4"
  local bin="$DC/htsim_tcp_fattree_multijob"
  [ "$v" = dctcp ] && bin="$DC/dctcp_bin/htsim_tcp_fattree_multijob"
  local d="$RUNS/$v/$pol/$mix/seed$seed"; mkdir -p "$d"

  printf '%-46s ' "$v/$pol/$mix/seed$seed"
  /usr/bin/time -f "%e %M" -o "$d/time" \
  timeout 3600 "$bin" -simtime 3600.1 -flowfiles "${MIX[$mix]}" -speed 100000 \
      -ofile "$d/out.txt" -nodes 128 -ssthresh 10000 -rttnet 1000 -rttrack 1000 \
      -q 10000 -placementseed "$seed" -placement "$pol" \
      > "$d/log.txt" 2> "$d/err.txt"
  local rc=$?
  local tl=$(tail -1 "$d/time" 2>/dev/null)
  local wall=$(echo "$tl" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tl" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')
  local rto=$(grep -c ' RTO ' "$d/log.txt" 2>/dev/null)

  local n=0
  while read -r name ps; do
    local job=$(echo "$name" | sed 's/FinalFinish_//')
    local nl=$(grep -m1 "^Nodes_.*${job} " "$d/out.txt" | cut -d' ' -f2-)
    local nn=$(echo "$nl" | tr ',' '\n' | grep -c .)
    local md5=$(echo "$nl" | md5sum | cut -c1-8)
    local v2=OK
    [ "$rc" -ne 0 ] && v2="EXIT$rc"
    [ "$ps" = 0 ] && v2=NO_ITERATION
    echo "$v,$pol,$mix,$seed,$job,$rc,$wall,$rss,$ps,$rto,$nn,$md5,$v2" >> "$CSV"
    n=$((n+1))
  done < <(grep FinalFinish "$d/out.txt" 2>/dev/null | sed 's|/home.*/tgs/||')

  if [ "$n" -eq 0 ]; then
    echo "$v,$pol,$mix,$seed,NA,$rc,$wall,$rss,NA,$rto,NA,NA,NO_RESULT" >> "$CSV"
    echo "NO_RESULT rc=$rc"
  else
    printf 'jobs=%-2s rto=%-5s rss=%-9s %ss\n' "$n" "$rto" "$rss" "$wall"
  fi
  tail -40 "$d/log.txt" > "$d/t" && mv "$d/t" "$d/log.txt"
  tail -40 "$d/err.txt" > "$d/t" && mv "$d/t" "$d/err.txt"
}

for v in roce dctcp; do
  echo "########## $v / scatter (3 seeds, as the original test) ##########"
  for seed in 13 17 23; do for mix in $ORDER; do run_one "$v" scatter "$mix" "$seed"; done; done
  echo "########## $v / compact (1 seed; policy ignores the seed) ##########"
  for mix in $ORDER; do run_one "$v" compact "$mix" 13; done
done

echo
echo "########## seed-independence check for compact ##########"
run_one roce compact mix4 17
awk -F, '$2=="compact" && $3=="mix4" {print "  seed "$4": "$5" -> nodes "$12}' "$CSV"

echo
column -s, -t "$CSV" | head -60
echo
awk -F, 'NR>1 && $13!="OK"{bad++} END{printf "%s non-OK job-row(s)\n", bad+0}' "$CSV"
