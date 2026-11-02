#!/bin/bash
set -uo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMLOCK="${SIMLOCK:-$HERE/../.simlock}"
DC="$HERE/../../FlexNetPacket/src/clos/datacenter"
RUNS="$HERE/${OUT_PREFIX:-}runs_ocs_mt"; CSV="$HERE/${OUT_PREFIX:-}ocs_mt_mdp.csv"
mkdir -p "$RUNS"

echo "series,arm,transport,placement,jobs,jobnodes,dims,exit,wall_s,rss_kb,links,diameter,reconfs,pfc_stop,pfc_resume,job0_ms,makespan_ms,job0_nodes,incomplete,verdict" > "$CSV"

run_one () {
  local series="$1" arm="$2" tr="$3" pol="$4" jobs="$5" jn="$6" tp="$7" dp="$8" pp="$9" ep="${10}"
  if [ -n "${RERUN_ONLY:-}" ] && [ "$tr:$arm" != "$RERUN_ONLY" ]; then return 0; fi
  local bin="$DC/htsim_tcp_mt_mdp_bench" queue_note=roce trx="${ROCE_EXTRA:-}"
  [ "$tr" = dctcp ] && { bin="$DC/dctcp_bin/htsim_tcp_mt_mdp_bench"; queue_note=dctcp; trx=""; }
  local extra="-logical $arm"
  [ "$arm" = dheu ] && extra=""
  local tag="${series}_${arm}_${tr}_${pol}_j${jobs}"
  local out="$RUNS/$tag.txt" log="$RUNS/$tag.log"

  printf '%-44s ' "$tag"
  flock "$SIMLOCK" /usr/bin/time -f "%e %M" -o "$RUNS/$tag.time" \
  timeout 3600 "$bin" -topo ocs $extra -deg 4 -rdelay 10 \
      -nodes 16 -jobs "$jobs" -jobnodes "$jn" \
      -jobtp "$tp" -jobdp "$dp" -jobpp "$pp" -jobep "$ep" -dims tp,dp,ep,pp \
      -speed 100000 -q 100 -ssthresh 10000 -simtime 60 \
      -routing updown -placement "$pol" -placementseed 13 $trx \
      -ofile "$out" > "$log" 2>&1
  local rc=$?
  local tl=$(tail -1 "$RUNS/$tag.time" 2>/dev/null)
  local wall=$(echo "$tl" | awk '{print ($1 ~ /^[0-9.]+$/) ? $1 : "NA"}')
  local rss=$(echo  "$tl" | awk '{print ($2 ~ /^[0-9]+$/)  ? $2 : "NA"}')
  gl () { grep -m1 "^$1 " "$log" 2>/dev/null | awk '{print $2}'; }
  gt () { grep -m1 "^$1 " "$out" 2>/dev/null | awk '{print $2}'; }
  local links=$(gl OCS_links) diam=$(gl OCS_diameter)
  local rec=$(grep -m1 '^DynSch_reconfs_started ' "$log" 2>/dev/null | awk '{print $2}')
  local stop=$(grep -c 'PAUSE 1000' "$log") resume=$(grep -c 'PAUSE 0' "$log")
  local j0=$(gt Job0_Makespan_ms) mk=$(gt Makespan_ms) inc=$(gt Incomplete)
  local j0n=$(grep -m1 '^Job0_Nodes ' "$out" 2>/dev/null | cut -d' ' -f2- | tr -d ' ' | md5sum | cut -c1-8)
  for v in links diam rec j0 mk inc; do eval "[ -z \"\$$v\" ] && $v=NA"; done

  local verdict=OK
  if   grep -qi "Assertion" "$log"; then verdict=ASSERT
  elif [ "$rc" -ne 0 ];             then verdict="EXIT$rc"
  elif [ "$mk" = NA ];              then verdict=NO_RESULT
  elif [ "$inc" != NA ] && [ "$inc" != 0 ]; then verdict=INCOMPLETE
  fi

  echo "$series,$arm,$tr,$pol,$jobs,$jn,tp$tp.dp$dp.pp$pp.ep$ep,$rc,$wall,$rss,$links,$diam,$rec,$stop,$resume,$j0,$mk,$j0n,$inc,$verdict" >> "$CSV"
  printf 'job0=%-10s makespan=%-10s inc=%-4s nodes=%s %ss %s\n' "$j0" "$mk" "$inc" "$j0n" "$wall" "$verdict"
}

ARMS="jellyfish flatbutterfly dragonfly megafly dheu"

echo "########## interference series: identical 4-node TP2/DP2 job, 1 -> 2 -> 4 tenants ##########"
for arm in $ARMS; do
  for tr in roce dctcp; do
    for pol in compact scatter; do
      for j in 1 2 4; do run_one tenancy "$arm" "$tr" "$pol" "$j" 4 2 2 1 1; done
    done
  done
done

echo
echo "########## full-MDP series: one 16-node job, all four dimensions populated ##########"
for arm in $ARMS; do
  for tr in roce dctcp; do run_one fullmdp "$arm" "$tr" compact 1 16 2 2 2 2; done
done

echo
column -s, -t "$CSV"
echo
awk -F, 'NR>1 && $20!="OK"{bad++} END{printf "%s non-OK run(s)\n", bad+0}' "$CSV"
