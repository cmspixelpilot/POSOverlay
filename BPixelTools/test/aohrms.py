import sys,time,os, re
from array import *
from datetime import date
import time

from math import *

sockdir="/home/uzhpixel/build_pixel/TriDAS/pixel/BPixelTools/tools/python/"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket

# get root into the path
#sys.path.append("/afs/psi.ch/user/d/danek/public/ROOT/root514/lib/")
#sys.path.append("/home/uzhpixel/root/lib/")
# do  setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:/usr/local/lib

sys.path.append("/home/uzhpixel/build_pixel/TriDAS/pixel/BPixelTools/test/p5_surface/")
from SECTORS import SECTOR


from ROOT import gStyle, TH2D, TCanvas, TGraph, TGraphErrors, TLegend, TF1, TLine,TPaveLabel,TFile
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
print "connecting to fed,",
fed=SimpleSocket( 'localhost', 2005 )
print " done"

print "connecting to ccu,",
ccu=SimpleSocket( 'localhost', 2002)
print " done"

caen=None   # not needed
pxfec=None  # not needed




def showRMSAOH(sector,aoh,wait=True,filename=None):
##   gStyle.SetOptStat(0)
##   gStyle.SetCanvasColor(0)
##   gStyle.SetTitleFillColor(0)

     
  sector.aoh[aoh].biasGraphs=[]
  sector.aoh[aoh].rmsGraphs=[]
  sector.aoh[aoh].slopeGraphs=[]
  data=sector.aoh[aoh].getBiasRMSCurves()
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
     f.SetYTitle("FED ADC")
     f.SetXTitle("aoh bias")
     f.Draw()
     color=1
   ##   biasGraph=graph([(t[0],t[1]) for t in data[ch]],     color=1, name="bias_"+str(aoh)+":"+str(ch))
##      rmsGraph=graph([(t[0],t[2]*100.) for t in data[ch]], color=2, name="rms_"+str(aoh)+":"+str(ch))
##      slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]],color=3, name="slope_"+str(aoh)+":"+str(ch))
##      sector.aoh[aoh].biasGraphs.append(biasGraph)
##      sector.aoh[aoh].rmsGraphs.append(rmsGraph)
##      sector.aoh[aoh].slopeGraphs.append(slopeGraph)
##      biasGraph.Write()
##      rmsGraph.Write()
##      slopeGraph.Write()
     
     c1.Update()

  c1.Print(filename)
  gCanvases.append(c1)
        
  if wait:
      raw_input("hit return to continue")




def rmsvstime(sector,aoh,ch):

   gStyle.SetOptStat(0)
   logfile=open('rmsvstime.log','w')
   c2=TCanvas("c2","rmsvstime",200,10,800,600)
   t0=time.time()
   tmax=5.
   h=TH2D('hd', "", 2,0,tmax,2,0,1024 )
   h.Draw()
   gm=TGraph(100)
   gr=TGraph(100)
   gm.SetMarkerSize(0.4)
   gm.SetMarkerColor(1)
   gr.SetMarkerSize(0.4)
   gr.SetMarkerColor(2)
   
   i=0
   while c2:
       gm.Draw('P')
       gr.Draw('P')
       c2.Update()
       t=time.time()-t0
       response=sector.aoh[aoh].fed.query("rms %d"%sector.aoh[aoh].fedchannels[ch])
       #response="24  %f    %f dummy"%(300+100*math.sin(t*0.1),5.+0.1*math.cos(t*0.2)); time.sleep(random.uniform(0.1,0.5))
       channel,mean,rms,name=response.split()
       logfile.write("%8.2f   %s    %s\n"%(t,mean,rms))
       gm.SetPoint(i,t,float(mean))
       gr.SetPoint(i,t,float(rms)*100.)
       i+=1
       if t>tmax:
           tmax=tmax*2
           h=TH2D('hd', "", 2,0,tmax,2,0,1024 )
           h.Draw()
     


try:
   sectorname=sys.argv[1]
except:
   print "usage: aohrms.py <sector>"
   sys.exit(1)
   


s=SECTOR(sectorname,fed,8,ccu,pxfec,caen)
ccu.send("cratereset").readlines()
#rmsvstime(s,4,1)
for l in ccu.send("scanccu").readlines():
   print l


filename=getFilename("testdata/myaoh%s.ps"%(s.name))
rootfile=TFile(os.path.splitext(filename)[0]+".root","CREATE")
##TCanvas('c1', '', 200, 10, 800, 800).Print(filename+"[")
TCanvas('c1').Print(filename+"[")
for aoh in range(1,1):
   showRMSAOH(s,aoh,wait=False,filename=filename)
TCanvas('c1').Print(filename+"]")
#for h in hList: h.Write()
#rootfile.Write()
rootfile.Close()
#showRMSAOH(s,4)

fed.close()
ccu.close()

