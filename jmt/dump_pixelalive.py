import sys, os
from pprint import pprint
from ROOTTools import *

in_fn = sys.argv[1]
run = in_fn.split('_')[-1].split('.')[0]
out_dir = '/uscms/home/tucker/asdf/plots/dump_pixelalive/Run%s' % run
out_dir = '/home/tucker/windows/dump_pixelalive/Run%s' % run
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

for d in dirs:
    c = ROOT.TCanvas('c'+d, '', 1300, 1000)
    c.Divide(4,4)
    for ikey, key in enumerate(f.Get(d).GetListOfKeys()):
        obj = key.ReadObj()
        name = obj.GetName().replace(' (inv)', '')
        rest, roc = name.split('ROC')
        if int(roc) < 10:
            name = rest + 'ROC0' + roc
        ntrigs = int(obj.Integral())
        by_ntrigs.append((ntrigs, name))
        c.cd(ikey+1)
        obj.Draw('colz')
    c.cd(0)
    c.SaveAs(os.path.join(out_dir, d.split('/')[-1] + '.gif'))

by_ntrigs.sort(key=lambda x: x[1])
by_ntrigs.sort(key=lambda x: x[0], reverse=True)
pprint(by_ntrigs)
