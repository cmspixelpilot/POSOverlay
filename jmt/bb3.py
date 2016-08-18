import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
import moduleSummaryPlottingTools as FNAL
from write_other_hc_configs import doer, HC, module_sorter_by_portcard
set_style(light=True)
ROOT.gStyle.SetOptStat(111110)
ROOT.gStyle.SetOptFit(1111)

if len(sys.argv) < 3:
    print 'usage: bb3.py disk out_fn in_fn'
    sys.exit(1)

disk = int(sys.argv[1])
out_fn = sys.argv[2]
in_fn = sys.argv[3]

the_doer = doer(disk)
t = trim_dat(in_fn)

raw  = defaultdict(lambda: [None]*4160)
norm = defaultdict(lambda: [None]*4160)

c = ROOT.TCanvas('c', '', 1920, 1000)
c.Divide(3,2)
c.cd(0)
c.SaveAs(out_fn + '.roc_fits.pdf[')

for iroc, (roc, l) in enumerate(t.ls.iteritems()):
    print iroc, roc
    #if iroc % 10 != 0:
    #    continue

    d = [[], []]
    means = [0., 0.]
    mins = [1e99, 1e99]
    maxs = [-1e99, -1e99]
    for i,e in enumerate(l):
        #assert type(e) != int
        if type(e) == int:
            continue
        col = i / 80
        row = i % 80
        j = col % 2
        d[j].append((col, row, e.th, e.sg))
        means[j] += e.th
        mins[j] = min(e.th, mins[j])
        maxs[j] = max(e.th, maxs[j])

    for j in (0,1):
        mins[j] = max(mins[j], 0.)
        maxs[j] = min(maxs[j], 200.)

    n = [len(d[0]), len(d[1])]
    rmses = [0., 0.]
    ws = [0., 0.]
    ranges = [None, None]
    nbins = [0, 0]
    medians = [0.,0.]
    fit_ranges = [None, None]
    for j in (0,1):
        means[j] /= n[j]
        ths = [th for col, row, th, sg in d[j]]
        ths.sort()
        medians[j] = ths[n[j]/2]
        fit_ranges[j] = [ths[n[j]/20], ths[19*n[j]/20]]
        ws[j] = 0.5
        #for th, _ in d[j]:
        #    rmses[j] += (th - means[j])**2
        #rmses[j] = (rmses[j] / (n[j] - 1))**0.5
        ##ws[j] = 3.49 * rmses[j] / n[j]**(1./3)
        extra = (maxs[j] - mins[j]) * 0.1
        ranges[j] = [mins[j] - extra, maxs[j] + extra]
        nbins[j] = int(round((ranges[j][1] - ranges[j][0]) / ws[j]))
        #print 'roc %s j=%i mean %f rms %f w %f nbins %i range %s' % (roc, j, means[j], rmses[j], ws[j], nbins[j], ranges[j])

    hs = []
    for j, x in enumerate(['even', 'odd']):
        hs.append({
                'raw':   ROOT.TH1F('h_%s_raw_%s'   % (x, roc), '%s on %s, threshold;VcThr units;pixels/0.8' % (x.capitalize() + 's', roc), nbins[j], ranges[j][0], ranges[j][1]),
                'noise': ROOT.TH1F('h_%s_noise_%s' % (x, roc), '%s on %s, width;VcThr units;pixels/0.2'     % (x.capitalize() + 's', roc), 100,  0, 10),
                'norm':  ROOT.TH1F('h_%s_norm_%s'  % (x, roc), '%s on %s, normed2;VcThr units;pixels/0.16'  % (x.capitalize() + 's', roc), 100, -10, 10),
                })

    for j in (0,1):
        for col, row, th, sg in d[j]:
            hs[j]['raw'].Fill(th)
            hs[j]['noise'].Fill(sg)

    fits = [None, None]
    for j in (0,1):
        for ix, x in enumerate(['raw', 'noise']):
            c.cd(1+j*3+ix)
            h = hs[j][x]
            h.Draw()
            if x == 'raw':
                f = ROOT.TF1('f', 'gaus', fit_ranges[j][0], fit_ranges[j][1])
                res = h.Fit(f, 'QRS')
                assert fits[j] is None
                fits[j] = (res.Parameter(1), res.Parameter(2))

    for j,(mean,sigma) in enumerate(fits):
        h = hs[j]['norm']
        for col, row, th, sg in d[j]:
            raw [roc][col*80 + row] = th
            norm[roc][col*80 + row] = v = (th - mean)/sigma
            h.Fill(v)
        c.cd(1+j*3+2)
        #h.Fit('gaus', 'q')
        h.Draw()

    c.cd(0)
    c.SaveAs(out_fn + '.roc_fits.pdf')

