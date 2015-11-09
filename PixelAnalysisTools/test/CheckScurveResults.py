#######################################################
# AnalyzeVanaCalibration.py
# Original author: M. Malberti 12/02/2014
#######################################################                                                                                                                                                                   
import os
import sys
import ROOT
from ROOT import *
sys.path.append('../scripts/')
from browseCalibFiles import *

def GetData(d1, d2, d3):
    for obj in gDirectory.GetListOfKeys(): ## ROC folder: find the histogram for each ROC
        histo = obj.ReadObj()
        hname   = histo.GetName()
        if 'Threshold1D' in hname:
            mean  = histo.GetMean()
            rms   = histo.GetRMS()
            npix  = histo.GetEntries()
            roc   = hname.replace('_Threshold1D','')
            d1[roc] = float(mean)
            d2[roc] = float(rms)
            d3[roc] = int(npix)
    return d1, d2, d3



from optparse import OptionParser
parser = OptionParser()
parser.add_option("--input",dest="inputfile",type="string",default="ThresholdMinimization/BmI_T-20deg/Run_1825/SCurve_Fed_33-34_Run_1825.root",help="Name of the SCurve root file")
parser.add_option("--output",dest="output",type="string",default="BmO",help="Output directory")
(options,args)=parser.parse_args()


thresholds={}
thresholdsrms={}
npixels={}
browseROCChain([options.inputfile], GetData, thresholds, thresholdsrms, npixels)

hthresholds = TH1F("hthresholds","thresholds",70,0,140)
hthresholds.SetXTitle("Threshold [VCal]")
hthresholds.SetYTitle("")
hthresholds.SetLineColor(kBlue+1)

hthresholdsrms = TH1F("hthresholdsrms","thresholds rms",100,0,20)
hthresholdsrms.SetXTitle("Threshold RMS [VCal]")
hthresholdsrms.SetYTitle("")
hthresholdsrms.SetLineColor(kBlue+1)

hnpixels = TH1F("hnpixels","Number of pixels per ROC",20,70,90)
hnpixels.SetXTitle("Threshold [VCal]")
hnpixels.SetYTitle("")
hnpixels.SetLineColor(kBlue+1)

for roc in thresholds:
    hthresholds.Fill(thresholds[roc])
    hthresholdsrms.Fill(thresholdsrms[roc])
    hnpixels.Fill(npixels[roc])


print 'ROCs with outliers in threshold distribution:\n'
for roc in thresholds:
    if (thresholds[roc]<30 or thresholds[roc]>70):
        print roc,' thr = ', thresholds[roc]

print 'ROCs with outliers in RMS threshold distribution:\n'
for roc in thresholds:
    if (thresholdsrms[roc]>7):
        print roc,' thr rms = ', thresholdsrms[roc]

print 'ROCs with nPixels < 81:\n'
for roc in thresholds:
    if (npixels[roc]<81 ):
        print roc,' npixels = ', npixels[roc]


cthr = TCanvas("cthr","cthr",500,500)
hthresholds.Draw()

crms = TCanvas("crms","crms",500,500)
hthresholdsrms.Draw()

cn = TCanvas("cn","cn",500,500)
cn.SetLogy()
hnpixels.Draw()

try:
    os.mkdir(options.output)
except:
    print 'Cannot create output directory: directory already exists'
else:
    cthr.SaveAs(options.output+'/thresholds.png')
    crms.SaveAs(options.output+'/thresholdsRMS.png')
    cn.SaveAs(options.output+'/npixels.png')


raw_input('ok?')








