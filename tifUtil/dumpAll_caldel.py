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

dirs = ['FPix/FPix_%(hc)s/FPix_%(hc)s_D%(dsk)i/FPix_%(hc)s_D%(dsk)i_BLD%(bld)i/FPix_%(hc)s_D%(dsk)i_BLD%(bld)i_PNL%(pnl)i/FPix_%(hc)s_D%(dsk)i_BLD%(bld)i_PNL%(pnl)i_RNG%(rng)i' % locals() for hc in ['BmI', 'BmO', 'BpI', 'BpO'] for dsk in range(1,4) for bld in range(1,18) for pnl in range(1,3) for rng in range(1,3)]

c = ROOT.TCanvas('c', '', 1300, 1000)
c.Divide(4,4)
c.cd(0)
pdf_fn = os.path.join(out_dir, 'all.pdf')
c.Print(pdf_fn + '[')

for d in dirs:
    if not f.Get(d):
        continue
    for ikey, key in enumerate(f.Get(d).GetListOfKeys()):
        obj = key.ReadObj()
        name = obj.GetName().replace('_c', '')
        rest, roc = name.split('ROC')
        iroc = int(roc)
        if int(roc) < 10:
            name = rest + 'ROC0' + roc
        c.cd(iroc+1)
        canvas = obj.GetListOfPrimitives()[0]
        canvas.Draw('colz')
        if len(obj.GetListOfPrimitives()) > 1:
            for line in obj.GetListOfPrimitives()[1:]:
                line.Draw()
    c.cd(0)
    c.SaveAs(os.path.join(out_dir, d.split('/')[-1]) + '.png')
    c.Print(pdf_fn)
c.Print(pdf_fn + ']')

if 'html' in sys.argv:
    pngs = [f for f in os.listdir(out_dir) if os.path.isfile(os.path.join(out_dir,f)) and f.endswith('png')]
    pngs.sort()

    html_fn = os.path.join(out_dir, 'index.html')
    html = open(html_fn, 'wt')
    html.write('<html><body>\n')

    for item in pngs:
        html.write('<br><h1>%s</h1>\n' %item)
        html.write('<img src="%s">\n' %item)

    html.write('</body></html>\n')
    html.close()
    os.system('firefox %s' %html_fn)
else:
    os.system('evince %s' %pdf_fn)


if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_caldel/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
