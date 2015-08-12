#!/usr/bin/env python

import sys, os
from collections import defaultdict
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

scp = 'scp' in sys.argv
if scp:
    sys.argv.remove('scp')

out_dir = sys.argv[1]
assert not os.path.isfile(out_dir)
os.mkdir(out_dir)
fns = sys.argv[2:]

zeros, ones, nums, avgs = defaultdict(int), defaultdict(int), defaultdict(int), defaultdict(float)
rocs = set()
roc = None

for fn in fns:
    for line in open(fn):
        line = line.strip()
        if not line:
            continue

        if line.startswith('ROC:'):
            roc = line.split()[1]
            rocs.add(roc)
        elif line.startswith('col'):
            trims = line.split()[1]

            for trim in trims:
                if trim == '0':
                    zeros[roc] += 1
                elif trim == '1':
                    ones[roc] += 1
                else:
                    nums[roc] += 1
                    avgs[roc] += int(trim, 16)

h_zeros = ROOT.TH1F('h_zeros', '', 4161, 0, 4161)
h_ones = ROOT.TH1F('h_ones', '', 4161, 0, 4161)
h_nums = ROOT.TH1F('h_nums', '', 4161, 0, 4161)
h_avgs = ROOT.TH1F('h_avgs', '', 16, 0, 16)

rocs = sorted(rocs)
print '# rocs:', len(rocs)
print 'counts:'
print 'roc'.ljust(50), 'zeros', ' ones', ' avgs'
for roc in rocs:
    if nums[roc]:
        avg = avgs[roc]/nums[roc]
        avg_s = '%5.1f' % avg
    else:
        avg = 0
        avg_s = '    -'
    h_zeros.Fill(zeros[roc])
    h_ones.Fill(ones[roc])
    h_nums.Fill(nums[roc])
    h_avgs.Fill(avg)
    print roc.ljust(50), str(zeros[roc]).rjust(5), str(ones[roc]).rjust(5), avg_s

c = ROOT.TCanvas('c', '', 1000, 800)
h_zeros.Draw()
c.SaveAs(os.path.join(out_dir, '0_zeros.png'))
c.SetLogy()
c.SaveAs(os.path.join(out_dir, '0_zeros_log.png'))
c.SetLogy(0)
h_ones.Draw()
c.SaveAs(os.path.join(out_dir, '1_ones.png'))
c.SetLogy()
c.SaveAs(os.path.join(out_dir, '1_ones_log.png'))
c.SetLogy(0)
h_nums.Draw()
c.SaveAs(os.path.join(out_dir, '2_nums.png'))
c.SetLogy()
c.SaveAs(os.path.join(out_dir, '2_nums_log.png'))
c.SetLogy(0)
h_avgs.Draw()
c.SaveAs(os.path.join(out_dir, '3_avgs.png'))
c.SetLogy()
c.SaveAs(os.path.join(out_dir, '3_avgs_log.png'))

if scp:
    remote_dir = 'public_html/qwer/count_trims/%s' % os.path.basename(out_dir)
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
