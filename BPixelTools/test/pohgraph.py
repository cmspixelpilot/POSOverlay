import sys,time,os, re, ROOT
from datetime import date
from time import sleep
sockdir="/home/cmspixel/TriDAS/pixel/BPixelTools/tools/python"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket
from SystemTests import GROUP, SECTOR, TestRedundancy, TestTriggerStatusFED, tDOH, AOH, DELAY25, PLL, MODULE, TestDCDCEnable, DCDCDisable
from Logger import Logger
from ROOT import *
from array import array
from contextlib import contextmanager
hList=[None]
gCanvases=[]


@contextmanager
def suppress_stdout():
    with open(os.devnull, "w") as devnull:
        old_stdout = sys.stdout
        sys.stdout = devnull
        try:  
            yield
        finally:
            sys.stdout = old_stdout

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



def showRMSPOH(sector,poh,gain,slopethr,wait=True,filename=None):
  gStyle.SetOptStat(0)
  gStyle.SetCanvasColor(0)
  gStyle.SetTitleFillColor(0)
  sector.poh[poh].biasGraphs=[]
  sector.poh[poh].rmsGraphs=[]
  sector.poh[poh].slopeGraphs=[]

  if sector.poh[poh].group.find('L12') != -1:
    sector.ccu.send("channel 0x11").readlines()
  elif sector.poh[poh].group.find('L34') != -1:
    sector.ccu.send("channel 0x13").readlines()
  for ch in range(1,5):
     sector.poh[poh].writegain(ch,gain)
  data=sector.poh[poh].getBiasRMSCurves()

  if filename is None:
     filename=getFilename("poh%s_%d.ps"%(sector.name,poh))

  c1 = TCanvas( 'c1', time.strftime("%Y-%m-%d-%m %H:%M"), 200, 10, 800, 800 )
  sector.poh[poh].pl=TPaveLabel(0.8,0.9,0.95,0.95,time.strftime("%Y-%m-%d-%m %H:%M"))
  sector.poh[poh].pl.Draw()
  c1.Divide(2,3)
  for ch in range(1,5):
     c1.cd(ch)
     title="POHBias_POH"+str(poh)+":"+str(ch)+" "+sector.name+" fed channel "+str(sector.poh[poh].fedchannels[ch])
     f=TH2D( 'h'+title, title, 2,0,60,2,0,1024 )
     hList.append(f)
     f.GetXaxis().SetTitle("poh bias")
     #f.GetYaxis().SetTitle("FED ADC")
     f.Draw()
     color=1
     checkgraph=TGraph()
     n=0
     for t in data[ch]:
         if t[1]>50 and  t[1]<1023-50:
             checkgraph.SetPoint(n,t[0],t[1])
             n=n+1
     checkgraph.Fit("pol1","q")
     if pol1.GetParameter(1)<slopethr:
         print "low slope"
     biasGraph=graph([(t[0],t[1]) for t in data[ch]],    style=20, color=1, name="bias_"+str(poh)+":"+str(ch))
     # rmsGraph=graph([(t[0],t[2]*10.) for t in data[ch]], style=21, color=31, name="rms_"+str(poh)+":"+str(ch))
     # slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]],style=24, color=2, name="slope_"+str(poh)+":"+str(ch))
     sector.poh[poh].biasGraphs.append(biasGraph)
     # sector.poh[poh].rmsGraphs.append(rmsGraph)
     # sector.poh[poh].slopeGraphs.append(slopeGraph)
     biasGraph.Write()
     # print title
     biasGraph.Print()
     # rmsGraph.Write()
     # slopeGraph.Write()
     # legend = TLegend(.64,.62,.85,.85)
     # legend.AddEntry(biasGraph,"Bias","p") 
    
     # legend.AddEntry(slopeGraph,"Slope*10,microW/mA","p")
     # legend.AddEntry(rmsGraph,"Noise","p")
     # legend.Draw()
     
     c1.Update()
  #################################################
  
#################################################
     
  c1.Print(filename)
  gCanvases.append(c1)
        
 


  if wait:
      raw_input("hit return to continue")





