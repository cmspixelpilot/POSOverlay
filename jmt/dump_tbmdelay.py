import sys, os
from math import log10
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

in_fn = None
out_dir = None
for arg in sys.argv:
    if arg.endswith('.root') and os.path.isfile(arg):
        in_fn = arg
        out_dir = in_fn.replace('.root', '')
        break
if in_fn is None:
    run = run_from_argv()
    run_dir = run_dir(run)
    in_fn = os.path.join(run_dir, 'TBMDelay.root')
    if not os.path.isfile(in_fn):
        raise RuntimeError('no file at %s' % in_fn)
    out_dir = os.path.join(run_dir, 'dump_tbmdelay')
os.system('mkdir -p %s' % out_dir)

f = ROOT.TFile(in_fn)

c = ROOT.TCanvas('c', '', 500, 500)

keys = f.GetListOfKeys()

#fmt = '%0' + str(int(log10(len(keys))+1)) + 'i_%s.'
fmt = '%03i_%s.'
def sv(i, x, ty='png'):
    if hasattr(x, 'GetName'):
        x = x.GetName()
    c.SaveAs(os.path.join(out_dir, fmt % (i, x) + ty))

def unflatten_pll(h):
    assert type(h) == ROOT.TH1F
    n = h.GetName().split('_')[-1]
    h2 = ROOT.TH2F(h.GetName() + '_unflattened', '%s;400 MHz phase;160 MHz phase' % n, 8, 0, 8, 8, 0, 8)
    h2.SetStats(0)
    h2.SetMarkerSize(1.)
    xax = h.GetXaxis()
    for ix in xrange(1, xax.GetNbins()+1):
        v = xax.GetBinLowEdge(ix)
        assert abs(v - int(v)) < 1e-3
        v = int(v)
        p400 = (v >> 2) & 0x7
        p160 = (v >> 5) & 0x7
        h2.SetBinContent(h2.FindBin(p400, p160), h.GetBinContent(ix))
    return h2

#def unflatten_abdel(h):
#    assert type(h) == ROOT.TH2F
#    hs = [
#        ROOT.TH2F(h.GetName() + '_unflattened_TI0HT0', 'TI = 0, HT = 0;TBM A ROC delay;TBM B ROC delay', 64, 0, 64, 64, 0, 64),
#        ROOT.TH2F(h.GetName() + '_unflattened_TI0HT1', 'TI = 0, HT = 1;TBM A ROC delay;TBM B ROC delay', 64, 0, 64, 64, 0, 64),
#        ROOT.TH2F(h.GetName() + '_unflattened_TI1HT0', 'TI = 1, HT = 0;TBM A ROC delay;TBM B ROC delay', 64, 0, 64, 64, 0, 64),
#        ROOT.TH2F(h.GetName() + '_unflattened_TI1HT1', 'TI = 1, HT = 1;TBM A ROC delay;TBM B ROC delay', 64, 0, 64, 64, 0, 64),
#        ]
#    for h2 in hs:
#        h2.SetStats(0)
#
#    xax = h.GetXaxis()
#    yax = h.GetYaxis()
#    for ix in xrange(1, xax.GetNbins()+1):
#        vx = xax.GetBinLowEdge(ix)
#        for iy in xrange(1, yax.GetNbins()+1):
#            vy = yax.GetBinLowEdge(iy)
#    
#    return hs

