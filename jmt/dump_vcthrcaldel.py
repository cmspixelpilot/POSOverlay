import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'VcThrCalDel_1.root')
if not os.path.isfile(in_fn):
    raise RuntimeError('no file at %s' % in_fn)
out_dir = os.path.join(run_dir, 'dump_vcthrcaldel')
os.system('mkdir -p %s' % out_dir)

f = ROOT.TFile(in_fn)

dirs = [
    'Pilt/Pilt_BmO/Pilt_BmO_D3/Pilt_BmO_D3_BLD10/Pilt_BmO_D3_BLD10_PNL1/Pilt_BmO_D3_BLD10_PNL1_PLQ1',
    'Pilt/Pilt_BmO/Pilt_BmO_D3/Pilt_BmO_D3_BLD10/Pilt_BmO_D3_BLD10_PNL2/Pilt_BmO_D3_BLD10_PNL2_PLQ1',
    'Pilt/Pilt_BmO/Pilt_BmO_D3/Pilt_BmO_D3_BLD11/Pilt_BmO_D3_BLD11_PNL1/Pilt_BmO_D3_BLD11_PNL1_PLQ1',
    'Pilt/Pilt_BmO/Pilt_BmO_D3/Pilt_BmO_D3_BLD11/Pilt_BmO_D3_BLD11_PNL2/Pilt_BmO_D3_BLD11_PNL2_PLQ1',
    'Pilt/Pilt_BmI/Pilt_BmI_D3/Pilt_BmI_D3_BLD2/Pilt_BmI_D3_BLD2_PNL1/Pilt_BmI_D3_BLD2_PNL1_PLQ1',
    'Pilt/Pilt_BmI/Pilt_BmI_D3/Pilt_BmI_D3_BLD2/Pilt_BmI_D3_BLD2_PNL2/Pilt_BmI_D3_BLD2_PNL2_PLQ1',
    'Pilt/Pilt_BmI/Pilt_BmI_D3/Pilt_BmI_D3_BLD3/Pilt_BmI_D3_BLD3_PNL1/Pilt_BmI_D3_BLD3_PNL1_PLQ1',
    'Pilt/Pilt_BmI/Pilt_BmI_D3/Pilt_BmI_D3_BLD3/Pilt_BmI_D3_BLD3_PNL2/Pilt_BmI_D3_BLD3_PNL2_PLQ1',
    ]

by_ntrigs = []
first = True

c = ROOT.TCanvas('c', '', 1300, 1000)
c.Divide(4,4)
c.cd(0)

for d in dirs:
    if not f.Get(d):
        continue
    for ikey, key in enumerate(f.Get(d).GetListOfKeys()):
        canvas = key.ReadObj()
        name = canvas.GetName().replace(' (inv)', '').replace('_Canvas', '')
        obj = canvas.FindObject(name)
        rest, roc = name.split('ROC')
        iroc = int(roc)
        if int(roc) < 10:
            name = rest + 'ROC0' + roc
        ntrigs = int(obj.Integral())
        by_ntrigs.append((ntrigs, name))
        c.cd(iroc+1)
        obj.Draw('colz')
        if 0:
            for x in canvas.GetListOfPrimitives():
                if x.GetName() == 'TLine':
                    x.Draw()
    c.cd(0)
    c.SaveAs(os.path.join(out_dir, d.split('/')[-1]) + '.png')
    c.SaveAs(os.path.join(out_dir, d.split('/')[-1]) + '.root')

by_ntrigs.sort(key=lambda x: x[1])
by_ntrigs.sort(key=lambda x: x[0], reverse=True)
pprint(by_ntrigs)

if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_vcthrcaldel/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
