########################################################
# SUPPLY TUBE TEST PROGRAM
########################################################
import sys,time,os, re, ROOT
from datetime import date
from time import sleep
sockdir="/home/cmspixel/TriDAS/pixel/BPixelTools/tools/python"
timer=[]
start_time=time.time()
timer.append("start")
timer.append(time.time()-start_time)
if not sockdir in sys.path: sys.path.append(sockdir)
########################################################
from SimpleSocket import SimpleSocket
from SystemTests import GROUP, SECTOR, TestRedundancy, TestTriggerStatusFED, tDOH, AOH, DELAY25, PLL, MODULE, TestDCDCEnable, DCDCDisable, POH
from Logger import Logger
from ROOT import *
from array import array
from contextlib import contextmanager
from pohgraph import suppress_stdout,graph,getFilename,showRMSPOH
###########################################################################################################################################################
########################################################
###############################
#######################
only23=True
inittest=False
redundancytest=False
resettest=False
fedchtest=False
pohbiastest=False
testsdarda=False
tbmplldelaytest=True
tbmplldelayonetest=False
pohdigitalfedtest=False
#######################
###############################
########################################################
###########################################################################################################################################################
def get_distinct(original_list):
    distinct_list = []
    for each in original_list:
        if each not in distinct_list:
            distinct_list.append(each)
    return distinct_list
########################################################
hList=[None]
gCanvases=[]
########################################################
# port
fechost = 'localhost'      
pxfechost = 'localhost' 
caenhost ='localhost'
fedhost = 'localhost'   
fecport =  2001  
fedport= 2004  
dfedport= 2006            
pxfecport= 2000              
caenport= 2005              
caenslot =4  

########################################################
ccu=SimpleSocket( fechost, fecport)
pxfec=SimpleSocket(fechost, pxfecport)
caen=SimpleSocket(caenhost,caenport)
fed=SimpleSocket(fedhost, fedport)
digfed=SimpleSocket( 'localhost', dfedport)
timer.append("Connection to simplesocket")
timer.append(time.time()-start_time)
########################################################
# print "sector and groups names definition"
log=Logger()
name= "+6P"
ccu.send("sector" + name   ).readlines()
out=( ccu.send( "which fec").readlines())
fecslot =int(out[1][-2:])
out=(ccu.send("which ring").readlines())
fecring =int(out[1][-2:])
sector= SECTOR(name,fed,ccu,pxfec,caen,log)
########################################################
module=[23,30]#,15,31]
d1=[13,53]#,48,42]
d2=[53,53]#,53,53]
tbmplldelayvalue=[[6,5],[6,5]]#],[3,3],[5,3]]
tbmplldelaydec=[]
fiber=[[11,12],[10,9]]#,[6,5],[8,7]]#,[],[]] # MAKE IT CONSISTENT
########################################################
ntrigger=1000#20000
slopethr=10 # threshold for the pohbias scan slope
print "power on mb and ccu, modules off "
#rep = raw_input(' when ready hit return: ')
########################################################
sector.pohlist= range(2,3)+range(4,5)+range(6,7)
sector.poh[2] =POH(fed, ccu, name+"L12","poh2", 1,[None, 5,6,7,8] ,log)
sector.poh[4] =POH(fed, ccu, name+"L12","poh3", 1,[None, 9,10,11,12] ,log)
sector.poh[6] =POH(fed, ccu, name+"L12","poh4", 1,[None, 1, 2, 3, 4] ,log)
# sector.poh[2].fedchannels=[None,5,6,7,8]
# sector.poh[4].fedchannels=[None,9,10,11,12]
# sector.poh[6].fedchannels=[None,1,2,3,4]
# for i in sector.pohlist:
#     sector.poh[i].bundle=1
sublist=[[2,4,6]]
########################################################
ccu.send("sector " + str(sector.name)).readlines()
ccu.send("reset").readlines()
ccu.send("piareset all").readlines()
########################################################
filename=getFilename("data/march3/2016poh%s.pdf"%(sector.name))
rootfile=TFile(os.path.splitext(filename)[0]+".root","RECREATE")
TCanvas('c1', '', 200, 10, 800, 800).Print(filename+"(")
########################################################
tag="FEC %d ring 0x%s :"%(fecslot,fecring)
fecringfound=False
fecfound=False
status=""
for l in ccu.send("mapccu").readlines():
    if l.strip().startswith("FEC %d"%(fecslot)):
        fecfound=True
    if l.strip().startswith(tag):
        fecringfound=True
        status=l.strip().split(":")[1].strip()
