
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelUtilities/PixelTKFECDataTools
PackageName=PixelTKFECDataTools

Sources=$(wildcard src/common/*.cc)

DynamicLibrary=PixelTKFECDataTools
StaticLibrary=

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(BUILD_HOME)/PixelUtilities/PixelTKFECDataTools/include 

LibraryDirs = 

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
