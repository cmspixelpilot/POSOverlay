#######################################################################
# FastPixelAliveAnalysisForThr.py
# Original author : M . Malberti 17/12/2013
#
#######################################################################

import os
import sys
from ROOT import *
import shutil
import glob
import itertools
from itertools import islice
import browseCalibFiles

pixelAnalysisExe   = './bin/linux/x86_64_slc6/PixelAnalysis.exe'
config             = 'configuration/PixelAliveAnalysis.xml'
rundir             = os.environ['POS_OUTPUT_DIRS']
dacdir             = os.environ['PIXELCONFIGURATIONBASE']+'/dac'


def RunPixelAliveAnalysis(run):
    cmd = '%s %s %d'%(pixelAnalysisExe, config, run)
    os.system(cmd) 


def CountDeadPixels (maxDeadPixels, outfile, excludedrocs):
    maxeff = 100

    for roc in gDirectory.GetListOfKeys(): ## ROC folder: find one TH2F for each ROC                                                                                                                    
        histo = roc.ReadObj()
        hname   = histo.GetName()
        xBins   = histo.GetNbinsX()
        yBins   = histo.GetNbinsY()

        # count dead pixels in each roc                                                                                                                                                                 
        numDeadPixels = 0
        for x in range(1,xBins+1):
            for y in range(1,yBins+1):
                if histo.GetBinContent(x,y) < maxeff:
                    numDeadPixels=numDeadPixels+1;
        if (numDeadPixels > maxDeadPixels):
            rocname = hname.replace(' (inv)','')
            print '%s - Number of dead pixels = %d' %(rocname,numDeadPixels)
            if (rocname not in excludedrocs):
                outfile.write('%s\n'%rocname)



def CheckEfficiency(run, filename, iteration, maxDeadPixels, skipFPix, skipBPix, excluded):
    
    # excluded rocs
    excludedrocs = []
    if (excluded != '' and  os.path.isfile(excluded)):
        excludedfile = open(excluded,'r')
        if (excludedfile):
            excludedrocs = [line.replace('\n','') for line in excludedfile]

    # prepare output file where ROCs failing PixelAlive will be written
    outfile = open("%s_%d.txt"%(filename,iteration),'w')

    # open PixelAlive root file to be analyzed
    path = '%s/Run_%d/Run_%d/' % (rundir,runfolder(run),run)
    files = [ file for file in os.listdir('%s'%path) if 'PixelAlive_Fed_' in file]
    print 'Found files: ', files
    if len(files)!=1:
        sys.exit('PixelAlive root file NOT found !!!')
    file = TFile( '%s/Run_%d/Run_%d/%s' % (rundir,runfolder(run),run,files[0]) )

    # navigate in the root file directories to get efficiency histograms for each ROC    
    dirs = []
    if not skipFPix and not skipBPix:
        dirs = ["FPix","BPix"]    
    elif skipBPix and not skipFPix:
        dirs = ["FPix"]
    elif skipFPix and not skipBPix:
        dirs = ["BPix"]
          
    for dir in dirs:        
        file.cd(dir)
        browseCalibFiles.browseROCChain(['%s/Run_%d/Run_%d/%s' % (rundir,runfolder(run),run,files[0])], CountDeadPixels, maxDeadPixels, outfile, excludedrocs)

                        
    outfile = open("%s_%d.txt"%(filename,iteration),'r')
    print 'Number of failing ROCs = %d'% len(outfile.readlines())
    outfile.close()


def runfolder(run):
    f = int(run/1000)*1000
    return f 


def findDacFromKey(key):
    dac = []
    with open(os.environ['PIXELCONFIGURATIONBASE']+'configurations.txt','r') as f:
        chunks = f.read().split('\n\n')
    for c in chunks:
        config = c.split('\n')  
        if 'key %s'%key in config:
            dac = [item.split()[1] for item in config if item.startswith('dac')]
    if len(dac)<1:
        sys.exit("Error: dac not found")
    print "Used key %s with dac %s"%(key,dac[0])
    return dac[0]


