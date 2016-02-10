#import sys,time,os, re
import sys,time,os, re, ROOT
from datetime import date
from time import sleep
from Logger import Logger
sockdir="/home/l_pixel/TriDAS/pixel/BPixelTools/tools/python"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket
from ROOT import gStyle, TH2D, TCanvas, TGraph, TGraphErrors, TLegend, TF1, TLine,TPaveLabel,TFile
from array import *
from math import *
hList=[None]
gCanvases=[]
########################
class SECTOR:
   def __init__(self,name,fed,fedslot,ccu,pxfec,caen,log=None):
      self.name=name
      self.fed=fed
      self.fedslot=fedslot
      self.ccu=ccu
      self.pxfec=pxfec
      self.caen=caen
      if log:
         self.log=log
      else:
         self.log=Logger()
      self.group={}
      self.group[1]=GROUP(fed, fedslot, ccu, pxfec, caen, name+"L12",0x11,self.log)
      self.group[2]=GROUP(fed, fedslot, ccu, pxfec, caen, name+"L3", 0x13,self.log)
      self.aoh={}
      self.aoh[1]=AOH(fed, fedslot, ccu, name+"L12","aoh1",[12, 11, 10,  9,  8,  7],self.log)
      self.aoh[2]=AOH(fed, fedslot, ccu, name+"L12","aoh2",[ 6,  5,  4,  3,  2,  1],self.log)
      self.aoh[3]=AOH(fed, fedslot, ccu, name+"L12","aoh3",[24, 23, 22, 21, 20, 19],self.log)
      self.aoh[4]=AOH(fed, fedslot, ccu, name+"L12","aoh4",[18, 17, 16, 15, 14, 13],self.log)
      self.aoh[5]=AOH(fed, fedslot, ccu, name+"L3", "aoh1",[36, 35, 34, 33, 32, 31],self.log)
      self.aoh[6]=AOH(fed, fedslot, ccu, name+"L3", "aoh2",[30, 29, 28, 27, 26, 25],self.log)
      self.aohlist=range(1,7)
      self.dohA=tDOH(ccu, "0x7e", "0x10",self.log)
      self.dohB=tDOH(ccu, "0x7d", "0x10",self.log)
      self.resetDOH=[]
      self.resetAOH=[]
      self.resetROC=[]

   def Init(self):
      self.InitFEC()
      self.InitFED()

   def InitMB(self):
      if not self.ccu: return
      self.group[1].InitMB()
      self.group[2].InitMB()

   def InitFEC(self):
      self.ccu.send("sector "+self.name).readlines()
      self.ccu.send("reset").readlines()
      print "Sector "+self.name+": FEC reset"

   def InitpxFEC(self):
      self.pxfec.send("cn "+self.name+"L12").readlines()
      print "Sector "+self.name+": Init pxFEC"
     
   def InitFED(self):
      #self.fed.send("fed %d"%self.fedslot).readlines()
      self.fed.send("reset").readlines()
      print "Sector "+self.name+": FED reset"

   def InitCAEN(self):
      print "init CAEN"
      self.caen.send("group "+self.name+"L12").readlines()
      
      #self.caen.send("group -6PL12").readlines()
      cnt=0
      while not (self.IsCAENOn()):
         cnt=cnt+1
         print "Group "+self.name+"L12: switch CAEN On"
         self.caen.send("pon").readlines()
         time.sleep(0.5)
         if (cnt>5):
            print "CAEN: Error when turning power on"
            break
      print "Group "+self.name+"L12: CAEN: power On"

   def IsCAENOn(self):
      for t in range(5):
         self.caen.send("group "+self.name+"L12").readlines()
         pw = self.caen.query("get pw").strip()
         #print "SECTOR::isCAENOn::pw=",pw
         if pw in ("On","Off"): return True
         print "\n no response from caen, retrying"
         time.sleep(1.0)
      print "no response from CAEN"
      sys.exit(1)


   def InitTrackerDOH(self):
      self.ccu.send("sector "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.dohA.SendValues()
      self.dohB.SendValues()
      self.ccu.send("sector "+self.name).readlines()
      self.ccu.send("reset").readlines()



   def isPowered(self):
      self.ccu.send('sector '+self.name).readlines()
      self.ccu.send('reset').readlines()
      response=self.ccu.send('pio read').readlines()
      if response[0]=='pio data reads ff':
         self.ccu.send('pio write 0xff').readlines()
         return True
      else:
         return False

      
      
   def mapAOH(self):
      print "mapping sector ",self.name
      for a in range(1,7):
         self.aoh[a].fedchannels=[None,0,0,0,0,0,0]
      for a in self.aohlist:
         self.aoh[a].findAll()
         print "aoh ",a,self.aoh[a].fedchannels[1:]



   def TestTriggerStatusFED(self):
      # this is a method of SECTOR
      self.InitFED()
      #self.fed.send("fed %d"%self.fedslot).readlines()
      self.fed.send("fed %s"%self.name).readlines()
      nloop=0
      e1=self.fed.iquery("event")
      sleep(0.001) 
      while(e1==self.fed.iquery("event")):
         sleep(0.001) 
         nloop=nloop+1
         if (nloop>100):
            print "Error: TestTriggerStatusFED(): FED does not receive trigger!"
            sys.exit(1)
            
      if (nloop<100):
         print "TestTriggerStatusFED(): OK. FED receives trigger!"

   def TestCCUandI2CConnection(self):
      self.InitFEC()
      print self.ccu.query("mapccu")
      wait=raw_input("\ncontinue")
      print self.ccu.query("scanpixeldevice")
      wait=raw_input("\ncontinue")

   def VerifyI2CProgrammingAOH(self):
      #for i in range(1,7):
      for i in self.aohlist:
         self.aoh[i].VerifyProgramming()
      
   def VerifyI2CProgrammingtDOH(self):
      self.dohA.VerifyProgramming()
      self.dohB.VerifyProgramming()
  

   def DrawMapAOH(self):
      self.mapAOH()
      gStyle.SetCanvasColor(0)
      gStyle.SetTitleFillColor(0)
      gStyle.SetOptStat("e")
      c1 = TCanvas( 'c1', '', 0, 0, 500, 500 )
      c1.Divide(1,1)
      histo = TH2D( 'histo', 'FED channels', 37,0,37,37,0,37 )
      histo.SetXTitle("FED channel")
      for i in range(6):
         histo.GetYaxis().SetBinLabel(6*i+1,self.aoh[i+1].name+"a_0")
         histo.GetYaxis().SetBinLabel(6*i+2,self.aoh[i+1].name+"a_1")
         histo.GetYaxis().SetBinLabel(6*i+3,self.aoh[i+1].name+"a_2")
         histo.GetYaxis().SetBinLabel(6*i+4,self.aoh[i+1].name+"b_0")
         histo.GetYaxis().SetBinLabel(6*i+5,self.aoh[i+1].name+"b_1")
         histo.GetYaxis().SetBinLabel(6*i+6,self.aoh[i+1].name+"b_2")
         
      for c in range(1,37):
         for i in range(1,7):
            for j in range(1,7):
               if (self.aoh[i].fedchannels[j]==c): histo.Fill(c,(i-1)*6+j)
               
      histo.Draw("COLBOX")
     
      filename=folder+"/AOHmap_Sector"+self.name+"-"+date.today().isoformat()
      if os.path.exists(filename+".gif"):
         template=filename+"-%03d"
         for n in range(100):
            filename=template%n
            if not os.path.exists(filename+".gif"): break
      c1.Print(filename+".gif")
      return c1
      #wait=raw_input("\ncontinue")
      #if wait=='x':
      #   sys.exit(0)

   def showAOHBias(self):
      gStyle.SetOptStat(0)
      gStyle.SetCanvasColor(0)
      gStyle.SetTitleFillColor(0)
      c1 = TCanvas( 'c1', '', 200, 10, 800, 800 )
      c1.Divide(2,3)
      h=[None]
      for aoh in range(1,7):
         title=folder+"/AOHBias_AOH"+str(aoh)+self.name
         c1.cd(aoh)
         h.append(TH2D( 'h'+title, title, 2,0,128,2,0,1024 ))
         h[aoh].SetYTitle("FED ADC")
         h[aoh].SetXTitle("aoh bias")
         h[aoh].Draw()
         color=1
         #if aoh ==1: continue
         data=self.aoh[aoh].getBiasCurves()
         self.aoh[aoh].biasGraphs=[]
         for ch in range(1,7):
            self.aoh[aoh].biasGraphs.append(graph(data[ch],color=color))
            color=color+1
         c1.Update()
      filename=folder+"/AOHbiasS"+self.name+"-"+date.today().isoformat()
      if os.path.exists(filename+".gif"):
         template=filename+"-%03d"
         for n in range(100):
            filename=template%n
            if not os.path.exists(filename+".gif"): break
      c1.Print(filename+".gif")
      raw_input("hit return to continue")

   ###############################################################
   def showRMSAOH(self,aoh,wait=True,filename=None,setbias=False):
      gStyle.SetOptStat(0)
      gStyle.SetCanvasColor(0)
      gStyle.SetTitleFillColor(0)
      
      self.aoh[aoh].biasGraphs=[]
      self.aoh[aoh].rmsGraphs=[]
      self.aoh[aoh].slopeGraphs=[]
      data=self.aoh[aoh].getBiasRMSCurves(setbias)
      if filename is None:
         filename=getFilename("aoh%s_%d.ps"%(self.name,aoh))
     

      c1 = TCanvas( 'c1', time.strftime("%Y-%m-%d-%m %H:%M"), 200, 10, 800, 800 )
      self.aoh[aoh].pl=TPaveLabel(0.8,0.9,0.95,0.95,time.strftime("%Y-%m-%d-%m %H:%M"))
      self.aoh[aoh].pl.Draw()
      c1.Divide(2,3)
      for ch in range(1,7):
         c1.cd(ch)
         title="AOHBias_AOH"+str(aoh)+":"+str(ch)+" "+self.name+" fed channel "+str(self.aoh[aoh].fedchannels[ch])
         f=TH2D( 'h'+title, title, 2,10,40,2,0,1024 )
         hList.append(f)
         f.SetYTitle("FED ADC")
         f.SetXTitle("aoh bias")
         f.Draw()
         color=1
         if len(data[ch])==0: continue
         biasGraph=graph([(t[0],t[1]) for t in data[ch]],     color=1, name="bias_"+str(aoh)+":"+str(ch))
         rmsGraph=graph([(t[0],t[2]*100.) for t in data[ch]], color=2, name="rms_"+str(aoh)+":"+str(ch))
         slopeGraph=graph([(t[0],t[3]*10.) for t in data[ch]],color=3, name="slope_"+str(aoh)+":"+str(ch))
         self.aoh[aoh].biasGraphs.append(biasGraph)
         self.aoh[aoh].rmsGraphs.append(rmsGraph)
         self.aoh[aoh].slopeGraphs.append(slopeGraph)
         biasGraph.Write()
         rmsGraph.Write()
         slopeGraph.Write()
         
         c1.Update()

      c1.Print(filename)
      gCanvases.append(c1)
        
      if wait:
         raw_input("hit return to continue")

   def runAOHscan(self,setbias=False):
      filename=getFilename("testdata/aoh%s.ps"%(self.name))
      rootfile=TFile(os.path.splitext(filename)[0]+".root","CREATE")
      TCanvas('c1', '', 200, 10, 800, 800).Print(filename+"[")
      #for aoh in range(1,7):
      for aoh in self.aohlist:
         self.showRMSAOH(aoh,wait=False,filename=filename,setbias=setbias)
      TCanvas('c1', '', 200, 10, 800, 800).Print(filename+"]")
      rootfile.Close()

   def setAOHBias(self,target=500):
      #for aoh in range(1,7):
      for aoh in self.aohlist:
         for ch in range(1,7):
            self.aoh[aoh].setBiasFed(ch,target)
            
   ###############################################################

   def TestPIAResetDOH(self):
      self.InitFEC()
            
      self.ccu.send("delay25 set d0 10").readlines()
      self.ccu.send("delay25 set d0 10").readlines()
      
      query="delay25 read"
      regexp="-->Delay0 : %d"
      v1 = self.ccu.iquery(query,regexp)
      if not (v1==10):
         #print "TestPIAResetDOH(): ERROR (programming delay25 failed)"
         self.log.error("TestPIAResetDOH(): ERROR (programming delay25 failed)")
         self.resetDOH.append(False)

      self.ccu.send("piareset doh").readlines() 
      v2 = self.ccu.iquery(query,regexp)
      if not (v2==0):
         #print "TestPIAResetDOH(): ERROR (reset failed)"
         self.log.error("TestPIAResetDOH(): ERROR (reset failed)")
         self.resetDOH.append(False)
      else:
         #print "TestPIAResetDOH(): CCU reset (DOH) OK"
         self.log.ok("TestPIAResetDOH(): CCU reset (DOH) OK")
         self.resetDOH.append(True)

   def TestPIAResetAOH(self):
      self.InitFEC()
     
      self.ccu.send("aoh1a set b2 1").readlines()
      self.ccu.send("aoh1a set b2 1").readlines()

      query="aoh1a read"
      regexp="-->Bias channel 2: %d"
      
      v1 = self.ccu.iquery(query,regexp)
      #print v1
      if not (v1==1):
         #print "TestPIAResetAOH(): ERROR (programming delay25 failed)"
         self.log.error("TestPIAResetAOH(): ERROR (programming delay25 failed)")
         self.resetAOH.append(False)

      self.ccu.send("piareset aoh").readlines() 
      v2 = self.ccu.iquery(query,regexp)
      #print v2
      if not (v2==0):
         #print "TestPIAResetAOH(): ERROR (reset failed)"
         self.log.error("TestPIAResetAOH(): ERROR (reset failed)")
         self.resetAOH.append(False)
      else:
         #print "TestPIAResetAOH(): CCU reset (AOH) OK"
         self.log.ok("TestPIAResetAOH(): CCU reset (AOH) OK")
         self.resetAOH.append(True)

   def TestPIAResetROC(self):
      self.InitpxFEC()
      self.InitFEC()
      self.InitCAEN()
      self.group[1].delay25.SendValues()

      #set Vdigi=0
      self.pxfec.send("module 0:31 roc 0:15 Vdig 0").readlines()
      print "Vdig set to 0"
      sleep(10)
      ia1=self.caen.fquery("get ia")
      id1=self.caen.fquery("get id")

      #pia reset roc
      self.ccu.send("piareset roc").readlines()
      print "piareset"
      sleep(10)
      ia2=self.caen.fquery("get ia")
      id2=self.caen.fquery("get id")

      print "Idig ", id1, id2
      print "Iana ", ia1, ia2

      if (id2-id1>0.2):
         print "TestPIAResetROC(): CCU reset (ROC) OK"
         self.log.ok("TestPIAResetROC(): CCU reset (ROC) OK")
         self.resetROC.append(True)
      else:
         print "TestPIAResetROC(): CCU reset (ROC) ERROR"
         self.log.error("TestPIAResetROC(): CCU reset (ROC) ERROR")
         self.resetROC.append(False)

   def PrintResults(self):
      filename=folder+"/Results_Sector"+self.name+"-"+date.today().isoformat()
      if os.path.exists(filename+".txt"):
         template=filename+"-%03d"
         for n in range(100):
            filename=template%n
            if not os.path.exists(filename+".txt"): break
      f=open(filename+".txt", 'w')
      f.write("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
      f.write("SECTOR"+self.name+'\n')
      f.write("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
      f.write("PIA RESETS"+'\n')
      if (len(self.resetDOH)==0): f.write("-->DOH reset: not tested\n")
      elif (self.resetDOH[0]):    f.write("-->DOH reset: OK\n") 
      else:                       f.write("-->DOH reset: ERROR\n")
      if (len(self.resetAOH)==0): f.write("-->AOH reset: not tested\n")
      elif (self.resetAOH[0]):    f.write("-->AOH reset: OK\n") 
      else:                       f.write("-->AOH reset: ERROR\n")
      if (len(self.resetROC)==0): f.write("-->ROC reset: not tested\n")
      elif (self.resetROC[0]):    f.write("-->ROC reset: OK\n") 
      else:                       f.write("-->ROC reset: ERROR\n")
      f.write("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
      f.write("AOH MAP\n")
      f.write("LAYER AOH LASER FEDCHANNEL\n")
      for i in range (1,5):
         for j in range (1,7):
            f.write("L12   "+str(i)+"   "+str(j)+"     "+str(self.aoh[i].fedchannels[j])+'\n')
      for i in range (5,7):
         for j in range (1,7):
            f.write("L3    "+str(i-4)+"   "+str(j)+"     "+str(self.aoh[i].fedchannels[j])+'\n')
      f.write('\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("GROUP L12\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("HUB ADDRESSES\n")
      f.write("hub  (readout)\n")
##    for i in range (len(self.group[1].modules)):
##          if not (self.group[1].modules[i].programming): f.write(str(self.group[1].modules[i].hubaddress)+'\n')
##          elif (self.group[1].modules[i].readout):       f.write(str(self.group[1].modules[i].hubaddress)+"     OK\n")
##          else:   
##          f.write(str(self.group[1].modules[i].hubaddress)+"     ERROR\n")
      for i in range (len(self.group[1].modules)):
         if not (self.group[1].modules[i].programming): f.write(str(self.group[1].modules[i].hubaddress)+'\n')
         elif (self.group[1].modules[i].readout and self.group[1].modules[i].hubaddress in self.group[1].hubaddresses0):       f.write(str(self.group[1].modules[i].hubaddress)+"     OK        OK\n")
         elif (self.group[1].modules[i].readout and not self.group[1].modules[i].hubaddress in self.group[1].hubaddresses0):      f.write(str(self.modules[i].group[1].hubaddress)+"     OK        ERROR\n")
         elif (not self.group[1].modules[i].readout and self.modules[i].group[1].hubaddress in self.group[1].hubaddresses0):      f.write(str(self.group[1].modules[i].hubaddress)+"     ERROR     OK\n")
         elif (not self.group[1].modules[i].readout and not self.group[1].modules[i].hubaddress in self.group[1].hubaddresses0):     f.write(str(self.group[1].modules[i].hubaddress)+"     ERROR     ERROR\n")
      if not(len(self.group[1].modules)==self.group[1].nmodules): f.write("ERROR: "+str(self.group[1].nmodules-len(self.group[1].modules))+" hub addresses missing!!!\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("DELAY25\n")
      f.write("SDA    "+str(self.group[1].delay25.SDA)+'\n')
      f.write("clock  "+str(self.group[1].delay25.clock)+'\n')
      f.write("RDA    "+str(self.group[1].delay25.RDA)+'\n')
      f.write("RCK    "+str(self.group[1].delay25.RCK)+'\n')
      f.write("CTR    "+str(self.group[1].delay25.CTR)+'\n')
      f.write("SDA range "+str(self.group[1].sdarange)+'\n')
      f.write("RCK range ("+str(self.group[1].nmaxrck)+") "+str(self.group[1].rckrange)+'\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("PLL\n")
      f.write("clockphase    "+str(self.group[1].pll.clockphase)+'\n')
      f.write("triggerdelay  "+str(self.group[1].pll.triggerdelay)+'\n')
      f.write('\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("GROUP L3\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("HUB ADDRESSES\n")
      f.write("hub  (readout)\n")
##       for i in range (len(self.group[2].modules)):
##          if not (self.group[2].modules[i].programming): f.write(str(self.group[2].modules[i].hubaddress)+'\n')
##          elif (self.group[2].modules[i].readout):       f.write(str(self.group[2].modules[i].hubaddress)+"     OK\n")
##          else:                                          f.write(str(self.group[2].modules[i].hubaddress)+"     ERROR\n")
      for i in range (len(self.group[2].modules)):
         if not (self.group[2].modules[i].programming): f.write(str(self.group[2].modules[i].hubaddress)+'\n')
         elif (self.group[2].modules[i].readout and self.group[2].modules[i].hubaddress in self.group[2].hubaddresses0):       f.write(str(self.group[2].modules[i].hubaddress)+"     OK        OK\n")
         elif (self.group[2].modules[i].readout and not self.group[2].modules[i].hubaddress in self.group[2].hubaddresses0):      f.write(str(self.group[2].modules[i].hubaddress)+"     OK        ERROR\n")
         elif (not self.group[2].modules[i].readout and self.group[2].modules[i].hubaddress in self.group[2].hubaddresses0):      f.write(str(self.group[2].modules[i].hubaddress)+"     ERROR     OK\n")
         elif (not self.group[2].modules[i].readout and not self.group[2].modules[i].hubaddress in self.group[2].hubaddresses0):     f.write(str(self.group[2].modules[i].hubaddress)+"     ERROR     ERROR\n")
      if not(len(self.group[2].modules)==self.group[2].nmodules): f.write("ERROR: "+str(self.group[2].nmodules-len(self.group[2].modules))+" hub addresses missing!!!\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("DELAY25\n")
      f.write("SDA    "+str(self.group[2].delay25.SDA)+'\n')
      f.write("clock  "+str(self.group[2].delay25.clock)+'\n')
      f.write("RDA    "+str(self.group[2].delay25.RDA)+'\n')
      f.write("RCK    "+str(self.group[2].delay25.RCK)+'\n')
      f.write("CTR    "+str(self.group[2].delay25.CTR)+'\n')
      f.write("SDA range "+str(self.group[1].sdarange)+'\n')
      f.write("RCK range ("+str(self.group[1].nmaxrck)+") "+str(self.group[1].rckrange)+'\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("PLL\n")
      f.write("clockphase    "+str(self.group[2].pll.clockphase)+'\n')
      f.write("triggerdelay  "+str(self.group[2].pll.triggerdelay)+'\n')
      f.close()



   def PrintResults1(self):
      filename=folder+"/Results1_Sector"+self.name+"-"+date.today().isoformat()
      if os.path.exists(filename+".txt"):
         template=filename+"-%03d"
         for n in range(100):
            filename=template%n
            if not os.path.exists(filename+".txt"): break
      f=open(filename+".txt", 'w')
      f.write("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
      f.write("SECTOR"+self.name+'\n')
      f.write("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
      f.write("PIA RESETS"+'\n')
      if (len(self.resetDOH)==0): f.write("-->DOH reset: not tested\n")
      elif (self.resetDOH[0]):    f.write("-->DOH reset: OK\n") 
      else:                       f.write("-->DOH reset: ERROR\n")
      if (len(self.resetAOH)==0): f.write("-->AOH reset: not tested\n")
      elif (self.resetAOH[0]):    f.write("-->AOH reset: OK\n") 
      else:                       f.write("-->AOH reset: ERROR\n")
      if (len(self.resetROC)==0): f.write("-->ROC reset: not tested\n")
      elif (self.resetROC[0]):    f.write("-->ROC reset: OK\n") 
      else:                       f.write("-->ROC reset: ERROR\n")
      f.write("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
      f.write("AOH MAP\n")
      f.write("LAYER AOH LASER FEDCHANNEL\n")
      for i in range (1,5):
         for j in range (1,7):
            f.write("L12   "+str(i)+"   "+str(j)+"     "+str(self.aoh[i].fedchannels[j])+'\n')
      for i in range (5,7):
         for j in range (1,7):
            f.write("L3    "+str(i-4)+"   "+str(j)+"     "+str(self.aoh[i].fedchannels[j])+'\n')
      f.write('\n')
      f.write("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("GROUP L12\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      if (len(self.group[1].dohfiber)==0): f.write("-->DOH fiber test: not tested\n")
      elif (self.group[1].dohfiber[0]):    f.write("-->DOH fiber test: OK\n") 
      else:                       f.write("-->DOH fiber test: ERROR\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("DELAY25\n")
      f.write("SDA    "+str(self.group[1].delay25.SDA)+'\n')
      f.write("clock  "+str(self.group[1].delay25.clock)+'\n')
      f.write("RDA    "+str(self.group[1].delay25.RDA)+'\n')
      f.write("RCK    "+str(self.group[1].delay25.RCK)+'\n')
      f.write("CTR    "+str(self.group[1].delay25.CTR)+'\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("PLL\n")
      f.write("clockphase    "+str(self.group[1].pll.clockphase)+'\n')
      f.write("triggerdelay  "+str(self.group[1].pll.triggerdelay)+'\n')
      f.write('\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("GROUP L3\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      if (len(self.group[2].dohfiber)==0): f.write("-->DOH fiber test: not tested\n")
      elif (self.group[2].dohfiber[0]):    f.write("-->DOH fiber test: OK\n") 
      else:                       f.write("-->DOH fiber test: ERROR\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("DELAY25\n")
      f.write("SDA    "+str(self.group[2].delay25.SDA)+'\n')
      f.write("clock  "+str(self.group[2].delay25.clock)+'\n')
      f.write("RDA    "+str(self.group[2].delay25.RDA)+'\n')
      f.write("RCK    "+str(self.group[2].delay25.RCK)+'\n')
      f.write("CTR    "+str(self.group[2].delay25.CTR)+'\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("PLL\n")
      f.write("clockphase    "+str(self.group[2].pll.clockphase)+'\n')
      f.write("triggerdelay  "+str(self.group[2].pll.triggerdelay)+'\n')
      f.close()

###--------- end of class SECTOR ------------











      
#################
class GROUP:
   def __init__(self, fed, fedslot, ccu, pxfec, caen, name, channel,log):
      self.fed=fed                            # fec access (SimpleCommand)
      self.fedslot=fedslot                    # fed slot number
      self.ccu=ccu                            # ccu access (SimpleCommand)
      self.pxfec=pxfec                        # pxfec access (SimpleCommand)
      self.caen=caen                          # caen access (SimpleCommand) 
      self.name=name                          # 
      self.channel=channel                    # 0x11 (L12), 0x13 (L3)
      self.nmodules=0                         # number of modules (read in from file hubaddresses.txt)
      self.hubaddresses0=[]                   # hubaddresses (design)
      self.modules=[]                         # modules
      self.delay25=DELAY25(ccu,name,channel,log)  # delay25
      self.pll=PLL(ccu,name,channel,log)          # pll
      self.doh=DOH(ccu,name,channel,log)          # doh
      self.sdarange=[]                        # valid SDA range
      self.rckrange=[]                        # valid SDA range
      self.nmaxrck=0                          # maximum number of modules at same RCK
      self.dohfiber=[]                        # result of test of DOH fiber
      self.log=log                            # message logger
     
   def Init(self):
      self.InitFEC()
      if self.pxFEC: self.InitpxFEC()
      self.InitFED()
      if self.caen: self.InitCAEN()

   def InitMB(self):
      if not self.ccu: return
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("doh setall 1 0 1 48 0 48").readlines()    # g0 g1 g2 b0 b1 b2
      self.ccu.send("pll reset").readlines()       #
      self.ccu.send("pll set  8").readlines()      # make restart pll
      self.ccu.send("pll setall 0 0").readlines()  # clk, trigger phase (what about CTR delay?)
      self.ccu.send("delay25 setall %d %d %d %d %d"%(10|0x40,10|0x40,48|0x40,0|0x40,0|0x40)).readlines()  # RCK,CTR,SDA,RDA,CLK + enable bits
      
   def InitFEC(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      print "Group "+self.name+": FEC reset"
      self.delay25.SendValues()
      self.pll.SendValues()
      self.doh.SendValues()

   def InitpxFEC(self):
      self.pxfec.send("cn "+self.name).readlines()
      print "Group "+self.name+": Init pxFEC"

   def resetHubs(self):
      self.pxfec.send("cn "+self.name+" reset tbm").readlines()
      self.pxfec.send("cn "+self.name+" reset roc").readlines()
      
      
   def InitFED(self):
      #self.fed.send("fed %d"%self.fedslot).readlines()
      self.fed.send("fed %s"%self.name[:3]).readlines()
      self.fed.send("reset").readlines()
      print "Group "+self.name+": FED reset"

   def InitCAEN(self):
      print "init CAEN"
      self.caen.send("group "+self.name).readlines()
      cnt=0
      while not (self.IsCAENOn()):
         cnt=cnt+1
         print "Group "+self.name+": switch CAEN On"
         self.caen.send("pon").readlines()
         time.sleep(1.0)
         if (cnt>5):
            print "Group "+self.name+": CAEN: Error when turning power on"
            break
      print "Group "+self.name+" CAEN: power On"
            
   def pon(self):
      print "switch on LV group",self.name
      self.caen.send("group "+self.name).readlines()
      cnt=0
      while not (self.IsCAENOn()):
         cnt=cnt+1
         print "Group "+self.name+": switch CAEN On"
         self.caen.send("pon").readlines()
         time.sleep(1.0)
         if (cnt>5):
            print "Group "+self.name+": CAEN: Error when turning power on"
            break
      print "Group "+self.name+" CAEN: power On"
            
   def poff(self):
      print "switch off LV group",self.name
      self.caen.send("group "+self.name).readlines()
      cnt=0
      while (self.IsCAENOn()):
         cnt=cnt+1
         print "Group "+self.name+": switch CAEN On"
         self.caen.send("poff").readlines()
         time.sleep(1.0)
         if (cnt>5):
            print "Group "+self.name+": CAEN: Error when turning power on"
            break
      print "Group "+self.name+" CAEN: power On"
            
   def IsCAENOn(self):
      for t in range(5):
         self.caen.send("group "+self.name).readlines()
         pw = self.caen.query("get pw").strip()
         if pw=="On": return True
         if pw=="Off": return False
         print "\n no response from caen, retrying"
         time.sleep(1.0)
      print "no response from CAEN"
      sys.exit(1)

   def VerifyI2CProgramming_noclock(self):
      self.delay25.VerifyProgramming()
      self.pll.VerifyProgramming()
    

   def VerifyI2CProgramming(self):
      self.delay25.VerifyProgramming()
      self.pll.VerifyProgramming()
      self.doh.VerifyProgramming()

   def TestDOHFiber(self):
      status = self.pxfec.query("cn "+self.name+" test fiber")
      status=status.split()[-1].strip()
      #print "TestDOHFiber(): \t", status
      msg="TestDOHFiber  %s %s"%(self.name,status)
      if (status=="ok"):
         self.dohfiber.append(True)
         self.log.ok(msg)
      else:
         self.log.error(msg)
           
   #get the register of the i2c devices (output file is created) 
   def GetCCURegister(self,verbose=False):
      filename=folder+"/CCURegister_Group"+self.name+"-"+date.today().isoformat()
      if os.path.exists(filename+".txt"):
         template=filename+"-%03d"
         for n in range(100):
            filename=template%n
            if not os.path.exists(filename+".txt"): break
      f=open(filename+".txt", 'w')

      self.InitFEC()
      self.ccu.send("displayregister")
      for l in self.ccu.readline():
         if l!="":
            if (verbose): print l
            f.write(l)
            f.write('\n')

      f.close()
      print "GetCCURegister(): "+filename+".txt written"

   def Geti2cValues(self):
      v=[]
      v.append(self.delay25.GetValues())
      v.append(self.pll.GetValues())
      v.append(self.doh.GetValues())
      print v
      return v
   
   #find the valid programming range by looking at the response of the power supply
   def GetSDARange(self,verbose=True):
      self.InitFEC()
      self.InitCAEN()
      self.pxfec.send("cn %s"%self.name).readlines()
      v = self.Geti2cValues()
      print v
      good=[]
      for d2 in range (0,63): #scan SDA values
         self.ccu.send("delay25 s d2 %d"%(d2)).readlines()
         if (verbose): print "SDA: ", d2,
         self.pxfec.send("cn reset roc").readlines()                #reset roc
         self.pxfec.send("module 0:31 roc 0:15 Vdig 0").readlines() #avoid high digital currents
         self.pxfec.send("module 0:31 roc 0:15 Vsf 0").readlines()  #avoid high digital currents
         self.pxfec.send("module 0:31 roc 0:15 Vana 0").readlines() #set Vana=0
         sleep(3)
         ia0=self.caen.fquery("get ia")                                 
         self.pxfec.send("module 0:31 roc 0:15 Vana 120").readlines()        #set Vana=120
         sleep(3)
         ia1=self.caen.fquery("get ia")
         if (verbose): print "IA= ",ia0, ia1
         if ia1-ia0>0.5 : good.append(d2)
      print "SDA range: ", good
      self.sdarange=good
      return good
   
   #choose SDA value in SDA range
   def FindSDA(self):
      value=0
      values=self.GetSDARange()
      if not (values):
         print "FindSDA(): no module responded!"
         self.log.error("FindSDA(): no module responded!")
         sys.exit(1)
      min1=values[0]
      max1=-1
      min2=-1
      max2=values[len(values)-1]
      for i in range(len(values)-1):
         if ( (values[i+1]-values[i])> 2):
            max1=values[i]
            min2=values[i+1]
            break
      print min1, max1, min2, max2
      if not(max1==-1):
         if ((max1-min1)>(max2-min2)): value=(min1+max1)/2
         else: value=(min2+max2)/2
      else: value=(max2+min1)/2
      print "value ", value
      self.log.info("SDA = %d"%(value))
      self.delay25.SDA=value
      self.SetSDA(value)

   def SetSDA(self,value): 
      self.InitFEC()
      self.delay25.SDA=value
      self.delay25.SendSDA(value)

   #read the design hubaddresses from a file
   def FillHubDesignHubAddresses(self):
      f=open('hubaddresses.txt')	
      for line in f.readlines():
         if self.name in line:
            for i in range(1,len(line.split())):
               self.hubaddresses0.append(line.split()[i])
      self.nmodules=len(self.hubaddresses0)
      print self.hubaddresses0
      print self.nmodules

   #add a module (hub address) to the description of the group
   def AddModule(self,hub):
      self.modules.append(MODULE(self.pxfec,self.name,hub,False,False,False,self.log))
 
   #add a list of modules (hub addresses) to the description of the group
   def AddModules(self,list):
      for i in range(len(list)):
         self.modules.append(MODULE(self.pxfec,self.name,list[i],False,False,False,self.log))

   #find hubaddresses:
   #1) scan RCK and check the answer of the pxFEC
   #2) if less than nmodules answer: check programming by looking at the response power supply
   def GetHubAddresses(self,verbose=True):
      print "GetHubAddresses():"
      noaddresses=[]
      hubreadout=self.GetHubAddressesReadout() #1)
      for i in range(32):
         if i in hubreadout:
            if (verbose): print "hub ", i, "readback"
            ii="%d"%i
            if ii not in self.hubaddresses0:
               print "WRONG HUB ADDRESS!"
               self.modules.append(MODULE(self.pxfec,self.name,i,True,True,False,self.log))
            else:
               self.modules.append(MODULE(self.pxfec,self.name,i,True,True,True,self.log))
         else: noaddresses.append(i)
      if (len(self.modules)==self.nmodules):
         print "hubaddresses for all modules found"
         self.log.ok("hubaddresses for all modules found")
         print "hubs: ",
         for i in range(len(self.modules)):
            print self.modules[i].hubaddress," ",
            print " "
      else: #2)
         for i in noaddresses:
            if (verbose): print "no hub ", i
            if (self.IsHubProgrammable(i)):
               if (verbose): print "hub ", i, "programmable"
               ii="%d"%i
               if ii not in self.hubaddresses0:
                  print "WRONG HUB ADDRESS!"
                  self.modules.append(MODULE(self.pxfec,self.name,i,True,False,False,self.log))
               else:
                  self.modules.append(MODULE(self.pxfec,self.name,i,True,False,True,self.log))
                  
         if (len(self.modules)==self.nmodules):
            print "hubaddresses for all modules found"
            self.log.ok("hubaddresses for all modules found")
            print "hubs: ",
            for i in range(len(self.modules)):
               print self.modules[i].hubaddress
               if not (self.modules[i].readout): "no readback, only programming!!!"
         else:
            print "NOT all hubaddresses found!!!!"
            self.log.error("NOT all hubaddresses found!!!!")
            print self.nmodules-len(self.modules), " hubaddresses missing!!!"
            self.log.info( "%d hubaddresses missing!!!"%(self.nmodules-len(self.modules)) )
            self.log.info(str([self.modules[i].hubaddress for i in range(len(self.modules))]))
            print "hubs: ", 
            for i in range(len(self.modules)):
               print self.modules[i].hubaddress," ",
               print " "

   #find hubaddresses:
   #needs result of RCK scan as input!
   #1) scan RCK and check the answer of the pxFEC
   #2) if less than nmodules answer: check programming by looking at the response power supply
   def GetHubAddresses2(self,map,verbose=True):
      noaddresses=[]
      hubreadout=self.GetHubAddressesReadout2(map) #1)
      for i in range(32):
         if i in hubreadout:
            if (verbose): print "hub ", i, "readback"
            #print self.hubaddresses0
            ii="%d"%i
            if not ii in self.hubaddresses0:
               #print "WRONG HUB ADDRESS!"
               self.modules.append(MODULE(self.pxfec,self.name,i,True,True,False,self.log))
            else:
               self.modules.append(MODULE(self.pxfec,self.name,i,True,True,True,self.log)) 
         else: noaddresses.append(i)
      if (len(self.modules)==self.nmodules):
         #print "hubaddresses for all modules found"
         self.log.ok("hubaddresses for all modules found in RDA scan")
         print "hubs: ",
         for i in range(len(self.modules)):
            print self.modules[i].hubaddress," ",
            print " "
      else: #2)
         #print "not all hubaddresses readback --> test programming"
         self.log.error("not all hubaddresses readback --> test programming")
         for i in noaddresses:
            if (self.IsHubProgrammable(i)):
               if (verbose): print "hub ", i, "programmable"
               ii="%d"%i
               if ii not in self.hubaddresses0:
                  print "WRONG HUB ADDRESS!"
                  self.modules.append(MODULE(self.pxfec,self.name,i,True,False,False,self.log))
               else:
                  self.modules.append(MODULE(self.pxfec,self.name,i,True,False,True,self.log))
               
         if (len(self.modules)==self.nmodules):
            print "hubaddresses for all modules found"
            self.log.ok("hubaddresses for all modules found with programming")
            print "hubs: ",
            for i in range(len(self.modules)):
               print self.modules[i].hubaddress
               if not (self.modules[i].readout): "no readback, only programming!!!"
         else:
            print "NOT all hubaddresses found!!!!"
            self.log.error("NOT all hubaddresses found with programming!!!!")
            print self.nmodules-len(self.modules), " hubaddresses missing!!!"
            print "hubs: ", 
            for i in range(len(self.modules)):
               print self.modules[i].hubaddress
            yesno=raw_input("Do you want an SDA scan (y/n)? ")
            if yesno=='y':
               self.FindSDA()

   #get list of modules that replied in RCK scan
   def GetHubAddressesReadout(self,verbose=False):
      hubs=[]
      map=self.ScanRCK()
      if (verbose): print "\nlist of modules: "
      if (map):
         for i in range(32) :
            for j in range(63):
               if i in map[j]:
                  hubs.append(i)
                  break
      if (verbose): print "hubs ", hubs
      return hubs

   #get list of modules that replied in RCK scan
   #needs result of RCK scan as input!
   def GetHubAddressesReadout2(self,map,verbose=False):
      hubs=[]
      if (verbose): print "\nlist of modules: "
      if (map):
         for i in range(32) :
            for j in range(63):
               if i in map[j]:
                  hubs.append(i)
                  break
      if (verbose): print "hubs ", hubs
      return hubs

   #check programming of modules by looking at the response of the power supply
   def IsHubProgrammable(self,hub,verbose=True):
      self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      self.InitCAEN()
      self.pxfec.send("module %d roc 0:15 Vana 0"%(hub)).readlines()     #set Vana=0
      sleep(3)
      ia0=self.caen.fquery("get ia")
      self.pxfec.send("module %d roc 0:15 Vana 100"%(hub)).readlines()   #set Vana=100
      sleep(3)
      ia1=self.caen.fquery("get ia")
      if (verbose):
         print "hub ", hub, ":",
         print ia0, ia1,
      if ia1-ia0>0.05 :
         if (verbose): print "programming ok"
         return True
      else:
         if (verbose): print "no programming"
         return False

   #compare hubaddresses to design hubaddresses
   def VerifyHubAddresses(self):
      hubdesign=self.hubaddresses0
      hubdesign.sort()
      hubfound=[]
      for i in len(self.modules):
         hubfound.append(self.modules[i].hubaddress)
      hubfound.sort()
      if (hubdesign==hubfound):
         print "all hub addresses agree"
      else:
         for i in range(len(hubfound)):
            if i not in hubdesign:
               print i, "not in design hubaddresses"
         for i in range(len(hubdesign)):
            if i not in hubfound:
               print i, "not found"
         

   #RCK range for all modules 
   def FindRCK(self):
      map=self.ScanRCK()
      if (map):
         length=[]
         for i in range(63):
            length.append(len(map[i]))
         rck=[]
         for i in range(63):
            if(len(map[i])==max(length)): rck.append(i)
         print rck
         self.rckrange=rck
         self.nmaxrck= max(length)
      else:
         print "FindRCK(): no results from ScanRCK()!"
         self.rckrange=[]
         self.nmaxrck= 0
         
   #RCK range for all modules 
   #needs result of RCK scan as input!
   def FindRCK2(self,map):
      if (map):
         length=[]
         for i in range(63):
            length.append(len(map[i]))
         #print "\nmaximum number of modules at one RCK setting:", max(length)
      
         rck=[]
         for i in range(63):
            if(len(map[i])==max(length)): rck.append(i)
         print rck
         self.rckrange=rck
         self.nmaxrck= max(length)
      else:
         print "FindRCK2(): no results from ScanRCK()!"
         self.rckrange=[]
         self.nmaxrck= 0

   #for testing
   def ScanRCKDummy(self):
      map={}
      for d0 in range(0,63):
         a=[0,1,4,5,6,7,8,9,11,12,13,14]
         map[d0]=a
      return map

   def ScanRCK(self):
      self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      for d0 in range(0,63):                                  #scan RCK 
         self.ccu.send("delay25 set d0 %d"%(d0)).readlines()
         print "RCK = %2d"%(d0),
         a=self.pxfec.aquery("cn scan hubs")                  #check status
         print a
         map[d0]=a
      self.ccu.send("delay25 set d0 0").readlines()
      return map
   
   def SetRCK(self,value):
      self.InitFEC()
      self.delay25.RCK=value
      self.delay25.SendRCK(value)

   def PlotRCKRange(self):
      v=self.Geti2cValues()
      map=self.ScanRCK()
      map={}
      gStyle.SetOptStat(0)
      gStyle.SetCanvasColor(0)
      gStyle.SetTitleFillColor(0)
      title="RCK "+self.name+"  (Delay25["+str(v[0][0])+" "+str(v[0][1])+" "+str(v[0][2])+" "+str(v[0][3])+" "+str(v[0][4])+"] PLL["+str(v[1][0])+" "+str(v[1][1])+"] DOH["+str(v[2][0])+" "+str(v[2][1])+" "+str(v[2][2])+" "+str(v[2][3])+" "+str(v[2][4])+" "+str(v[2][5])+"])"
      h=TH2D( 'h'+title, title, 63,0,63,32,0,32 )
      if (map):
         for d0 in range(63):
            for i in range(32):
               if i in map[d0] :
                  h.Fill(d0,i)
      else:
         print "PlotRCKRange(): no results from ScanRCK()!"
         return
         
      c1.cd(1)
      c1 = TCanvas( 'c1', '', 0, 0, 600, 600 )
      c1.Divide(1,1)
      h.Draw("BOX")
      filename=folder+"/RCK_Group"+self.name+"-"+date.today().isoformat()
      if os.path.exists(filename+".gif"):
         template=filename+"-%03d"
         for n in range(100):
            filename=template%n
            if not os.path.exists(filename+".gif"): break
      c1.Print(filename+".gif")
      raw_input("hit return to continue")

   def PlotRCKRange2(self,map,v):
      gStyle.SetOptStat(0)
      gStyle.SetCanvasColor(0)
      gStyle.SetTitleFillColor(0)
      title="RCK "+self.name+"  (Delay25["+str(v[0][0])+" "+str(v[0][1])+" "+str(v[0][2])+" "+str(v[0][3])+" "+str(v[0][4])+"] PLL["+str(v[1][0])+" "+str(v[1][1])+"] DOH["+str(v[2][0])+" "+str(v[2][1])+" "+str(v[2][2])+" "+str(v[2][3])+" "+str(v[2][4])+" "+str(v[2][5])+"])"
      h=TH2D( 'h'+title, title, 63,0,63,32,0,32 )
      if (map):
         for d0 in range(63):
            for i in range(32):
               if i in map[d0] :
                  h.Fill(d0,i)
      else:
         print "PlotRCKRange(): no results from ScanRCK()!"
         return
      c1 = TCanvas( 'c1', '', 0, 0, 600, 600 )
      c1.Divide(1,1)
      c1.cd(1)
      h.Draw("BOX")
      filename=folder+"/RCK_Group"+self.name+"-"+date.today().isoformat()
      if os.path.exists(filename+".gif"):
         template=filename+"-%03d"
         for n in range(100):
            filename=template%n
            if not os.path.exists(filename+".gif"): break
      c1.Print(filename+".gif")
      #raw_input("hit return to continue")
      return c1
 
   #for testing
   def ScanSDARDADummy(self):
      map={}
      for d3 in range(0,63):
         for d2 in range(0,63):
            a=[0,1,4,5,6,7,8,9,11,12,13,14]
            map[d2,d3]=a
      return map

   def ScanSDARDA(self):
      self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      for d3 in range(0,63):                                    #scan RDA 
         self.ccu.send("delay25 set d3 %d"%(d3)).readlines()
         for d2 in range(0,63):                                 #scan SDA
            self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
            print "SDA = ", d2, "RDA = ", d3
            self.pxfec.send("cn reset roc").readlines()         #reset ROC
            a=self.pxfec.aquery("cn scan hubs")                 #check status
            print a
            map[d2,d3]=a
      self.ccu.send("delay25 set d2 48").readlines()
      self.ccu.send("delay25 set d3 0").readlines()
      return map

   def ScanRDARCK(self):
      self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      for d3 in range(0,63):                                    #scan RDA
         self.ccu.send("delay25 set d3 %d"%(d3)).readlines()
         for d0 in range(0,63):
            self.ccu.send("delay25 set d0 %d"%(d0)).readlines() #scan RCK
            print "RCK = ", d0, "RDA = ", d3
            self.pxfec.send("cn reset roc").readlines()         #reset ROC
            a=self.pxfec.aquery("cn scan hubs")                 #check status
            print a
            map[d3,d0]=a
      self.ccu.send("delay25 set d0 0").readlines()
      self.ccu.send("delay25 set d3 0").readlines()
      return map

   def ScanSDARCK(self):
      self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      for d2 in range(0,63):                                    #scan SDA
         self.ccu.send("delay25 set d2 %d"%(d2)).readlines()      
         for d0 in range(0,63):                                 #scan RCK
            self.ccu.send("delay25 set d0 %d"%(d0)).readlines()
            print "RCK = ", d0, "SDA = ", d2
            self.pxfec.send("cn reset roc").readlines()         #reset ROC
            a=self.pxfec.aquery("cn scan hubs")                 #check status
            print a
            map[d2,d0]=a
      self.ccu.send("delay25 set d2 48").readlines()
      self.ccu.send("delay25 set d0 0").readlines()
      return map

   def PlotSDARDA(self):
      if (self.nmodules==0):
         print "PlotSDARDA(): Scan SDARDA for which modules?"
         return
      v=self.Geti2cValues()
      map=self.ScanSDARDA()
      self.Plot2D("SDA","RDA",map,v)

   def PlotRDARCK(self):
      if (self.nmodules==0):
         print "PlotRDARCK(): Scan RDARCK for which modules?"
         return
      v=self.Geti2cValues()
      map=self.ScanRDARCK()
      self.Plot2D("RDA","RCK",map,v)

   def PlotSDARCK(self):
      if (self.nmodules==0):
         print "PlotSDARCK(): Scan SDARCK for which modules?"
         return
      v=self.Geti2cValues()
      map=self.ScanSDARCK()
      self.Plot2D("SDA","RCK",map,v)

   def Plot2D(self,namex,namey,map,v):
      gStyle.SetOptStat(0)
      gStyle.SetCanvasColor(0)
      gStyle.SetTitleFillColor(0)
      if (map):
         c1 = TCanvas( 'c1', '', 0, 0, 1000, 1000 )
         c1.Divide(self.nmodules/4,4)
         h=[None]
         cn=0
         for m in self.modules:
            cn=cn+1
            i=m.hubaddress
            title="hub"+str(i)
            ctitle=title+" "+self.name+" "+namex+namey+"  (Delay25["+str(v[0][0])+" "+str(v[0][1])+" "+str(v[0][2])+" "+str(v[0][3])+" "+str(v[0][4])+"] PLL["+str(v[1][0])+" "+str(v[1][1])+"] DOH["+str(v[2][0])+" "+str(v[2][1])+" "+str(v[2][2])+" "+str(v[2][3])+" "+str(v[2][4])+" "+str(v[2][5])+"])"
            c1.cd(cn)
            h.append(TH2D( 'h'+title, ctitle, 63,0,63,63,0,63 ))
            h[cn].SetXTitle(namex)
            h[cn].SetYTitle(namey)
            for j in range(63):
               for k in range(63):
                  if i in map[j,k] :
                     h[cn].Fill(j,k)
            h[cn].Draw("BOX")
         filename=folder+"/"+namex+namey+"_Group"+self.name+"-"+date.today().isoformat()
         if os.path.exists(filename+".gif"):
            template=filename+"-%03d"
            for n in range(100):
               filename=template%n
               if not os.path.exists(filename+".gif"): break
         c1.Print(filename+".ps")
         raw_input("hit return to continue")
      else:
         print "Plot2D(): no results from Scan"+namex+namey+"()!"

   #for testing
   def ScanRCKCommandsDummy(self):
      map={}
      for d0 in range(0,63):
         map[d0,1]=[0,1,4,5,6,7,8,9,11,12,13,14]
         map[d0,2]=[0,1,4,5,6,7,8,9,11,12,13,14]
         map[d0,3]=[0,1,4,5,6,7,8,9,11,12,13,14]
         map[d0,4]=[0,1,4,5,6,7,8,9,11,12,13,14]
         map[d0,5]=[0,1,4,5,6,7,8,9,11,12,13,14]
      return map

   #get the valid range (RCK) for the commands: set DAC, TBM read, TBM write, rocinit, roctrimload  
   def ScanRCKCommands(self):
      self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      for d0 in range(0,63):                                    #scan RCK
         self.ccu.send("delay25 set d0 %d"%(d0)).readlines()
         print "RCK = %d"%(d0)                                 
         a=self.pxfec.aquery("cn scanhubs roc")                 #DAC
         print a
         b=self.pxfec.aquery("cn scanhubs tbmread")             #TBM read
         print b
         c=self.pxfec.aquery("cn scanhubs tbmwrite")            #TBM write
         print c
         d=self.pxfec.aquery("cn scanhubs rocinit")             #rocinit
         print d
         e=self.pxfec.aquery("cn scanhubs roctrimload")         #roctrimload
         print e
         map[d0,1]=a
         map[d0,2]=b
         map[d0,3]=c
         map[d0,4]=d
         map[d0,5]=e
      return map

   def PlotCommandRange(self):
      v=self.Geti2cValues()
      map=self.ScanRCKCommands()
      if (map):
         gStyle.SetOptStat(0)
         gStyle.SetCanvasColor(0)
         gStyle.SetTitleFillColor(0)
         c1 = TCanvas( 'c1', '', 0, 0, 1000, 1000 )
         c1.Divide(self.nmodules/4,4);
         h=[None]
         cn=0
         for m in self.modules:
            cn=cn+1
            i=m.hubaddress
            title="hub"+str(i)
            ctitle=title+" "+self.name+" RCK  (Delay25["+str(v[0][0])+" "+str(v[0][1])+" "+str(v[0][2])+" "+str(v[0][3])+" "+str(v[0][4])+"] PLL["+str(v[1][0])+" "+str(v[1][1])+"] DOH["+str(v[2][0])+" "+str(v[2][1])+" "+str(v[2][2])+" "+str(v[2][3])+" "+str(v[2][4])+" "+str(v[2][5])+"])"
            c1.cd(cn)
            h.append(TH2D( 'h'+title, ctitle, 63,0,63,7,0,7 ))
            h[cn].SetXTitle("RCK")
            h[cn].GetYaxis().SetBinLabel(2,"ROC (progDAQ)");
            h[cn].GetYaxis().SetBinLabel(3,"TBM read");
            h[cn].GetYaxis().SetBinLabel(4,"TBM write");
            h[cn].GetYaxis().SetBinLabel(5,"rocinit");
            h[cn].GetYaxis().SetBinLabel(6,"roctrimload");
            for j in range(63):
               for k in range(1,6):
                  if i in map[j,k] : h[cn].Fill(j,k)
            h[cn].Draw("BOX")
         filename=folder+"/CommandRange_Group"+self.name+"-"+date.today().isoformat()
         if os.path.exists(filename+".gif"):
            template=filename+"-%03d"
            for n in range(100):
               filename=template%n
               if not os.path.exists(filename+".gif"): break
         c1.Print(filename+".gif")
         raw_input("hit return to continue")
      else:
         print "PlotCommandRange(): no results from ScanRCKCommands()!"

   def PrintResults(self):
      filename=folder+"/Results_Group"+self.name+"-"+date.today().isoformat()
      if os.path.exists(filename+".txt"):
         template=filename+"-%03d"
         for n in range(100):
            filename=template%n
            if not os.path.exists(filename+".txt"): break
      f=open(filename+".txt", 'w')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("GROUP "+self.name+'\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("HUB ADDRESSES\n")
      f.write("hub   readout   address\n")
      for i in range (len(self.modules)):
         hubadr="%d"%self.modules[i].hubaddress
         if not (self.modules[i].programming): f.write(str(self.modules[i].hubaddress)+'\n')
         elif (self.modules[i].readout and hubadr in self.hubaddresses0):       f.write(str(self.modules[i].hubaddress)+"     OK        OK\n")
         elif (self.modules[i].readout and not hubadr in self.hubaddresses0):      f.write(str(self.modules[i].hubaddress)+"     OK        ERROR\n")
         elif (not self.modules[i].readout and hubadr in self.hubaddresses0):      f.write(str(self.modules[i].hubaddress)+"     ERROR     OK\n")
         elif (not self.modules[i].readout and not hubadr in self.hubaddresses0):     f.write(str(self.modules[i].hubaddress)+"     ERROR     ERROR\n")
        
      if not(len(self.modules)==self.nmodules):f.write("ERROR: "+str(self.nmodules-len(self.modules))+" hub addresses missing!!!\n")
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("DELAY25\n")
      f.write("SDA    "+str(self.delay25.SDA)+'\n')
      f.write("clock  "+str(self.delay25.clock)+'\n')
      f.write("RDA    "+str(self.delay25.RDA)+'\n')
      f.write("RCK    "+str(self.delay25.RCK)+'\n')
      f.write("CTR    "+str(self.delay25.CTR)+'\n')
      f.write("SDA range "+str(self.sdarange)+'\n')
      f.write("RCK range ("+str(self.nmaxrck)+") "+str(self.rckrange)+'\n')
      f.write("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
      f.write("PLL\n")
      f.write("clockphase    "+str(self.pll.clockphase)+'\n')
      f.write("triggerdelay  "+str(self.pll.triggerdelay)+'\n')
      
      f.close()     
 

########################################
class DELAY25:
   def __init__(self, ccu, name, channel,log=None):
      self.ccu=ccu                # ccu access (SimpleCommand)
      self.name=name              # group name
      self.channel=channel        # channel
      self.RCK=0                  # RCK
      self.CTR=10                 # CTR
      self.SDA=48                 # SDA
      self.RDA=0                  # RDA
      self.clock=0                # clock
      self.programming=False      # i2c programming
      self.log=log                # message logger

   def VerifyProgramming(self):
      if ( self.SendSDA(1) and self.SendRCK(2) and self.SendCTR(3) and self.SendRDA(4) and self.Sendclock(5) ):
         #print "Group "+self.name+":\tDELAY25: \tprogramming OK"
         if self.log: self.log.ok("Group "+self.name+":\tDELAY25: \tprogramming OK")
         self.programming=True
      else:
         #print "Group "+self.name+":\tDELAY25: \tERROR in programming"
         if self.log: self.log.error("Group "+self.name+":\tDELAY25: \tERROR in programming")

   def SendValues(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("delay25 setall %d %d %d %d %d"%(self.RCK,self.CTR,self.SDA,self.RDA,self.clock)).readlines()
     
   def SendSDA(self,value):
      #set CCU
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("delay25 s d2 %d"%(value)).readlines()
      verify=self.GetSDA()
      if not verify==value:
         #print "verification of delay25 SDA setting failed: wrote ",value," got ",verify
         self.log.error("verification of delay25 SDA setting failed: wrote %d  , got %d"%(value,verify))
         return False
      else:
         return True
    
   def SendRCK(self,value):
      #set CCU
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("delay25 s d0 %d"%(value)).readlines()
      verify=self.GetRCK()
      if not verify==value:
         #print "verification of delay25 RCK setting failed: wrote ",value," got ",verify
         if self.log: self.log.error("verification of delay25 RCK setting failed: wrote %d  got  %d"%(value,verify))
         return False
      else:
         return True
    
   def SendCTR(self,value):
      #set CCU
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("delay25 s d1 %d"%(value)).readlines()
      verify=self.GetCTR()
      if not verify==value:
         #print "verification of delay25 CTR setting failed: wrote ",value," got ",verify
         if self.log: self.log.error("verification of delay25 CTR setting failed: wrote %d  got  %d"%(value,verify))
         return False
      else:
         return True
    
   def SendRDA(self,value):
      #set CCU
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("delay25 s d3 %d"%(value)).readlines()
      verify=self.GetRDA()
      if not verify==value:
         print "verification of delay25 RDA setting failed: wrote ",value," got ",verify
         if self.log: self.log.error("verification of delay25 RDA setting failed: wrote %d  got  %d"%(value,verify))
         return False
      else:
         return True
    
   def Sendclock(self,value):
      #set CCU
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("delay25 s d4 %d"%(value)).readlines()
      verify=self.Getclock()
      if not verify==value:
         print "verification of delay25 clock setting failed: wrote ",value," got ",verify
         if self.log: self.log.error("verification of delay25 clock setting failed: wrote %d  got  %d"%(value,verify))
         return False
      else:
         return True
    
   def GetCR0(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      query="delay25 read"
      regexp="-->CR0    : %s \("
      r=self.ccu.query(query,regexp)
      return r

   def GetRCK(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      query="delay25 read"
      regexp="-->Delay0 : %d"
      r=self.ccu.iquery(query,regexp)
      #print "Delay25: RCK = ", r
      return r

   def GetCTR(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      query="delay25 read"
      regexp="-->Delay1 : %d"
      r=self.ccu.iquery(query,regexp)
      #print "Delay25: CTR = ", r
      return r

   def GetSDA(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      query="delay25 read"
      regexp="-->Delay2 : %d"
      r=self.ccu.iquery(query,regexp)
      #print "Delay25: SDA = ", r
      return r

   def GetRDA(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      query="delay25 read"
      regexp="-->Delay3 : %d"
      r=self.ccu.iquery(query,regexp)
      #print "Delay25: RDA = ", r
      return r

   def Getclock(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      query="delay25 read"
      regexp="-->Delay4 : %d"
      r=self.ccu.iquery(query,regexp)
      #print "Delay25: clock = ", r
      return r

   def GetValues(self):
      value=[]
      value.append(self.GetRCK())
      value.append(self.GetCTR())
      value.append(self.GetSDA())
      value.append(self.GetRDA())
      value.append(self.Getclock())
      return value
      
   def Status(self):
      value=self.GetCR0()
      if (value=="0x0"): print "Delay25: not enabled"
      elif (value): print "Delay25 OK"
      else: print "Delay25: no response"


########################################
class PLL:
   def __init__(self, ccu, name, channel,log=None):
      self.ccu=ccu                # ccu access (SimpleCommand)
      self.channel=channel        # channel
      self.name=name              # name
      self.clockphase=0           # clockphase
      self.triggerdelay=2         # triggerdelay
      self.programming=False      # i2c programming
      self.log=log                # message logger
      
   def VerifyProgramming(self):
      if ( self.SendClockPhase(1) and self.SendTriggerDelay(2) ):
         #print "Group "+self.name+":\tPLL: \t\tprogramming OK"
         if self.log: self.log.ok("Group "+self.name+":\tPLL: \t\tprogramming OK")
         self.programming=True
      else:
         #print "Group "+self.name+":\tPLL: \t\tERROR in programming"
         if self.log: self.log.error("Group "+self.name+":\tPLL: \t\tERROR in programming")
     
   def SendValues(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("pll setall %d %d"%(self.clockphase,self.triggerdelay)).readlines()

   def SendClockPhase(self,value):
      #set CCU
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("pll s clk %d"%(value)).readlines()
      verify=self.GetClockPhase()
      if not verify==value:
         print "verification of PLL clock phase setting failed: wrote ",value," got ",verify
         if self.log: self.log.error("verification of PLL clock phase setting failed: wrote %d   got %d"%(value,verify))
         return False
      else:
         return True

   def SendTriggerDelay(self,value):
      #set CCU
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("pll s tr %d"%(value)).readlines()
      verify=self.GetTriggerDelay()
      if not verify==value:
         print "verification of PLL trigger delay setting failed: wrote ",value," got ",verify
         if self.log: self.log.error("erification of PLL trigger delay setting failed: wrote %d   got %d"%(value,verify))
         return False
      else:
         return True
      
   def GetClockPhase(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      query="pll read"
      regexp="-->Clock Phase   : %d"
      r=self.ccu.iquery(query,regexp)
      #print "PLL: ClockPhase = ", r
      return r

   def GetTriggerDelay(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      query="pll read"
      regexp="-->Trigger Delay : %d"
      r=self.ccu.iquery(query,regexp)
      #print "PLL: TriggerDelay = ", r
      return r

   def GetValues(self):
      value=[]
      value.append(self.GetClockPhase())
      value.append(self.GetTriggerDelay())
      return value

   def Status(self):
      value=self.GetTriggerDelay()
      if (value==0): print "PLL: not enabled"
      elif (value): print "PLL OK"
      else: print "PLL: no response"
   
    
#########################################
class DOH:
   def __init__(self, ccu, name, channel,log=None):
      self.ccu=ccu              # ccu/aoh access (SimpleCommand) 
      self.name=name            # name
      self.channel=channel      # channel
      self.g0=3                 # gain
      self.g1=3                 # gain
      self.g2=3                 # gain
      self.b0=50                # bias
      self.b1=50                # bias
      self.b2=50                # bias
      self.programming=False    # i2c programming
      self.log=log

   def SendValues(self):
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("doh setall %d %d %d %d %d %d"%(self.g0,self.g1,self.g2,self.b0,self.b1,self.b2)).readlines()
     
   def VerifyProgramming(self):
      if ( self.writebias(0,30) and self.writebias(1,31) and self.writebias(2,32) and self.writegain(0,0) and self.writegain(1,1) and self.writegain(2,2)  ):
         #print "Group "+self.name+":\tDOH: \t\tprogramming OK"
         if self.log: self.log.ok("Group "+self.name+":\tDOH: \t\tprogramming OK")
         self.programming=True
      else:
         #print "Group "+self.name+":\tDOH: \t\tERROR in programming"
         if self.log: self.log.error("Group "+self.name+":\tDOH: \t\tERROR in programming")
         #sys.exit(1)
      
   def readbias(self,ch):
      # ch 0..2
      self.ccu.send("group "+self.name).readlines()
      query="doh read"
      regexp="-->Bias channel "+str(ch)+": %d"
      return self.ccu.iquery(query,regexp)      
      
   def writebias(self,ch,bias):
      # DOH !
      # ch 0..2, bias: 0..127 
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      cmd="doh set b"+str(ch)+" %d"%bias
      #print cmd
      self.ccu.send(cmd).readlines(),
      verify=self.readbias(ch)
      if not verify==bias:
         print "verification of DOH bias setting failed: wrote ",bias," got ",verify
         if self.log: self.log.error("verification of DOH bias setting failed: wrote %d  got  %d "%(bias,verify))
         return False
      return True

   def readgain(self,ch):
      # ch 0..2
      self.ccu.send("group "+self.name).readlines()
      query="doh read"
      regexp="-->Gain channel "+str(ch)+": %d"
      return self.ccu.iquery(query,regexp)      
      
   def writegain(self,ch,gain):
      # ch 0..2, gain: 0..3
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("reset").readlines()
      cmd="doh set g"+str(ch)+" %d"%gain
      #print cmd
      self.ccu.send(cmd).readlines(),
      verify=self.readgain(ch)
      if not verify==gain:
         print "verification of gain setting failed: wrote ",gain," got ",verify
         if self.log: self.log.error("verification of gain setting failed: wrote %d  got  %d "%(bias,verify))
         return False
      return True

   def GetValues(self):
      value=[]
      for i in range(3):
         value.append(self.readbias(i))
         value.append(self.readgain(i))
      return value

#########################################
class tDOH:
   def __init__(self, ccu, ccuAddress, channel,log=None):
      self.ccu=ccu                # ccu/aoh access (SimpleCommand) 
      self.ccuAddress=ccuAddress  # ccu address
      self.channel=channel        # channel
      self.g0=2                   # gain
      self.g1=2                   # gain
      self.g2=2                   # gain
      self.b0=30                  # bias
      self.b1=30                  # bias
      self.b2=30                  # bias
      self.programming=False      # i2c programming
      self.log=log

   def SendValues(self):
      self.ccu.send("ccu "+self.ccuAddress).readlines()
      self.ccu.send("channel "+self.channel).readlines()
      self.ccu.send("reset").readlines()
      self.ccu.send("doh setall %d %d %d %d %d %d"%(self.g0,self.g1,self.g2,self.b0,self.b1,self.b2)).readlines()
      if (self.ccuAddress=="0x7e"): print "DOHA (CCU) "+self.ccuAddress+ " set values: ", self.g0,self.g1,self.g2,self.b0,self.b1,self.b2
      if (self.ccuAddress=="0x7d"): print "DOHB (CCU) "+self.ccuAddress+ " set values: ", self.g0,self.g1,self.g2,self.b0,self.b1,self.b2

   def VerifyProgramming(self):
      if ( self.writebias(0,30) and self.writebias(1,31) and self.writebias(2,32) and self.writegain(0,0) and self.writegain(1,1) and self.writegain(2,2)  ):
         #print "CCU "+self.ccuAddress+" (channel "+self.channel+") DOH:\tprogramming OK"
         if self.log: self.log.ok("CCU "+self.ccuAddress+" (channel "+self.channel+") DOH:\tprogramming OK")
         self.programming=True 
      else:
         #print "CCU "+self.ccuAddress+" (channel "+self.channel+") DOH:\tERROR in programming"
         if self.log: self.log.error("CCU "+self.ccuAddress+" (channel "+self.channel+") DOH:\tERROR in programming")
         #sys.exit(1)
      
   def readbias(self,ch):
      # ch 0..2
      self.ccu.send("ccu "+self.ccuAddress).readlines()
      self.ccu.send("channel "+self.channel).readlines()
      query="doh read"
      regexp="-->Bias channel "+str(ch)+": %d"
      return self.ccu.iquery(query,regexp)      
      
   def writebias(self,ch,bias):
      # tDOH
      # ch 0..2, bias: 0..127
      self.ccu.send("ccu "+self.ccuAddress).readlines()
      self.ccu.send("channel "+self.channel).readlines()
      self.ccu.send("reset").readlines()
      cmd="doh set b"+str(ch)+" %d"%bias
      #print cmd
      self.ccu.send(cmd).readlines()
      verify=self.readbias(ch)
      if not verify==bias:
         #print "verification of bias setting failed: wrote ",bias," got ",verify
         if self.log: self.log.error("verification of bias setting failed: wrote %d, got %d"%(bias,verify))
         return False
      else:
         return True

   def readgain(self,ch):
      # ch 0..2
      self.ccu.send("ccu "+self.ccuAddress).readlines()
      self.ccu.send("channel "+self.channel).readlines()
      query="doh read"
      regexp="-->Gain channel "+str(ch)+": %d"
      return self.ccu.iquery(query,regexp)      
      
   def writegain(self,ch,gain):
      # ch 0..2, gain: 0..3
      self.ccu.send("ccu "+self.ccuAddress).readlines()
      self.ccu.send("channel "+self.channel).readlines()
      self.ccu.send("reset").readlines()
      cmd="doh set g"+str(ch)+" %d"%gain
      #print cmd
      self.ccu.send(cmd).readlines(),
      verify=self.readgain(ch)
      if not verify==gain:
         if self.log: self.log.error("verification of gain setting failed: wrote %d, got %d"%(bias,verify))
         return False
      else:
         return True

   def GetValues(self):
      value=[]
      for i in range(3):
         value.append(self.readbias(i))
         value.append(self.readgain(i))
      return value


#########################################
class AOH:
   def __init__(self, fed, fedslot, ccu, group,  name, fedchannels,log=None):
      self.fed=fed              # fec access (SimpleCommand)
      self.fedslot=fedslot      # fed slot number
      self.ccu=ccu              # ccu/aoh access (SimpleCommand) 
      self.group=group          # group identifier
      self.sectorname=group[:3]
      self.name=name            # aoh1/aoh2 for layer3, aoh1..4 for layer1&2
      layer=self.group[3:]
      if layer=="L12":
         self.logname=name
      elif layer=="L3":
         self.logname="aoh"+str(int(name[-1])+4)
      else:
         self.logname="???????"
      self.fedchannelsdefault=[None]+fedchannels # the six fed channels for a0 a1 a2 b0 b1 b2
      self.fedchannels=self.fedchannelsdefault
      self.cmd=[None]                     # command templates , aohxx set [b/g]x [value]"
      self.aoh=[None]
      for ch in range(3):
         self.aoh.append(name+"a")
         self.cmd.append(name+"a %s "+str(ch)+" %d")
      for ch in range(3):
         self.aoh.append(name+"b")
         self.cmd.append(name+"b"+str(ch)+" %d")
      self.programming=False
      self.log=log

   def VerifyProgramming(self):
      if ( self.writebias(1,11) and self.writebias(2,12) and self.writebias(3,13) and self.writebias(4,14) and self.writebias(5,15) and self.writebias(6,16) and
           self.writegain(1,0)  and self.writegain(2,1)  and self.writegain(3,2)  and self.writegain(4,0)  and self.writegain(5,1)  and self.writegain(6,2)  ):
         #print "Group "+self.group+":\t "+self.name+": \t\tprogramming OK"
         if self.log: self.log.ok("Group "+self.group+":\t "+self.name+": \t\tprogramming OK")
         self.programming=True
      else:
         #print "Group "+self.group+":\t "+self.name+": \t\tERROR in programming"
         if self.log: self.log.error("Group "+self.group+":\t "+self.name+": \t\tERROR in programming")
         #sys.exit(1)

   def readbias(self,ch):
      # ch 1..6
      self.ccu.send("group "+self.group).readlines()
      #self.ccu.send("reset").readlines()
      query=self.aoh[ch]+" read"
      regexp="-->Bias channel "+str((ch-1)%3)+": %d"
      return self.ccu.iquery(query,regexp)      
      
   def writebias(self,ch,bias,verify=True):
      # AOH
      # ch 1..6, bias: 0..127
      if not 1<=ch<=6 and 0<=bias<=127:
         print "invalid values in AOH.writebias(ch=%d,bias=%d)"%(ch,bias)
         return False
      self.ccu.send("group "+self.group).readlines()
      #self.ccu.send("reset").readlines()
      cmd=self.aoh[ch]+" set b"+str((ch-1)%3)+" %d"%bias
      #print cmd
      self.ccu.send(cmd).readlines()
      if verify:
         readvalue=self.readbias(ch)
         if not readvalue==bias:
            #print "verification of bias setting failed: wrote ",bias," got ",readvalue
            if self.log: self.log.error("verification of bias setting failed: wrote %d, got %d "%(bias,readvalue))
            return False
         else:
            return True
      else:
         return True
         

   def readgain(self,ch):
      # ch 1..6
      self.ccu.send("group "+self.group).readlines()
      self.ccu.send("reset").readlines()
      query=self.aoh[ch]+" read"
      regexp="-->Gain channel "+str((ch-1)%3)+": %d"
      return self.ccu.iquery(query,regexp)      
      
   def writegain(self,ch,gain):
      # ch 1..6, bias: 0..127
      self.ccu.send("group "+self.group).readlines()
      #self.ccu.send("reset").readlines()
      cmd=self.aoh[ch]+" set g"+str((ch-1)%3)+" %d"%gain
      #print cmd
      self.ccu.send(cmd).readlines(),
      verify=self.readgain(ch)
      if not verify==gain:
         #print "verification of gain setting failed: wrote ",gain," got ",verify
         if self.log: self.log.error("verification of gain setting failed: wrote %d, got %d "%(bias,verify))
         return False
      else:
         return True

   def find(self,ch,verbose=False):
      #self.fed.send("fed "+str(self.fedslot))#.readlines()
      self.fed.send("fed %s"%(self.sectorname)).readlines()
      if verbose: print "find aoh channel ",ch
      oldvalue=self.readbias(ch)
      self.writebias(ch,0)
      #time.sleep(1.0)
      self.fed.send("reset").readlines()
      b0=self.fed.aquery("read","baselines = %s")
      if verbose: print "b0=",b0
      self.writebias(ch,127)
      #time.sleep(1.0)
      self.fed.send("reset").readlines()
      b1=self.fed.aquery("read","baselines = %s")
      self.writebias(ch,oldvalue)
      diff=[b1[i]-b0[i] for i in range(36)]
      if verbose: print "b1=",b1
      dmax=max(diff)
      if dmax>100:
         fedch=diff.index(dmax)+1
         if dmax<1000:
            self.log.error("weak channel %s  %d"%(self.name,ch))
         if verbose: print "fed channel is ",fedch
         return fedch
      else:
         self.log.error("dead channel %s  %d"%(self.name,ch))
         if verbose: print "no fed channel responded"
         return 0

   def findAll(self):
      for ch in range(1,7):
         self.fedchannels[ch]=self.find(ch)
      if not self.fedchannels==self.fedchannelsdefault:
         for i in range(1,7):
            if self.fedchannels[i]==0: continue
            if self.fedchannels[i]==self.fedchannelsdefault[i]: continue
            self.log.error("non-standard fed channel map:  %s-%d  mapped to %d instead of %d"%(self.name,i,self.fedchannels[i],self.fedchannelsdefault[i]))


   def getBiasCurve(self,ch):
       data=[]
       oldvalue=self.readbias(ch)
       self.writebias(ch,0)
       #time.sleep(4)
       for bias in [8*x for x in range(16)]:
          self.writebias(ch,bias)
          #time.sleep(1.0)
          l=self.fed.aquery("read","baselines = %s")
          #time.sleep(1.0)
          l=self.fed.aquery("read","baselines = %s")
          print l
          data.append((bias,l[self.fedchannels[ch]-1]))
       self.writebias(ch,oldvalue)
       return data

    
   def getBiasCurves(self):
      data=[None]+[[] for ch in range(6)]
      #self.fed.send("fed "+str(self.fedslot)).readlines()
      self.fed.send("fed %s"%(self.sectorname)).readlines()
      oldvalues=[self.readbias(ch) for ch in range(1,7)]
      #print oldvalues
      self.writebias(ch,0)
      cnt=[0,0,0,0,0,0]
      
      for bias in [4*x for x in range(128/4)]:
         for ch in range(1,7):
            self.writebias(ch,bias)
            
         self.fed.send("reset fifo").readlines()
         l=self.fed.aquery("read","baselines = %s")
         print bias,l
         for ch in range(1,7):
            data[ch].append((bias,l[self.fedchannels[ch]-1]))
            if (l[self.fedchannels[ch]-1]==1023):
               cnt[ch-1]=cnt[ch-1]+1

         if (cnt[0]>2 and cnt[1]>2 and cnt[2]>2 and cnt[3]>2 and cnt[4]>2 and cnt[5]>2): break

      for ch in range(1,7):
         self.writebias(ch,oldvalues[ch-1])
      self.fed.send("reset fifo").readlines()

      print oldvalues
      return data

   
   def getBiasRMSCurves(self,setbias=False):
      data=[None]+[[] for ch in range(6)]
      self.workingBias=7*[0]
      #self.fed.send("fed "+str(self.fedslot)).readlines()
      self.fed.send("fed %s"%(self.sectorname)).readlines()
      oldvalues=[self.readbias(ch) for ch in range(1,7)]
      #print oldvalues
      for ch in range(1,7): self.writebias(ch,0)  # will also set the group for the ccu

      biasRange=range(10,41,1)
      nbp=0
      for bias in biasRange:
##          for ch in range(1,7):
##             #self.writebias(ch,bias,verify=False)
##             cmd=self.aoh[ch]+" set b"+str((ch-1)%3)+" %d"%bias
##             self.ccu.send(cmd).readlines()
##          # ALTERNATIVE  CODE
         g=3
         self.ccu.send(self.name+"a setall %d %d %d %d %d %d"%(g,g,g,bias,bias,bias)).readlines()
         self.ccu.send(self.name+"b setall %d %d %d %d %d %d"%(g,g,g,bias,bias,bias)).readlines()

         wupp=self.fed.send("reset fifo; rms %d %d %d %d %d %d"%tuple(self.fedchannels[1:])).readlines()
         chsum=0
         #print wupp
         d=wupp[1].split()
         for ch in range(1,7):
            mean,rms=d[ch*2-2],d[ch*2-1]
            data[ch].append([bias,float(mean),float(rms),0.])
            chsum+=float(mean)
         self.ccu.send(self.name+"a setall 0 0 0 0 0 0; "+self.name+"b setall 0 0 0 0 0 0").readlines()

##          self.fed.send("reset fifo").readlines()
##          chsum=0
##          for ch in range(1,7):
##             if self.fedchannels[ch]==0: continue
##             response=self.fed.query("rms %d"%self.fedchannels[ch])
##             channel,mean,rms,name=response.split()
##             data[ch].append([bias,float(mean),float(rms),0.])
##             chsum+=float(mean)
##             cmd=self.aoh[ch]+" set b"+str((ch-1)%3)+" %d"%0
##             self.ccu.send(cmd).readlines()

         #print "sum=",chsum
         nbp+=1
         if chsum>=(1023.*6): break
      # end of bias scan

      # restore old bias settings unless working point should be set
      if not setbias:
         for ch in range(1,7):
            self.writebias(ch,oldvalues[ch-1])

      # calculate the slopes
      for ch in range(1,7):
         # skip missing channels
         if self.fedchannels[ch]==0: continue
         # also count data points with high noise or low slope and try to find a good working point
         nslope,nnoise,b500=0,0,0

         # first point 
         data[ch][0][3]=float(data[ch][1][1]-data[ch][0][1])/float(data[ch][1][0]-data[ch][0][0])
         # intermediate points
         for idx in range(1,nbp-1):
            data[ch][idx][3]=0.5*float(data[ch][idx+1][1]-data[ch][idx-1][1])/float(data[ch][idx+1][0]-data[ch][idx-1][0])
         # last point
         data[ch][-1][3]=float(data[ch][-1][1]-data[ch][-2][1])/float(data[ch][-1][0]-data[ch][-2][0])

         # count data points with high noise or low slope and try to find a good working point
         nslope,nnoise,b500=0,0,0
         for idx in range(nbp):
            if data[ch][idx][3]>30: nslope+=1
            if data[ch][idx][2]>5.: nnoise+=1
            if data[ch][idx][1]>500 and b500==0: b500=data[ch][idx][0]
         if nslope<5: self.log.error(self.logname+"-"+str(ch)+" low slope")
         if nnoise>5: self.log.error(self.logname+"-"+str(ch)+" noisy")
         if b500>35:  self.log.error(self.logname+"-"+str(ch)+" high working point")
         self.workingBias[ch]=b500
         if setbias:
            self.writebias(ch,b500)
         
      return data


   
   def setBiasFed(self,ch,target=500):
      # tune the aoh bias to get a target FED reading for the baseline
      #self.fed.send("fed "+str(self.fedslot)).readlines()
      self.fed.send("fed %s"%(self.sectorname)).readlines()
      oldvalue=self.readbias(ch)
      bhigh,ahigh=127,1023
      blow,alow=0,0
      # start by finding  the mapping
      self.writebias(ch,0)
      self.fed.send("reset fifo").readlines()
      b0=self.fed.aquery("read","baselines = %s")
      self.writebias(ch,bhigh)
      self.fed.send("reset fifo").readlines()
      b1=self.fed.aquery("read","baselines = %s")
      diff=[b1[i]-b0[i] for i in range(36)]
      dmax=max(diff)
      if dmax>100:
         fedch=diff.index(dmax)+1
         ahigh=b1[fedch-1]
         self.fedchannels[ch]=fedch
         #print "fed channel is ",fedch
      else:
         self.fedchannels[ch]=0
         self.writebias(ch,oldvalue)
         print "no fed channel found for ",self.name
         if log: self.log.error("no fed channel found for "+self.name)
         return oldvalue
      
      if b1[fedch-1]<target:
         print "target ",target," cannot be reached, value at ",bhigh," = ",ahigh
         self.writebias(ch,oldvalue)
         return oldvalue
      
      # now tune bias with binary search (but beware of nonlinearities/jumps)

      while bhigh-blow>1:
         bias=int((bhigh+blow)/2)
         cmd=self.aoh[ch]+" set b"+str((ch-1)%3)+" %d"%bias
         self.ccu.send(cmd).readlines()

         self.fed.send("reset fifo").readlines()
         l=self.fed.iquery("read %d"%(fedch),"baseline =%d")
         #print ch,bias,l
         l=int(l)
         if l>=target:
            bhigh,ahigh=bias,l
         if l<=target:
            blow,alow=bias,l
      #print self.name," channel ",ch," set to ",bias,"     fed channel ",fedch," adc=",l
      if log: self.log.info(self.name+" channel "+str(ch)+" set to "+str(bias)+"     fed channel "+str(fedch)+" adc="+str(l))

      return oldvalue
  


########################################
class MODULE:
   def __init__(self, pxfec, name, hubaddress, programming, readout, goodaddress,log=None):
      self.pxfec=pxfec             # pxfec access (SimpleCommand)
      self.name=name               # name
      self.hubaddress=hubaddress   # hubaddress
      self.programming=programming # programming status
      self.readout=readout         # readout status
      self.goodaddress=goodaddress # is hubaddress in design hubaddresses?
      self.log=log
           
   def GetTBMEventNumber(self):
      query="cn "+self.name+" module %d tbm readstackA"%(self.hubaddress)
      regexp="31: eventnumber %d"
      return self.pxfec.iquery(query,regexp)

  ##  def FindReset(self):
##       query="cn "+self.name+" module %d tbm readstackA"%(self.hubaddress)
##       regexp="%d: statusbits 32"
##       print self.pxfec.iquery(query,regexp)
      
   def CheckTriggerStatus(self):
      value=[]
      for i in range (5):
         r=self.GetTBMEventNumber()
         sleep(0.2)
         #print r
         value.append(r)
      print "Eventnumbers: ", value
      cnt = value[4]-value[1]
      #print cnt
      if (cnt==0):
         print "hub "+str(self.hubaddress)+": ERROR: receives no trigger"
         self.log.error("hub "+str(self.hubaddress)+": ERROR: receives no trigger")
      else:
         print "hub "+str(self.hubaddress)+": OK: receives trigger"
         self.log.ok("hub "+str(self.hubaddress)+": OK: receives trigger")

##    def TestTBMTBMreset(self):
##       query="cn "+self.name+" reset tbm"
##       self.pxfec.send(query).readlines()
##       value=[]
##       for i in range (5):
##          r=self.GetTBMEventNumber()
##          sleep(0.2)
##          #print r
##          value.append(r)
##       print "Eventnumbers: ", value
##       cnt = value[4]-value[1]
##       #print cnt
##       if (cnt==0): print "hub "+str(self.hubaddress)+": ERROR: receives no trigger"
##       else: print "hub "+str(self.hubaddress)+": OK: receives trigger"

###################################
      
#global functions
def graph1(xy,style=20,size=0.8,color=1,name=None):

   ax=array('d')
   ay=array('d')
   for i in range(len(xy)):
      ax.append(xy[i][0])
      ay.append(xy[i][1])
   g=TGraph(len(xy),ax,ay)
   g.SetMarkerStyle(style)
   g.SetMarkerSize(size)
   g.SetMarkerColor(color)
   g.Draw("P")
   #f1.SetLineWidth(1)
   #f1.FixParameter(1,(color-3.)*118.)
   #f1.SetParameter(2,0.6)
   #f1.FixParameter(3,t0+5)
   #g.Fit(f1,'R')
   return g

def hexToI(hexvalue):
    dec={'0':0,'1':1,'2':2,'3':3,'4':4,'5':5,'6':6,'7':7,'8':8,'9':9,'a':10,'b':11,'c':12,'d':13,'e':14,'f':15}
    if len(hexvalue)==4:
        value=16*dec[hexvalue[2]]+dec[hexvalue[3]]
    elif len(hexvalue)==3:
        value=dec[hexvalue[2]]
    else:
        value=0
    return value
 
def TestTriggerStatusFED(fed,sector="",log=None):
   fed.send("reset").readlines()
   if not sector=="": fed.send("fed "+sector).readlines()
   #if fedslot:  fed.send("fed %d"%fedslot).readlines()
   nloop=0
   e1=fed.iquery("event")
   sleep(0.01)
   
   while e1==fed.iquery("event") and nloop<100:
      sleep(0.01) 
      nloop=nloop+1

   if (nloop<100):
      #print "TestTriggerStatusFED(): OK. FED receives trigger!"
      if log: log.ok("TestTriggerStatusFED(): OK. FED receives trigger!")
   else:
      if log: log.error("TestTriggerStatusFED(): FED does not receive trigger!")

def TestRedundancy(ccu,trfec=None,ring=None,verbose=False,log=None):

   #test FEC output B and first CCU input B
   #ccu.send("fec %s"%trfec).readlines()
   #ccu.send("ring %s"%ring).readlines()
   ccu.send("reset").readlines()
   ccu.send("redundancy fec a b").readlines()
   ccu.send("redundancy ccu 0x7e b a").readlines()
   ccu.send("scanccuallfec")
   nccu = 0
   passed=True  # innocent until proven guilty
   
   for s in ccu.readline():
      if (verbose): print s
      m=re.match(".*CCU (0[xX][\dA-Fa-f]+) found.*",s)
      if m:
         value = hexToI(m.group(1))
         for i in range (119,127):
            if (value == i):
               nccu = nccu + 1
               
   print " "          
   if (nccu == 8 ):
      print "FEC A B CCU 0x7e B A               : OK "
   else :
      print "FEC A B CCU 0x7e B A               : ERROR "
      passed=False

   #test FEC output B and second CCU input B
   ccu.send("reset").readlines()
   ccu.send("redundancy fec a b").readlines()
   ccu.send("redundancy ccu 0x7d b a").readlines()
   ccu.send("scanccuallfec")
   nccu = 0
   notbypassed = 0
   for s in ccu.readline():
      if (verbose): print s
      m=re.match(".*CCU (0[xX][\dA-Fa-f]+) found.*",s)
      if m:
         value = hexToI(m.group(1))
         for i in range (119,127):
            if (value == i ):
               nccu = nccu + 1
            if (value == 126 ):
               notbypassed = 1
          
   if (nccu == 7 and notbypassed ==0 ):
      print "FEC A B CCU 0x7d B A               : OK "
   else :
      print "FEC A B CCU 0x7d B A               : ERROR "


   #test bypass CCU 0x7d,...,0x78
   ccuAddress=0x7e
   for i in range (6):
      ccuAddressOutputB="0x%x"%(ccuAddress)
      ccuAddressInputB="0x%x"%(ccuAddress-2)
    
      ccu.send("reset").readlines()
      ccu.send("redundancy ccu  %s  a b"%(ccuAddressOutputB)).readlines()
      ccu.send("redundancy ccu  %s  b a"%(ccuAddressInputB)).readlines()
      ccu.send("scanccuallfec")
      nccu = 0
      notbypassed = 0
      for s in ccu.readline():
         if (verbose): print s
         m=re.match(".*CCU (0[xX][\dA-Fa-f]+) found.*",s)
         if m:
            value = hexToI(m.group(1))
            
            for i in range (119,127):
               if (value == i ):
                  nccu = nccu + 1
               if (value == (ccuAddress-1) ):
                  notbypassed = 1
                        
      if (nccu == 7 and notbypassed ==0 ):
         print "CCU %s A B CCU %s B A          : OK "%(ccuAddressOutputB, ccuAddressInputB)
      else :
         print "CCU %s A B CCU %s B A          : ERROR "%(ccuAddressOutputB, ccuAddressInputB)
         passed=False
      ccuAddress = ccuAddress-1

   #test FEC input B 
   ccu.send("reset").readlines()
   ccu.send("redundancy ccu 0x77 a b").readlines()
   ccu.send("redundancy ccu 0x76 a b").readlines()
   ccu.send("redundancy fec b a").readlines()
   ccu.send("scanccuallfec")

   nccu = 0
   for s in ccu.readline():
      if (verbose): print s
      m=re.match(".*CCU (0[xX][\dA-Fa-f]+) found.*",s)
      if m:
         value = hexToI(m.group(1))
         for i in range (118,127):
            if (value == i):
               nccu = nccu + 1
          
   if (nccu == 9 ):
      print "CCU 0x77 A B CCU 0x76 A B FEC B A  : OK "
   else :
      print "CCU 0x77 A B CCU 0x76 A B FEC B A  : ERROR "


   #test FEC input B with bypass of CCU 0x77 
   ccu.send("reset").readlines()
   ccu.send("redundancy ccu 0x78 a b").readlines()
   ccu.send("redundancy ccu 0x76 b a").readlines()
   ccu.send("redundancy ccu 0x76 b b").readlines()
   ccu.send("redundancy fec b a").readlines()
   ccu.send("scanccuallfec")
   
   nccu = 0
   notbypassed = 0
   for s in ccu.readline():
      if (verbose): print s
      m=re.match(".*CCU (0[xX][\dA-Fa-f]+) found.*",s)
      if m:
         value = hexToI(m.group(1))
         for i in range (118,127):
            if (value == i ):
               nccu = nccu + 1
            if (value == 119 ):
               notbypassed = 1
          
   if (nccu == 8 and notbypassed ==0 ):
      print "CCU 0x78 A B CCU 0x76 B B FEC B A  : OK "
   else :
      print "CCU 0x78 A B CCU 0x76 B B FEC B A  : ERROR "
      passed=False
      
   #reset fec
   ccu.send("reset").readlines()
   if passed:
      log.ok("CCU redundancy test ok")
   else:
      log.error("CCU redundancy test failed")




######################################################################
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
 
####################  main  #######################################
folder = "res"


## print "connecting to fed,",
## fed=SimpleSocket( 'localhost', 2003 )
## fed=1
## print " done"

## print "connecting to ccu,",
## ccu=SimpleSocket( 'localhost', 2002)
## ccu=1
## print " done"

## print "connecting to pxfec,",
## pxfec=SimpleSocket( 'localhost', 2000)
## pxfec=1
## print " done"

## print "connecting to caen,",
## caen=SimpleSocket( 'localhost', 2001)
## caen=1
## print " done"

## s1=SECTOR("-1P",fed,8,ccu,pxfec,caen)
## s2=SECTOR("-2P",fed,8,ccu,pxfec,caen)
## s3=SECTOR("-3P",fed,8,ccu,pxfec,caen)
## s4=SECTOR("-4P",fed,8,ccu,pxfec,caen)
## s5=SECTOR("-5P",fed,8,ccu,pxfec,caen)
## s6=SECTOR("-6P",fed,8,ccu,pxfec,caen)
## s7=SECTOR("-7P",fed,8,ccu,pxfec,caen)
## s8=SECTOR("-8P",fed,8,ccu,pxfec,caen)
## #s=SECTOR("-6P",fed, 8,ccu,pxfec,caen)

#s=SECTOR("-5P",fed,8,ccu,pxfec,caen)

## s8.TestTriggerStatusFED()
#s1.TestCCUandI2CConnection()

#for s in [s1,s2,s3,s4,s5,s6,s7,s8]
#for s in [s1]:
#s1.VerifyI2CProgramming()
##    for i in [1,2]:
##       s.group[i].VerifyI2CProgramming()
##       s.group[i].FillHubDesignHubAddresses()

#s8.TestPIAResetDOH()
#s8.TestPIAResetAOH()
   #s.TestPIAResetROC()


## folder="Sector"+s.name+"-"+date.today().isoformat()
## if os.path.exists(folder):
##    template=folder+"-%03d"
##    for n in range(100):
##       folder=template%n
##       if not os.path.exists(folder): break
    
## os.mkdir(folder)



#s.InitTrackerDOH()   
#TestRedundancy(True)

#s8.DrawMapAOH() #mapping between aoh and fed channel
#s8.showAOHBias()

## for i in [2]:
##   ##  v = s.group[i].Geti2cValues()
## ##    print v
## ##    s.group[i].FindSDA()
   
##    v = s.group[i].Geti2cValues()

##    s.group[i].FillHubDesignHubAddresses()
##    rck = s.group[i].ScanRCK()
##    s.group[i].GetHubAddresses2(rck)
##    s.group[i].FindRCK2(rck)
##    s.group[i].PlotRCKRange2(rck,v)
   
##    s.group[i].PlotSDARDA()
##    s.group[i].PlotRDARCK()
##    s.group[i].PlotSDARCK()

## s.TestPIAResetDOH()
## s.TestPIAResetAOH()
#s.TestPIAResetROC()
   
#s.PrintResults()

#s.group[2].AddModules([0,4])
#for i in range(len (s.group[2].modules)):
#   s.group[2].modules[i].FindReset()


#fed.close()
#ccu.close()
#pxfec.close()
#caen.close()




    
