import sys, os
from pprint import pprint
from JMTTools import *
from JMTROOTTools import *
set_style()

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'POHBias.root')
if not os.path.isfile(in_fn):
    raise RuntimeError('no file at %s' % in_fn)
out_dir = os.path.join(run_dir, 'dump_pohbias')
os.system('mkdir -p %s' % out_dir)

f = ROOT.TFile(in_fn)

gains = range(4)
feds = range(1294, 1298)

# JMT need ROOT os.walk...
all_graphs = defaultdict(dict)

for gainkey in f.GetListOfKeys():
    gainname = gainkey.GetName()
    gain = int(gainname.replace('gain', ''))
    for fedkey in f.Get(gainname).GetListOfKeys():
        fedname = fedkey.GetName()
        fed = int(fedname.replace('FED', ''))
        for graphkey in f.Get(gainname + '/' + fedname).GetListOfKeys():
            graphname = graphkey.GetName()
            fiber = int(graphname.split('_')[-1].replace('fiber', ''))
            graph = graphkey.ReadObj()
            all_graphs[(fed, fiber)][gain] = graph


for fedfiber, graphs in all_graphs.iteritems():
    c = ROOT.TCanvas('c', '', 1000, 800)
    leg = ROOT.TLegend()
    for gain in range(4):
        g = graphs[gain]
        g.SetTitle('FED %i fiber %i;' % fedfiber + g.GetTitle().split(';', 1)[-1])
        color = gain + 1
        g.SetLineColor(color)
        g.SetMarkerColor(color)
        g.GetYaxis().SetRangeUser(0, 0.5)
        #g.GetFunction("fit_to_rssi_response").Delete() #SetBit(ROOT.TF1.kNotDraw)
        if gain == 0:
            g.Draw('AP')
        else:
            g.Draw('P')
    c.SaveAs(os.path.join(out_dir, 'FED%i_fiber%i.png' % fedfiber))
    del c