def findDetConfigFromKey(key):
    detconfig = []
    with open(os.environ['PIXELCONFIGURATIONBASE']+'configurations.txt','r') as f:
        chunks = f.read().split('\n\n')
    for c in chunks:
        config = c.split('\n')
        if 'key %s'%key in config:
            detconfig = [item.split()[1] for item in config if item.startswith('detconfig')]
    if len(detconfig)<1:
        sys.exit("Error: detconfig not found")
    print "Used key %s with detconfig %s"%(key,detconfig[0])
    return detconfig[0]


def findRocsInDetConfig(key):    
    detconfig = findDetConfigFromKey(key)
    detconfigName = os.environ['PIXELCONFIGURATIONBASE']+'/detconfig/'+detconfig+'/detectconfig.dat'
    f = open(detconfigName)
    # return only active rocs
    rocs = [line.replace('\n','').replace(' ','') for line in f if ('noAnalogSignal' not in line and 'noInit' not in line)]
    return rocs


def ChangeVcThr(run,key,filename,iteration,excluded,deltafilename,singleStep,largeStep,safetyMargin,skipFPix,skipBPix):
    
    # -- Make the list of ROCs active used in the detconfig
    rocsInDetConfig = findRocsInDetConfig(key)
    #print rocsInDetConfig

    # --- Read file containing the list of rocs that you want to exclude from the procedure              ---
    # --- (for example: known problematic ROCs that fail PixelAlive no matter how high the threshold is) ---
    excludedrocs = []
    if (excluded != '' and  os.path.isfile(excluded)):
        excludedfile = open(excluded,'r')
        if (excludedfile):
            excludedrocs = [line.replace('\n','') for line in excludedfile]
    #print 'ROCs to be excluded at this iteration: ', excludedrocs
    
    # --- Read file containing the list of rocs that failed the PixelAlive at this iteration ---------------
    failedfile = open("%s_%d.txt"%(filename,iteration),'r')
    failedrocs = [line.replace('\n','') for line in failedfile]
    #print 'ROCs failing at this iteration: ', failedrocs                   
   
    # --- Read file containing the list of deltas from the previous iteration ------------------------------
    # --- build a dictionary with key = roc, value = delta
    rocsdelta = {}
    if os.path.isfile('%s_%d.txt'%(deltafilename,iteration-1) ):
        deltafile = open("%s_%d.txt"%(deltafilename,iteration-1),'r')
        list = [line.replace('\n','').split() for line in deltafile]
        rocsdelta = dict((el[0],int(el[1])) for el in list)
        #print rocsdelta

    # --- Copy dac settings used for the PixelAlive run locally ---------------------------------------------
    dac = findDacFromKey(key)    
    for f in glob.glob( dacdir+'/'+dac+'/*.dat'):
        #print f
        shutil.copy(f, './')

           
    # --- Prepare dir for new dac settings ------------------------------------------------------------------
    tmpdir = 'new'
    cmd = 'mkdir %s'%tmpdir
    os.system(cmd)

    # --- Make the list of .dat files where VcThr must be changed --------------------------------------------------
    # --- if BPix or FPix are not being analyzed, skip them    
    files = []

    if not skipBPix and not skipFPix:
        files = [file for file in os.listdir('./') if 'ROC_DAC_module_FPix' in file]
    elif skipBPix and not skipFPix:
        files  = [file for file in os.listdir('./') if 'ROC_DAC_module_FPix' in file]
        bfiles = [file for file in os.listdir('./') if 'ROC_DAC_module_BPix' in file]
        for f in bfiles:
            shutil.copy(f,tmpdir)
    elif skipFPix and not skipBPix:
        files  = [file for file in os.listdir('./') if 'ROC_DAC_module_BPix' in file]
        ffiles = [file for file in os.listdir('./') if 'ROC_DAC_module_FPix' in file]
        for f in ffiles:
            shutil.copy(f,tmpdir)

    # --- change VcThr for the ROCs contained in [files]
    deltafilenew = open("%s_%d.txt"%(deltafilename,iteration),'w')
    
    for f in files:
        fileold = open(f)
        filenew = open('%s/%s'%(tmpdir,f),'w')
        # --- group dac settings via a separator - the group separator is ROC
        for key,group in itertools.groupby(fileold,isa_group_separator):
            for item in group:
                if key:
                    roc,name = item.split()
                    filenew.write(item)
                elif 'VcThr' in item:
                    vcthr,value = item.split()
                    if (name in excludedrocs or name not in rocsInDetConfig): # exclude from the procedure rocs not in detconfis or in exlcudede.txt
                        newvalue = int(value)
                        delta = 0
                    else:
                        # -- if the ROC pass PixelAlive
                        if name not in failedrocs:
                            if iteration == 0:
                                newvalue = int(value) + largeStep
                                delta    = largeStep
                            else:                                     
                                if rocsdelta[name] == largeStep:
                                    if (iteration * largeStep >= 24): # don't go to low with thresholds...
                                        newvalue = int(value)
                                        delta = 0
                                    else:
                                        newvalue = int(value) + largeStep
                                        delta    = largeStep
                                elif rocsdelta[name] == 0:
                                    newvalue = int(value)
                                    delta    = 0
                                elif (rocsdelta[name] >  0 and rocsdelta[name] < largeStep) or (rocsdelta[name]<0):
                                    newvalue = int(value) - (safetyMargin - singleStep)
                                    delta    = 0
                        # -- if the ROC fails PixelAlive        
                        else:
                            if iteration == 0:
                                newvalue = int(value) - singleStep
                                delta    = - singleStep
                            else:
                                if rocsdelta[name] == largeStep:
                                    newvalue = int(value) - singleStep
                                    delta    = largeStep - singleStep
                                elif rocsdelta[name] <= 0:
                                    newvalue = int(value) - singleStep
                                    delta    = rocsdelta[name] - singleStep
                                elif (rocsdelta[name] >  0 and rocsdelta[name] < largeStep):
                                    newvalue = int(value) - singleStep
                                    if ( rocsdelta[name] != 2 ):
                                        delta    = rocsdelta[name] - singleStep
                                    else:
                                        delta    = 1
                    #print name, value, newvalue                
                    filenew.write('VcThr:         %d\n'%newvalue)
                    deltafilenew = open("%s_%d.txt"%(deltafilename,iteration),'a')
                    deltafilenew.write('%s %d\n'%(name,delta))
                else:
                    filenew.write(item)
        fileold.close()
        filenew.close()
        deltafilenew.close()


