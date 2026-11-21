#!/bin/bash
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNS="$HERE/runs_repro"; CSV="$HERE/repro.csv"
mkdir -p "$RUNS"
MDP_LABEL_COL=variant
export CP_EXTRA=""
source "$HERE/lib_mdp_ocs.sh"
mdp_header
ROCE_EXTRA="" mdp_run F16d4 16 4 jellyfish roce compact 1 16 2 2 2 2 rhd base
ROCE_EXTRA="-rocedemand 1" mdp_run F27d6 27 6 dheu roce compact 1 27 27 1 1 1 ring base
ROCE_EXTRA="" mdp_run B 128 9 flatbutterfly roce compact 1 128 4 4 4 2 rhd ph8c0 -phased 8
ROCE_EXTRA="" mdp_run tenancy 128 9 megafly roce scatter 4 32 2 4 2 2 rhd base
ROCE_EXTRA="" mdp_run S3tenancy 128 9 dragonfly roce scatter 4 32 2 4 2 2 rhd seed3 -placementseed 3 -logicalseed 3
ROCE_EXTRA="" mdp_run tenancy 16 4 megafly roce scatter 4 4 2 2 1 1 rhd xoff8 -pfcdelayns 500 -pfcxoff 8 -pfcxon 5
ROCE_EXTRA="-rocedemand 1" mdp_run B 128 9 dragonfly roce compact 1 128 4 4 4 2 rhd f1b8c25 -iter1f1b 8 -layers 4 -dpbuckets 4 -layerfwdus 25 -layerbwdus 50
ROCE_EXTRA="-rocedemand 1" mdp_run S256 256 12 dheu roce compact 1 256 4 4 4 4 rhd demand
ROCE_EXTRA="" mdp_run B 432 16 jellyfish roce compact 1 432 8 6 9 1 auto base
echo "REPRO RUNS DONE"
