#######################################################################
# runSCurveAnalysisForVana.py
# Original author : M . Malberti 15/11/2013
# This scripts
# - analyzes SCurve for in time and absolute thresholds
# - runs PixelIntimeVana to produce new dac settings
#######################################################################

import os
import sys
import glob 
import shutil
import subprocess

verbose = True
pixelAnalysisExe   = './bin/linux/x86_64_slc6/PixelAnalysis.exe'
SCurveInTimeConfig = 'configuration/SCurveAnalysis_WBC92.xml'
SCurveAbsConfig    = 'configuration/SCurveAnalysis.xml'
pixelIntimeVanaExe = './bin/linux/x86_64_slc6/PixelIntimeVana.exe'
rundir             = os.environ['POS_OUTPUT_DIRS']
dacdir             = os.environ['PIXELCONFIGURATIONBASE'] + '/dac'
#rundir  = '../../PixelRun/Runs/'
#dacdir  = '../../../pixel/PCDE_BmI/dac'


def runfolder(run):
    f = int(run/1000)*1000
    return f 


def RunSCurveAnalysisInTime(run):

    print  'Starting SCurve analysis for In-Time threshold...'
    currentdir = os.getcwd() 

    # --- run SCurve analysis for in-time threshold
    cmd = '%s %s %d'%(pixelAnalysisExe, SCurveInTimeConfig, run)
    if (verbose):
        print cmd
    writer = open("scurveInTime.log", 'w')
    process = subprocess.call(cmd, shell = True, stdout=writer)
    #    os.system(cmd) 

    # --- copy TrimOutputFile to TrimDefault.dat
    dir = '%s/Run_%d/Run_%d'%(rundir, runfolder(run), run)
    os.chdir('%s'%dir)
    if (verbose):
        print 'cd %s'%dir
        print os.getcwd()
        
    trimfile = [file for file in os.listdir('.') if 'TrimOutputFile_Fed_' in file]
    cmd = 'mv %s TrimDefault.dat'%trimfile[0]
    if verbose:
        print cmd
    os.system(cmd)

    os.chdir('%s'%currentdir)
    if (verbose):
        print os.getcwd()

def RunSCurveAnalysisAbsolute(run):

    print  'Starting SCurve analysis for Absolute threshold...'
    currentdir = os.getcwd() 

    # --- run SCurve analysis for absolute threshold
    cmd = '%s %s %d'%(pixelAnalysisExe, SCurveAbsConfig, run)
    if (verbose):
        print cmd
    writer = open("scurveAbs.log", 'w')
    process = subprocess.call(cmd, shell = True, stdout=writer)
    #os.system(cmd)

    # --- copy TrimOutputFile to TrimDefault.dat
    dir = '%s/Run_%d/Run_%d'%(rundir, runfolder(run), run)
    os.chdir('%s'%dir)
    if (verbose):
        print 'cd %s'%dir
        print os.getcwd()
        
    trimfile = [file for file in os.listdir('.') if 'TrimOutputFile_Fed_' in file]
    cmd = 'mv %s TrimDefault_abs.dat'%trimfile[0]
    if verbose:
        print cmd
    os.system(cmd)

    os.chdir('%s'%currentdir)
    if (verbose):
        print os.getcwd()


    
def CheckTrimDefaultFiles(run):

    print 'Checking TrimDefault.dat and TrimDefault_abs.dat files ...'
    currentdir = os.getcwd() 
    if (verbose):
        print currentdir

    trimfile1 = open('%s/Run_%d/Run_%d/TrimDefault.dat'%(rundir,
                                                         runfolder(run), run))
    trimfile2 = open('%s/Run_%d/Run_%d/TrimDefault_abs.dat'%(rundir,
                                                             runfolder(run), run))
    n1 = len([line for line in trimfile1])
    n2 = len([line for line in trimfile2])
    if (n1 != n2):
        cmd = "cat %s/Run_%d/Run_%d/"%(rundir, runfolder(run), run) +\
            "TrimDefault.dat | awk '{ print $2 }' > intime.txt"
        os.system(cmd)
        cmd = "cat %s/Run_%d/Run_%d/"%(rundir, runfolder(run), run) + \
            "TrimDefault_abs.dat | awk '{ print $2 }' > abs.txt"
        os.system(cmd)
        print 'Problems with the following ROCs:\n'
        cmd = 'diff intime.txt abs.txt'
        os.system(cmd)
        os.system('rm intime.txt')
        os.system('rm abs.txt')
        sys.exit('ERROR: TrimDefault.dat and TrimDefault_abs.dat have different number of lines !!!! ')


def RunPixelInTimeVana(key, run, logfile):
    
    cmd = '%s %d %d >& %s'%(pixelIntimeVanaExe, key, run, logfile)
    if (verbose):
        print cmd
    os.system(cmd)
    listoffiles = [file for file in os.listdir('.') if 'ROC_DAC_module' in file]    
    if (len(listoffiles) == 0):
        sys.exit('ERROR: no new DAC settings created !!! ')



def MakeNewDacSettings():
    currentdir = os.getcwd()

    cmd = 'cd %s'%dacdir
    if (verbose):
        print cmd
    os.chdir('%s'%dacdir)
        
    # make list of subdirectories in dac/ directory    
    subdirs = [int(x) for x in os.walk('.').next()[1]]
    #print subdirs    
    subdirs.sort()
    #print subdirs    
    print 'Last dac dir : ', subdirs[-1]    
    lastsettings = subdirs[-1]
    newsettings = subdirs[-1] + 1
    os.system('mkdir %d'%newsettings)
    for fold in glob.glob('%d'%lastsettings + '/*dat'):
        shutil.copy(fold, '%d/'%newsettings)
    
    cmd = 'cd %s'%currentdir
    if (verbose):
        print cmd
    os.chdir('%s'%currentdir)
    
    for fnew in glob.glob('./*dat'):
        shutil.copy(fnew, '%s/%d/'%(dacdir, newsettings))
    
    cmd = 'PixelConfigDBCmd.exe --insertVersionAlias dac %d Default'%newsettings
    if (verbose):
        print cmd
    os.system(cmd)


# -----------------   MAIN -------------------------------------------------------

# THIS script is supposed to  be run from PixelAnalysisTools/test dir
currentdir = os.getcwd() # this directory
print 'Working in %s'%currentdir

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-r", "--run", dest="run", type="int", help="Run number")
parser.add_option("-k", "--key", dest="key", type="int", help="Run key")
parser.add_option("-l", "--logfile", dest="logfile", type="string",
                  help="Name of the logfile")
(options, args) = parser.parse_args()

# --- run SCurve analysis 
RunSCurveAnalysisInTime(options.run)
RunSCurveAnalysisAbsolute(options.run)

# --- check that the two Trim files have the same number of lines
CheckTrimDefaultFiles(options.run)

# --- run PixelIntimeVana
RunPixelInTimeVana(options.key, options.run, options.logfile)

# --- make new dac settings the Default
#MakeNewDacSettings()


