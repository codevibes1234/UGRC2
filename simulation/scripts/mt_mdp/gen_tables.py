#!/usr/bin/env python3
"""
gen_tables.py - turn results.csv into the tables for MT_MDP_RESULTS.md

Run with -I (the files it reads are this project's own output, but the habit is
cheap): python3 -I gen_tables.py

RULES THIS SCRIPT ENFORCES
  1. A job with incomplete > 0 STALLED. Its timings come from whatever finished
     before the deadlock and are meaningless, so they are never printed as
     numbers and never enter a ratio - they print as "stall".
  2. A ratio is only formed between a mix row and an ALONE row that gave job 0
     the same physical nodes (equal nodes_md5). Otherwise the ratio would
     measure placement luck instead of contention, and the script says so
     loudly rather than printing a number.
  3. rc=HARNESS rows are a harness failure, not a result, and are listed
     separately.
"""
import csv, os, collections

HERE = os.path.dirname(os.path.abspath(__file__))

def load():
    with open(os.path.join(HERE, 'results.csv')) as f:
        return list(csv.DictReader(f))

def fnum(s):
    try:    return float(s)
    except (TypeError, ValueError): return None

def stalled(r):
    inc = r.get('incomplete', '')
    return inc not in ('', '0')

def runkey(r):
    """Identifies one simulation run (all its jobs share this)."""
    return (r['transport'], r['topo'], r['routing'], r['placement'],
            r['jobs'], r['jobnodes'], r['decomp'], r['tag'])

STALLS = {}

def stallnote(r):
    """' (3/4 jobs stalled)' when a run lost jobs, else ''. A job-0 number from a
    run whose neighbours deadlocked is not a contention measurement, and printing
    the ratio without this note would be misleading."""
    n, tot = STALLS.get(runkey(r), (0, 0))
    return '' if n == 0 else ' (%d/%d jobs stalled)' % (n, tot)

def shape(r):
    return (r['transport'], r['topo'], r['routing'], r['placement'],
            r['jobnodes'], r['decomp'])

