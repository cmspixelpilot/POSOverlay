import sys, os, tempfile
from array import array

if os.environ.has_key('JMT_ROOTTOOLS_NOBATCHMODE'):
    import ROOT
else:
    sys.argv.append('-b')     # Start ROOT in batch mode;
    import ROOT; ROOT.TCanvas # make sure libGui gets initialized while '-b' is specified;
    sys.argv.remove('-b')     # and don't mess up sys.argv.

def set_style(date_pages=False):
    ROOT.gROOT.SetStyle('Plain')
    ROOT.gStyle.SetFillColor(0)
    if date_pages:
        ROOT.gStyle.SetOptDate()
    ROOT.gStyle.SetOptStat(1222222)
    ROOT.gStyle.SetOptFit(2222)
    ROOT.gStyle.SetPadTickX(1)
    ROOT.gStyle.SetPadTickY(1)
    ROOT.gStyle.SetMarkerSize(.1)
    ROOT.gStyle.SetMarkerStyle(8)
    ROOT.gStyle.SetGridStyle(3)
    ROOT.gStyle.SetPaperSize(ROOT.TStyle.kA4)
    ROOT.gStyle.SetStatW(0.25)
    ROOT.gStyle.SetStatFormat('6.4g')
    ROOT.gStyle.SetPalette(1)
    ROOT.gStyle.SetTitleFont(42, 'XYZ')
    ROOT.gStyle.SetLabelFont(42, 'XYZ')
    ROOT.gStyle.SetStatFont(42)
    ROOT.gStyle.SetLegendFont(42)
    ROOT.gErrorIgnoreLevel = 1001 # Suppress TCanvas::SaveAs messages.
