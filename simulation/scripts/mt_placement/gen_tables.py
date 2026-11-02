#!/usr/bin/env python3
"""
gen_tables.py - turn results.csv into tables for MT_PLACEMENT_RESULTS.md

Run: python3 -I gen_tables.py

RULES ENFORCED
  1. finalfinish_ps == 0 means the job never completed an iteration. For this
     executor that IS the stall criterion - first_iter_time is only set on a
     completed iteration - unlike the MDP benchmark, where a deadlocked run
     still reports a plausible makespan and an incomplete COUNT is needed.
     rc == 124 means the 3600 s timeout fired.
  2. A slowdown is formed only WITHIN one placement policy, against that
     policy's own alone-* baseline. A compact job and a scatter job occupy
     different nodes and get different bandwidth; they are not the same job.
  3. A ratio is only formed when the job under test held the same physical
     nodes in both runs (equal nodes_md5), else PLACEMENT MISMATCH is printed.
"""
import csv, os, collections

HERE = os.path.dirname(os.path.abspath(__file__))
TOPOS = ['fattree','zcubepartial','jellyfish','flatbutterfly','dragonfly','megafly']
DLRM, VGG = '100dlrm_16.fbuf', '100vgg_16.fbuf'

def load():
    with open(os.path.join(HERE,'results.csv')) as f:
        return list(csv.DictReader(f))

def ms(ps):
    try: return float(ps)/1e9
    except (TypeError, ValueError): return None

def dead(r):
    return r['finalfinish_ps'] in ('', '0') or r['rc'] == '124'

def main():
    rows = load()
    idx = {}
    for r in rows:
        idx[(r['transport'], r['topo'], r['placement'], r['mix'], r['job'])] = r

    out = []; W = out.append

    def ratio(tr, topo, pol, mix, job, alone):
        m = idx.get((tr,topo,pol,mix,job)); b = idx.get((tr,topo,pol,alone,job))
        if m is None or b is None: return '-'
        if dead(b): return 'base stall'
        if dead(m): return '**stall**'
        if m['nodes_md5'] != b['nodes_md5']: return '**MISMATCH**'
        a, v = ms(b['finalfinish_ps']), ms(m['finalfinish_ps'])
        if not a: return '-'
        return '%.1f &rarr; %.1f = **%.2fx**' % (a, v, v/a)

    for mix, alone, label in [('mix2', 'alone-dlrm', 'two jobs (DLRM + VGG)'),
                              ('mix4', 'alone-dlrm', 'four jobs (DLRM, VGG, Transformer, Candle)')]:
        W('### DLRM iteration time, alone vs %s (ms)\n' % label)
        W('| Topology | RoCE compact | RoCE scatter | DCTCP compact | DCTCP scatter |')
        W('| --- | --- | --- | --- | --- |')
        for topo in TOPOS:
            cells = [ratio(tr, topo, pol, mix, DLRM, alone)
                     for tr in ('roce','dctcp') for pol in ('compact','scatter')]
            W('| %s | %s |' % (topo, ' | '.join(cells)))
        W('')

    W('### Which placement finishes the DLRM job sooner in the 4-job mix (ms)\n')
    W('Absolute iteration time, lower is better. This is the operational question;')
    W('the tables above answer "what does contention cost within a policy?".\n')
    W('| Topology | Transport | compact | scatter | better |')
    W('| --- | --- | --- | --- | --- |')
    for topo in TOPOS:
        for tr in ('roce','dctcp'):
            c = idx.get((tr,topo,'compact','mix4',DLRM))
            s = idx.get((tr,topo,'scatter','mix4',DLRM))
            if c is None or s is None: continue
            cv = None if dead(c) else ms(c['finalfinish_ps'])
            sv = None if dead(s) else ms(s['finalfinish_ps'])
            cs = 'stall' if cv is None else '%.1f' % cv
            ss = 'stall' if sv is None else '%.1f' % sv
            if cv is None and sv is None: verd = 'both stall'
            elif sv is None: verd = '**compact** (scatter stalls)'
            elif cv is None: verd = '**scatter** (compact stalls)'
            else: verd = '**%s** by %.2fx' % (('compact','scatter')[sv<cv], max(cv,sv)/min(cv,sv))
            W('| %s | %s | %s | %s | %s |' % (topo, tr, cs, ss, verd))
    W('')

    W('### PFC pause frames in the 4-job mix (RoCE only)\n')
    W('PAUSE frames counted over the whole run. These are the mechanism behind the')
    W('timing differences: a pause stops a whole link and propagates backwards.\n')
    W('| Topology | compact | scatter |')
    W('| --- | --- | --- |')
    for topo in TOPOS:
        c = idx.get(('roce',topo,'compact','mix4',DLRM))
        s = idx.get(('roce',topo,'scatter','mix4',DLRM))
        if c is None or s is None: continue
        W('| %s | %s | %s |' % (topo, c['pause'], s['pause']))
    W('')

    W('### Stalls and timeouts\n')
    bad = [r for r in rows if dead(r)]
    if not bad:
        W('- No job failed to complete an iteration in any of the 96 runs.')
    else:
        W('| Transport | Topology | Placement | Mix | Job | rc |')
        W('| --- | --- | --- | --- | --- | --- |')
        for r in bad:
            W('| %s | %s | %s | %s | %s | %s |' %
              (r['transport'], r['topo'], r['placement'], r['mix'], r['job'], r['rc']))
    W('')
    per_run = {}
    for r in rows:
        k = (r['transport'], r['topo'], r['placement'], r['mix'])
        if r['wall_s']:
            per_run[k] = float(r['wall_s'])
    W('- %d job-rows from %d runs; sweep wall time %.0f minutes.'
      % (len(rows), len(per_run), sum(per_run.values())/60))

    txt = '\n'.join(out)
    open(os.path.join(HERE,'tables.md'),'w').write(txt+'\n')
    print(txt)

if __name__ == '__main__':
    main()