if not fecfound:
    log.error("No such FEC " +str(fecslot))
    sys.exit(1)
if not fecringfound:
    log.error("ring/mfec not found "+tag)
    sys.exit(1)
    
if status=="no CCU found":
    log.error("no CCU found "+tag)
    # print "verify ccu power"
    while True:
        yesno=raw_input("continue anyway (y/n) ")
        if yesno=='n': sys.exit(1)
        if yesno=='y': break

else:
    log.ok("CCUs found")    
timer.append("CCU mapping")
timer.append(time.time()-start_time)
########################################################
#print "start test"
########################################################
if only23==True:
    module=[23]
    d1=[13]
    d2=[53]
    tbmplldelayvalue=[[6,5]]
    tbmplldelaydec=[]
    fiber=[[11,12]]
    sector.pohlist= range(4,5)
    sector.poh[4] =POH(fed, ccu, name+"L12","poh3", 1,[None, 9,10,11,12] ,log)
    sublist=[[4]]
########################################################
if redundancytest==True:
    TestRedundancy(ccu,log=log) 
    timer.append("Redundancy test done")
    timer.append(time.time()-start_time)
########################################################
if resettest==True:
    sector.TestPIAResetDOH()
    sector.VerifySectorDevicesProgramming()
    for i in sector.group:
        ccu.send("group " + i.name).readlines()
        sector.ResetAllandInitAll() 
    timer.append("Reset Test done")
    timer.append(time.time()-start_time)


########################################################
if inittest==True:
    sector.init_tube()
########################################################

if fedchtest==True:
    counter =0
    errorc=0
    print "power on mb and ccu, modules off "
    print "START TRIGGER"
    rep = raw_input(' when ready hit return: ')
    print "POH bias scan"
    for n in sublist:
        counter= counter+1
        print "START TRIGGER"
        print  " fiber bundle n.",counter, " in the analog fed"
        rep = raw_input(' when ready hit return: ')
        for i in n:
            print "poh",i
            pos=0
            for ch in sector.poh[i].fedchannels[1:]:
                out ="group "+sector.poh[i].group
                sector.ccu.send(out).readlines()
                out =sector.poh[i].name + " setall 0 0 0 0 0 0 0 0"
                sector.ccu.send(out).readlines()
                sleep(0.5)
                fed.send("reset fifo").readlines()
                response=fed.query("rms " + str(ch))
                channel,mean,rms,name=response.split()
                out =sector.poh[i].name +" set g"+str(3-pos)+" 3"
                sector.ccu.send(out).readlines()
                out =sector.poh[i].name +" set b"+str(3-pos)+" 60"
                sector.ccu.send(out).readlines()
                sleep(0.5)
                fed.send("reset fifo").readlines()
                response=fed.query("rms " + str(ch))
                channel,mean1,rms,name=response.split()
                
                if  float(mean1) - float(mean) < 1000: 
                    errorc=errorc+1
                    print "fiber",12-ch ,"failure"
                pos=pos+1
    if errorc==0:
        print "ok"
########################################################
if pohbiastest==True:
    counter =0
    sector.init_tube()
    # change it using the bundle variable from poh
    print "power on mb and ccu, modules off "
    print "START TRIGGER"
    rep = raw_input(' when ready hit return: ')
    print "POH bias scan"
    for n in sublist:
        counter= counter+1
        print "START TRIGGER"
        print  " fiber bundle n.",counter, " in the analog fed"
        rep = raw_input(' when ready hit return: ')
        for i in n:
            print i
            gain=3
            suppressor=showRMSPOH(sector,i,int(gain),slopethr,wait=False,filename=filename)
    TCanvas('c1').Print(filename)
    timer.append("POH test done")
    timer.append(time.time()-start_time)
    print "POH bias scan finished"
