import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'CalDel_1.root')
if not os.path.isfile(in_fn):
    raise IOError('no root file %s' % in_fn)
out_dir = os.path.join(run_dir, 'dump_caldel')
os.system('mkdir -p %s' % out_dir)

f = ROOT.TFile(in_fn)

dirs = ['FPix/FPix_BmI/FPix_BmI_D1/FPix_BmI_D1_BLD%(bld)i/FPix_BmI_D1_BLD%(bld)i_PNL%(pnl)i/FPix_BmI_D1_BLD%(bld)i_PNL%(pnl)i_RNG%(rng)i' % locals() for bld in range(1,18) for pnl in range(1,3) for rng in range(1,3)]

c = ROOT.TCanvas('c', '', 1300, 1000)
c.Divide(4,4)
c.cd(0)
#pdf_fn = os.path.join(out_dir, 'all.pdf')
#c.Print(pdf_fn + '[')

for d in dirs:
    if not f.Get(d):
        continue
    for ikey, key in enumerate(f.Get(d).GetListOfKeys()):
        obj = key.ReadObj().GetListOfPrimitives()[0]
        name = obj.GetName().replace('_c', '')
        rest, roc = name.split('ROC')
        iroc = int(roc)
        if int(roc) < 10:
            name = rest + 'ROC0' + roc
        c.cd(iroc+1)
        obj.Draw('colz')
    c.cd(0)
    c.SaveAs(os.path.join(out_dir, d.split('/')[-1]) + '.png')
    #c.Print(pdf_fn)
#c.Print(pdf_fn + ']')

if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_caldel/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
