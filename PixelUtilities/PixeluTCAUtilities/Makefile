
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelUtilities/PixeluTCAUtilities
PackageName=PixeluTCAUtilities

Sources=$(wildcard src/common/*.cc)

DynamicLibrary=PixeluTCAUtilities 
StaticLibrary=

CACTUS_ROOT = /opt/cactus

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(CACTUS_ROOT)/include

LibraryDirs = 

UserCFlags = -O
UserCCFlags = -std=c++0x -g -O -Wno-long-long

DependentLibraryDirs = $(BUILD_HOME)/pixel/lib \
                        $(XDAQ_ROOT)/lib \
	         	${CACTUS_ROOT}/lib
DependentLibraries = \
	boost_system \
	boost_thread \
	boost_regex \
	boost_filesystem \
	cactus_extern_pugixml \
	cactus_uhal_log \
	cactus_uhal_grammars \
	cactus_uhal_uhal


#UserDynamicLinkFlags = $(ROOTLibs)
UserDynamicLinkFlags =


include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