########################################################
########################################################
if testsdarda==True:
    sector.init_tube()
    print "power on mb and ccu and modules"
    rep = raw_input(' when ready hit return: ')
    print "RDA range"
    k = sector.group[0].GetSDARDA3(module)
    c1= TCanvas()
    c1.Divide(3,2)
    c1.cd()
    n=1
    for i in k:
        c1.cd(n)
        i.Draw("COLZ")
        n=n+1
    c1.Write()
    timer.append("SDARDA range test done")
    timer.append(time.time()-start_time)
# # take from here rda sda
########################################################
if tbmplldelaytest==True:
    print "header trailer"
    print "STOP TRIGGER"
    print "poh in position " + str(sublist) + " fiber bundle in the digital fed"
    rep = raw_input(' when ready hit return: ')
    digfed.send("reset").readlines()
    digfed.send("regreset").readlines()
    digfed.send("PLLreset").readlines()
    digfed.send("PLLresetPiggy").readlines()
    digfed.send("setupTTC").readlines()
    digfed.send("initFitelN").readlines()
    digfed.send("fitel").readlines()
    n=0
    for i in module:
        sendd1 = "delay25 set d1 " + str(d1[n])
        sendd2 = "delay25 set d2 " + str(d2[n])
        output=ccu.send(sendd1).readlines() #SDA d2 x
        print output
        output=ccu.send(sendd2).readlines()  #RDA d1 y
        print output
        output=pxfec.send("cn scan hubs").readlines()
        print"hope is ", str(i)
        print output
        pxfec.send("module " + str(i)).readlines()
        pxfec.send("tbm disable triggers").readlines()
        n=n+1

    graph=[]
    histo2=[]
    histo3=[]
    name=[]
    nheader = 0
    ntrailer = 0
    nht = 0
    g=3
    b=40
    ss=[]
    sector.init_tube()
    for n in range(0,len(module)):
      for fb in fiber[n]:
        print "module ", module[n], " fiber ", fb
        sendd1 = "delay25 set d1 " + str(d1[n])
        sendd2 = "delay25 set d2 " + str(d2[n])
        ccu.send(sendd1).readlines() #SDA d2 x
        ccu.send(sendd2).readlines()  #RDA d1 y
        pxfec.send("module " + str(i)).readlines()
        pxfec.send("tbm").readlines()
        name= "Fedch"+str(fb)
        histo2.append(TH2D(name,name,8,-0.5,7.5,8,-0.5,7.5))
        histo3.append(TH2D(name+"lazy",name+"lazy",8,-0.5,7.5,8,-0.5,7.5))
        graph.append(TGraph())
        
        out ="fiber " + str(fb)
        digfed.send(out).readlines()
        for delay in range(0,64):
            
          inputo= str(bin(delay))+str(11)
          inputo= int(inputo,2)
          
          print "inputo " +str(inputo)
          pxfec.send("tbmplldelay "+ str(inputo)).readlines()
          nheader = 0
          ntrailer = 0
          bdelay=bin(delay)
          if len(bdelay)>5:
            x=bdelay[-3:] # (last 3 elements)
            y=bdelay[:-3] # (first 3 elements)

          if len(bdelay) <=5:
            x=bdelay
            y=bin(0)
          
          for ntr in range(0,ntrigger): 
              for s in digfed.send("getFromPiggy").readline():
                  header=re.match("011111111100 : TBM_Header",s)
                  trailer=re.match("011111111110 : TBM Trailer",s)
                  if header:
                      nheader = nheader+1
                  if trailer:
                      ntrailer = ntrailer+1
              with open("z_output_test.txt", "a") as myfile:
                  myfile.write( str(module[n]) +" " +str(fb) +" "+str(ntr)+" " +str(int(x,2)) +" " +str(int(y,2)) +" " +str(nheader/2) +" " +str(ntrailer/2)+ "\n")
         
          print "header: "+str(nheader/2)
          print "trailer: "+str(ntrailer/2)
          for hist in histo2:
            if hist.GetName()== "Fedch"+str(fb):
              hist.Fill(float(int(x,2)),float(int(y,2)),float(nheader+ntrailer)/4/ntrigger*100)
          for hist in histo3:
            if hist.GetName()== "Fedch"+str(fb)+"lazy":
              hist.Fill(float(int(x,2)),float(int(y,2)),inputo)
          
    timer.append("tbmplldelay scan done")
    timer.append(time.time()-start_time)  
    print "done"

    k=0
    c2=TCanvas()
    c2.cd()
    c2.Divide(int(len(histo2)/2),2)
    for i in histo2:
        c2.cd(k+1)
        i.Draw("COLZ TEXT")
        i.Write()
        k=k+1
    c2.Update()
    c2.Print()
    c2.Write()

    c3=TCanvas()
    c3.cd()
    c3.Divide(int(len(histo2)/2),2)
    for i in histo3:
        c3.cd(k+1)
        i.Draw("TEXT")
        i.Write()
        k=k+1
    c3.Update()
    c3.Print()
    c3.Write()
    digfed.close()