def isa_group_separator(line):
    return 'ROC:' in line



def MakeNewDacSettings():
    currentdir = os.getcwd()

    cmd = 'cd %s'%dacdir
    os.chdir('%s'%dacdir)
        
    # --- Make list of subdirectories in dac/ directory    
    subdirs = [ int(x) for x in os.walk('.').next()[1] ]
    subdirs.sort()
    print 'Last dac dir : ', subdirs[-1]    
    lastsettings = subdirs[-1]
    newsettings = subdirs[-1]+1
    os.system('mkdir %d'%newsettings)
    for fold in glob.glob('%d'%lastsettings+'/*dat'):
        shutil.copy(fold,'%d/'%newsettings)
    
    cmd = 'cd %s'%currentdir
    os.chdir('%s'%currentdir)

    # --- Copy .dat files in the new/ directory to the dac directory
    for fnew in glob.glob('new/*dat'):
        shutil.copy(fnew,'%s/%d/'%(dacdir,newsettings))

    # --- Make the new dac the default
    cmd = 'PixelConfigDBCmd.exe --insertVersionAlias dac %d Default'%newsettings
    print cmd
    os.system(cmd)




from optparse import OptionParser
parser = OptionParser()
parser.add_option("-r","--run",dest="run",type="int",help="Run number")
parser.add_option("-k","--key",dest="key",type="string",help="Run key")
parser.add_option("-i","--iteration",dest="iteration",type="int",default=-1,help="Iteration")
parser.add_option("-o","--outputFile",dest="output",type="string",default="failed",help="Name of the output file containing the list of failing rocs. Default is failed.txt")
parser.add_option("-d","--deltaFile",dest="delta",type="string",default="delta",help="Name of the output file containing the deltaVcThr. Default is delta.txt")
parser.add_option("-e","--exclude",dest="exclude",type="string",default="",help="List of the ROCs you want to exclude from the iterative procedure")
parser.add_option("","--singleStep",dest="singleStep",type="int",default=2,help="Step width. Default is 2")
parser.add_option("","--largeStep",dest="largeStep",type="int",default=6,help="Large step width. Default is 6")
parser.add_option("","--safetyMargin",dest="safetyMargin",type="int",default=6,help="Safety margin. Default is 6")
parser.add_option("","--maxDeadPixels",dest="maxDeadPixels",type="int",default=10,help="Maximum number of dead pixels per ROC. Default is 10.")

