
# XDAQ_ROOT should be defined 
# define XDAQ_OS amd XDAQ_PLATRFORM
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)


MainDir=PixelAnalysisTools
ExtDir=SiPixelObjects
Project=pixel
Package=$(MainDir)/test

#Sources = MyTest.cc
Executables=PixelAnalysis.cc PixelAlive.cc FastHistoFromRaw.cc PixelTrim.cc PixelTrimBits.cc PixelTrimAnalysis.cc PixelModVana.cc PixelTrimVcThr.cc PixelIntimeVana.cc

#Executables=myPixelCalibTest.cc   

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(BUILD_HOME)/$(Project)/$(MainDir)/include \
  $(shell root-config --incdir) \
	$(XERCESCROOT)/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
	$(BUILD_HOME)/$(Project)/CalibFormats/$(ExtDir)/interface
#$(ROOTSYS)/include 

LibraryDirs = \
	$(BUILD_HOME)/$(Project)/$(MainDir)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        $(BUILD_HOME)/$(Project)/lib \
	$(XDAQ_ROOT)/lib \
  $(shell root-config --libdir) \
	$(XERCESCROOT)/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(BUILD_HOME)/$(Project)/CalibFormats/$(ExtDir)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)\
        $(BUILD_HOME)/$(Project)/PixelUtilities/PixelFEDDataTools/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)\
        $(BUILD_HOME)/$(Project)/PixelUtilities/PixelRootUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)\
        $(BUILD_HOME)/$(Project)/PixelUtilities/PixelXmlUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
#$(ROOTSYS)/lib 

UserCFlags = rdynamic
UserCCFlags = -O -Wno-deprecated -pg

#Libraries= PixelConfigDataFormats PixelConfigDBInterface occi nnz10 clntsh
#Libraries= $(MainDir) $(ExtDir) xerces-c
Libraries= $(MainDir) PixelConfigDBInterface SiPixelObjects PixelFEDDataTools PixelRootUtilities PixelXmlUtilities xerces-c\
           Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread pthread m dl \
           occi nnz11 clntsh xcept toolbox log4cplus asyncresolv config uuid
#	   Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics pthread m dl Gui X3d Thread 

include $(XDAQ_ROOT)/config/Makefile.rules

clean: cleancore
	rm -rf *~
	rm -rf $(BUILD_HOME)/$(Project)/$(Package)/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.exe

cleancore:
	rm -rf core*

test:
	cd test; make; cd ..

