import sys,time,os, re
from datetime import date
from time import sleep
sockdir="/home/cmspixel/TriDAS/pixel/BPixelTools/tools/python"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket
from SystemTests import SECTOR, TestRedundancy, TestTriggerStatusFED, tDOH
from Logger import Logger

########################################################


# configuration
fechost = 'localhost'      
fecport =  2002    


fedslot =  99
fed=0                      # not used
pxfec=0                    # not used
caen=0                     # not used

########################################################


# connect

print "connecting to ccu,",fechost, fecport
ccu=SimpleSocket( fechost, fecport)
print " done"

log=Logger()

# choose an arbitray sector in that shell and get ring and slot from the server
ccu.send("cratereset").readlines()
ccu.send("reset").readlines()
ccu.send("piareset all").readlines()
fecring = '0x7'
fecslot = 9
print "fecring=",fecring
print "fecslot=",fecslot
tag="FEC %d ring %s :"%(fecslot,fecring)

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
    log.error("No such FEC %d"%fecslot)
    sys.exit(1)
if not fecringfound:
    log.error("ring/mfec not found "+tag)
    sys.exit(1)
    
if status=="no CCU found":
    log.error("no CCU found "+tag)
    print "verify ccu power"
    while True:
        yesno=raw_input("continue anyway (y/n) ")
        if yesno=='n': sys.exit(1)
        if yesno=='y': break

else:
    log.ok("CCUs found")
    
       

# start tests
print "test tracker DOHs"
dohA=tDOH(ccu, "0x7b", "0x10",log)
dohA.VerifyProgramming()
dohB=tDOH(ccu, "0x7c", "0x10",log)
dohB.VerifyProgramming()
print "start redundancy test"
TestRedundancy(ccu,log=log) 


ccu.close()

log.printLog()
log.saveLog("log/testCCU")
