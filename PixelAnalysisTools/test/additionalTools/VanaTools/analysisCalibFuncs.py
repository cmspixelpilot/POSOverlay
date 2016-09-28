
#!/usr/bin/env python

# *************************************************************************************
#                                
#  Author: Annapaola de Cosa
#          decosa@cern.ch
#          March/2014
#          
#          N.B: PixelAlive dedicated functions were adapted from Martina Malberti's code
#
#  analysisCalibFuncs.py
#  Description: Collection of utility functions for the analysis
#  of calibration-result files.   
#  
#  - RunSCurveSmartRangeAnalysis(run): Run analysis of SCurveSmartRange
#
#  - RunPixelAliveAnalysis(run): Run analysis of PixelAlive 
#
#  - fitVcalVcThr(savePlots, ignore): loop over the objects within the folder
#    and pick up the canvanses saved for each ROC, perform a fit to the Vcal(VcThr)
#    distribution and write down to an output file (ofile) the values of the Parameters
#    a and b and the chi2/NDF - Fit function => Vcal = a + b*VcThr
#    --savePlots: save fit plots in plot folder, default is False
#    --ignore   : ignore checks on Chi2/NDOF, default is False
#
# 
#  - checkROCthr(path, iteration): pick up the threshold histogram ("Threshold1D") for each ROC
#    and check whether the mean is less than 35, in this case flag the ROC as failing
#    and add it to the list of failing ROCs in the output file
#    -- path: position of the output file. No need to specify the name of the file,
#       if it does not exist it will be created in the run folder, if it does already
#       it will be just updated (new info appended to the file).
#    -- iteration: iteration number
#
#  - readHistoInfo(name): pick up the histogram corresponding to
#    the specified name and print Mean and RMS of the distribution
#    -- name: name of the histo
# **************************************************************************************



import sys
import os, commands
import re
#import io
import time
import subprocess
import glob
import shutil
import shlex
import ROOT
from array import array
import string
from browseCalibFiles import *


#dacdir      = os.environ['PIXELCONFIGURATIONBASE'] +'dac/'
#detconfigdir      = os.environ['PIXELCONFIGURATIONBASE'] +'/detconfig/'
#confpath    = os.environ['PIXELCONFIGURATIONBASE'] +"/configurations.txt"
#runDir    = os.environ['POS_OUTPUT_DIRS']
#pixelAnalysisExe   = os.environ['BUILD_HOME'] + '/pixel/PixelAnalysisTools/test/bin/linux/x86_64_slc5/PixelAnalysis.exe'
#config             = os.environ['BUILD_HOME'] + '/pixel/PixelAnalysisTools/test/configuration/SCurveAnalysis_FPix.xml'
#runpath            = os.environ['BUILD_HOME'] + '/pixel/PixelRun/'
#configPixelAlive   = os.environ['BUILD_HOME'] + '/pixel/PixelAnalysisTools/test/configuration/PixelAliveAnalysis.xml'


dacdir             = os.environ['PIXELCONFIGURATIONBASE'] +'/dac/'
detconfigdir       = os.environ['PIXELCONFIGURATIONBASE'] +'/detconfig/'
confpath           = os.environ['PIXELCONFIGURATIONBASE'] +"/configurations.txt"
#runpath     = os.environ['HOME'] + '/run/'
runpath            = os.environ['BUILD_HOME'] + '/pixel/PixelRun/'
runDir             = os.environ['POS_OUTPUT_DIRS']
pixelAnalysisExe   = os.environ['BUILD_HOME'] + '/pixel/PixelAnalysisTools/test/bin/linux/i386_slc5/PixelAnalysis.exe'
config             = os.environ['BUILD_HOME'] + '/pixel/PixelAnalysisTools/test/configuration/SCurveAnalysis_BaseExample.xml'
configPixelAlive   = os.environ['BUILD_HOME'] + '/pixel/PixelAnalysisTools/test/configuration/PixelAliveAnalysis_BaseExample.xml'


