
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelUtilities/Pixelb2inUtilities
PackageName=Pixelb2inUtilities

Sources=$(wildcard src/common/*.cc)

DynamicLibrary=Pixelb2inUtilities 
StaticLibrary=

IncludeDirs = \
	$(BUILD_HOME)/$(Project) 

LibraryDirs = 


DependentLibraryDirs = 
DependentLibraries = 


#UserDynamicLinkFlags = $(ROOTLibs)
UserDynamicLinkFlags =


include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