c.SaveAs(out_fn + '.roc_fits.pdf]')
del c

assert set(raw.keys()) == set(norm.keys())

c = None  #ROOT.TCanvas('c', '', 1920, 1000)
this_out_fn = out_fn + '.module_maps.pdf'

bad_counts = defaultdict(int)

max_pcnum = 3 if disk == 1 else 4
for pcnum in xrange(1,max_pcnum+1):
    print pcnum
#    t = ROOT.TPaveText(0,0,1,1)
#    t.AddText(
#    c.cd(1)
#    t.Draw()
#    c.cd(0)
#    c.SaveAs(this_out_fn)

    modules = [m for m in sorted(the_doer.modules, key=module_sorter_by_portcard) if the_doer.moduleOK(m) and m.portcardnum == pcnum]

    for module in modules:
        print module.name
        for label, d in [('raw', raw), ('norm', norm), ('bad', norm)]:
            hs = []
            for i in xrange(16):
                roc = module.name + '_ROC' + str(i)
                hs.append(ROOT.TH2F('h_%s_%s' % (roc, label), roc + ' : ' + label, 52, 0, 52, 80, 0, 80))
                h.SetStats(0)

            any_ok = False
            for i in xrange(16):
                roc = module.name + '_ROC' + str(i)
                if not d.has_key(roc):
                    continue
                any_ok = True
                h = hs[i]
                l = d[roc]
                for i,x in enumerate(l):
                    col = i / 80
                    row = i % 80
                    if x is None:
                        if label == 'norm':
                            x = 6
                        elif label == 'bad':
                            x = 2
                    elif label == 'bad':
                        x = 1 if x>5 else 0
                        
                    if label == 'bad' and disk == 3 and pcnum in (1,2,4) and row in (58,59):
                        x = 0
                        
                    if label == 'bad' and x != 0:
                        bad_counts[roc] += 1

                    if x is not None:
                        h.SetBinContent(col+1, row+1, float(x))

            if any_ok:
                h = FNAL.makeMergedPlot(hs, 'pos')
                if label == 'raw':
                    FNAL.setZRange(h, (0,100)) #FNAL.findZRange(hs))
                elif label == 'norm':
                    FNAL.setZRange(h, (-5, 6.1)) #FNAL.findZRange(hs))

                fc = FNAL.setupSummaryCanvas(h, moduleName=module.name)
                #fc.SaveAs(module.name + '_' + label + '.pdf')
                if c is None:
                    c = fc
                    c.SaveAs(this_out_fn + '[')
                else:
                    c.cd()
                    c.Clear()
                    fc.DrawClonePad()

                pt = ROOT.TPaveText(-100,405,1395,432)
                pt.AddText(label + '   ' + m.portcard + ' ' + m.portcard_hj[1] + ' ' + str(m.portcard_connection) + '   ' + module.name + '   ' + module.module + '   ' + module.internal_name)
                pt.SetTextAlign(12)
                pt.SetTextFont(42)
                pt.SetFillColor(0)
                pt.SetBorderSize(0)
                pt.SetFillStyle(0)
                pt.Draw()

                #c.SaveAs(module.name + '_' + label + '.root')

                c.SaveAs(this_out_fn)
        
c.SaveAs(this_out_fn + ']')

print 'bad by roc:'
for roc, n in bad_counts.iteritems():
    module_name = roc.split('_ROC')[0]
    m = the_doer.modules_by_name[module_name]
    print n, module_name, 'D%i-%s' % (disk, 'outer' if m.rng == 2 else 'inner'), m.internal_name, m.module