def RunSCurveSmartRangeAnalysis(run):
    path = '%s/Run_%s/Run_%d/'%(runDir, runfolder(run), run)
    filename = 'SCurve'
    filelist = [ path + file for file in os.listdir(path) if file.startswith(filename) and file.endswith(".dmp")]
    newfiles = [f.replace('SCurveSmartRange', 'SCurve') for f in filelist]
    for i in xrange(len(filelist)):
        cmdcp = "cp %s %s"%(filelist[i], newfiles[i])
        os.system(cmdcp) 
    
    print "\n=======> Running SCurve Analysis <=======\n"
    cmd = '%s %s %d'%(pixelAnalysisExe, config, run)
    print cmd
    writer =open("scurve.log", 'w') 
    process = subprocess.call(cmd, shell = True, stdout=writer)
    cmdcpOffset = ('cp '+ runpath + 'mapRocOffset.txt ' + path)
    print "copy cmd ", cmdcpOffset
    os.system(cmdcpOffset)
    cmdrm = ('rm '+ runpath + 'mapRocOffset.txt')
    print cmdrm
    os.system(cmdrm)


def RunPixelAliveAnalysis(run):
    path = '%s/Run_%s/Run_%d/'%(runDir, runfolder(run), run)
    print "\n=======> Running PixelAlive Analysis <=======\n"
    cmd = '%s %s %d'%(pixelAnalysisExe, configPixelAlive, run)
    print cmd
    writer = open("pixelAlive.log", 'w') 
    process = subprocess.call(cmd, shell = True, stdout=writer)

    
def CountDeadPixels (maxDeadPixels, outfile, excludedrocs):
    maxeff = 100

    for roc in ROOT.gDirectory.GetListOfKeys(): ## ROC folder: find one TH2F for each ROC                                                                                                                    
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



def CheckEfficiency(files, filename, iteration, maxDeadPixels, skipFPix, skipBPix, excluded):
    
    # excluded rocs
    excludedrocs = []
    if (excluded != '' and  os.path.isfile(excluded)):
        excludedfile = open(excluded,'r')
        if (excludedfile):
            excludedrocs = [line.replace('\n','') for line in excludedfile]

    # prepare output file where ROCs failing PixelAlive will be written
    outfile = open("%s_%d.txt"%(filename,iteration),'w')

    # open PixelAlive root file to be analyzed
    #path = '%s/Run_%d/Run_%d/' % (runDir,runfolder(run),run)
    #files = [ file for file in os.listdir('%s'%path) if 'PixelAlive_Fed_' in file]
    #print 'Found files: ', files
    #if len(files)!=1:
        #sys.exit('PixelAlive root file NOT found !!!')
    #file = TFile( '%s/Run_%d/Run_%d/%s' % (runDir,runfolder(run),run,files[0]) )

    # navigate in the root file directories to get efficiency histograms for each ROC    
    #dirs = []
    #if not skipFPix and not skipBPix:
        #dirs = ["FPix","BPix"]    
    #elif skipBPix and not skipFPix:
        #dirs = ["FPix"]
    #elif skipFPix and not skipBPix:
        #dirs = ["BPix"]
          
    #for dir in dirs:        
        #file.cd(dir)
    browseROCChain(files, CountDeadPixels, maxDeadPixels, outfile, excludedrocs)

                        
    outfile = open("%s_%d.txt"%(filename,iteration),'r')
    print 'Number of failing ROCs = %d'% len(outfile.readlines())
    outfile.close()


    
def fitVcalVcThr( savePlots, ignore):
    failingRocs = 0 
    if not os.path.isfile(runpath + 'mapRocVcalVcThr.txt'):
        print "Saving New  Vcal VcThr map in ",runpath + 'mapRocVcalVcThr.txt'
        ofile = open(runpath + 'mapRocVcalVcThr.txt', 'w')
        ofile.write('='*80)    
        ofile.write('\nROC name                              a      b     chi2/NDF   LowestThreshold \n')
        ofile.write('='*80)
        ofile.write('\nVcThr = a + b*Vcal \n')
        ofile.write('='*80)
    else:
        ofile = open(runpath + 'mapRocVcalVcThr.txt', 'a')

    for roc in ROOT.gDirectory.GetListOfKeys(): # ROCs, e.g.:  BmI_SEC4_LYR1_LDR5F_MOD1_ROC0
        cName =  roc.GetName()
        h = roc.ReadObj().GetPrimitive(cName)
        step = h.GetYaxis().GetBinWidth(1)
        ### Fit range for intime WBC 
        #VcalMin = 50
        #VcalMax = 140    

        ### Fit range for next WBC 
        VcalMin = 68
        VcalMax = 90


        # define the two arrays VcThrs and Vcals,
        # the former storing the values of VcThr to consider 
        # and the latter the corresponding Vcal values


        Vcals = range(VcalMin, VcalMax, int(step))
        Vcals_err = [2.]*len(Vcals)
        VcThrs  = [] 
        minVcThr = []        
        VcThrs_err  = [1.]*len(Vcals) 
        for Vcal in Vcals:
            bin = h.GetYaxis().FindBin(Vcal)
            h_VcThr= h.ProjectionX("VcThr", bin, bin)
            firstBin = h_VcThr.FindFirstBinAbove(0.4)
            minVcThr.append( h_VcThr.GetBinCenter( h_VcThr.FindLastBinAbove(0.9) ) )
            VcThrs.append(h_VcThr.GetBinCenter(firstBin))
        

        VcThrArray = array('f', VcThrs)
        VcalArray = array('f', Vcals)
        VcThrArray_err = array('f', VcThrs_err)
        VcalArray_err = array('f', Vcals_err)
