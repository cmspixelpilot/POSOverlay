import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style(light=True)
ROOT.gStyle.SetOptStat(111110)
ROOT.gStyle.SetOptFit(1111)

if len(sys.argv) < 3:
    print 'usage: bb3.py out_dir in_fn'
    sys.exit(1)

out_dir = sys.argv[1]
in_fn = sys.argv[2]

mkdir_p(out_dir)

t = trim_dat(in_fn)
for iroc, (roc, l) in enumerate(t.ls.iteritems()):
    if iroc > 10:
        break

    d = [[], []]
    means = [0., 0.]
    for i,e in enumerate(l):
        #assert type(e) != int
        if type(e) == int:
            continue
        col = i / 80
        j = col % 2
        d[j].append((e.th, e.sg))
        means[j] += e.th

    nbins = 50
    hs = []
    for j, x in enumerate(['even', 'odd']):
        hs.append({
                'raw':   ROOT.TH1F('h_%s_raw_%s'   % (x, roc), 'Evens on %s, threshold;VcThr units;pixels/0.8' % roc, nbins, 60, 100),
                'noise': ROOT.TH1F('h_%s_noise_%s' % (x, roc), 'Evens on %s, width;VcThr units;pixels/0.2'     % roc, nbins, 0, 10),
                'norm':  ROOT.TH1F('h_%s_norm_%s'  % (x, roc), 'Evens on %s, normed;VcThr units;pixels/0.16'    % roc, nbins, -10, 10),
                })

    for j in (0,1):
        means[j] /= len(d[j])
        for th, sg in d[j]:
            if th > 100:
                print th, sg
            hs[j]['raw'].Fill(th)
            hs[j]['noise'].Fill(sg)
            if sg > 1e-9:
                hs[j]['norm'].Fill((th - means[j]) / sg)
            else:
                hs[j]['norm'].Fill(1e9)

    c = ROOT.TCanvas('c_' + roc, '', 1920, 1000)
    c.Divide(3,2)
    for j in (0,1):
        for ix, x in enumerate(['raw', 'noise', 'norm']):
            c.cd(1+j*3+ix)
            h = hs[j][x]
            h.Draw()
            if x == 'norm':
                h.Fit('gaus', 'q')

    c.cd(0)
    c.SaveAs(os.path.join(out_dir, roc + '.png'))