def main():
    rows = load()
    harness = [r for r in rows if r.get('rc') == 'HARNESS']
    rows = [r for r in rows if r.get('rc') != 'HARNESS']

    byrun = collections.defaultdict(list)
    for r in rows: byrun[runkey(r)].append(r)
    for k, rs in byrun.items():
        STALLS[k] = (sum(1 for r in rs if stalled(r)), len(rs))

    base = {}
    for r in rows:
        if r['jobs'] == '1' and r['job_idx'] == '0':
            base[shape(r)] = r

    out = []
    W = out.append

    W('### A. One 32-node job alone vs four sharing the fabric (compact placement)\n')
    W('Job 0 only. "stall" = the job never completed, so no timing is meaningful.\n')
    W('| Topology | Transport | alone (ms) | in 4-job mix (ms) | slowdown |')
    W('| --- | --- | --- | --- | --- |')
    for topo in ['fattree128','zcubepartial','jellyfish','flatbutterfly','dragonfly','megafly']:
        for tr in ['roce','dctcp']:
            mix = [r for r in rows if r['topo']==topo and r['transport']==tr
                   and r['routing']=='ecmp' and r['placement']=='compact'
                   and r['jobs']=='4' and r['jobnodes']=='32' and r['job_idx']=='0']
            if not mix: continue
            m = mix[0]; b = base.get(shape(m))
            W('| %s | %s | %s' % (topo, tr, cell_pair(b, m)))
    W('')

    W('### B. Tenancy scaling at fixed job size (32 nodes, compact)\n')
    W('Job 0 as the cluster fills. J=1 is 25% full, J=4 is 100% full. The job is')
    W('identical in every column; only the number of neighbours changes.\n')
    W('| Topology | Transport | J=1 | J=2 | J=3 | J=4 |')
    W('| --- | --- | --- | --- | --- | --- |')
    for topo in ['fattree128','jellyfish']:
        for tr in ['roce','dctcp']:
            cells = []
            for J in ['1','2','3','4']:
                rs = [r for r in rows if r['topo']==topo and r['transport']==tr
                      and r['routing']=='ecmp' and r['placement']=='compact'
                      and r['jobs']==J and r['jobnodes']=='32' and r['job_idx']=='0']
                cells.append((one(rs[0]) + stallnote(rs[0])) if rs else '-')
            W('| %s | %s | %s |' % (topo, tr, ' | '.join(cells)))
    W('')

    W('### C. Placement policy: compact vs scatter\n')
    W('The same four jobs, placed two ways. `scatter` is what')
    W('`FFApplication::choose_gpus` does and what paragraph 2 used; `compact`')
    W('gives each job a contiguous block.\n')
    W('| Topology | Transport | compact alone | compact mix4 | compact ratio | scatter alone | scatter mix4 | scatter ratio |')
    W('| --- | --- | --- | --- | --- | --- | --- | --- |')
    for topo in ['fattree128','zcubepartial','jellyfish','flatbutterfly','dragonfly','megafly']:
        for tr in ['roce','dctcp']:
            cs = []
            for pol in ['compact','scatter']:
                mix = [r for r in rows if r['topo']==topo and r['transport']==tr
                       and r['routing']=='ecmp' and r['placement']==pol
                       and r['jobs']=='4' and r['jobnodes']=='32' and r['job_idx']=='0']
                if not mix: cs.append('- | - | - |'); continue
                m = mix[0]; b = base.get(shape(m))
                cs.append(cell_pair(b, m))
            W('| %s | %s | %s' % (topo, tr, ' '.join(cs)))
    W('')

    W('### D. The cyclic families under deadlock-free routing (up*/down*, RoCE)\n')
    W('Compare against table C. Under plain ECMP all three deadlock when the jobs are')
    W('SCATTERED, but FlatButterfly and Dragonfly complete when the jobs are COMPACT -')
    W('so whether these topologies deadlock under PFC depends on how jobs are placed,')
    W('not on the topology alone. up*/down* removes the deadlock in every case here.\n')
    W('| Topology | Placement | alone (ms) | in 4-job mix (ms) | slowdown |')
    W('| --- | --- | --- | --- | --- |')
    for topo in ['jellyfish','flatbutterfly','dragonfly']:
        for pol in ['compact','scatter']:
            mix = [r for r in rows if r['topo']==topo and r['transport']=='roce'
                   and r['routing']=='updown' and r['placement']==pol
                   and r['jobs']=='4' and r['job_idx']=='0']
            if not mix: continue
            m = mix[0]; b = base.get(shape(m))
            W('| %s | %s | %s' % (topo, pol, cell_pair(b, m)))
    W('')

    W('### E. Is compact isolation about compact, or about pod alignment?\n')
    W('A 32-node compact job on Fat-Tree(8) occupies exactly two of eight pods')
    W('(16 hosts each), and every fat-tree link is incident to one pod. A 24-node')
    W('job straddles pods, so if alignment is what matters, isolation should break.\n')
    W('| Topology | Transport | job size | alone (ms) | in 4-job mix (ms) | slowdown |')
    W('| --- | --- | --- | --- | --- | --- |')
    for topo in ['fattree128','zcubepartial']:
        for tr in ['roce','dctcp']:
            for S, dec in [('32','2x4x2x2'), ('24','2x2x2x3')]:
                mix = [r for r in rows if r['topo']==topo and r['transport']==tr
                       and r['routing']=='ecmp' and r['placement']=='compact'
                       and r['jobs']=='4' and r['jobnodes']==S and r['job_idx']=='0']
                if not mix: continue
                m = mix[0]; b = base.get(shape(m))
                W('| %s | %s | %s nodes | %s' % (topo, tr, S, cell_pair(b, m)))
    W('')

    W('### G. Which policy should you actually use? (4-job mix, absolute ms)\n')
    W('Table C gives each policy its own baseline, which answers "how much does')
    W('contention cost?". This answers the operational question instead: given four')
    W('jobs to place, which placement finishes job 0 sooner? Lower is better.\n')
    W('| Topology | Transport | compact | scatter | better |')
    W('| --- | --- | --- | --- | --- |')
    for topo in ['fattree128','zcubepartial','jellyfish','flatbutterfly','dragonfly','megafly']:
        for tr in ['roce','dctcp']:
            got = {}
            for pol in ['compact','scatter']:
                rs = [r for r in rows if r['topo']==topo and r['transport']==tr
                      and r['routing']=='ecmp' and r['placement']==pol
                      and r['jobs']=='4' and r['jobnodes']=='32' and r['job_idx']=='0']
                got[pol] = rs[0] if rs else None
            c, sc = got['compact'], got['scatter']
            if c is None or sc is None: continue
            cv = None if stalled(c) else fnum(c['makespan_ms'])
            sv = None if stalled(sc) else fnum(sc['makespan_ms'])
            cs = 'stall' if cv is None else '%.3f%s' % (cv, stallnote(c))
            ss = 'stall' if sv is None else '%.3f%s' % (sv, stallnote(sc))
            if cv is None and sv is None: verdict = 'both stall'
            elif sv is None:              verdict = '**compact** (scatter deadlocks)'
            elif cv is None:              verdict = '**scatter** (compact deadlocks)'
            else: verdict = '**%s** by %.2fx' % (('compact','scatter')[sv<cv],
                                                 max(cv,sv)/min(cv,sv))
            W('| %s | %s | %s | %s | %s |' % (topo, tr, cs, ss, verdict))
    W('')

    W('### F. Which parallelism dimension absorbs the cross-job contention (RoCE, 4x32)\n')
    W('Per-dimension completion for job 0, alone vs in the mix. The alone run already')
    W('contains the job\'s own self-interference - all four dimensions run together in')
    W('both columns - so these ratios isolate the ADDITIONAL cost of neighbours, which')
    W('is exactly the question SCOPE.md set out to answer.\n')
    for pol in ['compact','scatter']:
        W('**%s placement**\n' % pol)
        W('| Topology | TP | DP | EP | PP |')
        W('| --- | --- | --- | --- | --- |')
        for topo in ['fattree128','zcubepartial','jellyfish','flatbutterfly','dragonfly','megafly']:
            mix = [r for r in rows if r['topo']==topo and r['transport']=='roce'
                   and r['routing']=='ecmp' and r['placement']==pol
                   and r['jobs']=='4' and r['jobnodes']=='32' and r['job_idx']=='0']
            if not mix: continue
            m = mix[0]; b = base.get(shape(m))
            if b is None: W('| %s | no baseline | | | |' % topo); continue
            if stalled(m) or stalled(b):
                W('| %s | stall | stall | stall | stall |' % topo); continue
            cells = []
            for dim in ['tp','dp','ep','pp']:
                a, mv = fnum(b[dim+'_ms']), fnum(m[dim+'_ms'])
                if a is None or mv is None or a == 0: cells.append('-')
                else: cells.append('%.3f&rarr;%.3f = **%.1fx**' % (a, mv, mv/a))
            W('| %s%s | %s |' % (topo, stallnote(m), ' | '.join(cells)))
        W('')

    W('### Validity checks\n')
    probs = []
    for r in rows:
        if r['jobs'] == '1' or r['job_idx'] != '0': continue
        b = base.get(shape(r))
        if b is None:
            probs.append('no alone baseline for %s' % (shape(r),))
        elif b['nodes_md5'] != r['nodes_md5']:
            probs.append('job 0 moved between alone and mix for %s' % (shape(r),))
    if harness:
        probs.append('%d harness-failure rows' % len(harness))
    if probs:
        for p in probs: W('- **%s**' % p)
    else:
        W('- Every mix row has an alone baseline that placed job 0 on the same')
        W('  physical nodes (matching `nodes_md5`), so every ratio above compares')
        W('  the same job with and without neighbours.')
        W('- No harness-failure rows.')
    W('')
    n_stall = sum(1 for r in rows if stalled(r))
    W('- %d of %d job-rows stalled (incomplete > 0).' % (n_stall, len(rows)))

    txt = '\n'.join(out)
    open(os.path.join(HERE, 'tables.md'), 'w').write(txt + '\n')
    print(txt)

def one(r):
    if stalled(r): return 'stall'
    v = fnum(r['makespan_ms'])
    return '%.3f' % v if v is not None else '-'

def cell_pair(b, m):
    """alone | mix | ratio, with stalls and placement mismatches handled."""
    if b is None:              return '- | %s | no baseline |' % one(m)
    if stalled(b):             return 'stall | %s | - |' % one(m)
    if stalled(m):             return '%s | **stall** | -%s |' % (one(b), stallnote(m))
    if b['nodes_md5'] != m['nodes_md5']:
        return '%s | %s | **PLACEMENT MISMATCH** |' % (one(b), one(m))
    a, v = fnum(b['makespan_ms']), fnum(m['makespan_ms'])
    if a is None or v is None or a == 0: return '%s | %s | -' % (one(b), one(m))
    return '%.3f | %.3f%s | **%.2fx** |' % (a, v, stallnote(m), v/a)

if __name__ == '__main__':
    main()
