import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'Iana.root')
if not os.path.isfile(in_fn):
    raise RuntimeError('no file at %s' % in_fn)
out_dir = os.path.join(run_dir, 'dump_iana')
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

for d in dirs:
    if not f.Get(d):
        continue
    for ikey, key in enumerate(f.Get(d).GetListOfKeys()):
        canvas = key.ReadObj()
        canvas.SaveAs(os.path.join(out_dir, canvas.GetName() + '.png'))

if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_iana/%i' % run
    cmds = [
        'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir,
        'scp -r %s/Iana.root %s/iana.dat %s/* jmt46@lnx201.lns.cornell.edu:%s' % (run_dir, run_dir, out_dir, remote_dir),
        ]
    for cmd in cmds:
        print cmd
        os.system(cmd)
