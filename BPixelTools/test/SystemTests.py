# import sys,time,os, re
import sys,time,os, re, ROOT
from datetime import date
from time import sleep
from Logger import Logger
sockdir="/home/cmspixel/TriDAS/pixel/BPixelTools/tools/python"
#sockdir="/home/l_pixel/TriDAS/pixel/BPixelTools/tools/python"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket
from ROOT import *
from array import *
from math import *
from operator import itemgetter
hList=[None]
gCanvases=[]
########################
class SECTOR:
   def __init__(self,name,fed,ccu,pxfec,caen,log=None):

      self.name=name
      self.fed=fed
      self.ccu=ccu
      self.pxfec=pxfec
      self.caen=caen
      if log:
         self.log=log
      else:
         self.log=Logger()
      self.group=[]
      self.group.append(GROUP(fed, ccu, pxfec, caen, name+"L12",0x11,self.log))
      self.group.append(GROUP(fed, ccu, pxfec, caen, name+"L34", 0x13,self.log))
      self.pll12=PLL( ccu, name+"L12",0x11,log)
      self.pll34=PLL( ccu, name+"L34",0x13,log)
      self.delay25_1=DELAY25(ccu,name+"L12",0x11,log)
      self.delay25_2=DELAY25(ccu,name+"L34",0x13,log)
      self.dohA=tDOH(ccu, "0x7b", "0x10",self.log)
      self.dohB=tDOH(ccu, "0x7c", "0x10",self.log)
      self.resetDOH=[]
      self.resetAOH=[]
      self.resetROC=[]
      self.poh={}
      self.caengroup=["L14","L23"]
      
      # if self.name=="test":
      #    self.poh[1] =POH(fed, ccu, name+"L12","poh1",[],log)
      #    self.poh[2] =POH(fed, ccu, name+"L12","poh2",[],log)
      #    self.poh[3] =POH(fed, ccu, name+"L34","poh1",[],log)
      #    self.poh[4] =POH(fed, ccu, name+"L12","poh3",[],log)
      #    self.poh[5] =POH(fed, ccu, name+"L34","poh2",[],log)
      #    self.poh[6] =POH(fed, ccu, name+"L12","poh4",[],log)
      #    self.poh[7] =POH(fed, ccu, name+"L12","poh5",[],log)
      #    self.poh[8] =POH(fed, ccu, name+"L34","poh3",[],log)
      #    self.poh[9] =POH(fed, ccu, name+"L12","poh6",[],log)
      #    self.poh[10]=POH(fed, ccu, name+"L34","poh4",[],log)
      #    self.poh[11]=POH(fed, ccu, name+"L34","poh5",[],log)
      #    self.poh[12]=POH(fed, ccu, name+"L12","poh7",[],log)
      #    self.poh[13]=POH(fed, ccu, name+"L34","poh6",[],log)
      #    self.poh[14]=POH(fed, ccu, name+"L34","poh7",[],log)

      if "1" in self.name or "8" in self.name:
          self.pohlist=range(1,14)
          self.poh[1] =POH(fed, ccu, name+"L12","poh1", 1, [None, 1, 2, 3, 4] ,log)
          self.poh[2] =POH(fed, ccu, name+"L12","poh2", 1, [None, 5, 6, 7, 8] ,log)
          self.poh[3] =POH(fed, ccu, name+"L34","poh1", 1, [None, 9, 10, 11, 12] ,log)
          self.poh[4] =POH(fed, ccu, name+"L12","poh3", 2, [None, 1, 2, 3, 4] ,log)
          self.poh[5] =POH(fed, ccu, name+"L34","poh2", 2, [None, 5, 6, 7, 8] ,log)
          self.poh[6] =POH(fed, ccu, name+"L12","poh4", 2, [None, 9, 10, 11, 12] ,log)
          self.poh[7] =POH(fed, ccu, name+"L12","poh5", 3, [None, 1, 2, 3, 4] ,log)
          self.poh[8] =POH(fed, ccu, name+"L34","poh3", 3, [None, 5, 6, 7, 8] ,log)
          self.poh[9] =POH(fed, ccu, name+"L12","poh6", 3, [None, 9, 10, 11, 12] ,log)
          self.poh[10] =POH(fed, ccu, name+"L34","poh4", 4,  [None, 1, 2, 3, 4] ,log)
          self.poh[11] =POH(fed, ccu, name+"L34","poh5", 4,  [None, 5, 6, 7, 8] ,log)
          self.poh[12] =POH(fed, ccu, name+"L12","poh7", 4,  [None, 9, 10, 11, 12] ,log)
          self.poh[13] =POH(fed, ccu, name+"L34","poh6", 5, [None, 1, 2, 3, 4] ,log)

      for i in range(2,6) + range(7,8):
         if str(i) in self.name:
             self.pohlist=range(1,15)
             self.poh[1] =POH(fed, ccu, name+"L12","poh1", 1,[None, 1, 2, 3, 4] ,log)
             self.poh[2] =POH(fed, ccu, name+"L12","poh2", 1, [None, 5, 6, 7, 8] ,log)
             self.poh[3] =POH(fed, ccu, name+"L34","poh1", 1, [None, 9, 10, 11, 12] ,log)
             self.poh[4] =POH(fed, ccu, name+"L12","poh3", 2, [None, 1, 2, 3, 4] ,log)
             self.poh[5] =POH(fed, ccu, name+"L34","poh2", 2, [None, 5, 6, 7, 8] ,log)
             self.poh[6] =POH(fed, ccu, name+"L12","poh4", 2, [None, 9, 10, 11, 12] ,log)
             self.poh[7] =POH(fed, ccu, name+"L12","poh5", 3, [None, 1, 2, 3, 4] ,log)
             self.poh[8] =POH(fed, ccu, name+"L34","poh3", 3, [None, 5, 6, 7, 8] ,log)
             self.poh[9] =POH(fed, ccu, name+"L12","poh6", 3, [None, 9, 10, 11, 12] ,log)
             self.poh[10] =POH(fed, ccu, name+"L34","poh4", 4, [None, 1, 2, 3, 4] ,log)
             self.poh[11] =POH(fed, ccu, name+"L34","poh5", 4, [None, 5, 6, 7, 8] ,log)
             self.poh[12] =POH(fed, ccu, name+"L12","poh7", 4, [None, 9, 10, 11, 12] ,log)
             self.poh[13] =POH(fed, ccu, name+"L34","poh6", 5, [None, 1, 2, 3, 4] ,log)
             self.poh[14] =POH(fed, ccu, name+"L34","poh7", 5, [None, 5, 6, 7, 8] ,log)
             

      if "3" in self.name or "6" in self.name:
         self.pohlist= range(1,7) + range(8,12) +range(13,15)
         self.poh[1] =POH(fed, ccu, name+"L12","poh1", 1, [None, 1, 2, 3, 4] ,log)
         self.poh[2] =POH(fed, ccu, name+"L12","poh2", 1, [None, 5, 6, 7, 8] ,log)
         self.poh[3] =POH(fed, ccu, name+"L34","poh1", 1, [None, 9, 10, 11, 12] ,log)
         self.poh[4] =POH(fed, ccu, name+"L12","poh3", 2, [None, 1, 2, 3, 4] ,log)
         self.poh[5] =POH(fed, ccu, name+"L34","poh2", 2, [None, 5, 6, 7, 8] ,log)
         self.poh[6] =POH(fed, ccu, name+"L12","poh4", 2, [None, 9, 10, 11, 12] ,log)
         self.poh[7] =POH(fed, ccu, name+"L12","poh5", None, [None] ,log)
         self.poh[8] =POH(fed, ccu, name+"L34","poh3", 3,[None, 1, 2, 3, 4] ,log)
         self.poh[9] =POH(fed, ccu, name+"L12","poh6", 3,[None, 5, 6, 7, 8] ,log)
         self.poh[10] =POH(fed, ccu, name+"L34","poh4", 3, [None, 9, 10, 11, 12] ,log)
         self.poh[11] =POH(fed, ccu, name+"L34","poh5", 4, [None, 1, 2, 3, 4] ,log)
         self.poh[12] =POH(fed, ccu, name+"L12","poh7", None, [None] ,log)
         self.poh[13] =POH(fed, ccu, name+"L34","poh6", 4, [None, 5, 6, 7, 8] ,log)
         self.poh[14] =POH(fed, ccu, name+"L34","poh7", 4, [None, 9, 10, 11, 12] ,log)
         

      #  self.name :
      #self.pohlist=range(1,15)
      
      # self.pohlist=range(1,14)
      # self.pohlist=range(1,7) + range(8,12)+ range(13,15)

   def init_tube(self):

    self.ccu.send("reset").readlines()
    self.ccu.send("scanccu").readlines()
    self.ccu.send("piareset all").readlines()

    for gr in self.group:
       self.ccu.send("group " + gr.name).readlines()
       print "group " + gr.name
       self.ccu.send("delay25 init").readlines()
       self.ccu.send("pll reset").readlines()
       self.ccu.send("pll init").readlines()
       self.ccu.send("doh init").readlines()
    for p in self.pohlist:
       self.ccu.send("group " +self.poh[p].group).readlines()
       print "group " +self.poh[p].group
       self.ccu.send(self.poh[p].name+" init").readlines() # like this one
       print self.poh[p].name+" init"
    




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
      #print "Sector "+self.name+": FEC reset"

   def InitpxFEC(self):
      self.pxfec.send("cn "+self.name+"L12").readlines()
      print "Sector "+self.name+": Init pxFEC"
     
   def InitFED(self):
      #self.fed.send("fed %d"%self.fedslot).readlines()
      self.fed.send("reset").readlines()
      print "Sector "+self.name+": FED reset"

   def InitCAEN(self):
      print "init CAEN"
      for i in self.caengroup:
         self.caen.send("group "+self.name+i).readlines()
         self.caen.send("pon").readlines()
         if (self.IsCAENOn()):
            print "Group "+self.name+ i+": CAEN: power On"
         else:
            print "group "+self.name+i+"CAEN: Error when turning power on"

   def poweron(self):
      print "ENABLING DCDC 0:13"
     #  self.ccu.send("ccu" +ccuAddress).readlines()
      self.ccu.send("power all").readlines()

   def poweroff(self):
      print "DISABLING DCDC 0:13"
      # self.ccu.send("ccu" +ccuAddress).readlines()
      self.ccu.send("power none").readlines()
      

   def VerifySectorDevicesProgramming(self):
      #print "Verify Programming"
      # self.ccu.send("ccu " + ccuAddress).readlines()
      # print "test Delay25"
      self.delay25_1.VerifyProgramming()
      self.delay25_2.VerifyProgramming()
      # print "test PLL"
      self.pll12.VerifyProgramming()
      self.pll34.VerifyProgramming()
      # print "test TrDOH"
      self.dohA.VerifyProgramming()
      self.dohB.VerifyProgramming()
      # print "test POH"
      # self.pohlist=self.FindPOH()
      for i in self.pohlist:
         self.poh[i].VerifyProgramming()
      #print "pohlist " 
      #print self.pohlist 
   

   
   def ResetAllandInitAll(self):
     # print "test reset functionality for: pxDOH, trDOH, POH, PLL,Delay 25"
      # print "Reset All, Pia Reset All. pxDoH reset"
      self.ccu.send("reset").readlines()
      self.ccu.send("piareset all").readlines()
      self.ccu.send("pll reset").readlines()
      self.ccu.send("delay25 reset").readlines()
      # self.pxfec.send("cn reset doh").readlines()
      # print "Initializing pll, delay25,doh and poh"
      self.ccu.send("doh init").readlines()
      self.ccu.send("pll init").readlines()
      self.ccu.send("delay25 init").readlines()
      for i in self.pohlist:
         self.ccu.send("group "+ str(self.poh[i].group)).readlines()
         self.ccu.send("poh"+ str(i) +" setall 3 3 3 3 40 40 40 40").readlines()
         # sector
   def GetSDARange(self, name,verbose=True):
      self.InitFEC()
      self.InitCAEN()
      self.InitpxFEC()
   
      self.pxfec.send("cn +6PL12").readlines()
      # v = self.Geti2cValues()
      # print v
      good=[]
      for d2 in range (0,63,2): #scan SDA values
         self.ccu.send("delay25 s d2 %d"%(d2)).readlines()
         if (verbose): print "SDA: ", d2,
         self.pxfec.send("cn reset roc").readlines()                #reset roc
         self.pxfec.send("module 0:31 roc 0:15 Vdig 0").readlines() #avoid high digital currents
         self.pxfec.send("module 0:31 roc 0:15 Vsf 0").readlines()  #avoid high digital currents
         self.pxfec.send("module 0:31 roc 0:15 Vana 0").readlines() #set Vana=0
         sleep(3)
         self.caen.send("group "+ name).readlines()
         print "group "+ name
         ia0=self.caen.fquery("get ia")                                 
         self.pxfec.send("module 0:31 roc 0:15 Vana 120").readlines()        #set Vana=120
         sleep(3)
         ia1=self.caen.fquery("get ia")
         if (verbose): print "IA= ",ia0, ia1
         if ia1-ia0>0.4 : good.append(d2)
      print "SDA range: ", good
      self.sdarange=good
      return good





   def SetPOHFEDchannel(self,file):
      case=["Sector 1 8","Sector 2 4 5 7","Sector 3 6"]
      if "1" in self.name or "8" in self.name:
         self.pohlist=range(1,14)
         whichcase=case[0]
      for i in range(2,6) + range(7,8):
         if str(i) in self.name:
            self.pohlist=range(1,15)
            whichcase=case[1]
      if "3" in self.name or "6" in self.name:
         self.pohlist= range(1,7) + range(8,12) +range(13,15)
         whichcase=case[2]
      fedchannelsvalue=[]
      ok= None
      print whichcase
      for l in range(0,15):
         fedchannelsvalue.append([])
      for l in range(0,15):
         fedchannelsvalue[l].append(None)
      for line in file:
         if ("Sector" in line) and (whichcase in line): ok=True
         if ("Sector" in line) and (whichcase not in line): ok= None
         if ("POH" in line) and ok:
            useless,pohname,ribbon,fiber= line.split()
            fedchannelsvalue[int(pohname[:-2])].append(int(fiber))
       

      print "fedchannelsvalue" 
      for l in self.pohlist:
         print fedchannelsvalue[l]
         self.poh[l].fedchannels=fedchannelsvalue[l]
  


   def lazyprint(self,file):
      case=["Sector 1 8","Sector 2 4 5 7","Sector 3 6"]
      if "1" in self.name or "8" in self.name:
         self.pohlist=range(1,14)
         whichcase=case[0]
      for i in range(2,6) + range(7,8):
         if str(i) in self.name:
            self.pohlist=range(1,15)
            whichcase=case[1]
      if "3" in self.name or "6" in self.name:
         self.pohlist= range(1,7) + range(8,12) +range(13,15)
         whichcase=case[2]
      fedchannelsvalue=[]
      whichcase=case[2]
      ok= None
      print whichcase
      for l in range(0,15):
         fedchannelsvalue.append([])
      for l in range(0,15):
         fedchannelsvalue[l].append(None)
      for line in file:
         if ("Sector" in line) and (whichcase in line): ok=True
         if ("Sector" in line) and (whichcase not in line): ok= None
         if ("POH" in line) and ok:
            useless,pohname,ribbon,fiber= line.split()
            fedchannelsvalue[int(pohname[:-2])].append(int(fiber))
            
      for l in range(0,15):  
            print '# self.poh['+str(l)+'] =POH(fed, ccu, name+"L12","poh'+str(l)+'",' ,fedchannelsvalue[l],',log)'
   








   def GetSDARange1(self,verbose=True):
      print "self.InitFEC()"
      self.InitFEC()
      print "self.CAEN()"
      self.InitCAEN()
      print "self.InitpxFEC()"
      self.InitpxFEC()
      value=[[],[],[],[]]
      ia0=[0.0,0.0,0.0,0.0]
      ia1=[0.0,0.0,0.0,0.0]
      group=["L12","L34"]
      caengroup=["L14","L23"]
      for d2 in range (0,63,2): #scan SDA values

         self.ccu.send("delay25 s d2 %d"%(d2)).readlines()
         if (verbose): print "SDA: ", d2
         for k in group:
            self.ccu.send( "group " + self.name + k).readlines()
            self.ccu.send("delay25 s d2 %d"%(d2)).readlines()
            self.pxfec.send("cn " +self.name +k).readlines()
            self.pxfec.send("cn reset roc").readlines()                #reset roc
            self.pxfec.send("module 0:31 roc 0:15 Vdig 0").readlines() #avoid high digital currents
            self.pxfec.send("module 0:31 roc 0:15 Vsf 0").readlines()  #avoid high digital currents
            self.pxfec.send("module 0:31 roc 0:15 Vana 0").readlines() #set Vana=0
            self.pxfec.send("cn reset roc").readlines()
            sleep(0.5)
            for l in caengroup:
               self.caen.send("group "+ self.name +l).readlines()
               if k== group[0] and l==caengroup[0]: # l12 && l14 ==l1
                  ia0[0]=self.caen.fquery("get ia")
               if k== group[0] and l==caengroup[1]: # l12 && l23 ==l2
                  ia0[1]=self.caen.fquery("get ia")
               if k== group[1] and l==caengroup[1]: # l34 && l23 ==l3
                  ia0[2]=self.caen.fquery("get ia")
               if k== group[1] and l==caengroup[0]: # l34 && l14 ==l4
                  ia0[3]=self.caen.fquery("get ia")

         for k in group:
            self.pxfec.send("cn " +self.name +k).readlines()
            self.pxfec.send("module 0:31 roc 0:15 Vana 120").readlines()        #set Vana=120
            sleep(0.5)
         for l in caengroup:
               self.caen.send("group "+ self.name +l).readlines()
               if k== group[0] and l==caengroup[0]: # l12 && l14 ==l1
                  ia1[0]=self.caen.fquery("get ia")
               if k== group[0] and l==caengroup[1]: # l12 && l23 ==l2
                  ia1[1]=self.caen.fquery("get ia")
               if k== group[1] and l==caengroup[1]: # l34 && l23 ==l3
                  ia1[2]=self.caen.fquery("get ia")
               if k== group[1] and l==caengroup[0]: # l34 && l14 ==l4
                  ia1[3]=self.caen.fquery("get ia")

         for l in range(0,4):
            if ia1[l]-ia0[l]>0.5: value[l].append(d2)
            print "Layer "+str(l+1)+" " + +str(ia1[l]) +" - " +str(ia0[l]) + " = " +  str(ia1[l]-ia0[l])
            
      print value
      
      good_L12=list(set(value[0]) and setset(value[1]))
      good_L34=list(set(value[2]) and setset(value[3]))
      print "SDA range: ", good
      self.sdarange=good
      return good


        
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

   def SearchPOH(self): 
         for i in range(1,15):
            if (self.poh[i].writebias(1,20)  and self.poh[i].writegain(1,0)   ):
               print "Sector"+self.name+":\t "+self.poh[i].name+" found"
          #if self.log: self.log.ok("Group "+self.group+":\t "+self.name+": \t\tprogramming OK")
          #self.programming=True
            else:
               print"Sector"+self.name+" :\t "+self.poh[i].name+" not found"

   def FindPOH(self):
       pohlist=[]
       for i in range(1,15):
            if (self.poh[i].writebias(1,20)  and self.poh[i].writegain(1,0)   ):
               print "Sector"+self.name+":\t "+self.poh[i].name+" found"
               pohlist.append(i)
            else:
               print"Sector"+self.name+" :\t "+self.poh[i].name+" not found"
       return pohlist
      
   def mapAOH(self):
      print "mapping sector ",self.name
      for a in range(1,7):
         self.aoh[a].fedchannels=[None,0,0,0,0,0,0]
      for a in self.aohlist:
         self.aoh[a].findAll()
         print "aoh ",a,self.aoh[a].fedchannels[1:]

   def mapPOH(self):
      print "mapping sector ",self.name
      for a in range(1,7):
         self.poh[a].fedchannels=[None,0,0,0,0,0,0]
      for a in self.pohlist:
         self.poh[a].findAll()
         print "poh ",a,self.poh[a].fedchannels[1:]


   def TestTriggerStatusFED(self):
      # this is a method of SECTRO
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

      self.ccu.send("pll set clk 1").readlines()
      self.ccu.send("pll set clk 1").readlines()
      
      query="pll read"
      regexp="-->Clock Phase   : %d"
      v1 = self.ccu.iquery(query,regexp)
      if not (v1==1):
         #print "TestPIAResetDOH(): ERROR (programming delay25 failed)"
         self.log.error("TestPIAResetDOH(): ERROR (programming pll failed)")
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
            f.write("L34    "+str(i-4)+"   "+str(j)+"     "+str(self.aoh[i].fedchannels[j])+'\n')
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
      f.write("GROUP L34\n")
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
            f.write("L34    "+str(i-4)+"   "+str(j)+"     "+str(self.aoh[i].fedchannels[j])+'\n')
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
      f.write("GROUP L34\n")
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
   def __init__(self, fed, ccu, pxfec, caen, name, channel,log):

      self.fed=fed                            # fec access (SimpleCommand)
      self.ccu=ccu                            # ccu access (SimpleCommand)
      self.pxfec=pxfec                        # pxfec access (SimpleCommand)
      self.caen=caen                          # caen access (SimpleCommand) 
      self.name=name                          # 
      self.channel=channel                    # 0x11 (L12), 0x13 (L34)
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
      # self.group={}
      # self.group[1]=GROUP(fed, fedslot, ccu, pxfec, caen, name+"L12",0x11,log)
      # self.group[2]=GROUP(fed, fedslot, ccu, pxfec, caen, name+"L34",0x13,log)
      
   def Init(self):
      self.InitFEC()
      if self.pxFEC: self.InitpxFEC()
      self.InitFED()
      if self.caen: self.InitCAEN()

   def InitMB(self):
      if not self.ccu: return
      self.ccu.send("group "+self.name).readlines()
      self.ccu.send("doh init").readlines()        # g0 g1 g2 b0 b1 b2
      self.ccu.send("pll reset").readlines()       #
      self.ccu.send("pll init").readlines()        # make restart pll
      self.ccu.send("delay25 init").readlines()  # RCK,CTR,SDA,RDA,CLK + enable bits
      
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
    
   def poweron(self):
      print "Powering on group "+self.name
      # self.ccu.send("ccu "+ ccuAddress)
      self.ccu.send("power "+ self.name[-3:])
      
   def poweroff(self):
      print "Powering off group "+self.name
      # self.ccu.send("ccu "+ ccuAddress)
      self.ccu.send("power none")
      
        
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
      print "fec"
      self.InitFEC()
      print "caen"
      self.InitCAEN()
      print "pxfec"
      self.InitpxFEC()
   
      
      self.pxfec.send("cn %s",self.name).readlines()
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
   

   def GetSDARange1(self,verbose=True):
      print "fec"
      self.InitFEC()
      # print "caen"
      #self.InitCAEN()
      print "pxfec"
      self.InitpxFEC()
      
      value=[[],[]]
      ia0=[0.0,0.0]
      ia1=[0.0,0.0]
      caengroup=["L14","L23"]
      k=1
      print "caen"
      for l in range(0,len(caengroup)):
            print self.caen.send("group "+ self.name[:-3] +caengroup[l]).readlines()
            print self.caen.send("pon").readlines()
            if (self.IsCAENOn()):
               print "Group "+ self.name[:-3] +caengroup[l]+": CAEN: power On"
            else:
               print "Group "+ self.name[:-3] +caengroup[l]+": CAEN: Error when turning power on"
      
      for d2 in range (0,63,2): #scan SDA values
         
         self.ccu.send("delay25 s d2 %d"%(d2)).readlines()
         if (verbose): print "SDA: ", d2
         self.ccu.send( "group " + self.name).readlines()
         self.ccu.send("delay25 s d2 %d"%(d2)).readlines()
         self.pxfec.send("cn " +self.name).readlines()
         self.pxfec.send("cn reset roc").readlines()                #reset roc
         self.pxfec.send("module 0:31 roc 0:15 Vdig 0").readlines() #avoid high digital currents
         self.pxfec.send("module 0:31 roc 0:15 Vsf 0").readlines()  #avoid high digital currents
         self.pxfec.send("module 0:31 roc 0:15 Vana 0").readlines() #set Vana=0
         self.pxfec.send("cn reset roc").readlines()
         sleep(0.5)
         for l in range(0,len(caengroup)):
            self.caen.send("group "+ self.name[:-3] +caengroup[l]).readlines()
            ia0[l]=self.caen.fquery("get ia")
               

         
         self.pxfec.send("cn " +self.name).readlines()
         self.pxfec.send("module 0:31 roc 0:15 Vana 120").readlines()        #set Vana=120
         sleep(0.5)
         for l in range(0,len(caengroup)):
            self.caen.send("group "+ self.name[:-3] +caengroup[l]).readlines()
            ia1[l]=self.caen.fquery("get ia")
      

         for l in range(0,2):
            if ia1[l]-ia0[l]>0.5: value[l].append(d2)
            #print str(ia1[l]) +" - " +str(ia0[l]) + " = " +  str(ia1[l]-ia0[l])
            
      print value
      
      good=list(set(value[0]) and setset(value[1]))
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

  
 
   #for testing
   def ScanSDARDADummy(self):
      map={}
      for d3 in range(0,63):
         for d2 in range(0,63):
            a=[0,1,4,5,6,7,8,9,11,12,13,14]
            map[d2,d3]=a
      return map


   def GetSDARDA1(self,realmodule,mino=3):# x1-x2 min-max RDA scan range,y1-y2 min-max SDA scan range, both 0-63 
      self.delay25.SendValues()
      self.InitpxFEC()
      temp=[]
      memo=0
      temp2=[]
      rdasetting=[]
      sdasetting=[]
      count=0
      counter=0
      flipflop=True
      validd2range=[]
      validd2mean=[]
      for d2 in range(0,64): # scan sda
         temp.append([])
         temp[d2].append(d2)
         self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
         a= self.pxfec.aquery("cn scan hubs")
         temp[d2].append(list(set(a)&set(realmodule)))
      for i in temp:
         print i
         if len(i[1]) !=0:
          validd2range.append(i[0])      
      temp=sorted(validd2range)
      print temp
      if len(temp)>0:
         for i in range(1,len(temp)):
            if temp[i]-temp[i-1]>1 or i==len(temp)-1:
               temp2=temp[memo:i]
               memo=i
               print temp2 ," sum ",sum(temp2), " len ",len(temp2)
               print "mean ", sum(temp2)/len(temp2)
               if len(temp2)>2:
                  validd2mean.append(sum(temp2)/len(temp2))
                  
      print "d2value ",validd2mean
      temp=[]
      for i in realmodule:
         temp.append([])
      for d2 in validd2mean:    #scan Rda
         
         for d1 in range(0,64):
            self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
            self.ccu.send("delay25 set d1 %d"%(d1)).readlines()
            a= self.pxfec.aquery("cn scan hubs")
            for i in set(a)&set(realmodule):
               for k in range(0,len(realmodule)):
                  if i==realmodule[k]:
                      temp[k].append(d1)
         for k in range(0,len(realmodule)):
            for i in range(1,len(temp[k])):
               if temp[k][i]-temp[k][i-1]>1 or i==len(temp[k])-1:
                  temp2=temp[k][memo:i]
                  memo=i
                  print "module ",realmodule[k]
                  print temp2 ," sum ",sum(temp2), " len ",len(temp2)
                  print "mean ", sum(temp2)/len(temp2) 
      ##############temp[[mean d2,d1,module1,module2..],[mean d2,d1,module1,module2..] ]
            
       


   def GetSDARDA(self,realmodule,mino=3):# x1-x2 min-max RDA scan range,y1-y2 min-max SDA scan range, both 0-63 
      self.delay25.SendValues()
      self.InitpxFEC()
      temp=[]
      temp2=[]
      todo=[]
      rdasetting=[]
      sdasetting=[]
      count=0
      counter=0
      flipflop=True
      for i in realmodule:
         rdasetting.append([])
      for d2 in range(0,64): # scan sda
         for d1 in range(0,64):    #scan Rda
            todo.append([])
            todo[-1].append(d1)
            todo[-1].append(d2)
            #################################
            # counter=counter+1
            # if count ==64:
            #    flipflop=False
            #    count=count-1
            #    print " ",100*counter/4096,"%",
            # print" "
            # if count ==0:
            #    flipflop=True
            # if count<64 and flipflop==True:
            #    count=count+1
            #    for n in range(0,count):
            #       print "#",
            # if count<64 and flipflop==False:
            #    for n in range(0,count):
            #       print "#",
            #    count=count-1
            #################################
               

            sendd1=self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
            sendd2=self.ccu.send("delay25 set d1 %d"%(d1)).readlines()
            print send1
            a= self.pxfec.aquery("cn scan hubs")
            # print a
            todo[-1].append(list(set(a)&set(realmodule)))
            for i in set(a)&set(realmodule):
               for k in range(0,len(realmodule)):
                  if i==realmodule[k]:
                     rdasetting[k].append(d1)
            
            for i in a:
               if i not in temp:
                  temp.append(i)
            # print "diff ",set(realmodule)-set(a)
         if len(set(realmodule)-set(temp))==0:
            sdasetting.append(d2)
            temp=[]
            # print"d1(rda) ",d1, " HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
      print "###############################################################################################"
      memo=0
      temp=[]
      for i in sdasetting:
         if i not in temp:
            temp.append(i)
      temp=sorted(temp)
      print temp
      if len(temp)>0:
         for i in range(1,len(temp)):
            if temp[i]-temp[i-1]>1 or i==len(temp)-1:
               temp2=temp[memo:i]
               memo=i
               print temp2 ," sum ",sum(temp2), " len ",len(temp2)
               print "mean ", sum(temp2)/len(temp2)
               # temp2=temp[i:] 
               # print temp2 ," sum ",sum(temp2), " len ",len(temp2)
               # print "mean ", sum(temp2)/len(temp2)
               # break
      print "###############################################################################################"
      for k in  range(0,len(realmodule)):
         print "realmodule ", realmodule[k]
         memo=0
         temp=[]
         for i in rdasetting[k]:
             if i not in temp:
                temp.append(i)
         temp=sorted(temp)
         for i in range(1,len(temp)):
             if temp[i]-temp[i-1]>1 or i==len(temp)-1:
                temp2=temp[memo:i]
                memo=i
                print temp2 ," sum ",sum(temp2), " len ",len(temp2)
                print "mean ", sum(temp2)/len(temp2)
                # temp2=temp[i:]
                # print temp2 ," sum ",sum(temp2), " len ",len(temp2)
                # print "mean ", sum(temp2)/len(temp2)
                # break
                



   def GetSDARDA2(self,realmodule,mino=3):# x1-x2 min-max RDA scan range,y1-y2 min-max SDA scan range, both 0-63 
      self.delay25.SendValues()
      self.InitpxFEC()
      modulesdarda=[]
      temp=[]
      temp1=[]
      temp2=[]
      temp3=[]
      memo=0
      histo=[]
      name="histo"
      n=0
      dummypoint=[]
      for i in realmodule:
            modulesdarda.append([]) 
            # temp1.append([])
            name= "module " + str(realmodule[n])
            histo.append(TH2D( name , name , 64,0,63,64,0,63 ))
            n=n+1
      count=0
      counter=0
      flipflop=True
      dummyrange=range(0,64)
      for d2 in dummyrange: # scan sda
         for d1 in dummyrange:    #scan Rda
            counter =counter+1
            #################################

            # if count ==64:
            #    flipflop=False
            #    count=count-1
            #    print " ",100*counter/4096,"%",
            # print" "
            # if count ==0:
            #    flipflop=True
            # if count<64 and flipflop==True:
            #    count=count+1
            #    for n in range(0,count):
            #       print "#",
            # if count<64 and flipflop==False:
            #    for n in range(0,count):
            #       print "#",
            #    count=count-1
            #################################
               

            self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
            self.ccu.send("delay25 set d1 %d"%(d1)).readlines()
            a= self.pxfec.aquery("cn scan hubs")
            print d1, " ", d2, " " , a
            for i in set(a)&set(realmodule):
               for k in range(0,len(realmodule)):
                  if i==realmodule[k]:
                     modulesdarda[k].append([d1,d2])
                     histo[k].Fill(d1,d2,1)
      n=0       
      for i in modulesdarda:
         
         print "module ", realmodule[n]
         
         temp=sorted(i, key=itemgetter(0))
         for i in range(1,len(temp)):
             if temp[i][0]-temp[i-1][0]<2:
                temp1.append(temp[i-1])
                memo=i
             if temp[i][0]-temp[i-1][0]>1 or i==len(temp)-1:
                # print temp1     
                temp4=[]
                temp4=sorted(temp1, key=itemgetter(1))
                temp1=[]
                for i in range(1,len(temp4)):
                   if temp4[i][1]-temp4[i-1][1]<2 :
                      temp1.append(temp4[i])
                      temp2.append(temp4[i][0])
                      temp3.append(temp4[i][1])
                      
                   if temp4[i][1]-temp4[i-1][1]>1 or i==len(temp4)-1:
                      print "temp2 ",temp2 
                      print "temp3 ",temp3
                      if len(temp2) >1:
                         print "module ", realmodule[n],  " d1,d2 = ( " ,  sum(temp2)/len(temp2), " " ,sum(temp3)/len(temp3), " )"
                         histo[n].Fill(sum(temp2)/len(temp2),sum(temp3)/len(temp3),10)
                      else:
                         print "module ", realmodule[k]," problem"
                      temp1=[]
                      temp2=[]
                      temp3=[]
                      # temp4=[]
         n=n+1
                      
      return histo
   #############################################################################################
   

   def GetSDARDA3(self,realmodule,):# x1-x2 min-max RDA scan range,y1-y2 min-max SDA scan range, both 0-63 
      self.delay25.SendValues()
      self.InitpxFEC()
      modulesdarda=[]
      temp=[]
      temp1=[]
      temp2=[]
      temp3=[]
      memo=0
      histo=[]
      name="histo"
      n=0
      dummypoint=[]
      for i in realmodule:
            modulesdarda.append([]) 
            # temp1.append([])
            name= "module " + str(realmodule[n])
            histo.append(TH2D( name , name , 64,0,63,64,0,63 ))
            n=n+1
      count=0
      counter=0
      flipflop=True
      dummyrange=range(40,60)
      for d2 in dummyrange: # scan sda
         for d1 in dummyrange:    #scan Rda
            counter =counter+1
            self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
            self.ccu.send("delay25 set d1 %d"%(d1)).readlines()
            a= self.pxfec.aquery("cn scan hubs")
            print d1, " ", d2, " " , a
            for i in set(a)&set(realmodule):
               for k in range(0,len(realmodule)):
                  if i==realmodule[k]:
                     modulesdarda[k].append([d1,d2])
                     histo[k].Fill(d1,d2,1)
      n=0       
      for i in modulesdarda:
         
         print "module ", realmodule[n]
         
         temp=sorted(i, key=itemgetter(0))
         for i in range(1,len(temp)):
             if temp[i][0]-temp[i-1][0]<2:
                temp1.append(temp[i-1])
                memo=i
             if temp[i][0]-temp[i-1][0]>1 or i==len(temp)-1:
                # print temp1     
                temp4=[]
                temp4=sorted(temp1, key=itemgetter(1))
                temp1=[]
                for i in range(1,len(temp4)):
                   if temp4[i][1]-temp4[i-1][1]<2 :
                      temp1.append(temp4[i])
                      temp2.append(temp4[i][0])
                      temp3.append(temp4[i][1])
                      
                   if temp4[i][1]-temp4[i-1][1]>1 or i==len(temp4)-1:
                      print "temp2 ",temp2 
                      print "temp3 ",temp3
                      if len(temp2) >1:
                         print "module ", realmodule[n],  " d1,d2 = ( " ,  sum(temp2)/len(temp2), " " ,sum(temp3)/len(temp3), " )"
                         histo[n].Fill(sum(temp2)/len(temp2),sum(temp3)/len(temp3),10)
                      else:
                         print "module ", realmodule[k]," problem"
                      temp1=[]
                      temp2=[]
                      temp3=[]
                      # temp4=[]
         n=n+1
                      
      return histo
   #############################################################################################
   def ScanSDARDA(self,x1,x2,y1,y2):# x1-x2 min-max RDA scan range,y1-y2 min-max SDA scan range, both 0-63 
      #self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      max=(x2-x1)*(y2-y1)
      n=1
      for d1 in range(x1,x2):                                    #scan RDA 
         for d2 in range(y1,y2):
            self.ccu.send("delay25 set d1 %d"%(d1)).readlines()
            self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
            stop=""
            #stop= self.pxfec.send("cn "+ self.name + " module 0:31 roc 0:15 Vana 0").readlines()
            #self.pxfec.send("cn reset roc").readlines()         #reset ROC
            a=self.pxfec.aquery("cn scan hubs")                 #check status
            print "SDA d2= ", d2, "RDA d1= ", d1, a
            map[d2,d1]=a
         # time.sleep(2)
            if (stop ==['unknown ControlNetwork command '] ): 
               print "GROUP "+ self.name + " UNKNOWN"
               break
      self.ccu.send("delay25 set d2 2").readlines()
      self.ccu.send("delay25 set d1 0").readlines()
      # print map
      return map 
   def ScanSDAClock(self):
      #self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      for d3 in range(0,63):                                    #scan RDA 
         self.ccu.send("delay25 set d4 %d"%(d3)).readlines()
         for d2 in range(0,63):                                 #scan SDA
            self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
            #print "SDA = ", d2, "Clock = ", d3
            self.pxfec.send("cn reset roc").readlines()         #reset ROC
            a=self.pxfec.aquery("cn scan hubs")                 #check status
            #print a
            map[d2][d1]=a
           # map[d2,d1]=a
      self.ccu.send("delay25 set d2 48").readlines()
      self.ccu.send("delay25 set d4 0").readlines()
      return map  


   def ScanSDA(self):
      #self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      
      for d2 in range(0,63):                                 #scan SDA
         self.ccu.send("delay25 set d2 %d"%(d2)).readlines()
         print "SDA = ", d2
         #self.pxfec.send("module 0:31 tbm disableclock").readlines()         #reset ROC
         self.pxfec.send("module 0:31 roc 0:15 Vana 0").readlines()   
         time.sleep(3)
         self.pxfec.send("module 0:31 roc 0:15 Vana 255").readlines()   
      self.ccu.send("delay25 set d2 48").readlines()
      self.ccu.send("delay25 set d3 0").readlines()
      return map

   def ScanRDARCK(self):
      self.InitFEC()
      self.delay25.SendValues()
      self.InitpxFEC()
      map={}
      for d1 in range(0,63):                                    #scan RDA
         self.ccu.send("delay25 set d1 %d"%(d1)).readlines()
         #for d0 in range(0,63):
         for d0 in range(0,1):
            self.ccu.send("delay25 set d0 %d"%(d0)).readlines() #scan RCK
            print "RCK = ", d0, "RDA = ", d1
            self.pxfec.send("cn reset roc").readlines()         #reset ROC
            a=self.pxfec.aquery("cn scan hubs")                 #check status
            print a
            map[d1,d0]=a
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
      self.SDA=4                 # SDA
      self.RDA=0                  # RDA
      self.clock=0                # clock
      self.programming=False      # i2c programming
      self.log=log                # message logger

   def VerifyProgramming(self):
      # print "our names ", self.name, self.channel
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
      self.ccu.send("delay25 s d3 %d"%(value)).readlines()
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
      self.ccu.send("delay25 s d1 %d"%(value)).readlines()
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
      regexp="-->Delay3 : %d"
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
      regexp="-->Delay1 : %d"
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
      # print cmd
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
   def __init__(self, fed, ccu, group,  name, fedchannels,log=None):
      self.fed=fed              # fec access (SimpleCommand)
      self.ccu=ccu              # ccu/aoh access (SimpleCommand) 
      self.group=group          # group identifier
      self.sectorname=group[:3]
      self.name=name            # aoh1/aoh2 for layer3, aoh1..4 for layer1&2
      layer=self.group[3:]
      if layer=="L12":
         self.logname=name
      elif layer=="L34":
         self.logname="aoh"+str(int(name[-1])+4)
      else:
         self.logname="???????"
      self.fedchannelsdefault=[None]+fedchannels # the six fed channels for a0 a1 a2 b0 b1 b2
      self.fedchannels=self.fedchannelsdefault
      self.cmd=[None]                     # command templates , aohxx set [b/g]x [value]"
      self.aoh=[None]
      for ch in range(5):
         self.aoh.append(name)
         self.cmd.append(name+" %s "+str(ch)+" %d")
      self.programming=False
      self.log=log

   def VerifyProgramming(self): 
      if ( self.writebias(1,11) and self.writebias(2,12) and self.writebias(3,13) and self.writebias(4,14) and self.writebias(5,15) and self.writebias(6,16) and
           self.writegain(1,0)  and self.writegain(2,1)  and self.writegain(3,2)  and self.writegain(4,0)  and self.writegain(5,1)  and self.writegain(6,2)  ):
         print "Group "+self.group+":\t "+self.name+": \t\tprogramming OK"
         if self.log: self.log.ok("Group "+self.group+":\t "+self.name+": \t\tprogramming OK")
         self.programming=True
      else:
         print "Group "+self.group+":\t "+self.name+": \t\tERROR in programming"
         if self.log: self.log.error("Group "+self.group+":\t "+self.name+": \t\tERROR in programming")
         #sys.exit(1)

   def readbias(self,ch):
      # ch 1..6
      #print "readbias", ch
      #print self.group
      self.ccu.send("group "+self.group).readlines()
      #self.ccu.send("reset").readlines()
      query=self.aoh[ch]+" read"
      regexp="-->Bias channel "+str((ch-1)%4)+": %d"
      #print query
      #print regexp
      #print self.ccu
      return self.ccu.iquery(query,regexp) 
      #result = self.ccu.iquery(query,regexp)    
      #print result
      #return result
      
   def writebias(self,ch,bias,verify=True):
      # AOH
      # ch 1..6, bias: 0..127
      if not 1<=ch<=4 and 0<=bias<=127:
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

   def getBiasRMSCurves(self):
      #print "start"
      data=[None]+[[] for ch in range(6)]
      #self.fed.send("fed "+str(self.fedslot)).readlines()
      oldvalues=[self.readbias(ch) for ch in range(1,5)]
      for ch in range(1,5): self.writebias(ch,0)  # will also set the group for the ccu
      #print "wrote bias"

      debug = 0
      biasRange=range(0,60,30)
      nbp=0
      for bias in biasRange:
         if (debug>0):
            print "setting bias: %i" %bias
         for ch in range(1,5):
            self.writebias(ch,bias,verify=False)
            cmd=self.aoh[ch]+" set b"+str((ch-1)%4)+" %d"%bias
            self.ccu.send(cmd).readlines()
            
         if (debug>0):
            print "resetting fifo"
         self.fed.send("reset fifo").readlines()
         
         if(debug>0):
            print "reading rms"
         chsum=0
         for ch in range(1,5):
            if self.fedchannels[ch]==0: continue
            response=self.fed.query("rms %d"%self.fedchannels[ch])
            #if (debug>0):
            #   print response
            channel,mean,rms,name=response.split()
            data[ch].append([bias,float(mean),float(rms),0.])
            if (debug>0):
               print mean," ",
            chsum+=float(mean)
            cmd=self.aoh[ch]+" set b"+str((ch-1)%4)+" %d"%0
            self.ccu.send(cmd).readlines()
         if (debug>0):
            print "sum=",chsum
         nbp+=1
         if chsum>=(1023.*6): break

      for ch in range(1,5):
         self.writebias(ch,oldvalues[ch-1])
      self.fed.send("reset fifo").readlines()

      # calculate the slopes
      # for ch in range(1,5):
      #    #print "AOH channel "+str(ch)
      #    if self.fedchannels[ch]==0:
      #       continue
      #    # also count data points with high noise or low slope and try to find a good working point
      #    nslope,nnoise,b500=0,0,0
      #    #print [data[ch][i][1] for i in range(nbp)]
      #    # first point 
      #    data[ch][0][3]=float(data[ch][1][1]-data[ch][0][1])/float(data[ch][1][0]-data[ch][0][0])
      #    # intermediate points
      #    for idx in range(1,nbp-1):
      #       data[ch][idx][3]=0.5*float(data[ch][idx+1][1]-data[ch][idx-1][1])/float(data[ch][idx+1][0]-data[ch][idx-1][0])
      #    # last point
      #    data[ch][-1][3]=float(data[ch][-1][1]-data[ch][-2][1])/float(data[ch][-1][0]-data[ch][-2][0])

      #    # count data points with high noise or low slope and try to find a good working point
      #    nslope,nnoise,b500=0,0,0
      #    for idx in range(nbp):
      #       if data[ch][idx][3]>30: nslope+=1
      #       if data[ch][idx][2]>5.: nnoise+=1
      #       if data[ch][idx][1]>500 and b500==0: b500=data[ch][idx][0]
      #    if nslope<5: print self.name+"-"+str(ch)+" low slope"
      #    if nnoise>5: print self.name+"-"+str(ch)+" noisy"
      #    if b500>35:  print self.name+"-"+str(ch)+" high working point"
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
  
