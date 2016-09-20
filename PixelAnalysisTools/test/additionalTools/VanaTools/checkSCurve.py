#!/usr/bin/env python

# **************************************************************************************
#
#  Author: Annapaola de Cosa
#          decosa@cern.ch
#          March/2014
#
#  Description: Script to run Threshold Minimization
#               procedure with SCurveSmartRange
#
#  Iteration 0: python SCurveSR.py -r 'VcThrVcalRunNumber' -i 0
#
#               This step analyzes the results from VcThrVcalRunNumber and creates 
#               the map VcThr-Vcal for each ROC used in the next 
#               Add  --savePlots True to save fit plots.
#               Specify --ignore True to ignore errors in case the fits are fine
#               but the Chi2 exceeds the threshold set in the script
#
#
#  Iteration N: python SCurveSR.py -r 'SCurveSmartRangeRunNumber' -i N --makeNewDac True
#
#               It performs the analysis of SCurveSR and looks at the results.
#               It creates new DAC settings accordingly
#               
#
#
# ****************************************************************************************



import sys
import os, commands
import ROOT
from array import array
import optparse 
from browseCalibFiles import *
from analysisCalibFuncs import *


usage = 'usage: %prog -r runNum'
parser = optparse.OptionParser(usage)
parser.add_option('-r', '--run', dest='run', type='int', help='Number of the run to analyze')
parser.add_option('-i', '--iter', dest='iter', type='int', help='Iteration')
parser.add_option('-k', '--key', dest='key', type='int', help='Starting Key Number')
#parser.add_option('-d', '--dac', dest='dac', type='int', help='Starting dac Number')
parser.add_option('-s', '--savePlots', dest='savePlots', default='False', help='Set this flag to True to save fits to graphs as pdf')
parser.add_option('', '--ignore', dest='ignore', default='False', help='IgnoreError')
parser.add_option("-m","--modality",dest="mod",type="string",default="minimize",help="Modality of dac setting: \"minimize\" for minimizing thresholds and \"increase\" to only increase thresholds of failing rocs. Default is \"minimize\"")
parser.add_option("-o","--outputFile",dest="output",type="string",default="failed",help="Name of the output file containing the list of failing rocs. Default is failed.txt")
parser.add_option("-d","--deltaFile",dest="delta",type="string",default="delta",help="Name of the output file containing the deltaVcThr. Default is delta.txt")
parser.add_option("","--makeNewDac",dest="makeNewDac",type="int",default=0,help="If 1, new dac is created. Default is 0.")


(opt, args) = parser.parse_args()
sys.argv.append('-b')


if opt.run is None:
    parser.error('Please define the run number')
elif opt.iter is None:
    parser.error('Please define the iteration number')



print sys.argv[0]

#runDir = os.environ['POS_OUTPUT_DIRS']
path = '%s/Run_%s/Run_%d/'%(runDir, runfolder(opt.run), opt.run)
print "Current directory is ", os.getcwd()
print "Directory to analyze is ", path

filename = '2DEfficiency'
if(opt.iter != 0): filename = 'SCurve'
files = []


#RunSCurveSmartRangeAnalysis(opt.run)
print os.listdir(path) 
files = [ path + file for file in os.listdir(path) if file.startswith(filename) and file.endswith("root")]
if len(files)<1:
    sys.exit('Could not find ', filename, ' file')
else: 
    browseROCChain(files, checkROCthr, path, opt.iter)
    print "End"
#if(opt.makeNewDac==0): print "N.B: new dac settings were not saved -> set makeNewDac to ture if you want to save them"

        


