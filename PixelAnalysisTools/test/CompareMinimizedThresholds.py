#######################################################
# AnalyzeVanaCalibration.py
# Original author: M. Malberti 12/02/2014
#######################################################                                                                                                                                                                   
import os
import sys
import ROOT
from ROOT import *

def GetThresholdsFromFile(filename):
    file = open(filename)
    lines = [line.replace('\n','') for line in file]
    # fill dictionary with key = roc, value = list of thrsholds 
    d = {}
    for line in lines:
        roc   = line.split()[1]
        value = line.split()[5]
        if roc not in d:
            d[roc] = [float(value)]
        else:
            d[roc].append(float(value))            
    return d        



def GetThresholdsFromHistogram(filename):
    
    d = {}
   
    # loop over histograms and get average roc threshold
    file = TFile(filename)
    file.cd("FPix")
   
    for obj0 in gDirectory.GetListOfKeys(): #FPix_BmI
        if obj0.IsFolder():
            obj0.ReadObj().cd()

            for obj1 in gDirectory.GetListOfKeys(): # DISK folder 
                if obj1.IsFolder():
                    obj1.ReadObj().cd()

                    for obj2 in gDirectory.GetListOfKeys(): ## BLD folder
                        if  obj2.IsFolder():
                            obj2.ReadObj().cd()

                            for obj3 in gDirectory.GetListOfKeys(): ## PNL folder
                                if  obj3.IsFolder():
                                    obj3.ReadObj().cd()

                                    for obj4 in gDirectory.GetListOfKeys(): ## PLQ folder
                                        if  obj4.IsFolder():
                                            obj4.ReadObj().cd()

                                            for obj5 in gDirectory.GetListOfKeys(): ## ROC folder: find one TH2F for each ROC
                                                histo = obj5.ReadObj()
                                                hname   = histo.GetName()
                                                if 'Threshold1D' in hname:
                                                    mean  = histo.GetMean()
                                                    roc   = hname.replace('_Threshold1D','')
                                                    d[roc] = [float(mean)]
    file.Delete()                                                    
    return d





def average (list):
    tot = 0
    for el in list:
        tot = tot + el
    ave = tot / len(list)
    return ave    






from optparse import OptionParser
parser = OptionParser()
parser.add_option("--file1",dest="file1",type="string",default="ThresholdMinimization/BmI_T-20deg/Run_1261/TrimOutputFile_Fed_37-38.dat",help="Name of first file with thresholds")
parser.add_option("--file2",dest="file2",type="string",default="ThresholdMinimization/BmI_T-20deg/Run_1260/TrimOutputFile_Fed_37-38.dat",help="Name of second file with thresholds")

(options,args)=parser.parse_args()




#d1 = GetThresholdsFromFile(options.file1)
#d2 = GetThresholdsFromFile(options.file2)


d1 = GetThresholdsFromHistogram(options.file1)
d2 = GetThresholdsFromHistogram(options.file2)


h1  = TH1F("h1","h1",50,0,100)
h1.SetXTitle("Threshold [VCal]")
h1.SetYTitle("")
h1.SetLineColor(kBlue+1)

h2  = TH1F("h2","h2",50,0,100)
h2.SetXTitle("Threshold [VCal]")
h2.SetYTitle("")
h2.SetLineColor(kRed)

hdiff  = TH1F("hdiff","hdiff",50,-50,50)
hdiff.SetXTitle("#Delta(Threshold) [VCal]")
hdiff.SetYTitle("")

hh = TH2F("hh","hh",100,20,60,100,20,60)
hh.SetXTitle("Old Threshold (VCal) ")
hh.SetYTitle("New Threshold (VCal) ")


for roc in d1:
    mean1 = average(d1[roc])
    mean2 = average(d2[roc])
    hdiff.Fill(mean2-mean1)
    hh.Fill(mean1,mean2)
    h1.Fill(mean1)
    h2.Fill(mean2)
    if abs(mean2-mean1)>15:
        print roc


c = TCanvas("c","c",500,500)
h1.Draw()
h2.Draw("sames")

cdiff = TCanvas("cdiff","c",500,500)
hdiff.Draw()

cc = TCanvas("cc","cc",500,500)
cc.SetGridx()
cc.SetGridy()
hh.Draw()


raw_input('ok?')