#########################################
class POH:
   def __init__(self, fed, ccu, group,  name, bundle, fedchannels,log):
      self.fed=fed              # fec access (SimpleCommand)
      self.ccu=ccu              # ccu/aoh access (SimpleCommand) 
      self.group=group          # group identifier
      self.sectorname=group[:34]
      self.name=name            # aoh1/aoh2 for layer3, aoh1..4 for layer1&2
      layer=self.group[34:]
      if layer=="L12":
         self.logname=name
      elif layer=="L34":
         self.logname="poh"+str(int(name[-1])+4)
      else:
         self.logname="???????"
      self.bundle=bundle
      self.fedchannelsdefault=fedchannels # the six fed channels for a0 a1 a2 b0 b1 b2
      self.fedchannels=self.fedchannelsdefault
      self.cmd=[None]                     # command templates , aohxx set [b/g]x [value]"
      self.poh=[None]
      for ch in range(5):
         self.poh.append(name)
         self.cmd.append(name+" %s "+str(ch)+" %d")
      self.programming=False
      self.log=log
###
# if ( self.writebias(0,30) and self.writebias(1,31) and self.writebias(2,32) and self.writegain(0,0) and self.writegain(1,1) and self.writegain(2,2)  ):

###


   def SearchPOH(self): 
         if (self.writebias(1,20)  and self.writegain(1,0)   ):
            print "Group "+self.group+":\t "+self.name+" found"
          #if self.log: self.log.ok("Group "+self.group+":\t "+self.name+": \t\tprogramming OK")
          #self.programming=True
         else:
            print "Group "+self.group+":\t "+self.name+" not found"
         #if self.log: self.log.error("Group "+self.group+":\t "+self.name+": \t\tERROR in programming")
         #sys.exit(1)




   def VerifyProgramming(self): 
      if ( self.writebias(1,20) and self.writebias(2,21) and self.writebias(3,29)  and self.writebias(4,30) and
       self.writegain(1,0)  and self.writegain(2,1)  and self.writegain(3,2)    and self.writegain(4,3)  ):
     
         #print "Group "+self.group+":\t "+self.name+": \t\tprogramming OK"
          if self.log: self.log.ok("Group "+self.group+":\t "+self.name+": \t\tprogramming OK")
          self.programming=True
      else:
         #print "Group "+self.group+":\t "+self.name+": \t\tERROR in programming"
         if self.log: self.log.error("Group "+self.group+":\t "+self.name+": \t\tERROR in programming")
         #sys.exit(1)

   def readbias(self,ch):
      # ch 1..6
      #print "readbias", ch
      #print self.group
      self.ccu.send("group "+self.group).readlines()
      #self.ccu.send("reset").readlines()
      query=self.poh[ch]+" read"
      regexp="-->Bias channel "+str((ch-1)%4)+": %d"
      #print query
      #print regexp
      #print self.ccu
      return self.ccu.iquery(query,regexp) 
      #for l in self.ccu.send(query).readlines():
      #   print "answer ", l
      #return 0
      #result = self.ccu.iquery(query,regexp)    
      #print result
      #return result
      
   def writebias(self,ch,bias,verify=True):
      # POH
      # ch 1..6, bias: 0..127
      if not 1<=ch<=4 and 0<=bias<=127:
         print "invalid values in POH.writebias(ch=%d,bias=%d)"%(ch,bias)
         return False
      self.ccu.send("group "+self.group).readlines()
      #self.ccu.send("reset").readlines()
      cmd=self.poh[ch]+" set b"+str((ch-1)%4)+" %d"%bias
      #print cmd
      self.ccu.send(cmd).readlines()
      #for l in self.ccu.send(cmd).readlines()
         #print l
      if verify:
         readvalue=self.readbias(ch)
         if not readvalue==bias:
            print "verification of bias setting failed: wrote ",bias," got ",readvalue
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
      query=self.poh[ch]+" read"
      regexp="-->Gain channel "+str((ch-1)%4)+": %d"
      return self.ccu.iquery(query,regexp)      
      
   def writegain(self,ch,gain):
      # ch 1..6, bias: 0..127
      self.ccu.send("group "+self.group).readlines()
      #self.ccu.send("reset").readlines()
      cmd=self.poh[ch]+" set g"+str((ch-1)%4)+" %d"%gain
      #print cmd
      self.ccu.send(cmd).readlines(),
      verify=self.readgain(ch)
      if not verify==gain:
         print "verification of gain setting failed: wrote ",gain," got ",verify
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
   #POH
   def getBiasRMSCurves(self):
      #print "start"
      data=[None]+[[] for ch in range(6)]
      # self.fed.send("fed "+str(self.fedslot)).readlines()
      
      oldvalues=[self.readbias(ch) for ch in range(1,5)]
      for ch in range(1,5): self.writebias(ch,0)  # will also set the group for the ccu
      #print "wrote bias"
      ##########################################################
      list 
      self.fed.send("set_offset_dac " + str(1) + " ").readlines()
      ###########################################################################3    
      debug = 0
      biasRange=range(0,60,2)
      nbp=0
      for bias in biasRange:
         # if (debug>0):
         #    print "setting bias: %i" %bias
         for ch in range(1,5):
            self.writebias(ch,bias,verify=False)
            #cmd=self.poh[ch]+" set b"+str((ch-1)%4)+" %d"%bias
            #self.ccu.send(cmd).readlines()
            sleep(0.02)
            
         # if (debug>0):
         #    print "resetting fifo"
         self.fed.send("reset fifo").readlines()
         
         # if(debug>0):
         #    print "reading rms"
         chsum=0
         for ch in range(1,5):
            if self.fedchannels[ch]==0: continue
            response=self.fed.query("rms %d"%self.fedchannels[ch])
            # if (debug>0):
            #    print "rms %d"%self.fedchannels[ch]
            #    print response
            channel,mean,rms,name=response.split()
            data[ch].append([bias,float(mean),float(rms),0.])
            # if (debug>0):
            #    print mean," ",
            chsum+=float(mean)
            cmd=self.poh[ch]+" set b"+str((ch-1)%4)+" %d"%0
            self.ccu.send(cmd).readlines()
         # if (debug>0):
         #    print "sum=",chsum
         nbp+=1
         if chsum>=(1023.*6): break

      for ch in range(1,5):
         self.writebias(ch,oldvalues[ch-1])
      self.fed.send("reset fifo").readlines()

      # # calculate the slopes
      # for ch in range(1,5):
      #    #print "AOH channel "+str(ch)
      #    if self.fedchannels[ch]==0:
      #       continue
      #    # also count data points with high noise or low slope and try to find a good working point
      #    nslope,nnoise,b500=0,0,0
      #    #print [data[ch][i][1] for i in range(nbp)]
      #    # first point 
      #    data[ch][0][3]=float(data[ch][1][1]-data[ch][0][1])/float(data[ch][1][0]-data[ch][0][0])
      #    # intermediate points
      #    for idx in range(1,nbp-1):
      #       data[ch][idx][3]=0.5*float(data[ch][idx+1][1]-data[ch][idx-1][1])/float(data[ch][idx+1][0]-data[ch][idx-1][0])
      #    # last point
      #    data[ch][-1][3]=float(data[ch][-1][1]-data[ch][-2][1])/float(data[ch][-1][0]-data[ch][-2][0])

      #    # count data points with high noise or low slope and try to find a good working point
      #    nslope,nnoise,b500=0,0,0
      #    #print "--------- channel ", ch
      #    for idx in range(nbp):
      #       if data[ch][idx][3]>30: nslope+=1
      #       if data[ch][idx][2]>5.: nnoise+=1
      #       if data[ch][idx][1]>500 and b500==0: b500=data[ch][idx][0]
      #       #print "slope ", data[ch][idx][3], nslope 
      #       #print "noise ", data[ch][idx][2], nnoise 
      #       #print "bias  ", data[ch][idx][1], b500
      #       #print " " 
      #    if nslope<5: print self.name+"-"+str(ch)+" low slope"
      #    if nnoise>5: print self.name+"-"+str(ch)+" noisy"
      #    if b500>35:  print self.name+"-"+str(ch)+" high working point"
      return data

 

   def ScanDACoffset(self):
      #print "start"
      data=[None]+[[] for ch in range(4)]
      # self.fed.send("fed "+str(self.fedslot)).readlines()
      
     # oldvalues=[self.readbias(ch) for ch in range(1,5)]
      for ch in range(1,5): self.writebias(ch,0)  # will also set the group for the ccu
      #print "wrote bias"

      debug = 0
      #DACRange=range(150,254,2)
      DACRange=range(0,254,2)
      #DACRange=range(0,15,1)
      nbp=0
      self.fed.send("set_opt_inadj 1  9").readlines()
      for bias in DACRange:
         #if (debug>0):
         print "setting dac: %i" %bias
         # for ch in range(1,5):
         #    self.writebias(ch,bias,verify=False)
         #    cmd=self.poh[ch]+" set b"+str((ch-1)%4)+" %d"%bias
         #    self.ccu.send(cmd).readlines()
            
         if (debug>0):
            print "resetting fifo"
         self.fed.send("reset fifo").readlines()
         
         if(debug>0):
            print "reading rms"
         chsum=0
         for ch in range(1,5):
            if self.fedchannels[ch]==0: continue
            self.fed.send("set_offset_dac " + str(self.fedchannels[ch]) + " %d "%bias).readlines()
            #self.fed.send("set_opt_inadj 1  %d "%bias).readlines()
            #self.fed.send("set_opt_inadj 2  %d "%bias).readlines()
            #self.fed.send("set_opt_inadj 3  %d "%bias).readlines()
            #self.fed.send("reset fifo").readlines()
            response=self.fed.query("rms %d"%self.fedchannels[ch])
            #if (debug>0):
            print response
            channel,mean,rms,name=response.split()
            data[ch].append([bias,float(mean),float(rms),0.])
            if (debug>0):
               print mean," ",
            chsum+=float(mean)
            #cmd=self.poh[ch]+" set b"+str((ch-1)%4)+" %d"%0
            #self.ccu.send(cmd).readlines()
         if (debug>0):
            print "sum=",chsum
         nbp+=1
         if chsum>=(1023.*6): break

     # for ch in range(1,5):
         #self.writebias(ch,oldvalues[ch-1])
      self.fed.send("reset fifo").readlines()

      # calculate the slopes
      # for ch in range(1,5):
      #    #print "AOH channel "+str(ch)
      #    if self.fedchannels[ch]==0:
      #       continue
      #    # also count data points with high noise or low slope and try to find a good working point
      #    nslope,nnoise,b500=0,0,0
      #    #print [data[ch][i][1] for i in range(nbp)]
      #    # first point 
      #    data[ch][0][3]=float(data[ch][1][1]-data[ch][0][1])/float(data[ch][1][0]-data[ch][0][0])
      #    # intermediate points
      #    for idx in range(1,nbp-1):
      #       data[ch][idx][3]=0.5*float(data[ch][idx+1][1]-data[ch][idx-1][1])/float(data[ch][idx+1][0]-data[ch][idx-1][0])
      #    # last point
      #    data[ch][-1][3]=float(data[ch][-1][1]-data[ch][-2][1])/float(data[ch][-1][0]-data[ch][-2][0])

      #    # count data points with high noise or low slope and try to find a good working point
      #    nslope,nnoise,b500=0,0,0
      #    for idx in range(nbp):
      #       if data[ch][idx][3]>30: nslope+=1
      #       if data[ch][idx][2]>5.: nnoise+=1
      #       if data[ch][idx][1]>500 and b500==0: b500=data[ch][idx][0]
      #    if nslope<5: print self.name+"-"+str(ch)+" low slope"
      #    if nnoise>5: print self.name+"-"+str(ch)+" noisy"
      #    if b500>35:  print self.name+"-"+str(ch)+" high working point"
         

      return data




   def ScanDACInputOffset(self):
      #print "start"
      data=[None]+[[] for ch in range(4)]
      # self.fed.send("fed "+str(self.fedslot)).readlines()
      
     # oldvalues=[self.readbias(ch) for ch in range(1,5)]
      for ch in range(1,5): self.writebias(ch,0)  # will also set the group for the ccu
      #print "wrote bias"

      debug = 0
      #DACRange=range(150,254,2)
      #DACRange=range(0,254,2)
      DACRange=range(0,15,1)
      nbp=0
      for bias in DACRange:
         #if (debug>0):
         print "setting imput offset: %i" %bias
         self.fed.send("set_opt_inadj 1  %i " %bias).readlines()
         # for ch in range(1,5):
         #    self.writebias(ch,bias,verify=False)
         #    cmd=self.poh[ch]+" set b"+str((ch-1)%4)+" %d"%bias
         #    self.ccu.send(cmd).readlines()
            
         if (debug>0):
            print "resetting fifo"
         self.fed.send("reset fifo").readlines()
         
         if(debug>0):
            print "reading rms"
         chsum=0
         for ch in range(1,5):
            if self.fedchannels[ch]==0: continue
            #self.fed.send("set_offset_dac " + str(self.fedchannels[ch]) + " %d "%bias).readlines()
            #self.fed.send("set_opt_inadj 1  %d "%bias).readlines()
            #self.fed.send("set_opt_inadj 2  %d "%bias).readlines()
            #self.fed.send("set_opt_inadj 3  %d "%bias).readlines()
            #self.fed.send("reset fifo").readlines()
            response=self.fed.query("rms %d"%self.fedchannels[ch])
            #if (debug>0):
            print response
            channel,mean,rms,name=response.split()
            data[ch].append([bias,float(mean),float(rms),0.])
            if (debug>0):
               print mean," ",
            chsum+=float(mean)
            #cmd=self.poh[ch]+" set b"+str((ch-1)%4)+" %d"%0
            #self.ccu.send(cmd).readlines()
         if (debug>0):
            print "sum=",chsum
         nbp+=1
         if chsum>=(1023.*6): break

     # for ch in range(1,5):
         #self.writebias(ch,oldvalues[ch-1])
      self.fed.send("reset fifo").readlines()

      # calculate the slopes
      # for ch in range(1,5):
      #    #print "AOH channel "+str(ch)
      #    if self.fedchannels[ch]==0:
      #       continue
      #    # also count data points with high noise or low slope and try to find a good working point
      #    nslope,nnoise,b500=0,0,0
      #    #print [data[ch][i][1] for i in range(nbp)]
      #    # first point 
      #    data[ch][0][3]=float(data[ch][1][1]-data[ch][0][1])/float(data[ch][1][0]-data[ch][0][0])
      #    # intermediate points
      #    for idx in range(1,nbp-1):
      #       data[ch][idx][3]=0.5*float(data[ch][idx+1][1]-data[ch][idx-1][1])/float(data[ch][idx+1][0]-data[ch][idx-1][0])
      #    # last point
      #    data[ch][-1][3]=float(data[ch][-1][1]-data[ch][-2][1])/float(data[ch][-1][0]-data[ch][-2][0])

      #    # count data points with high noise or low slope and try to find a good working point
      #    nslope,nnoise,b500=0,0,0
      #    for idx in range(nbp):
      #       if data[ch][idx][3]>30: nslope+=1
      #       if data[ch][idx][2]>5.: nnoise+=1
      #       if data[ch][idx][1]>500 and b500==0: b500=data[ch][idx][0]
      #    if nslope<5: print self.name+"-"+str(ch)+" low slope"
      #    if nnoise>5: print self.name+"-"+str(ch)+" noisy"
      #    if b500>35:  print self.name+"-"+str(ch)+" high working point"
         

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
      self.rda=rda
      self.sda=sda
      self.channel=[]
           
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

   verbose = False
   #test FEC output B and first CCU input B
   #ccu.send("fec %s"%trfec).readlines()
   #ccu.send("ring %s"%ring).readlines()
   ccu.send("reset").readlines()
   ccu.send("piareset all").readlines()
   #ccu.send("cratereset").readlines()
   ccu.send("redundancy fec a b").readlines()
   ccu.send("redundancy ccu 0x7e b a").readlines()
   ccu.send("scanccu")
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
      #print "FEC A B CCU 0x7e B A               : OK "
      dummy=1
   else :
      print "FEC A B CCU 0x7e B A               : ERROR "
      passed=False

   # test FEC output B and second CCU input B
   #ccu.send("cratereset").readlines()
   ccu.send("reset").readlines()
   ccu.send("piareset all").readlines()
   ccu.send("redundancy fec a b").readlines()
   ccu.send("redundancy ccu 0x7d b a").readlines()
   ccu.send("scanccu")
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
      #print "FEC A B CCU 0x7d B A               : OK "
      dummy=1
   else :
      print "FEC A B CCU 0x7d B A               : ERROR "


   #test bypass CCU 0x7d,...,0x78
   ccuAddress=0x7e
   for i in range (6):
      ccuAddressOutputB="0x%x"%(ccuAddress)
      ccuAddressInputB="0x%x"%(ccuAddress-2)
      ccu.send("reset").readlines()
      ccu.send("piareset all").readlines()
      # ccu.send("cratreset").readlines()
      ccu.send("redundancy ccu  %s  a b"%(ccuAddressOutputB)).readlines()
      ccu.send("redundancy ccu  %s  b a"%(ccuAddressInputB)).readlines()
      ccu.send("scanccu").readlines()
      ccu.send("scanccu")
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
         #print "CCU %s A B CCU %s B A          : OK "%(ccuAddressOutputB, ccuAddressInputB)
         dummy=1
      else :
         print "CCU %s A B CCU %s B A          : ERROR "%(ccuAddressOutputB, ccuAddressInputB)
         passed=False
      ccuAddress = ccuAddress-1

   #test FEC input B 
   # ccu.send("cratereset").readlines()
   ccu.send("reset").readlines()
   ccu.send("piareset all").readlines()
   ccu.send("redundancy ccu 0x77 a b").readlines()
   ccu.send("redundancy ccu 0x76 a a").readlines()
   ccu.send("redundancy fec b a").readlines()
   ccu.send("scanccu").readlines()
   ccu.send("scanccu")

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
      #print "CCU 0x77 A B CCU 0x76 A A FEC B A  : OK "
      dummy=1
   else :
      print "CCU 0x77 A B CCU 0x76 A A FEC B A  : ERROR "


   #test FEC input B with bypass of CCU 0x77 
   # ccu.send("cratereset").readlines()
   ccu.send("reset").readlines()
   ccu.send("piareset all").readlines()
   ccu.send("redundancy ccu 0x78 a b").readlines()
   ccu.send("redundancy ccu 0x76 b a").readlines()
   ccu.send("redundancy ccu 0x76 b a").readlines()
   ccu.send("redundancy fec b a").readlines()
   ccu.send("scanccu").readlines()
   ccu.send("scanccu")
   
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
      #print "CCU 0x78 A B CCU 0x76 B A FEC B A  : OK "
      dummy=1
   else :
      print "CCU 0x78 A B CCU 0x76 B A FEC B A  : ERROR "
      passed=False
      
   #reset fec
   # ccu.send("cratereset").readlines()
   ccu.send("reset").readlines()
   ccu.send("piareset all").readlines()
   if passed:
      log.ok("CCU redundancy test: OK")
   else:
      log.error("CCU redundancy test: FAILED")
