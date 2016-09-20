import re,urllib,sys
import sys,os,re
from xml.dom.minidom import parseString
import datetime

# print a run summary
# usage
# python ~/scripts/runsum.py  <runnumber>
#
# or, if the current working directory is the run directory
#
# python ~/scripts/runsum.py
#

###2010 path
#datadir="/pixel/data1"
###2011 path
#datadir="/pixelscratch/pixelscratch/data0/"
###Link to 2011 path
datadir="/nfshome0/pixelpro/TriDAS/pixel/PixelRun/Runs"

if len(sys.argv)==2:
    try:
        run=int(sys.argv[1])
        rundir=datadir+"/Run_%05d/Run_%05d/"%(run-run%1000,run)

    except ValueError:
        print "expect a run number   (got ",sys.argv[1],")"
        sys.exit(1)
else:
    rundir=os.getcwd()
    try:
        run=int(rundir.split("_")[-1])
    except ValueError:
        print "cant extract run number form "+rundir
        sys.exit(1)
        
print run,rundir        


if not os.path.exists(rundir):
    print "run directory "+rundir+"  not found"
    sys.exit(1)


# get the xml from wbm
url="http://cmswbm.cms/cmsdb/servlet/RunSummary?RUN=%d&DB=cms_omds_lb&FORMAT=XML"%(run)
#print url
doc=parseString(urllib.urlopen(url).read())


def walk(name,line):
    for n in line.childNodes:
        walk(name+"/"+line.nodeName,n)
    if line.nodeValue:
        print name.strip(),"=",line.nodeValue.strip()
#walk("",doc)




# extract some values of interest
dict={}
for key in ["run","startTime","stopTime","triggers","bField","l1Rate","sequence"]:
    for n in doc.getElementsByTagName(key):
        try:
            value=n.childNodes[0].nodeValue
            dict[key]=str(value)
        except:
            dict[key]=""
            value=None
        #print key,value




## now print the summary, 1st part
        
print "Run summary ",dict["run"]
print "Number of Triggers:",dict["triggers"]

try:
    print "L1 rate :  %6.3f kHz"%(float(dict["l1Rate"])/1000.)
except:
    print "L1 rate (Hz):",dict["l1Rate"]


def getDatetime(s):
    try:
        date=datetime.date(*[int(x) for x in s.split()[0].split(".")])
        time=datetime.time(*[int(x) for x in s.split()[1].split(":")])
        dt=datetime.datetime.combine(d1,t1)
        correction=dt.now()-dt.utcnow()
        return dt+correction
    except:
        return s
    
print "Run Start (UTC):",getDatetime(dict["startTime"])
print "Run End  (UTC) :",getDatetime(dict["stopTime"])
print "Magnetic Field Value (T):",dict["bField"]
print "Included pixel FEDs: "


#
##  part 2 ,  fed errors
#

dumpErrorFiles="/nfshome0/pixelpro/bin/dumpErrorFiles"
key="----"
if os.path.exists(rundir+"/PixelConfigurationKey.txt"):
    for l in open(rundir+"/PixelConfigurationKey.txt"):
        if l.startswith("Pixel Global Configuration Key"):
            key=l.split()[-1]
print "Online configuration key",key
totalErrors=0 
for fed in range(40):
    path=rundir+"/ErrorDataFED_%d_%d.err"%(fed,run)
    if not os.path.exists(path):
        print "file not found ",path
        continue
    
    nerr=0
    nerrene=0
    nerrtra=0
    for l in os.popen(dumpErrorFiles+" "+path).readlines():
        if l.strip().startswith("run number"): continue
        if l.strip().startswith("time in sec."): continue
        if l.startswith("End of input file"): break
        if l.strip().endswith("TBM status:0x60 TBM-Reset received :event: 1"): continue
        if l.strip().startswith("Event Number Error"):
            nerrene+=1
        if l.strip().startswith("Trailer Error"):
            nerrtra+=1
        nerr+=1
        totalErrors+=1
    
    if nerr>0:
        print "fed %2d has %d errors (Event Number Errors:  %d, Trailer Errors, %d)" %(fed,nerr,nerrene,nerrtra)
print "FED errors      %d" %(totalErrors)




#
# part 3,  fed baseline
#

print "Please wait for a minute or two for the baseline analysis to finish..."
# look at Baselien corrections, too ?
r=re.compile("Time: (.*) : Baseline Correction for FED (.*), Channel (.*)has Mean=(.*) and StdDev=(.*)")
minAll,maxAll=1025,-1025
minFed,maxFed=-1,-1
for fed in range(40):
    min,max=1025,-1025
    path=rundir+"/BaselineCorrectionFED_%d_%d.txt"%(fed,run)
    for l in open(path,'r').readlines():
        m=r.match(l)
        if m:
            channel=int(m.group(3))
            when=m.group(1)
            mean=float(m.group(4))
            if mean>max: max=mean
            if mean<min: min=mean
        else:
            print l
    if min<minAll:
        minAll=min
        minFed=fed
    if max>maxAll:
        maxAll=max
        maxFed=fed
        
    
    
print "FED baseline corrections min= %5.1f (fed %d),  max= %5.1f (fed %d)"%(minAll,minFed,maxAll,maxFed)
print
print
