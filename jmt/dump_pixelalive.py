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

def CountDeadPixels (maxDeadPixels, outfile, excludedrocs):
    maxeff = 100

    for roc in gDirectory.GetListOfKeys(): ## ROC folder: find one TH2F for each ROC                                                                                                                    
        histo = roc.ReadObj()
        hname   = histo.GetName()
        xBins   = histo.GetNbinsX()
        yBins   = histo.GetNbinsY()

        # count dead pixels in each roc                                                                                                                                                                 
        numDeadPixels = 0
        for x in range(1,xBins+1):
            for y in range(1,yBins+1):
                if histo.GetBinContent(x,y) < maxeff:
                    numDeadPixels=numDeadPixels+1;
        if (numDeadPixels > maxDeadPixels):
            rocname = hname.replace(' (inv)','')
            print '%s - Number of dead pixels = %d' %(rocname,numDeadPixels)
            if (rocname not in excludedrocs):
                outfile.write('%s\n'%rocname)

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
        c.cd(iroc+1)
        obj.Draw('colz')
    c.cd(0)
    c.SaveAs(os.path.join(out_dir, d.split('/')[-1]) + '.png')
    #c.Print(pdf_fn)
#c.Print(pdf_fn + ']')

by_ntrigs.sort(key=lambda x: x[1])
by_ntrigs.sort(key=lambda x: x[0], reverse=True)
pprint(by_ntrigs)

if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_pixelalive/tif/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
