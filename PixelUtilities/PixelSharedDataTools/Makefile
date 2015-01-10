
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelUtilities/PixelSharedDataTools
PackageName=PixelSharedDataTools

Sources=$(wildcard src/common/*.cc)

DynamicLibrary=PixelSharedDataTools
StaticLibrary=

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(BUILD_HOME)/PixelUtilities/PixelSharedDataTools/include 

LibraryDirs = 

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
