from JMTTools import *

run = run_from_argv()
calib = calib_dat(run)
detconfig = detconfig_dat(run)
trim_fn = glob(run_fn(run, 'Trim*dat'))[0]

print 'run:', run

class TrimResult:
    def __init__(self, s):
        s = s.split()
        assert len(s) == 9
        dummy, self.roc, r, c, noise, thresh, istat, chi2, prob = s
        self.row = int(r)
        self.col = int(c)
        self.noise = float(noise)
        self.thresh = float(thresh)
        self.istat = int(istat)
        self.chi2 = float(chi2)
        self.prob = float(prob)

trims = [TrimResult(x.strip()) for x in open(trim_fn).read().split('\n') if x.strip()]
trims_by_roc = defaultdict(list)
trims_by_roc_px = defaultdict(list)
for t in trims:
    trims_by_roc[t.roc].append(t)
    trims_by_roc_px[(t.roc, t.row, t.col)].append(t)
assert all(len(v) == 1 for v in trims_by_roc_px.itervalues())

assert calib.rocs == ['all']
for roc, quals in detconfig.rocs['qual']:
    assert quals == ('noAnalogSignal',)

print '# trims:', len(trims)
print '# rocs:', len(detconfig.rocs['noqual'])
print '# pix:', len(calib.pixels)
should = len(detconfig.rocs['noqual']) * len(calib.pixels)
print '-> should have %i trims, missing %i' % (should, should-len(trims))

no_trim = defaultdict(list)
for roc in detconfig.rocs['noqual']:
    for r,c in calib.pixels:
        if not trims_by_roc_px.has_key((roc, r, c)):
            no_trim[roc].append((r,c))
if no_trim:
    print 'no trim for:'
    for roc in sorted(no_trim.iterkeys()):
        print roc.ljust(40), '# pix:', len(no_trim[roc])
print 'sum:', sum(len(v) for v in no_trim.itervalues())
        
    