#        VcThrVcal_graph = ROOT.TGraphErrors( len(VcalArray), VcalArray, VcThrArray, VcalArray_err, VcThrArray_err )
        VcThrVcal_graph = ROOT.TGraph( len(VcalArray), VcalArray, VcThrArray)
        VcThrVcal_graph.Fit("pol1", "Q")
        VcThrVcal_graph.SetTitle(cName)
        VcThrVcal_graph.GetXaxis().SetTitle("Vcal")
        VcThrVcal_graph.GetYaxis().SetTitle("VcThr")
        fitRes = VcThrVcal_graph.GetFunction("pol1")
        ofile.write('\n%s   %.2f   %.2f   %.2f   %d '%(cName, fitRes.GetParameter(0), fitRes.GetParameter(1), fitRes.GetChisquare()/fitRes.GetNDF(), min(minVcThr)))                      

        if(fitRes.GetChisquare()/fitRes.GetNDF() > 10.): 
            print fitRes.GetChisquare()/fitRes.GetNDF()
            print ignore
            if(ignore== 'False'):
                print "hello"
                failingRocs = failingRocs + 1
                print failingRocs

        if(savePlots == 'True'):
            if(not os.path.isdir("plots")): os.system('mkdir plots')
            c = ROOT.TCanvas(cName+'_fit')
            c.cd()
            VcThrVcal_graph.Draw("A*")
            c.Print("plots/" + cName+".pdf")
    if(failingRocs > 0): print "There were ", failingRocs, " failing ROCs in module: ", cName


### The following functions look into the SCurve results and check for each ROCs
### if there are failing ones. For the moment a ROC is defined as failing only if the mean threshold is
### less than 35. Creates a file named failed_N.txt where failing ROCs are listed. N is the number
### of iterations 


def checkROCthr(path, iteration):
    #print "Entering checkROCthr"
    filename = "failed_%d.txt"%(iteration)
    if not os.path.isfile(filename):
        ofile = open(filename,'w')
        print "File ", filename, " does not exist. Creating a new one "
        ofile.write('='*60)    
        ofile.write('\nFalingROC name                              ThrMean      ThrRMS     \n')
        ofile.write('='*60)    
    else:
      ofile = open(filename, 'a')

    for roc in ROOT.gDirectory.GetListOfKeys(): # ROCs, e.g.:  BmI_SEC4_LYR1_LDR5F_MOD1_ROC0
        name =  roc.GetName()
        rocname =  name.replace("_Threshold1D", "")
        if(name.endswith("Threshold1D")):
            h = roc.ReadObj()
            #print "ROC Name: ", name
            #print "ROC Mean: %.2f"%(h.GetMean())
            nPixelsOutRange = h.Integral(0, h.FindBin(30)) + h.Integral( h.FindBin(120), h.GetNbinsX()+2) 
            
            if(h.GetMean()<30 or h.GetMean()>60 or nPixelsOutRange >2):
                ofile.write('\n%s  %.2f  %.2f'%(rocname, h.GetMean(), h.GetRMS()) )
            #else: continue

            if(h.GetMean()<30):
                print "ROC failing because of mean Thr <30: ", rocname, " (", h.GetMean(), ")"
            elif(h.GetMean()>60):
                print "ROC failing because of mean Thr >60: ", rocname, " (", h.GetMean(), ")"
            #elif(nPixelsOutRange >2):
                #print "ROC failing because pixel Thr out of range: ", rocname
                #print "Number of bad pixels: " , nPixelsOutRange