######################################################################
def DCDCDisable(ccu,trfec=None,ring=None,verbose=False,log=None):

   verbose = False 
   # ccu.send("ccu "+ccuAddress).readlines()
   ndcdcok = 0
   for i in range (0,13):
      ccu.send("disable  dcdc %s"%(i))
      nok = 0
      for s in ccu.readline():
         if (verbose): print s
         m1=re.match(".*Disable OK*",s)
         #m2=re.match(".*Enable OK*",s)
         if m1:
            nok = nok + 1


      if (nok==1):
         print "DCDC dis %s:  OK"%(i)
         ndcdcok = ndcdcok + 1
      else:
         print "DCDC %s: ERROR"%(i)
      
   # if (ndcdcok==13):
   #    log.ok("DCDC enable test: OK")
######################################################################

def TestDCDCEnable(ccu,trfec=None,ring=None,verbose=False,log=None):

   verbose = False 
   # ccu.send("ccu "+ccuAddress).readlines()

   ndcdcok = 0
   for i in range (0,13):
      ccu.send("enabletest dcdc %s"%(i))

      nok = 0
      for s in ccu.readline():
         if (verbose): print s
         m1=re.match(".*Disable OK*",s)
         m2=re.match(".*Enable OK*",s)
         if m1:
            nok = nok + 1
         if m2:
            nok = nok + 1
               
      if (nok==2):
         print "DCDC %s: OK"%(i)
         ndcdcok = ndcdcok + 1
      else:
         print "DCDC %s: ERROR"%(i)

  
   # if (ndcdcok==13):
   #    log.ok("DCDC enable test: OK")
   # else:
   #    log.error("DCDC enable test: FAILED")



######################################################################


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





    
