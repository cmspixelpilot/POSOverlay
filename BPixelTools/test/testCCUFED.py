import sys,time,os, re, ROOT
from array import *
from datetime import date
import time 
from time import sleep
from Logger import Logger
from optparse import OptionParser
from math import *

sockdir="/home/cmspixel/TriDAS/pixel/BPixelTools/test_CLEANROOM/"
# sockdir="/home/cmspixel/TriDAS/pixel/BPixelTools/tools/python"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket


from SystemTests import SECTOR


from ROOT import gStyle, TH2D, TH1D, TCanvas, TGraph, TGraphErrors, TLegend, TF1, TLine,TPaveLabel,TFile,TTree
hList=[None]
gCanvases=[]
#
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
fed=SimpleSocket( 'localhost', 2004 )
print " done"

caen=None   # not needed
pxfec=None  # not needed

name= "+6P"
sector= SECTOR(name,fed,ccu,pxfec,caen,log)
sector.pohlist= range(2,3)+range(4,5)+ range(6,7)


def showRMSPOHTestCCU(sector,poh,wait=True,filename=None):
  gStyle.SetOptStat(0)
  gStyle.SetCanvasColor(0)
  gStyle.SetTitleFillColor(0)
  sector.poh[poh].biasGraphs=[]
  sector.poh[poh].rmsGraphs=[]
  sector.poh[poh].slopeGraphs=[]
  data=sector.poh[poh].getBiasRMSCurvesTestCCU()

  if filename is None:
     filename=getFilename("poh%s_%d.ps"%(sector.name,poh))
     

  c1 = TCanvas( 'c1', time.strftime("%Y-%m-%d-%m %H:%M"), 200, 10, 800, 800 )
  sector.poh[poh].pl=TPaveLabel(0.8,0.9,0.95,0.95,time.strftime("%Y-%m-%d-%m %H:%M"))
  sector.poh[poh].pl.Draw()
  c1.Divide(2,3)
  for ch in range(1,5):
     c1.cd(ch)
     title="POHBias_POH"+str(poh)+":"+str(ch)+" "+sector.name+" fed channel "+str(sector.poh[poh].fedchannels[ch])


     f=TH2D( 'h'+title, title, 2,10,40,2,0,1024 )
     hList.append(f)
     f.GetXaxis().SetTitle("poh bias")
     #f.GetYaxis().SetTitle("FED ADC")
     f.Draw()
     color=1
     biasGraph=graph([(t[0],t[1]) for t in data[ch]],     color=1, name="bias_"+str(poh)+":"+str(ch))
     rmsGraph=graph([(t[0],t[2]*100.) for t in data[ch]],  color=2, name="rms_"+str(poh)+":"+str(ch))
     slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]], color=3, name="slope_"+str(poh)+":"+str(ch))
     sector.poh[poh].biasGraphs.append(biasGraph)
     sector.poh[poh].rmsGraphs.append(rmsGraph)
     sector.poh[poh].slopeGraphs.append(slopeGraph)
     biasGraph.Write()
     rmsGraph.Write()
     slopeGraph.Write()
     
     c1.Update()

  c1.Print(filename)
  gCanvases.append(c1)
        
  if wait:
      raw_input("hit return to continue")