########################################################
if tbmplldelayonetest==True:
   
    print "header trailer one value"
    print "STOP TRIGGER"
    print "poh in position " + str(sublist) + " fiber bundle in the digital fed"
    rep = raw_input(' when ready hit return: ')
    ccu.send("reset").readlines()
    ccu.send("piareset all").readlines()
    digfed.send("reset").readlines()
    digfed.send("regreset").readlines()
    digfed.send("PLLreset").readlines()
    digfed.send("PLLresetPiggy").readlines()
    digfed.send("setupTTC").readlines()
    digfed.send("initFitelN").readlines()
    digfed.send("fitel").readlines()
    n=0
    pxfec.send("exec data/d.ini").readlines()
    for i in module:
        output=pxfec.send("cn scan hubs").readlines()
        print"hope is ", str(i)
        print output
        sendd1 = "delay25 set d1 " + str(d1[n])
        sendd2 = "delay25 set d2 " + str(d2[n])
        ccu.send(sendd1).readlines() #SDA d2 x
        ccu.send(sendd2).readlines()  #RDA d1 y
        pxfec.send("module " + str(i)).readlines()
        pxfec.send("tbm disable triggers").readlines()
        n=n+1

    ccu.send("channel 0x11").readlines()
    ccu.send("pll reset").readlines()
    ccu.send("pll init").readlines()
    print "setting poh bias and gain"  
    for k in sector.pohlist:
        ccu.send("group " + sector.poh[k].group).readlines()
        out= sector.poh[k].name +" setall 3 3 3 3 40 40 40 40"
        a =ccu.send(out).readlines()
    graph=[]
    histo2=[]
    name=[]
    nheader = 0
    ntrailer = 0
    nht = 0
    g=3
    b=40
    ss=[]
    count=0
    sector.init_tube()
    pxfec.send("exec data/d.ini").readlines()
    for n in range(0,len(module)):
        for fb in fiber[n]:
            print "module ", module[n], " fiber ", fb,
            sendd1 = "delay25 set d1 " + str(d1[n])
            sendd2 = "delay25 set d2 " + str(d2[n])
            ccu.send(sendd1).readlines() #SDA d2 x
            ccu.send(sendd2).readlines()  #RDA d1 y
            pxfec.send("module " + str(i)).readlines()
            pxfec.send("tbm").readlines()
            out ="fiber " + str(fb)
            digfed.send(out).readlines()
            first =str(bin(tbmplldelayvalue[n][0]))
            second =str(bin(tbmplldelayvalue[n][1]))
            inputo= first[2:]+second[2:] +str(11)
            inputo = int(inputo,2)
            print "inputo " +str(inputo)
            pxfec.send("tbmplldelay "+ str(inputo)).readlines()
           # pxfec.send("tbmplldelay "+ str(inputo)).readlines()
            nheader = 0
            ntrailer = 0
            for ntr in range(0,ntrigger):
                out=digfed.send("getFromPiggy").readline()
                sleep(0.1)
                for s in out:
                    header=re.match("011111111100 : TBM_Header",s)
                    trailer=re.match("011111111110 : TBM Trailer",s)
                    if header:
                        nheader = nheader+1
                    if trailer:
                        ntrailer = ntrailer+1
                with open("output_test_070312.txt", "a") as myfile:
                    myfile.write( str(module[n])+ " "+ str(ntr) + "  " + str(nheader/2) + "  "+ str(ntrailer/2) + "\n")
       
            print "header: "+str(nheader/2) ,
            print "trailer: "+str(ntrailer/2)
        


    timer.append("tbmplldelay scan done")
    timer.append(time.time()-start_time)  
    print "done"
    digfed.close()
