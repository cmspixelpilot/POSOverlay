import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

scp = 'scp' in sys.argv
if scp:
    sys.argv.remove('scp')
fn = sys.argv[1]
assert '.txt' in fn
new_fn = os.path.basename(fn.replace('.txt', '.pdf'))

'''FEDID:40 phase stats:
ch  1/ 2: #locks: 21/24  mean  1.7 rms 1.7062
ch  3/ 4: #locks: 24/24  mean  2.0 rms 0.0000
ch  7/ 8: #locks: 24/24  mean  2.0 rms 0.0000
ch  9/10: #locks: 24/24  mean  6.0 rms 0.0000
ch 25/26: #locks: 24/24  mean  6.8 rms 0.4149
ch 27/28: #locks: 24/24  mean  2.0 rms 0.0000
ch 31/32: #locks: 24/24  mean  0.1 rms 0.2823
ch 33/34: #locks: 24/24  mean  5.0 rms 0.0000'''

colors = {
    '1-2': 1,
    '3-4': 2,
    '7-8': 3,
    '9-10': 4, 
    '25-26': 6,
    '27-28': 8,
    '31-32': 9,
    '33-34': 40,
}
chs =  [
    '1-2',
    '3-4',
    '7-8',
    '9-10',
    '25-26',
    '27-28',
    '31-32',
    '33-34',
    ]
vs = {}

for line in open(fn):
    line = line.strip().replace('/ ', '/').split()
    if not line or not line[0] == 'ch':
        continue
    ch = line[1].replace('/', '-').replace(':', '')
    nlocks, means = vs.setdefault(ch, ([],[]))
    nlocks.append(int(line[3].split('/')[0]))
    rms = float(line[7])
    if rms == 0:
        rms = 0.001
    means.append((float(line[5]), rms))

hss = [[],[]]
for ch in chs:
    v = vs[ch]
    for l,n,hs in zip(v, 'nlocks means'.split(), hss):
        nbins = len(l)
        h = ROOT.TH1F('h_%s_%s' % (n, ch), '', nbins, 0, nbins)
        if n == 'nlocks':
            h.SetTitle('channel %s;sample period;# locks/24' % ch)
        else:
            h.SetTitle('channel %s;sample period;profile of phases over 24 samples' % ch)
        h.SetStats(0)
        h.SetLineWidth(2)
        #h.SetLineColor(colors[ch])
        hs.append(h)
        for ibin in xrange(1, nbins+1):
            if n == 'nlocks':
                h.SetBinContent(ibin, l[ibin-1])
            else:
                h.SetBinContent(ibin, l[ibin-1][0])
                h.SetBinError  (ibin, l[ibin-1][1])

c = ROOT.TCanvas('c', '', 800, 1600)
c.Divide(1,2)
c.cd(0)
c.Print(new_fn + '[')

nh = len(hss[0])
for i in xrange(nh):
    for j in xrange(2):
        c.cd(j+1)
        h = hss[j][i]
        if j == 0:
            h.Draw()
            h.GetYaxis().SetRangeUser(0, 25)
        elif j == 1:
            h.Draw('e0')
            h.GetYaxis().SetRangeUser(-5, 15)
    c.cd(0)
    c.Print(new_fn)
c.Print(new_fn + ']')

if scp:
    cmd = 'scp %s jmt46@lnx201.lns.cornell.edu:public_html/qwer/' % new_fn
    print cmd
    os.system(cmd)
