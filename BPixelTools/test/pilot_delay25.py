import sys
from time import sleep
sockdir="/nfshome0/pixelpilot/build/TriDAS/pixel/BPixelTools/tools/python"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket
from SystemTests import PortCard
from Logger import Logger

########################################################

fechost = 'localhost'      
fecport = 2002    
fecslot = 9
fecring = None
group = '-6PL12'
i2cchan = '0x10'

RCK = None
CTR = None
SDA = None
RDA = None
CLK = None

bmi = False
bmo = False

if 'bmi' in sys.argv:
    bmi = True
    fecring = 7

    # 0x40 = enable will be added in ccu program
    RCK = 0x20  
    CTR = 0x00  
    SDA = 0x1c  
    RDA = 0x12  
    CLK = 0x18

if 'bmo' in sys.argv:
    bmo = True
    fecring = 8

    RCK = 0x20  
    CTR = 0x00  
    SDA = 0x1c  
    RDA = 0x12  
    CLK = 0x18

if bmi + bmo != 1:
    raise ValueError("must specify one of 'bmi' or 'bmo' at cmd line")
for x in (RCK, CTR, SDA, RDA, CLK):
    assert 0 <= x <= 255

########################################################

print "connecting to ccu prog %s:%i" % (fechost, fecport)
ccu = SimpleSocket(fechost, fecport)
log = Logger()
print
print 'not sending cratereset, reset, piareset all as old script did'

tag="FEC %d ring 0x%i :"%(fecslot,fecring)
print 'tag is', tag
fecringfound=False
fecfound=False
status=""
for l in ccu.send("mapccu").readlines():
    print l
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

ccu.send('ring %i' % fecring).readlines()

pc = PortCard(ccu, group, i2cchan, log)

pc.SendRDA(RDA) 
pc.Sendclock(CLK)
pc.SendSDA(SDA) 
pc.SendRCK(RCK) 
pc.SendCTR(CTR) 

if 'lasers' in sys.argv:
    log.info("frickin' lasers")
    cmds = '''i2c 10 10 22
i2c 10 11 22
i2c 10 12 22
i2c 10 18 22'''
    for cmd in cmds.split('\n'):
        print ccu.send(cmd).readlines()
        sleep(0.1)

    if bmo:
        print ccu.send('pixdcdc on 2').readlines()
        sleep(0.1)

log.printLog()
log.saveLog("log/testCCU")