########################################################
if pohdigitalfedtest==True:
    histo2d=[]
    print "header trailer poh bias scan"
    print "STOP TRIGGER"
    print "poh in position " + str(sublist) + " fiber bundle in the digital fed"
    #rep = raw_input(' when ready hit return: ')
    ccu.send("reset").readlines()
    ccu.send("piareset all").readlines()
    digfed.send("reset").readlines()
    digfed.send("regreset").readlines()
    digfed.send("PLLreset").readlines()
    digfed.send("PLLresetPiggy").readlines()
    digfed.send("setupTTC").readlines()
    digfed.send("initFitelN").readlines()
    digfed.send("fitel").readlines()
    pxfec.send("exec data/d.ini").readlines()
    ccu.send("channel 0x11").readlines()
    ccu.send("pll reset").readlines()
    ccu.send("pll init").readlines()  
    name=[]
    nheader = 0
    ntrailer = 0
    nht = 0
    count=0
    sector.init_tube()
    pxfec.send("exec data/d.ini").readlines()
    ccu.send("Power L12").readlines()
    for n in range(0,len(module)):
        for fb in fiber[n]:
            print "module ", module[n], " fiber ", fb,
            sendd1 = "delay25 set d1 " + str(d1[n])
            sendd2 = "delay25 set d2 " + str(d2[n])
            ccu.send(sendd1).readlines() #SDA d2 x
            ccu.send(sendd2).readlines()  #RDA d1 y
            pxfec.send("module " + str(module[n])).readlines()
            pxfec.send("tbm").readlines()
            pxfec.send("tbm disable triggers").readlines()
            out ="fiber " + str(fb)
            digfed.send(out).readlines()
            first =str(bin(tbmplldelayvalue[n][0]))
            second =str(bin(tbmplldelayvalue[n][1]))
            inputo= first[2:]+second[2:] +str(11)
            inputo = int(inputo,2)
            print "inputo " +str(inputo)
            pxfec.send("tbmplldelay "+ str(inputo)).readlines()
            nheader = 0
            ntrailer = 0
            name="module_"+ str(module[n])+ "_fiber_"+str(fb)
            histo2d.append(TH2F(name,name,20,0,40.5,4,0,4))
            for g in range(3,4):
                for b in range(10,40,2):
                    print "gain bias",g,b
                    for poh in sector.pohlist:
                        a=" "+str(g)
                        c=" "+str(b)
                        command= sector.poh[poh].name + "setall "+ a+ a+ a+ a+ c+ c+ c+ c
                        print command
                        ccu.send(command).readlines()
                    for ntr in range(0,10):
                        out=digfed.send("getFromPiggy").readline()
                        sleep(0.1)
                        for s in out:
                            header=re.match("011111111100 : TBM_Header",s)
                            trailer=re.match("011111111110 : TBM Trailer",s)
                        if header:
                            print "ok!"
                            nheader = nheader+1
                        if trailer:
                            print "ok!"
                            ntrailer = ntrailer+1
                        
                        #     header=re.search("011111111100",s)
                        #     trailer=re.search("011111111110",s)
                        # if header:
                        #     print "ok!"
                        #     nheader = nheader+1
                        # if trailer:
                        #     print "ok!"
                        #     ntrailer = ntrailer+1
       
                    print "header: "+str(nheader/2) ,
                    print "trailer: "+str(ntrailer/2)
                    histo2d[-1].Fill(b,g,(nheader+ntrailer)/4)

            

    c3=TCanvas()
    c3.Divide(len(histo2d/2,2))
    for i in range(0,len(histo2d)):
        c3.cd(i+1)
        histo2d[i].Draw("COLZ")
    c3.Write()
 
    timer.append("poh digitalfed bias scan done")
    timer.append(time.time()-start_time)  
    print "done"
    sleep(1000)
########################################################
for i in range(0,len(timer),2):
    if i+1 < len(timer):
        print timer[i]," " ,timer[i+1], endl
# ccu.close()
log.printLog()
log.saveLog("log/testCCU")
