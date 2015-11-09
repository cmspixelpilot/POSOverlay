import sys,time,os, re, ROOT
from array import *
from datetime import date
import time 
from time import sleep
from Logger import Logger

from math import *

sockdir="/home/uzhpixel/build_pixel/TriDAS/pixel/BPixelTools/tools/python/"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket


from SECTORS import SECTOR


from ROOT import gStyle, TH2D, TH1D, TCanvas, TGraph, TGraphErrors, TLegend, TF1, TLine,TPaveLabel,TFile
hList=[None]
gCanvases=[]



###################################
#global functions
def graph(xy,style=20,size=0.8,color=1,name=None):

   ax=array('d')
   ay=array('d')
   for i in range(len(xy)):
      ax.append(xy[i][0])
      ay.append(xy[i][1])
   g=TGraph(len(xy),ax,ay)
   g.SetMarkerStyle(style)
   g.SetMarkerSize(size)
   g.SetMarkerColor(color)
   if name:
      g.SetName(name)
   g.Draw("P")
   return g

def getFilename(filename):
   """ add date and version """
   base,ext=os.path.splitext(filename)
   f=base+"_"+date.today().isoformat()
   if os.path.exists(f+ext):
         template=f+"-%03d"
         for n in range(1,100):
            if not os.path.exists(template%n+ext):
               return template%n+ext
         print "no valid filename found for ",filename
         return "temp"+ext
   else:
      return f+ext

########################################################

fed=None
ccu=None
caen=None   # not needed
pxfec=None  # not needed




def showRMSAOH(sector,aoh,wait=True,filename=None):
  gStyle.SetOptStat(0)
  gStyle.SetCanvasColor(0)
  gStyle.SetTitleFillColor(0)
     
  sector.aoh[aoh].biasGraphs=[]
  sector.aoh[aoh].rmsGraphs=[]
  sector.aoh[aoh].slopeGraphs=[]
#  data=sector.aoh[aoh].getBiasRMSCurves()
  data=[]

  if filename is None:
     filename=getFilename("aoh%s_%d.ps"%(sector.name,aoh))
     

  c1 = TCanvas( 'c1', time.strftime("%Y-%m-%d-%m %H:%M"), 200, 10, 800, 800 )
  sector.aoh[aoh].pl=TPaveLabel(0.8,0.9,0.95,0.95,time.strftime("%Y-%m-%d-%m %H:%M"))
  sector.aoh[aoh].pl.Draw()
  c1.Divide(2,3)
  for ch in range(1,7):
     c1.cd(ch)
     title="AOHBias_AOH"+str(aoh)+":"+str(ch)+" "+sector.name+" fed channel "+str(sector.aoh[aoh].fedchannels[ch])
     f=TH2D( 'h'+title, title, 2,10,40,2,0,1024 )
     hList.append(f)
     f.GetXaxis().SetTitle("aoh bias")
     #f.GetYaxis().SetTitle("FED ADC")
     f.Draw()
     color=1
     
     c1.Update()

  c1.Print(filename)
  gCanvases.append(c1)
        
  if wait:
      raw_input("hit return to continue")




try:
   sectorname=sys.argv[1]
except:
   print "usage: aohrms.py <sector>"
   sys.exit(1)
   

s=SECTOR(sectorname,fed,8,ccu,pxfec,caen)


filename=getFilename("testdata/aoh%s.pdf"%(s.name))
rootfile=TFile(os.path.splitext(filename)[0]+".root","RECREATE")
TCanvas('c1', '', 200, 10, 800, 800).Print(filename+"[")


for aoh in range(1,2):
   print "showRMSAOH"
   showRMSAOH(s,aoh,wait=False,filename=filename)
 

TCanvas('c1').Print(filename+"]")
