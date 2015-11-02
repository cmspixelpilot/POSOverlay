import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'PixelAlive_Fed_40_Run_%i.root' % run)
if not os.path.isfile(in_fn):
    raise RuntimeError('need to make the root file: /nfshome0/pixelpilot/build/TriDAS/pixel/jmt/pxalive.sh %i' % run)
out_dir = os.path.join(run_dir, 'dump_pixelalive')
os.system('mkdir -p %s' % out_dir)

f = ROOT.TFile(in_fn)

dirs = [
    'Pilt/Pilt_BmI/Pilt_BmI_D3/Pilt_BmI_D3_BLD2/Pilt_BmI_D3_BLD2_PNL1/Pilt_BmI_D3_BLD2_PNL1_PLQ1',
    'Pilt/Pilt_BmI/Pilt_BmI_D3/Pilt_BmI_D3_BLD5/Pilt_BmI_D3_BLD5_PNL1/Pilt_BmI_D3_BLD5_PNL1_PLQ1',
    'Pilt/Pilt_BmI/Pilt_BmI_D3/Pilt_BmI_D3_BLD5/Pilt_BmI_D3_BLD5_PNL2/Pilt_BmI_D3_BLD5_PNL2_PLQ1',
    'Pilt/Pilt_BmI/Pilt_BmI_D3/Pilt_BmI_D3_BLD6/Pilt_BmI_D3_BLD6_PNL1/Pilt_BmI_D3_BLD6_PNL1_PLQ1',
    ]

by_ntrigs = []
first = True

c = ROOT.TCanvas('c', '', 1300, 1000)
c.Divide(4,4)
c.cd(0)
#pdf_fn = os.path.join(out_dir, 'all.pdf')
#c.Print(pdf_fn + '[')

num_dead = defaultdict(int)
eff_thresh = 100.

for d in dirs:
    if not f.Get(d):
        continue
    for ikey, key in enumerate(f.Get(d).GetListOfKeys()):
        obj = key.ReadObj()
        name = obj.GetName().replace(' (inv)', '')
        rest, roc = name.split('ROC')
        iroc = int(roc)
        if int(roc) < 10:
            name = rest + 'ROC0' + roc
        ntrigs = int(obj.Integral())
        by_ntrigs.append((ntrigs, name))
        for x in xrange(1, obj.GetNbinsX()+1):
            for y in xrange(1, obj.GetNbinsY()+1):
                if obj.GetBinContent(x,y) < eff_thresh:
                    num_dead[name] += 1
            
        c.cd(iroc+1)
        obj.Draw('colz')
    c.cd(0)
    c.SaveAs(os.path.join(out_dir, d.split('/')[-1]) + '.png')
    #c.Print(pdf_fn)
#c.Print(pdf_fn + ']')

by_ntrigs.sort(key=lambda x: x[1])
by_ntrigs.sort(key=lambda x: x[0], reverse=True)
pprint(by_ntrigs)

print '# dead pixels (eff lower than %f):' % eff_thresh
for roc in sorted(num_dead.keys()):
    if num_dead[roc]:
        print '%s %10i' % (roc.ljust(35), num_dead[roc])

if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_pixelalive/tif/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
