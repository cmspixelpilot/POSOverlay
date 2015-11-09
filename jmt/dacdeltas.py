from JMTTools import *
from JMTROOTTools import *

dir1, dir2 = sys.argv[1:3]

assert os.path.isdir(dir1) and os.path.isdir(dir2)

fns1 = sorted(glob(os.path.join(dir1, 'ROC_DAC*dat')))
fns2 = sorted(glob(os.path.join(dir2, 'ROC_DAC*dat')))

assert [os.path.basename(fn) for fn in fns1] == [os.path.basename(fn) for fn in fns2]

hists = {}
order = []
order_done = False

for ifn, (fn1, fn2) in enumerate(izip(fns1, fns2)):
    lines1 = [x.strip() for x in open(fn1).read().split('\n') if x.strip()]
    lines2 = [x.strip() for x in open(fn2).read().split('\n') if x.strip()]

    assert len(lines1) == len(lines2)

    for line1, line2 in izip(lines1, lines2):
        if line1.startswith('ROC:'):
            assert line1 == line2
            if order:
                order_done = True
        else:
            dac1, val1 = line1.split()
            dac2, val2 = line2.split()
            assert dac1 == dac2
            dac = dac1.replace(':', '')
            if not order_done:
                order.append(dac)

            if not hists.has_key(dac):
                hists[dac] = (ROOT.TH2F('h2_' + dac, ';%s;%s' % (dir1, dir2), 256, 0, 256, 256, 0, 256),
                              ROOT.TH1F('h_'  + dac, ';%s - %s' % (dir2, dir1), 511, -255, 256))
            h2, h = hists[dac]
            h2.Fill(float(val1), float(val2))
            h.Fill(float(val2) - float(val1))

c = ROOT.TCanvas('c', '', 800, 800)
c.SaveAs('dacdeltas.pdf[')
for i in xrange(2):
    c.SetLogy(i == 0)
    for d in order:
        h2, h = hists[d]
        if i == 0:
            h.Draw()
        else:
            h2.Draw('colz')
        c.SaveAs('dacdeltas.pdf')
c.SaveAs('dacdeltas.pdf]')
    
