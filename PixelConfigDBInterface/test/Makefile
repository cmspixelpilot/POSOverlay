
# XDAQ_ROOT should be defined 
# define XDAQ_OS amd XDAQ_PLATRFORM
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)


MainDir=PixelConfigDBInterface
ExtDir=SiPixelObjects
Project=pixel
Package=$(MainDir)/test
PackageName=PixelConfigDBInterfacetest

#Sources = MyTest.cc
Executables= MyTest.cc PixelConfigDBCmd.cc

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(XDAQ_ROOT)/daq/extern/oracle/$(XDAQ_OS)$(XDAQ_PLATFORM)/sdk/include

LibraryDirs = \
	$(BUILD_HOME)/$(Project)/$(MainDir)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/$(Project)/$(ExtDir)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/lib \
	$(BUILD_HOME)/$(Project)/CalibFormats/$(ExtDir)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

UserCFlags = 
UserCCFlags = -O -Wno-deprecated 

#Libraries= PixelConfigDataFormats PixelConfigDBInterface occi nnz10 clntsh
Libraries= $(MainDir) $(ExtDir) occi nnz11 clntsh xcept xerces-c toolbox log4cplus uuid config asyncresolv

include $(XDAQ_ROOT)/config/Makefile.rules

clean: cleancore
	rm -rf *~
	rm -rf $(XDAQ_ROOT)/$(Project)/$(Package)/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.exe

cleancore:
	rm -rf core*

include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
