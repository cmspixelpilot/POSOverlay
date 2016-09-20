import sys, os
import argparse

parser = argparse.ArgumentParser(description='Find a ROOT file among many with the run number you input.')
parser.add_argument('-r', '--runs', type=str, default="", help="Comma-separated list of runs to find vetoed modules in (e.g. 262204,262205,262235)")
parser.add_argument('--modStrToNum',  default="modList.txt", help="Text file containing lines like \'302055700 BPix_BpI_SEC4_LYR1_LDR5F_MOD1\'")
parser.add_argument('-o', '--output', default="vetoModules.txt", help="Name of output file.")
args = parser.parse_args()

if args.runs=="":
    print "No runs to search.  Exiting."
    sys.exit(1)

runs=args.runs.split(",")

globalConfigKeys=[]

moduleStrToNum={}
modLines=[]
if os.path.isfile(args.modStrToNum):
    modStrToNumFile=open(args.modStrToNum)
    modLines=modStrToNumFile.readlines()
    modStrToNumFile.close()
else:
    print "No ",args.modStrToNum

for modLine in modLines:
    try:
        num=int(modLine.split()[0])
        string=modLine.split()[1]
        moduleStrToNum[string]=num
    except:
        print "Can't do",modLine

for run in runs:
    runDir=int(run)/1000
    runDir=runDir*1000
###    pixelConfigFileName="/pixel/data0/Run_"+str(runDir)+"/Run_"+str(run)+"/PixelConfigurationKey.txt"
    pixelConfigFileName="/pixelscratch/pixelscratch/data0/Run_"+str(runDir)+"/Run_"+str(run)+"/PixelConfigurationKey.txt"
    if os.path.isfile(pixelConfigFileName):
        pixelConfigFile=open(pixelConfigFileName)
    else:
        print "No ",pixelConfigFileName
        continue

    lines=pixelConfigFile.readlines()
    for line in lines:
        if line.find("Pixel Global Configuration Key = ")!=-1:
            key=(line.split("Pixel Global Configuration Key = ")[1]).split("\n")[0]
            #print key
            if key not in globalConfigKeys:
                globalConfigKeys.append(key)

    pixelConfigFile.close()
    
print "globalConfigKeys",globalConfigKeys


pixelCfgMapFileName="/pixelscratch/pixelscratch/config/Pix/configurations.txt"
if  os.path.isfile(pixelCfgMapFileName):
    pixelCfgMapFile=open(pixelCfgMapFileName)
else:
    print "No",pixelCfgMapFileName
    sys.exit(1)

pixelCfgMapLines=pixelCfgMapFile.readlines()
pixelCfgMapFile.close()

detConfigs=[]

iLine=0
for iLine in range(len(pixelCfgMapLines)):
    mapLine=pixelCfgMapLines[iLine]
    if mapLine.find("key ")!=-1:
        key=mapLine.split()[1]
        if key in globalConfigKeys:
            #print pixelCfgMapLines[iLine+1]
            if pixelCfgMapLines[iLine+1].find("detconfig")!=-1:
                thisdetconfig=pixelCfgMapLines[iLine+1].split()[1]
                #print thisdetconfig
                if thisdetconfig not in detConfigs:
                    detConfigs.append(thisdetconfig)

print "detector configurations",detConfigs

badModules=[]

for detConfig in detConfigs:
    detectorConfigFileName="/pixelscratch/pixelscratch/config/Pix/detconfig/"+detConfig+"/detectconfig.dat"
    detectorConfigFile=open(detectorConfigFileName)

    lines=detectorConfigFile.readlines()
    detectorConfigFile.close()

    for line in lines:
        if line.find("noAnalogSignal")!=-1:
            ## if any ROC is missing remove the entire module
            module=line.split("_ROC")[0]
            if module not in badModules:
                badModules.append(module)

badModules.sort()
print "bad modules",badModules

outputFile=open(args.output,"w")
print "Writing bad module numbers to",args.output
for badModule in badModules:
    outputFile.write(str(moduleStrToNum[badModule])+"\n")

outputFile.close()
                


