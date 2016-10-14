#######################################################
# Original author: M. Malberti 12/02/2014
#######################################################                                                                                                                                                                   
import os
import sys
import ROOT
from ROOT import *



runs = [1346, 1489]
labels = ['old','new']
hroc = {}
hpix = {}

for run in runs:
    print run
    f = TFile.Open('Run_%d/SCurve_Fed_37-38_Run_%d.root'%(run,run))
    hroc[run] = f.Get("Summaries/MeanThreshold")
    hpix[run] = f.Get("Summaries/ThresholdOfAllPixels")


leg = TLegend(0.12,0.69,0.32,0.89)
leg.SetFillColor(0)

croc = TCanvas("croc","croc",500,500)
i=0
for run in runs:
    hroc[run].SetLineWidth(2)
    hroc[run].SetLineColor(i+1)
    leg.AddEntry(hroc[run], labels[i],"L")
            
    if i==0:
        hroc[run].GetXaxis().SetRangeUser(0,100)
        hroc[run].GetXaxis().SetTitle("threshold (VCal)")
        hroc[run].Draw()
        croc.Update()
        hroc[run].FindObject("stats").SetLineColor(i+1)
    else:
        hroc[run].Draw("sames")
        croc.Update()
        hroc[run].FindObject("stats").SetLineColor(i+1)
        hroc[run].FindObject("stats").SetY1NDC(0.6)
        hroc[run].FindObject("stats").SetY2NDC(0.77)
    i=i+1 
leg.Draw("same")



cpix = TCanvas("cpix","cpix",500,500)
i=0
for run in runs:
    hpix[run].SetLineWidth(2)
    hpix[run].SetLineColor(i+1)
    if i==0:
        hpix[run].GetXaxis().SetRangeUser(0,100)
        hpix[run].GetXaxis().SetTitle("threshold (VCal)")
        hpix[run].Draw()
        cpix.Update()
        hpix[run].FindObject("stats").SetLineColor(i+1)
    else:
        hpix[run].Draw("sames")
        cpix.Update()
        hpix[run].FindObject("stats").SetLineColor(i+1)
        hpix[run].FindObject("stats").SetY1NDC(0.6)
        hpix[run].FindObject("stats").SetY2NDC(0.77)
    i=i+1
leg.Draw("same")



for type in 'png','pdf':
    croc.SaveAs('ComparisonMeanThresholds.%s'%type)
    cpix.SaveAs('ComparisonThresholdAllPixels.%s'%type)


raw_input('ok?')
