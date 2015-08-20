from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'SCurve_Fed_40_Run_%i.root' % run)
if not os.path.isfile(in_fn):
    raise RuntimeError('need to make the root file: /nfshome0/pixelpilot/build/TriDAS/pixel/jmt/scurve.sh %i' % run)
out_dir = os.path.join(run_dir, 'dump_scurve')
os.system('mkdir -p %s' % out_dir)

f = ROOT.TFile(in_fn)

dirs = [
    'NoisyCells',
    'ErrorCells',
    'GoodFits'
    ]

def new_canvas(n):
    global c
    del c
    c = ROOT.TCanvas(n, '', 1300, 1000)
    c.Divide(4,4)
    c.cd(0)
    return c

c = None

count = defaultdict(int)

for d in dirs:
    if not f.Get(d):
        continue
    for ikey, key in enumerate(f.Get(d).GetListOfKeys()):
        obj = key.ReadObj()
        ct = count[d] % 16
        if ct == 0:
            if c is not None:
                c.cd(0)
                c.SaveAs(os.path.join(out_dir, c.GetName()))
            n = '%s_%05i.png' % (d, count[d])
            c = new_canvas(n)
        c.cd(ct+1)
        count[d] += 1
        obj.SetStats(0)
        obj.Draw()
    c.cd(0)
    c.SaveAs(os.path.join(out_dir, c.GetName()))
    c = None

pprint(dict(count))

trim_file = glob(os.path.join(run_dir, 'Trim*dat'))
if trim_file:
    trim_file = trim_file[0]
    c = ROOT.TCanvas('c', '', 1000, 800)
    fs = []
    xs = []
    for line in open(trim_file):
        line = line.strip()
        if not line:
            continue
        line = line.split()
        fc = ROOT.TF1('f%i' % len(fs), '(0.5*(1+TMath::Erf((x-[0])/([1]*1.41421356))))', 0, 255)
        fc.SetLineWidth(1)
        fc.SetLineColor(ROOT.kRed)
        fs.append(f)
        x0, x1 = float(line[4]), float(line[5])
        xs.append((x0,x1))
        fc.SetParameters(x0, x1)
        if len(fs) == 1:
            fc.Draw()
        else:
            fc.Draw('same')
    c.SaveAs(os.path.join(out_dir, 'all_trim_fits.png'))

    h_0 = ROOT.TH1F('h_0', '', 256, 0, 256)
    h_1 = ROOT.TH1F('h_1', '', 256, 0, 256)
    for x0, x1 in xs:
        h_0.Fill(x0)
        h_1.Fill(x1)
    h_0.Draw()
    c.SaveAs(os.path.join(out_dir, 'all_trim_x0.png'))
    h_1.Draw()
    c.SaveAs(os.path.join(out_dir, 'all_trim_x1.png'))
    
if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_scurve/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