def readHistoInfo(name):
    a =    ROOT.gDirectory.Get(name)
    print a.GetTitle()
    print "RMS   Mean"
    print  '%.2f  %.2f'%(a.GetRMS(), a.GetMean())
    return a


def findDetConfigFromPath(path):
    
    filename = path + 'PixelConfigurationKey.txt'    
    key, detconfig = 0, 0
    # Open the PIxelConfigurationKey file and look for the key 
    try: 
        f = open(filename, 'r')
    except IOError:
        print "Cannot open file ", filename
    else:
        lines = f.readlines()
        key = lines[1].split()[-1]
        #print key 
        detconfig = findDetConfigFromKey(key)
    return detconfig


### Create new dac settings staring from the key and so the dac number used for
###the current Calibration. It checks if there are failing ROCs, and for those
###increases the threshold of 2 units. It would be possible to add a check on the previous
###iterations, but let s keep this for a second moment. 

def findDacFromPath(path):
    filename = path + 'PixelConfigurationKey.txt'    
    key, dac = 0, 0
    # Open the PIxelConfigurationKey file and look for the key 
    try: 
        f = open(filename, 'r')
    except IOError:
        print "Cannot open file ", filename
    else:
        lines = f.readlines()
        key = lines[1].split()[-1]
        #print key 
        dac = findDacFromKey(key)
    return dac

def listFromFile(filepath):
    f =open(filepath)
    flist = f.readlines()[1:]
    flist = [l.replace(" \n", "") for l in flist] 
    f.close()
    return flist



def listFromFilename(filepath):
    f =open(filepath)
    flist = f.readlines()
    flist = [l.replace("\n", "") for l in flist] 
    f.close()
    return flist



def listFromDeltaFile(filepath, mod):
    f = open(filepath)
    flist = f.readlines()
    flist = [l.replace(" \n", "") for l in flist] 
    if (mod == "minimize"): flist = [l.split()[0] for l in flist if (l.split()[1] == "0" or l.split()[1] == "-4")]
    else: flist = [l.split()[0] for l in flist if (l.split()[1] == "0" )]
    f.close()
    return flist


def createModuleList(path):
    detconfig = findDetConfigFromPath(path)
    if(detconfig!=0):
        detconfiglist =listFromFile(detconfigdir + str(detconfig) + "/detectconfig.dat")
        mods =[] 
        for m in detconfiglist:
            if (("noAnalogSignal" not in m) and ("noInit" not in m)):
                if("FPix" in m ):mod = m.split("_")[:-2]
                else: mod = m.split("_")[:-1]
                mod = "_".join(mod)
                if mod not in mods:mods.append(mod)
                #           else:
                #print "noAnalogSignal"
        print "Number of Modules: ", len(mods)
        print "Total number of ROCs in detconfig file: ", len(detconfiglist)
        files = [ "ROC_DAC_module_"+mod+".dat" for mod in mods]
        return files



def createDetconfigList(detconfig):
    if(detconfig!=0):
        detconfiglist =listFromFile(detconfigdir + str(detconfig) + "/detectconfig.dat")
        mods =[] 
        for m in detconfiglist:
            if (("noAnalogSignal" not in m) and ("noInit" not in m)):
                if("FPix" in m ):mod = m.split("_")[:-2]
                else: mod = m.split("_")[:-1]
                mod = "_".join(mod)
                if mod not in mods:mods.append(mod)
                #           else:
                #print "noAnalogSignal"
        print "Number of Modules: ", len(mods)
        print "Total number of ROCs in detconfig file: ", len(detconfiglist)
        files = [ "ROC_DAC_module_"+mod+".dat" for mod in mods]
        return files

              
