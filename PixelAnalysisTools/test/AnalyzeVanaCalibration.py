#######################################################
# AnalyzeVanaCalibration.py
# Original author: M. Malberti 02/12/2013
# This script takes as input the log files from the Vana
# calibration iterations and produces some control plots
# vs number of iteration:
# - DeltaVana distribution
# - DeltaVana vs numer of iteration
# - Difference between in-time and absolute threshold
# - absolute thresholds distribution
#######################################################
import os
import sys
import ROOT
from ROOT import *

def fillDict(filename):
    file = open(filename)
    vanalist=[line.replace('\n','').split('deltaVana_corr= ') for line in file if 'Vana_corr' in line]
    vanadict= {el[0]:[float(el[1])] for el in vanalist } # for Python2.7
    #vanadict=dict( (el[0],[float(el[1])]) for el in vanalist )# for Python2.4
    file.seek(0)
    inthrlist=[line.replace('\n','').replace('(ben) ROC: ','').split(' avgThr: ') for line in file if 'avgThr:' in line]
    inthrdict= {el[0]:[float(el[1])] for el in inthrlist } # for Python2.7
    #inthrdict= dict( (el[0],[float(el[1])]) for el in inthrlist ) # for Python2.4
    file.seek(0)
    absthrlist=[line.replace('\n','').replace('(ben) ROC: ','').split(' avgThr_abs: ') for line in file if 'avgThr_abs:' in line] 
    absthrdict= {el[0]:[float(el[1])] for el in absthrlist }
    #absthrdict= dict( (el[0],[float(el[1])]) for el in absthrlist ) # for Python2.4
    return vanadict, inthrdict, absthrdict

def PlotHistogram(canvas, histogram, index1, index2, xtitle, legend):
    ymax = max(histogram[index2].GetMaximum(),histogram[index1].GetMaximum())
    histogram[index2].GetYaxis().SetRangeUser(0.0001,ymax*1.1)
    histogram[index2].SetXTitle(xtitle)
    histogram[index2].SetYTitle("number of ROCs")
    histogram[index2].SetLineColor(ROOT.kBlue)
    histogram[index2].Draw()
    histogram[index1].SetLineColor(ROOT.kRed)
    histogram[index1].Draw("sames")
    canvas.Update()
    stat1 = histogram[index1].FindObject("stats")
    stat1.SetY1NDC(0.81)
    stat1.SetY2NDC(0.68)
    stat1.SetTextColor(kRed)
    stat2 = histogram[index2].FindObject("stats")
    stat2.SetTextColor(kBlue)
    histogram[index1].Draw("sames")
    canvas.Update()
    legend.Clear()
    legend.AddEntry(histogram[index1],"first iteration","F")
    legend.AddEntry(histogram[index2],"last iteration","F")
    legend.Draw("same")    

def SavePlot(canvas, name):
    canvas.cd()
    canvas.SaveAs(name)


from optparse import OptionParser
parser = OptionParser()
parser.add_option("-i","--input",dest="inputfile",type="string",default="iteration",help="Name of the input files")
parser.add_option("-m","--minIteration",dest="minIteration",type="int",default=1,help="First iteration")
parser.add_option("-M","--maxIteration",dest="maxIteration",type="int",default=10,help="Last iteration")
(options,args)=parser.parse_args()

ROOT.gStyle.SetOptStat(1110)
ROOT.gStyle.SetOptTitle(0)

miniter=int(options.minIteration)
maxiter=int(options.maxIteration) 

# load first iteration results
vanacorr,inthr, absthr = fillDict('%s%d.log'%(options.inputfile,miniter))

# read next iterations and append vanacorr, inthr, absthr
for iteration in range(miniter+1,maxiter+1):
    vanacorr_i,inthr_i, absthr_i = fillDict('%s%d.log'%(options.inputfile,iteration)) 
    for roc in vanacorr:
        vanacorr[roc].append(vanacorr_i[roc][0])
        inthr[roc].append(inthr_i[roc][0])
        absthr[roc].append(absthr_i[roc][0])

# plot results
hvanacorr=[]
h2vanacorr=[]
hdt=[]
habs=[]
hvanacorr_vs_iter = {}

for iter in range(miniter,maxiter+1):
    # fill distribution of deltaVana at this iteration 
    htemp = TH1F("hvanacorr_%d"%iter,"hvanacorr_%d"%iter,200,-10,10)
    hvanacorr.append(htemp)
    htempdt = TH1F("hdt_%d"%iter,"hdt_%d"%iter,100,0,50)
    hdt.append(htempdt)
    htempabs = TH1F("habsthr_%d"%iter,"habsthr_%d"%iter,400,0,200)
    habs.append(htempabs)
    for roc in vanacorr:
        #print roc, iter, vanacorr[roc][iter]
        hvanacorr[iter-miniter].Fill(vanacorr[roc][iter-miniter])
        hdt[iter-miniter].Fill(inthr[roc][iter-miniter]-absthr[roc][iter-miniter])
        habs[iter-miniter].Fill(absthr[roc][iter-miniter])
        #habs[iter-miniter].Fill(inthr[roc][iter-miniter])

# fill deltaVana vs number of iteration
col=1
k=0
j=0
l=0
for roc in vanacorr:
    hvanacorr_vs_iter[roc] = TH1F("hvanacorr_vs_iter_%s"%roc,"hvanacorr_vs_iter_%s"%roc,21,-0.5,20.5)
    if 'D1_BLD6_PNL1' in roc:
        col = ROOT.kRed + k%3
        k = k+1
    elif 'D1_BLD11_PNL2' in roc:
        col = ROOT.kGreen + j%3
        j = j+1
    else:
        col = ROOT.kCyan + l%3
        l = l+1

    hvanacorr_vs_iter[roc].SetLineColor(col)
#    if (vanacorr[roc][1] > 5):
#    if ('D1_BLD6' in roc ):
#    if ('D1_BLD11' in roc):
    for iter in range(miniter,maxiter+1):
        hvanacorr_vs_iter[roc].Fill(iter, vanacorr[roc][iter-miniter])


legend = TLegend(0.60,0.50,0.80,0.70)
legend.SetFillStyle(0)
legend.SetBorderSize(0)

cVanaCorr=TCanvas("cVanaCorr","cVanaCorr")
PlotHistogram(cVanaCorr, hvanacorr, 0, maxiter-miniter, "#DeltaVana",legend)

cDT=TCanvas("cDT","cDT")
PlotHistogram(cDT, hdt, 0, maxiter-miniter, "DT (VCal)",legend)

cAbsThr=TCanvas("cAbsThr","cAbsThr")
PlotHistogram(cAbsThr, habs, 0, maxiter-miniter, "abs threshold (VCal)",legend)

cc=TCanvas("cc","cc")
cc.SetGridy()
hdummy =  TH2F("hdummy","", 21,-0.5,20.5,25,-12.5,12.5)
hdummy.GetXaxis().SetRangeUser(miniter,maxiter) 
hdummy.SetXTitle("iteration")
hdummy.SetYTitle("#DeltaVana")
hdummy.Draw()
for roc in vanacorr:
    if (hvanacorr_vs_iter[roc].GetEntries()>0):
        hvanacorr_vs_iter[roc].Draw("lsame")


raw_input("ok?")

SavePlot(cVanaCorr,"VanaCorrection.png")
SavePlot(cDT,"DT.png")
SavePlot(cAbsThr,"AbsThr.png")
SavePlot(cc,"VanaCorrection_vs_iteration.png")
