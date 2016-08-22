from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = glob.glob(os.path.join(run_dir, 'SCurve_Fed_*_Run_%i.root' % run))
if not in_fn:
    raise RuntimeError('need to make the root file: /nfshome0/pixelpilot/build/TriDAS/pixel/jmt/scurve.sh %i' % run)
if len(in_fn) > 1:
    raise RuntimeError('too many root files')

f = ROOT.TFile(in_fn)

c = ROOT.TCanvas('c', '', 1920, 1000)
c.Divide(4,2)

hists = [x.strip() for x in '''
MeanThreshold
MeanNoise
MeanChisquare
MeanProbability
RmsThreshold
RmsNoise
ThresholdOfAllPixels
NoiseOfAllPixels
'''.split('\n') if x.strip()]

for i,hn in enumerate(hists):
    h = f.Get('Summaries/%s' % hn)
    c.cd(i+1)
    h.Draw()
c.cd(0)
c.SaveAs(os.path.join(run_dir, 'scurve_simple.pdf')
    
if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_scurve_simple/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/scurve_simple.pdf jmt46@lnx201.lns.cornell.edu:%s' % (run_dir, remote_dir)
    print cmd
    os.system(cmd)
