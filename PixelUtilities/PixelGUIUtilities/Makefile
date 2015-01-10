
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelUtilities/PixelGUIUtilities
PackageName=PixelGUIUtilities

Sources=$(wildcard src/common/*.cc)

DynamicLibrary=PixelGUIUtilities
StaticLibrary=

IncludeDirs = \
	$(BUILD_HOME)/$(Project) 

LibraryDirs = 

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
