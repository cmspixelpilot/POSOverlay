from JMTTools import *

try:
    run = run_from_argv()
except ValueError:
    run = None

if run is not None:
    calib = calib_dat(run)
    calib_rocs = calib.rocs
    pixels = calib.pixels

    detconfig = detconfig_dat(run)
    rocs_qual = detconfig.rocs['qual']
    rocs_noqual = detconfig.rocs['noqual']

    trim_fn = glob(run_fn(run, 'Trim*dat'))[0]
else:
    calib_rocs = ['all']
    pixels = [(r,c) for r in xrange(80) for c in range(52)]
    rocs_qual = []
    rocs_noqual =  ['Pilt_BmO_D3_BLD%i_PNL%i_PLQ1_ROC%i' % (bld, pnl, roc) for bld in (10,11) for pnl in (1,2) for roc in range(16) if (bld,pnl) != (11,1)]
    rocs_noqual += ['Pilt_BmI_D3_BLD%i_PNL%i_PLQ1_ROC%i' % (bld, pnl, roc) for bld in   (2,3) for pnl in (1,2) for roc in range(16)]
    trim_fn = sys.argv[1]

print 'run:', run

trims = [TrimResult(x.strip()) for x in open(trim_fn).read().split('\n') if x.strip()]
trims_by_roc = defaultdict(list)
trims_by_roc_px = defaultdict(list)
for t in trims:
    trims_by_roc[t.roc].append(t)
    trims_by_roc_px[(t.roc, t.row, t.col)].append(t)
assert all(len(v) == 1 for v in trims_by_roc_px.itervalues())

assert calib_rocs == ['all']
for roc, quals in rocs_qual:
    assert quals == ('noAnalogSignal',)

print '# trims:', len(trims)
print '# rocs:', len(rocs_noqual)
print '# pix:', len(pixels)
should = len(rocs_noqual) * len(pixels)
print '-> should have %i trims, missing %i' % (should, should-len(trims))

no_trim = defaultdict(list)
for roc in rocs_noqual:
    for r,c in pixels:
        if not trims_by_roc_px.has_key((roc, r, c)):
            no_trim[roc].append((r,c))
if no_trim:
    print 'no trim for:'
    for roc in sorted(no_trim.iterkeys()):
        print roc.ljust(40), '# pix:', len(no_trim[roc])
print 'sum:', sum(len(v) for v in no_trim.itervalues())
