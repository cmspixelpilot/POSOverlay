#!/usr/bin/env python

from JMTTools import *
from JMTROOTTools import *
set_style()

fns = sys.argv[1:]
scp = 'scp' in fns
if scp:
    fns.remove('scp')
out_fn = 'trim_summary.root'
for fn in fns:
    if fn.endswith('.root'):
        out_fn = fn
        fns.remove(fn)
        break
if not fns:
    sys.exit('usage: trim_summary.py [scp] [name] fn1 .. fnN')

rf = ROOT.TFile(out_fn, 'recreate')
rf.mkdir('thresh')
rf.mkdir('noise')
rf.mkdir('chi2')
rf.mkdir('summary')

hs = {}

def book(name):
    if hs.has_key(name):
        return
    h = []
    rf.cd('thresh')
    h.append(ROOT.TH1F('h_thresh_' + name, '', 256, 0, 256))
    rf.cd('noise')
    h.append(ROOT.TH1F('h_noise_'  + name, '', 30, 0, 30))
    rf.cd('chi2')
    h.append(ROOT.TH1F('h_chi2_'   + name, '', 20, 0, 20))
    rf.cd()
    hs[name] = tuple(h)

def fill(name, tr):
    if not hs.has_key(name):
        book(name)
    h_thresh, h_noise, h_chi2 = hs[name]
    h_thresh.Fill(tr.thresh)
    h_noise.Fill(tr.noise)
    h_chi2.Fill(tr.chi2)

def roc_groups(roc):
    roc = roc.replace('Pilt_', '').replace('_D3', '').replace('PLQ1_', '')

    rb, rn = roc.split('ROC', 1)
    rn = int(rn)

    roc = rb + 'ROC%02i' % rn

    rg = rn / 4
    rg = rb + 'rocport%02i-%02i' % (rg*4, (rg+1)*4-1)

    rc = rn / 8
    rc = rb + 'tbmcore%02i-%02i' % (rc*8, (rc+1)*8-1)

    rm = rb + 'module'
    
    return roc, rg, rc, rm

def loop():
    for fn in fns:
        for iline, line in enumerate(open(fn)):
            line = line.strip()
            if not line:
                continue
            #if iline == 300:
            #    break

            tr = TrimResult(line)
            yield tr

# this dumb crap is to get it in the right order in the file
rocs = set()
for tr in loop():
    roc = roc_groups(tr.roc)[0]
    rocs.add(roc)
rocs = sorted(rocs)

rocs, ports, cores, modules = groups = [rocs, set(), set(), set()]
for roc in rocs:
    roc, port, core, mod = roc_groups(roc)
    ports.add(port)
    cores.add(core)
    modules.add(mod)
rocs, ports, cores, modules = groups = [sorted(g) for g in groups]

for g in groups:
    for x in g:
        book(x)

for tr in loop():
    for r in roc_groups(tr.roc):
        fill(r, tr)

rf.cd('summary')
hsums = []
for iwhich, which in enumerate(('thresh', 'noise', 'chi2')):
    for kind in ('nentries', 'overflows', 'aboveX', 'mean', 'median', 'rms', 'rmsinmeanpm10', 'rmsinmeanpm5'):
        if any(((kind == 'nentries' or kind == 'overflows') and which != 'thresh',
                which == 'chi2' and kind not in ('mean', 'rms'),
                )
               ):
            continue
                
        for gn, g in zip(('roc', 'port', 'core', 'module'), groups):
            hname = which + '_' + kind + '_by' + gn
            ng = len(g)
            hsum = ROOT.TH1F(hname, '', ng, 0, ng)
            hsum.SetStats(0)
            hsum.SetLineWidth(2)
            hsum.SetLineColor(4)
            hsums.append(hsum)
            xax = hsum.GetXaxis()
            for i, x in enumerate(g):
                h = hs[x][iwhich]
                ibin = i+1
                xax.SetBinLabel(ibin, x)
                if kind == 'nentries':
                    c = h.GetEntries()
                    e = 1 # c**0.5
                elif kind == 'overflows':
                    c = h.GetBinContent(h.GetNbinsX()+1)
                    e = 1 # c**0.5
                elif kind == 'aboveX':
                    X = 90 if which == 'thresh' else 7
                    c = h.Integral(h.FindBin(X), 100000)
                    e = 1 # c**0.5
                elif kind == 'mean':
                    c = h.GetMean()
                    e = h.GetMeanError()
                elif kind == 'median':
                    probsum  = array('d', [0.5])
                    median = array('d', [0.])
                    h.GetQuantiles(1, median, probsum)
                    c = median[0]
                    e = h.GetMeanError() # meh
                elif kind.startswith('rms'):
                    if 'meanpm' in kind:
                        h = h.Clone(h.GetName() + '_tmp')
                        h.SetDirectory(0)
                        m = h.GetMean()
                        pm = int(kind.split('meanpm')[1])
                        h.GetXaxis().SetRangeUser(m-pm, m+pm)
                    c = h.GetRMS()
                    e = h.GetRMSError()
                h.GetXaxis().UnZoom()

                hsum.SetBinContent(ibin, c)
                hsum.SetBinError  (ibin, e)

rf.Write()
rf.Close()

if scp:
    cmd = 'scp %s jmt46@lnx201.lns.cornell.edu:public_html/qwer/trim_summary/%s' % (out_fn, out_fn)
    print cmd
    os.system(cmd)
