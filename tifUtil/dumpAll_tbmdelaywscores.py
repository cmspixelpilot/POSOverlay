import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'TBMDelay.root')
if not os.path.isfile(in_fn):
    raise RuntimeError('no file at %s' % in_fn)
out_dir = os.path.join(run_dir, 'dump_tbmdelaywscores')
os.system('mkdir -p %s' % out_dir)

f = ROOT.TFile(in_fn)

# JMT need ROOT os.walk...
all_graphs = defaultdict(dict)

c = ROOT.TCanvas('c', '', 1300, 1000)
c.Divide(3,3)
c.cd(0)
pdf_fn = os.path.join(out_dir, 'all.pdf')
c.Print(pdf_fn + '[')

for ikey, key in enumerate(f.GetListOfKeys()):
    obj = key.ReadObj()
    c.cd(ikey % 9 + 1)
    obj.Draw()
    c.Update()
    if ikey % 9 == 8:
        c.cd(0)
        c.Print(pdf_fn)
c.cd(0)
c.Print(pdf_fn + ']')  