def showRMSPOH(sector,poh,gain,wait=True,filename=None):
   
  gStyle.SetOptStat(0)
  gStyle.SetCanvasColor(0)
  gStyle.SetTitleFillColor(0)
  sector.poh[poh].biasGraphs=[]
  sector.poh[poh].rmsGraphs=[]
  sector.poh[poh].slopeGraphs=[]
  
  # for poh in range(1,15):
  #    print sector.poh[poh].group

 
  if sector.poh[poh].group.find('L12') != -1:
     sector.ccu.send("channel 0x11").readlines()
  elif sector.poh[poh].group.find('L34') != -1:
     sector.ccu.send("channel 0x13").readlines()

 
  #cmd=sector.poh[poh].name+" setall %i %i %i %i %i %i %i %i"%(gain,gain,gain,gain,0,0,0,0)
  #sector.ccu.send(cmd).readlines()
  sector.ResetAllandInitAll()
  for ch in range(1,5):
     sector.poh[poh].writegain(ch,gain)
  #for ch in range(1,5):
  #   print ch
  #   sector.poh[poh].writegain(ch,gain)

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
     f=TH2D( 'h'+title, title, 2,0,40,2,0,1024 )
     hList.append(f)
     f.GetXaxis().SetTitle("poh bias")
     #f.GetYaxis().SetTitle("FED ADC")
     f.Draw()
     color=1
     biasGraph=graph([(t[0],t[1]) for t in data[ch]],    style=20, color=1, name="bias_"+str(poh)+":"+str(ch))
     rmsGraph=graph([(t[0],t[2]*10.) for t in data[ch]], style=21, color=31, name="rms_"+str(poh)+":"+str(ch))
     slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]],style=24, color=2, name="slope_"+str(poh)+":"+str(ch))
     sector.poh[poh].biasGraphs.append(biasGraph)
     sector.poh[poh].rmsGraphs.append(rmsGraph)
     sector.poh[poh].slopeGraphs.append(slopeGraph)
     biasGraph.Write()
     print title
     biasGraph.Print()
     rmsGraph.Write()
     slopeGraph.Write()
     legend = TLegend(.64,.62,.85,.85)
     legend.AddEntry(biasGraph,"Bias","p") 
    
     legend.AddEntry(slopeGraph,"Slope*10,microW/mA","p")
     legend.AddEntry(rmsGraph,"Noise","p")
     legend.Draw()
     
     c1.Update()
  #################################################
  
#################################################
     
  c1.Print(filename)
  gCanvases.append(c1)
        
 


  if wait:
      raw_input("hit return to continue")



def showRMSPOH_InputOffset(sector,poh,gain,wait=True,filename=None):
  gStyle.SetOptStat(0)
  gStyle.SetCanvasColor(0)
  gStyle.SetTitleFillColor(0)
     
  sector.poh[poh].biasGraphs=[]
  sector.poh[poh].rmsGraphs=[]
  sector.poh[poh].slopeGraphs=[]

  sector.ResetAllandInitAll()


  data=sector.poh[poh].ScanDACInputOffset()


  if filename is None:
     filename=getFilename("poh%s_%d.ps"%(sector.name,poh))

  c1 = TCanvas( 'c1', time.strftime("%Y-%m-%d-%m %H:%M"), 200, 10, 800, 800 )
  sector.poh[poh].pl=TPaveLabel(0.8,0.9,0.95,0.95,time.strftime("%Y-%m-%d-%m %H:%M"))
  sector.poh[poh].pl.Draw()
  c1.Divide(2,3)
  for ch in range(1,5):
     c1.cd(ch)
     title="POHBias_POH"+str(poh)+":"+str(ch)+" "+sector.name+" fed channel "+str(sector.poh[poh].fedchannels[ch])
     f=TH2D( 'h'+title, title, 2,0,20,2,0,1024 )
     hList.append(f)
     f.GetXaxis().SetTitle("DAC")
     #f.GetYaxis().SetTitle("FED ADC")
     f.Draw()
     color=1
     biasGraph=graph([(t[0],t[1]) for t in data[ch]],     color=1, name="bias_"+str(poh)+":"+str(ch))
     rmsGraph=graph([(t[0],t[2]*10.) for t in data[ch]], color=2, name="rms_"+str(poh)+":"+str(ch))
     slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]],color=3, name="slope_"+str(poh)+":"+str(ch))
     sector.poh[poh].biasGraphs.append(biasGraph)
     sector.poh[poh].rmsGraphs.append(rmsGraph)
     sector.poh[poh].slopeGraphs.append(slopeGraph)
     biasGraph.Write()
     rmsGraph.Write()
     slopeGraph.Write()
     
     c1.Update()

  c1.Print(filename)
  gCanvases.append(c1)
        
  if wait:
      raw_input("hit return to continue")





