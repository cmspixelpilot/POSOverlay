#######################################################
# plotCurrents.py
# Original author: M. Malberti 02/12/2013
# This script plots the analog currents vs Vana iteration.
# It takes as input a txt file with the values of the
# currents for each readout group.
#######################################################
import os
import sys
import ROOT
from ROOT import *
ROOT.gStyle.SetOptStat(1110)
ROOT.gStyle.SetOptTitle(0)

if os.path.exists(sys.argv[1]):
    file = open(sys.argv[1])
    currents = [line.replace('Iteration ','').replace(':','').replace('\n','').split(' ') for line in file if 'Iteration' in line]
else:
    sys.exit("File %s doesn't exist"%sys.argv[1])

max = len(currents)
nr = 135

# 1-D histogram
hcurrent = []
for iter in range(1,max+1):
    htmp = TH1F("hcurrent%d"%iter,"hcurrent%d"%iter,140,0,7./nr)
    hcurrent.append(htmp)
    for rog in range(1,9):
        hcurrent[iter-1].Fill(float(currents[iter-1][rog])/nr)


# currents vs iteration number
h2 = {}
for rog in range(1,9):
    h2[rog-1] = TH1F("hcurrent_vs_iter_%s"%rog,"hcurrent_vs_iter_%s"%rog,21,-0.5,20.5)
    h2[rog-1].SetLineColor(kBlue+rog%2) 
    for iter in range(1,max+1):
        print iter-1,currents[iter-1][rog]
        h2[rog-1].Fill(iter,float(currents[iter-1][rog])/135.)
        

c = TCanvas("c","c")
hcurrent[0].SetLineColor(kRed)
hcurrent[max-1].SetLineColor(kBlue)
hcurrent[max-1].GetXaxis().SetTitle("<current>/ROC (mA)")
hcurrent[max-1].Draw()
hcurrent[0].Draw("sames")
 
cc=TCanvas("cc","cc")
cc.SetGridy()
hdummy =  TH2F("hdummy","", 21,-0.5,20.5,120,2./nr,5./nr)
hdummy.GetXaxis().SetRangeUser(1,max)
hdummy.SetXTitle("iteration")
hdummy.SetYTitle("<current>/ROC(mA)")
hdummy.Draw()
for rog in range(1,9):
    h2[rog-1].Draw("lsame") 

raw_input('ok?')