def analyze_abdel(f, fifo, chip, tbmh_req, tbmt_req, roch_req, wpix_req, rpix_req, dang_req):
    s = ''
    if fifo == 1:
        s = 's'
        tbmh_req *= 2
        tbmt_req *= 2
        roch_req *= 2
        rpix_req *= 2
        
    htbmh = f.Get('TBMBDelay_v_TBMADelay_F%i%inTBMHeader%s'  % (fifo, chip, s))
    htbmt = f.Get('TBMBDelay_v_TBMADelay_F%i%inTBMTrailer%s' % (fifo, chip, s))
    hroch = f.Get('TBMBDelay_v_TBMADelay_F%i%inROCHeaders'   % (fifo, chip))
    hwpix = f.Get('TBMBDelay_v_TBMADelay_F%i%iwrongPix' % (fifo, chip))
    hrpix = f.Get('TBMBDelay_v_TBMADelay_F%i%irightPix' % (fifo, chip))
    hdang = f.Get('TBMBDelay_v_TBMADelay_F%i%idangling' % (fifo, chip)) if fifo == 2 else None
    if any(not h for h in (htbmh, htbmt, hroch, hwpix, hrpix)) or (fifo == 2 and hdang is None):
        pprint((htbmh, htbmt, hroch, hwpix, hrpix, hdang))
        return None

    xax, yax = htbmh.GetXaxis(), htbmh.GetYaxis()
    nbx, nby = xax.GetNbins(), yax.GetNbins()
    h = ROOT.TH2F('FIFO%i%iok_%i_%i_%i' % (fifo, chip, tbmh_req, tbmt_req, roch_req), '', nbx, xax.GetBinLowEdge(1), xax.GetBinLowEdge(nbx+1), nby, yax.GetBinLowEdge(1), yax.GetBinLowEdge(nby+1))
    h.SetStats(0)

    print '%5s %5s %5s %5s %5s %5s %5s %5s' % tuple('vx vy ntbmh ntbmt nroch nwpix nrpix ndang'.split())
    for ix in xrange(1, nbx+1):
        vx = xax.GetBinLowEdge(ix)
        for iy in xrange(1, nby+1):
            vy = yax.GetBinLowEdge(iy)

            ntbmh = htbmh.GetBinContent(ix, iy)
            ntbmt = htbmt.GetBinContent(ix, iy)
            nroch = hroch.GetBinContent(ix, iy)
            nwpix = hwpix.GetBinContent(ix, iy)
            nrpix = hrpix.GetBinContent(ix, iy)
            ndang = hdang.GetBinContent(ix, iy) if fifo == 2 else -1

            tbmh_ok = tbmh_req == -1 or htbmh.GetBinContent(ix, iy) == tbmh_req
            tbmt_ok = tbmt_req == -1 or htbmt.GetBinContent(ix, iy) == tbmt_req
            roch_ok = roch_req == -1 or hroch.GetBinContent(ix, iy) == roch_req
            wpix_ok = wpix_req == -1 or hwpix.GetBinContent(ix, iy) == wpix_req
            rpix_ok = rpix_req == -1 or hrpix.GetBinContent(ix, iy) == rpix_req
            dang_ok = fifo != 2 or dang_req == -1 or hdang.GetBinContent(ix, iy) == dang_req
            ok = tbmh_ok and tbmt_ok and roch_ok and wpix_ok and rpix_ok and dang_ok
            print '%5i %5i %5i %5i %5i %5i %5i %5i %s' % (vx, vy, ntbmh, ntbmt, nroch, nwpix, nrpix, ndang, ('OK' if ok else ''))
            if ok:
                h.Fill(vx, vy)

    return h

if 0:
    for ikey, key in enumerate(keys):
        obj = key.ReadObj()
        if issubclass(type(obj), ROOT.TH1):
            h = obj
            name = h.GetName()
            h.SetStats(0)
            if '_v_' in name:
                h.Draw('colz')
            else:
                h.Draw()
            sv(ikey, h)
            sv(ikey, h, 'root')

            if name.startswith('TBMPLL_') and '_v_' not in name:
                h2 = unflatten_pll(h)
                h2.Draw('colz text')
                sv(ikey, h2)

for fifo in (1,2):
    for chip in (1,):
        h = analyze_abdel(f, fifo, chip, tbmh_req=4, tbmt_req=4, roch_req=8*4, wpix_req=0, rpix_req=2*4*8, dang_req=0)
        if h is not None:
            h.Draw('colz')
            sv(999, h)
            sv(999, h, 'root')

if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_tbmdelay/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