def createNewDACsettings(path, iteration, deltafile, outfile, mod, makeNewDac):
    #delta = "delta"
    detconfiglist = createModuleList(path)
    #print "detconfiglist size: ", len(detconfiglist)
    minimizedROCs = []
    if(iteration>1):
        minimizedROCs = listFromDeltaFile("%s_%d.txt"%(deltafile, iteration-1), mod)
        #print "ROCs already fixed ", minimizedROCs
    dac = findDacFromPath(path)
    if(dac!=0):
        subdirs = [ int(x) for x in os.walk(dacdir).next()[1] ] 
        subdirs.sort()
        print 'Last dac dir: ', subdirs[-1]    
        lastsettings = subdirs[-1]
        newsettings = subdirs[-1]+1       
        newdir =  os.getcwd() +  '/ThresholdMinimization/dac/' + str(newsettings)
        print 'New dir: ', newdir
        os.makedirs(newdir)
        os.makedirs(dacdir + str(newsettings))
        cmd_cpdac = 'cp -r ' + dacdir + str(dac) + '/* ' + dacdir + str(newsettings)

        print cmd_cpdac
        os.system(cmd_cpdac)


        failingRocs = getFailingRocs(path, outfile, iteration)
        orgdacpath = dacdir + dac
        deltafilenew = open("%s_%d.txt"%(deltafile, iteration),'a')
        
        for f in detconfiglist:
            newdacfile = open(newdir + '/'+f, 'w')
            openfile = open(orgdacpath + '/'+ f, 'r')
            #print 'Original file: ', orgdacpath + '/'+ f
            #print 'New file: ', newdir + '/'+f
            delta = 0
            
            for line in openfile.readlines():
                if (line.startswith("ROC")):
                    #if line.split()[1].startswith("BPix_BmI_SEC4"): print line
                    rocname = line.split()[1]
                    #print rocname
                    delta = setDelta(rocname, minimizedROCs, failingRocs, mod)
                    deltafilenew.write('%s %d\n'%(rocname,delta))
                        
                elif (line.startswith('VcThr') ): 
                    newVcThr = int(line.split()[1]) + delta
                    #print "old dac: ", line.split()[1]
                    #print "new dac: ", newVcThr
                    #print "old line: ",line
                    line = string.replace(line, str(line.split()[1]), str(newVcThr))
                    #print "new line: ",line
                                        
                newdacfile.write(line)

            newdacfile.close()
            openfile.close()
         

        deltafilenew.close()
        # --- Print a summary         
        deltafilenew = open("%s_%d.txt"%(deltafile,iteration),'r')
        rocs = deltafilenew.readlines()
        print 'Number of ROCs already failing at the previous iterations:'
        print        len([ l for l in rocs if re.search(' 0',l) ])
        print 'Number of ROC failing at this iteration:'
        print        len([ l for l in rocs if re.search(' -4',l) ])
        print 'Number of ROC still succeeding:'
        print        len([ l for l in rocs if re.search(' 2',l) ])
        deltafilenew.close()
        # --- Copy the new dac files in the newly created dac directory 
        dest_dir = dacdir + str(newsettings)
        #print "destination folder: ", dest_dir + "/"
        #print "source folder:", newdir + "/*.dat "
        for f in glob.glob( newdir + "/*.dat"):
            #print f
            shutil.copy(f, dest_dir)

        # --- Make the new dac the default
        if(makeNewDac):
            cmd = 'PixelConfigDBCmd.exe --insertVersionAlias dac %d Default'%newsettings
            print cmd
            print "New DAC settings dir: dac/%d"%newsettings
            os.system(cmd)