def showRMSPOH_DAC(sector,poh,gain,wait=True,filename=None):
  gStyle.SetOptStat(0)
  gStyle.SetCanvasColor(0)
  gStyle.SetTitleFillColor(0)
     
  sector.poh[poh].biasGraphs=[]
  sector.poh[poh].rmsGraphs=[]
  sector.poh[poh].slopeGraphs=[]

  sector.ResetAllandInitAll()


  data=sector.poh[poh].ScanDACoffset()

  if filename is None:
     filename=getFilename("poh%s_%d.ps"%(sector.name,poh))

  c1 = TCanvas( 'c1', time.strftime("%Y-%m-%d-%m %H:%M"), 200, 10, 800, 800 )
  sector.poh[poh].pl=TPaveLabel(0.8,0.9,0.95,0.95,time.strftime("%Y-%m-%d-%m %H:%M"))
  sector.poh[poh].pl.Draw()
  c1.Divide(2,3)
  for ch in range(1,5):
     c1.cd(ch)
     title="POHBias_POH"+str(poh)+":"+str(ch)+" "+sector.name+" fed channel "+str(sector.poh[poh].fedchannels[ch])+ "gain =" + str(gain)
     f=TH2D( 'h'+title, title, 2,0,255,2,0,1024 )
     hList.append(f)
     f.GetXaxis().SetTitle("DAC")
     #f.GetYaxis().SetTitle("FED ADC")
     f.Draw()
     color=1
     biasGraph=graph([(t[0],t[1]) for t in data[ch]],     color=1, name="bias_"+str(poh)+":"+str(ch))
     rmsGraph=graph([(t[0],t[2]*10.) for t in data[ch]], color=2, name="rms_"+str(poh)+":"+str(ch))
     slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]],color=3, name="slope_"+str(poh)+":"+str(ch))
     sector.poh[poh].biasGraphs.append(biasGraph)
     sector.poh[poh].rmsGraphs.append(rmsGraph)
     sector.poh[poh].slopeGraphs.append(slopeGraph)
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
   #print "usage: pohrms.py <sector>"
   #sys.exit(1)  

s=SECTOR(sectorname,fed,ccu,pxfec,caen)
print "Sector   ",sectorname
#print "FED slot ",s.fedslot

s.ccu.send("sector "+sectorname).readlines()
s.ccu.send("channel 0x11 ").readlines()

#ccu.send("cratereset").readlines()

for l in ccu.send("scanccu").readlines():
   print l

for l in ccu.send("which fec").readlines():
   print l

for l in ccu.send("which ring").readlines():
   print l

for l in ccu.send("which ccu").readlines():
   print l 

for l in ccu.send("which channel").readlines():
   print l 


#print "finished"
filename=getFilename("testdata/gainscan/2016poh%s.pdf"%(s.name))
#filename=getFilename("testdata/2016poh%s.pdf"%(s.name))
rootfile=TFile(os.path.splitext(filename)[0]+".root","RECREATE")
TCanvas('c1', '', 200, 10, 800, 800).Print(filename+"[")
print "in real life we have this layout:"
#s.pohlist= range(2,3)+range(4,5)+range(6,7)
#s.poh[2].fedchannels=[None,8,7,6,5]
#s.poh[4].fedchannels=[None,12,11,10,9]
#s.poh[6].fedchannels=[None,4,3,2,1]
s.pohlist= range(7,8)+range(9,10)+range(12,13)
s.poh[7].fedchannels=[None,12,11,10,9]
s.poh[9].fedchannels=[None,8,7,6,5]
s.poh[12].fedchannels=[None,4,3,2,1]
for i in sector.pohlist:
    s.poh[i].bundle=1
#pohs = [12]
for poh in s.pohlist:
#for poh in pohs:
   print "showRMSPOH %i" %poh
   showRMSPOH(s,poh,gain,wait=False,filename=filename) 
   #showRMSPOH_DAC(s,poh,gain,wait=False,filename=filename)
   #showRMSPOH_InputOffset(s,poh,gain,wait=False,filename=filename)
TCanvas('c1').Print(filename+"]")
#######################################
########################################