parser.add_option("","--skipFPix",dest="skipFPix",default=False,action="store_true",help="Skip FPix")
parser.add_option("","--skipBPix",dest="skipBPix",default=False,action="store_true",help="Skip BPix")
parser.add_option("","--makeNewDac",dest="makeNewDac",type="int",default=1,help="If 1, new dac is created")

(options,args)=parser.parse_args()


# --- Do some sanity checks before starting...

# --- Check that all relevant arguments (run, key, iteration) are passed
if not options.run or  not options.key  or options.iteration < 0:
    sys.exit('Usage: PixelAliveAnalysisForThr.py -r <run> -k <key> -i <iteration>. The first iteration must be 0. \n Exiting.')

# --- Check that the script is run from PixelAnalysisTools/test directory
thisdir = os.getcwd()
pixelanalysisdir = os.environ['BUILD_HOME']+'/pixel/PixelAnalysisTools/test'  
if thisdir != pixelanalysisdir:
    sys.exit('Error: wrong working directory!!! The script must be run from %s.'%pixelanalysisdir)

# --- Check that the first iteration ever used is iteration = 0
if options.iteration > 0:
    if not os.path.isfile("%s_0.txt"%options.output) or not os.path.isfile("%s_0.txt"%options.delta):
        sys.exit('Error: the first iteration must be 0.')

# --- Check if failed_N.txt file already exists
if os.path.isfile("%s_%d.txt"%(options.output,options.iteration)):
    sys.exit('Error: file %s_%d.txt exists'%(options.output,options.iteration))

# --- Check if delta_N.txt file already exists
if os.path.isfile("%s_%d.txt"%(options.delta,options.iteration)):
    sys.exit('Error: file %s_%d.txt exists'%(options.delta,options.iteration))

# --- Check that the steps are ok
if (options.largeStep%options.singleStep != 0) :
    sys.exit('Error: used largeStep=%d and singleStep=%d are not valid: largeStep must be a multiple of the singleStep'%(options.largeStep,options.singleStep))


# --- Analyze PixelAlive run
RunPixelAliveAnalysis(options.run)

# --- Check the efficiency of all ROCS and make a list of failed rocs (i.e. rocs with more than maxDeadPixels pixels)
CheckEfficiency(options.run,options.output,options.iteration,options.maxDeadPixels,options.skipFPix, options.skipBPix, options.exclude)

# --- Prepare new dac settings (change VcThr)
if (options.makeNewDac==1):
    ChangeVcThr(options.run,options.key,options.output,options.iteration,options.exclude,options.delta,options.singleStep,options.largeStep, options.safetyMargin, options.skipFPix, options.skipBPix)
    MakeNewDacSettings()




