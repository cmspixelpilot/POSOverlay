#!/usr/bin/env python

# ************************************************************************************************************
#
#  Authors: Annapaola de Cosa
#           decosa@cern.ch
#           Martina Malberti
#           malberti@cern.ch
#           March/2014
#  
#  Description:
#  CalibUtilies are a set of tools aimed to carry out common functions 
#  for the analysis of calibrations results which are stored in root files.
#
#  - browseROCChain(files,  func, *args): opens all the files from the list
#    and browse the directories up to the last one containing info on the ROCs 
#    saved as canvanses or histos, 
#    e.g  BPix/BPix_BmI/BPix_BmI_SEC4/BPix_BmI_SEC4_LYR1/BPix_BmI_SEC4_LYR1_LDR5F/BPix_BmI_SEC4_LYR1_LDR5F_MOD1
#    -- files:   list of files to open and browse 
#    -- func:    function to execute
#    -- *args:   arguments to pass to the function
#
#  - browseFolder(files, dirName,  func, *args): opens all the files
#    of the list and accesses the directory called dirName, and 
#    executes the function func.
#    -- files:   list of files to open and browse 
#    -- dirName: name of the directory to access
#    -- func:    function to execute
#    -- *args:   arguments to pass to the function
#
#  - runfolder(run): select the right folder for the given run.
#    Runs are categorized in folders such as Run_0, Run_1000, Run_2000 etc
#    -- run: run number
# ***************************************************************************************************************



import sys
import os, commands
import ROOT

def runfolder(run):
    f = int(run/1000)*1000
    return f 



def browseROCChain(files,  func, *args):
    for file in files:
        f = ROOT.TFile.Open(file)
        try:
            f = ROOT.TFile.Open(file)
        except IOError:
            print "Cannot open ", file
        else:
            print "Opening file ",  file
            f.cd()
            
            ### Browse the file up to the last directory

            for r in ROOT.gDirectory.GetListOfKeys(): # BPIX or FPIX
                if(r.ReadObj().GetName()=="BPix" or r.ReadObj().GetName()=="FPix"):
                    if r.IsFolder():
                        r.ReadObj().cd()

                        for shell in ROOT.gDirectory.GetListOfKeys(): # BmI, BmO, BpI, BpO folders
                            shell.ReadObj().cd()
                            
                            for sect in ROOT.gDirectory.GetListOfKeys(): # Sector folders, e.g: BmI_SEC4
                                if sect.IsFolder():
                                    sect.ReadObj().cd()
                                    
                                    for ly in ROOT.gDirectory.GetListOfKeys(): # Layer folders, e.g.: BmI_SEC4_LYR1
                                        if ly.IsFolder():
                                            ly.ReadObj().cd()

                                            for ld in ROOT.gDirectory.GetListOfKeys(): # ladder folders, e.g.:  BmI_SEC4_LYR1_LDR5F
                                                if ld.IsFolder():
                                                    ld.ReadObj().cd()
                                                    
                                                    for mod in ROOT.gDirectory.GetListOfKeys(): # module folders, e.g.:  BmI_SEC4_LYR1_LDR5F_MOD1
                                                        if mod.IsFolder():
                                                            mod.ReadObj().cd()
                                                            
                                                            func(*args)

                                                            

        



                                                      

def browseFolder(files, treeName,  func, *args):
    for file in files:
        f = ROOT.TFile.Open(file)
        try:
            f = ROOT.TFile.Open(file)
        except IOError:
            print "Cannot open ", file
        else:
            print "Opening file ", file
            f.cd()
            
            obj = f.Get(treeName)
            try:
                obj.cd()
            except TypeError:
                print "This is not a directory."
            else:
                list = ROOT.gDirectory.GetListOfKeys()
                func(*args)
                

