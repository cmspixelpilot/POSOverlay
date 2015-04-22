#BUILD_HOME:=$(shell pwd)/../../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

include $(XDAQ_ROOT)/config/mfDefs.coretools
include $(XDAQ_ROOT)/config/mfDefs.extern_coretools
include $(XDAQ_ROOT)/config/mfDefs.general_worksuite
include $(XDAQ_ROOT)/config/mfDefs.hardware_worksuite

Project = pixel
Package = PixelTCDSSupervisor

Sources = \
    version.cc \
    Exception.cc \
    HwLeaseHandler.cc \
    PixelTCDSBase.cc \
    PixelTCDSSupervisor.cc \
    PixeliCISupervisor.cc \
    PixelPISupervisor.cc


IncludeDirs = \
    $(BUILD_HOME)/$(Project) \
    $(BUILD_HOME)/$(Project)/$(Package)/include \
    $(CONFIG_INCLUDE_PREFIX) \
    $(LOG4CPLUS_INCLUDE_PREFIX) \
    $(TOOLBOX_INCLUDE_PREFIX) \
    $(XCEPT_INCLUDE_PREFIX) \
    $(XDAQ_INCLUDE_PREFIX) \
    $(XDATA_INCLUDE_PREFIX) \
    $(XGI_INCLUDE_PREFIX) \
    $(XOAP_INCLUDE_PREFIX) \
    $(XOAP_FILTER_INCLUDE_PREFIX)

DependentLibraryDirs =

DependentLibraries =

UserCFlags =
UserCCFlags =
# Make this work for XDAQ11 too.
XDAQ_TARGET ?= XDAQ11
ifeq ($(XDAQ_TARGET), XDAQ12)
UserCCFlags = -DXDAQ12
endif
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

ExternalObjects =

DynamicLibrary = PixelTCDSSupervisor
StaticLibrary =
Executables =

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
