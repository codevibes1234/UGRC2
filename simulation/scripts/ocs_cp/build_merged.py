import csv, os

HERE = os.path.dirname(os.path.abspath(__file__))
OCS = os.path.join(HERE, "..", "ocs")
OUT = os.path.join(HERE, "..", "ocs_merged")
PRE_FIX = ["ocs_mt_mdp.csv", "pfc_realism_mdp.csv", "decomp.csv", "phased.csv", "seeds_mdp.csv", "ocs_scale.csv"]
POST_FIX = ["ocs_432.csv", "dheu_scale.csv", "framework.csv"]
KEYS = ["series", "nodes", "deg", "arm", "transport", "placement", "jobs", "jobnodes", "decomp", "dims",
        "allreduce", "variant", "pfc_variant"]


def key(r):
    return tuple(r[k] for k in KEYS if k in r)


def load(path):
    with open(path) as f:
        rd = csv.DictReader(f)
        return rd.fieldnames, list(rd)


def write(path, fields, rows):
    with open(path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def merge(name, keep_blind):
    fields, old = load(os.path.join(OCS, name))
    nfields, new = load(os.path.join(HERE, name))
    assert nfields == fields, name
    nk = {key(r): r for r in new}
    used, out, kept = set(), [], 0
    for r in old:
        k = key(r)
        replace = r["transport"] == "roce" and not (keep_blind and r["arm"] == "dheu")
        if replace and k in nk:
            out.append(nk[k])
            used.add(k)
        else:
            out.append(r)
            kept += replace
    write(os.path.join(OUT, name), fields, out)
    print(f"{name:24s} replaced {len(used):4d}  lossless rows left as stored {kept:3d}  new rows unmatched {len(nk) - len(used)}")


def replace_rd(name):
    fields, old = load(os.path.join(OCS, name))
    nfields, new = load(os.path.join(HERE, name))
    assert nfields == fields, name
    ok = sorted(map(key, old)) == sorted(map(key, new))
    write(os.path.join(OUT, name), fields, new)
    print(f"{name:24s} rows {len(new):4d}  same cells as stored: {ok}")


def main():
    os.makedirs(OUT, exist_ok=True)
    for n in PRE_FIX:
        merge(n, True)
    for n in POST_FIX:
        merge(n, False)
    for n in PRE_FIX:
        replace_rd("rd_" + n)
    done = set(os.listdir(OUT))
    for n in os.listdir(OCS):
        if n not in done and (n.endswith(".csv") or n.startswith("runs_pfc_classes")):
            os.symlink(os.path.abspath(os.path.join(OCS, n)), os.path.join(OUT, n))


main()
