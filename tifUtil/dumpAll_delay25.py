import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'delay25_1.root')
if not os.path.isfile(in_fn):
    raise IOError('no root file %s' % in_fn)
out_dir = os.path.join(run_dir, 'dump_delay25')
os.system('mkdir -p %s' % out_dir)

f = ROOT.TFile(in_fn)

dirs = ['FPix/FPix_%(hc)s/FPix_%(hc)s_D%(dsk)i' % locals() for hc in ['BmI', 'BmO', 'BpI', 'BpO'] for dsk in range(1,4)]

c = ROOT.TCanvas('c', '', 1300, 1000)
c.Divide(3,2)
c.cd(0)
pdf_fn = os.path.join(out_dir, 'all.pdf')
c.Print(pdf_fn + '[')

for d in dirs:
    if not f.Get(d):
        continue
    for ikey, key in enumerate(f.Get(d).GetListOfKeys()):
        obj = key.ReadObj()#.GetListOfPrimitives()[0]
        name = obj.GetName().replace(';1', '')
        pc = name.split(' ')[5]
        md = name.split(' ')[8]
        num = 0
        if 'command' in md:
            num = int(md.split('command')[-1])
        c.cd(num+1)
        canvas = obj.GetListOfPrimitives()[0]
        canvas.Draw()
        if len(obj.GetListOfPrimitives()) > 1:
            for x in obj.GetListOfPrimitives()[1:]:
                x.Draw()
        if num == 5:
            c.cd(0)
            c.Print(pdf_fn)
c.Print(pdf_fn + ']')

os.system('evince %s' %pdf_fn)