def createNewDAC(detconfig, dac, rocfilename, step, var, makeNewDac):
    # failingRocs = getFailingRocs(path, outfile, iteration)
    detconfiglist = createDetconfigList(detconfig)
    roclist = listFromFilename(rocfilename)
    print roclist
    #print "detconfiglist size: ", len(detconfiglist)
    if(dac!=0):
        subdirs = []
        for x in  os.walk(dacdir).next()[1]:
            try:
                subdirs.append(int(x))
            except:
                continue
        #subdirs = [ int(x) for x in os.walk(dacdir).next()[1] ]
        subdirs.sort()
        print 'Last dac dir: ', subdirs[-1]
        lastsettings = subdirs[-1]
        newsettings = subdirs[-1]+1
        newdir =  os.getcwd() +  '/ThresholdMinimization/dac/' + str(newsettings)
        print 'New dir: ', newdir
        os.makedirs(newdir)
        os.makedirs(dacdir + str(newsettings))
        cmd_cpdac = 'cp -r ' + dacdir + str(dac) + '/* ' + dacdir + str(newsettings)

        print cmd_cpdac
        os.system(cmd_cpdac)

        orgdacpath = dacdir + str(dac)
        #deltafilenew = open("%s_%d.txt"%(deltafile, iteration),'a')

        for f in detconfiglist:
            newdacfile = open(newdir + '/'+f, 'w')
            #print 'Original file: ', orgdacpath + '/'+ f
            openfile = open(orgdacpath + '/'+ f, 'r')
            #print 'New file: ', newdir + '/'+f
            delta = 0

            for line in openfile.readlines():
                if (line.startswith("ROC")):
                    rocname = line.split()[1]
                    #print rocname
                    if (rocname in roclist):
                        print rocname
                        delta = step
                    else: delta = 0
                    #deltafilenew.write('%s %d\n'%(rocname,delta))

                elif (line.startswith(var) ):
                    newVar = int(line.split()[1]) + delta
                    #print "old dac: ", line.split()[1]
                    #print "new dac: ", newVar
                    #print "old line: ",line
                    line = string.replace(line, str(line.split()[1]), str(newVar))
                    #print "new line: ",line

                newdacfile.write(line)

            newdacfile.close()
            openfile.close()

        # --- Copy the new dac files in the newly created dac directory
        dest_dir = dacdir + str(newsettings)
        #print "destination folder: ", dest_dir + "/"
        #print "source folder:", newdir + "/*.dat "
        for f in glob.glob( newdir + "/*.dat"):
            #print f
            shutil.copy(f, dest_dir)

        # --- Make the new dac the default
        if(makeNewDac):
            cmd = 'PixelConfigDBCmd.exe --insertVersionAlias dac %d Default'%newsettings
            print cmd
            print "New DAC settings dir: dac/%d"%newsettings
            os.system(cmd)



        
def setDelta(rocname, minimizedROCs, failingRocs, mod):
    if(rocname in minimizedROCs):
        delta = 0 
    elif(rocname in failingRocs):
        delta = -4 # decrease VcThr of 4 units making the threshold higher   
    else:
        if(mod == "increase"): delta = 0 # leave unchanged if in modality increaseThreshold
        else: delta = +2 # lower the threshold
    return delta


def findDacFromKey(key):

    dac = []

    with open(os.environ['PIXELCONFIGURATIONBASE']+'/configurations.txt','r') as f:
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
    print 'Key ',key
    detconfig = []

    with open(os.environ['PIXELCONFIGURATIONBASE']+'/configurations.txt','r') as f:
        chunks = f.read().split('\n\n')
    for c in chunks:
        config = c.split('\n')
        if 'key %s'%key in config:
            detconfig = [item.split()[1] for item in config if item.startswith('detconfig')]
    if len(detconfig)<1:
        sys.exit("Error: detconfig not found")
    print "Used key %s with detconfig %s"%(key,detconfig[0])
    return detconfig[0]

        
### Analyse the file produced by CheckROCThr and get a list of failing ROCs
### to use it later by createNewDACsettings
    
def getFailingRocs(path, outfile = "", iteration = -999):
    if(iteration != 0 and iteration!=-999):
        try:
            ofile = open("%s_%d.txt"%(outfile, iteration),'r')
        except IOError:
            print "Cannot open ", "failed_%d.txt"%(iteration)
            
        else:
            lines = ofile.readlines() 
            print "Failing ROCs: ", len(lines) 
            rocs = [l.split()[0] for l in lines]
            failrocs = [l.split()[0] for l in lines if (l.startswith("BPix") or l.startswith("FPix"))]
            ofile.close()
            return failrocs
    else:  return []



## def getFailingRocsAlive(path, iteration):
##     if(iteration != 0):
##         try:
##             ofile = open("failedAlive_%d.txt"%(iteration),'r')
##         except IOError:
##             print "Cannot open ", "failedAlive_%d.txt"%(iteration)
            
##         else:
##             lines = ofile.readlines() 
##             print "Failing ROCs: ", len(lines) 
##             rocs = [l.split()[0] for l in lines]
##             failrocs = [l.split()[0] for l in lines if (l.startswith("BPix") or l.startswith("FPix"))]
##             ofile.close()
##             return failrocs
##     else:   return []


