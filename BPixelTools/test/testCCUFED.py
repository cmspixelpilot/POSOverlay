import sys,time,os, re, ROOT
from array import *
from datetime import date
import time 
from time import sleep
from Logger import Logger
from optparse import OptionParser
from math import *

sockdir = os.path.expanduser("~") + "/TriDAS/pixel/BPixelTools/test_CLEANROOM/"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket


from SECTORS import SECTOR


from ROOT import gStyle, TH2D, TH1D, TCanvas, TGraph, TGraphErrors, TLegend, TF1, TLine,TPaveLabel,TFile
hList=[None]
gCanvases=[]



###################################
#global functionsS
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


print "connecting to ccu,",
ccu=SimpleSocket( 'localhost', 2001)
print " done "

print "connecting to fed,",
fed=SimpleSocket( 'localhost', 2005 )
print " done"

caen=None   # not needed
pxfec=None  # not needed




def showRMSAOHTestCCU(sector,aoh,wait=True,filename=None):
  gStyle.SetOptStat(0)
  gStyle.SetCanvasColor(0)
  gStyle.SetTitleFillColor(0)
     
  sector.aoh[aoh].biasGraphs=[]
  sector.aoh[aoh].rmsGraphs=[]
  sector.aoh[aoh].slopeGraphs=[]
  data=sector.aoh[aoh].getBiasRMSCurvesTestCCU()

  if filename is None:
     filename=getFilename("aoh%s_%d.ps"%(sector.name,aoh))
     

  c1 = TCanvas( 'c1', time.strftime("%Y-%m-%d-%m %H:%M"), 200, 10, 800, 800 )
  sector.aoh[aoh].pl=TPaveLabel(0.8,0.9,0.95,0.95,time.strftime("%Y-%m-%d-%m %H:%M"))
  sector.aoh[aoh].pl.Draw()
  c1.Divide(2,3)
  for ch in range(1,5):
     c1.cd(ch)
     title="AOHBias_AOH"+str(aoh)+":"+str(ch)+" "+sector.name+" fed channel "+str(sector.aoh[aoh].fedchannels[ch])
     f=TH2D( 'h'+title, title, 2,10,40,2,0,1024 )
     hList.append(f)
     f.GetXaxis().SetTitle("aoh bias")
     #f.GetYaxis().SetTitle("FED ADC")
     f.Draw()
     color=1
     biasGraph=graph([(t[0],t[1]) for t in data[ch]],     color=1, name="bias_"+str(aoh)+":"+str(ch))
     rmsGraph=graph([(t[0],t[2]*100.) for t in data[ch]], color=2, name="rms_"+str(aoh)+":"+str(ch))
     slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]],color=3, name="slope_"+str(aoh)+":"+str(ch))
     sector.aoh[aoh].biasGraphs.append(biasGraph)
     sector.aoh[aoh].rmsGraphs.append(rmsGraph)
     sector.aoh[aoh].slopeGraphs.append(slopeGraph)
     biasGraph.Write()
     rmsGraph.Write()
     slopeGraph.Write()
     
     c1.Update()

  c1.Print(filename)
  gCanvases.append(c1)
        
  if wait:
      raw_input("hit return to continue")


def showRMSAOH(sector,aoh,gain,wait=True,filename=None):
  gStyle.SetOptStat(0)
  gStyle.SetCanvasColor(0)
  gStyle.SetTitleFillColor(0)
     
  sector.aoh[aoh].biasGraphs=[]
  sector.aoh[aoh].rmsGraphs=[]
  sector.aoh[aoh].slopeGraphs=[]

  if sector.aoh[aoh].group.find('L12') != -1:
     sector.ccu.send("channel 0x11").readlines()
  elif sector.aoh[aoh].group.find('L3') != -1:
     sector.ccu.send("channel 0x13").readlines()
 
  cmd=sector.aoh[aoh].name+" setall %i %i %i %i %i %i %i %i"%(gain,gain,gain,gain,0,0,0,0)
  sector.ccu.send(cmd).readlines()
  
  #for ch in range(1,5):
  #   print ch
  #   sector.aoh[aoh].writegain(ch,gain)

  data=sector.aoh[aoh].getBiasRMSCurves()

  if filename is None:
     filename=getFilename("poh%s_%d.ps"%(sector.name,aoh))

  c1 = TCanvas( 'c1', time.strftime("%Y-%m-%d-%m %H:%M"), 200, 10, 800, 800 )
  sector.aoh[aoh].pl=TPaveLabel(0.8,0.9,0.95,0.95,time.strftime("%Y-%m-%d-%m %H:%M"))
  sector.aoh[aoh].pl.Draw()
  c1.Divide(2,3)
  for ch in range(1,5):
     c1.cd(ch)
     title="POHBias_POH"+str(aoh)+":"+str(ch)+" "+sector.name+" fed channel "+str(sector.aoh[aoh].fedchannels[ch])
     f=TH2D( 'h'+title, title, 2,0,80,2,0,1024 )
     hList.append(f)
     f.GetXaxis().SetTitle("poh bias")
     #f.GetYaxis().SetTitle("FED ADC")
     f.Draw()
     color=1
     biasGraph=graph([(t[0],t[1]) for t in data[ch]],     color=1, name="bias_"+str(aoh)+":"+str(ch))
     rmsGraph=graph([(t[0],t[2]*10.) for t in data[ch]], color=2, name="rms_"+str(aoh)+":"+str(ch))
     slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]],color=3, name="slope_"+str(aoh)+":"+str(ch))
     sector.aoh[aoh].biasGraphs.append(biasGraph)
     sector.aoh[aoh].rmsGraphs.append(rmsGraph)
     sector.aoh[aoh].slopeGraphs.append(slopeGraph)
     biasGraph.Write()
     rmsGraph.Write()
     slopeGraph.Write()
     
     c1.Update()

  c1.Print(filename)
  gCanvases.append(c1)
        
  if wait:
      raw_input("hit return to continue")

argv = sys.argv
parser = OptionParser()
parser.add_option("--sector", "--sector", dest="sectorname", default="", action="store", type="string", help="sector name")
parser.add_option("--gain"  , "--gain"  , dest="gain"      , default=3 , action="store", type="int"   , help="gain used for the bias scan")

(opts, args) = parser.parse_args(argv)

sectorname = opts.sectorname
gain = opts.gain

#try:
#   sectorname=sys.argv[1]
#except:
#   sectorname=''
   #print "usage: aohrms.py <sector>"
   #sys.exit(1)  

s=SECTOR(sectorname,fed,17,ccu,pxfec,caen)
print "Sector   ",sectorname
print "FED slot ",s.fedslot

ccu.send("sector "+sectorname).readlines()

#ccu.send("cratereset").readlines()

for l in ccu.send("scanccu").readlines():
   print l

## for l in ccu.send("which fec").readlines():
##    print l

## for l in ccu.send("which ring").readlines():
##    print l

## for l in ccu.send("which ccu").readlines():
##    print l 


#print "finished"

filename=getFilename("testdata/poh%s.pdf"%(s.name))
rootfile=TFile(os.path.splitext(filename)[0]+".root","RECREATE")
TCanvas('c1', '', 200, 10, 800, 800).Print(filename+"[")

pohs = [1,2,3]
#for aoh in range(1,3):
for aoh in pohs:
   print "showRMSAOH %i" %aoh
   showRMSAOH(s,aoh,gain,wait=False,filename=filename)
 
TCanvas('c1').Print(filename+"]")
