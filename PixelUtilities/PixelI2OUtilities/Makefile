
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelUtilities/PixelI2OUtilities
PackageName=PixelI2OUtilities

Sources= $(wildcard src/common/*.cc)


DynamicLibrary=PixelI2O
StaticLibrary=

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(XDAQ_ROOT)/include/ 

LibraryDirs = 

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
