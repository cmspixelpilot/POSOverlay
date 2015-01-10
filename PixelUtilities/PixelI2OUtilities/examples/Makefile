XDAQ_BACK_TO_ROOT:=../../../..
include $(XDAQ_BACK_TO_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel/PixelUtilities/PixelI2OUtilities
Package=examples

Sources=I2OExample.cc
DynamicLibrary=I2OExample
StaticLibrary=
ExternalObjects= $(XDAQ_ROOT)/pixel/PixelUtilities/PixelI2OUtilities/lib/linux/x86/libPixelI2O.a
IncludeDirs = \
	$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/extern/gmp/gmp-4.1.4/ \
	$(XDAQ_ROOT)/daq/extern/cgicc/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/toolbox/include \
	$(XDAQ_ROOT)/daq/toolbox/include/$(XDAQ_OS) \
	$(XDAQ_ROOT)/daq/toolbox/include/solaris \
	$(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
        $(XDAQ_ROOT)/daq/xoap/include \
	$(XDAQ_ROOT)/daq/xdata/include \
	$(XDAQ_ROOT)/daq/pt/include \
	$(XDAQ_ROOT)/daq/xcept/include \
	$(XDAQ_ROOT)/daq/xgi/include \
	$(XDAQ_ROOT)/daq/ \
	$(XDAQ_ROOT)/daq/i2o/include/ \
	$(XDAQ_ROOT)/daq/i2o/utils/include/ \
	$(XDAQ_ROOT)/daq/i2o/include/i2o \
	$(XDAQ_ROOT)/daq/i2o/include/i2o/shared \
	$(XDAQ_ROOT)/daq/extern/i2o/include/ \
	$(XDAQ_ROOT)/daq/extern/i2o/include/i2o \
        $(XDAQ_ROOT)/daq/extern/i2o/include/i2o/shared \
	$(XDAQ_ROOT)/daq/xdaq/include \
	$(XDAQ_ROOT)/daq/xdaq/include/solaris \
	$(XDAQ_ROOT)/daq/xdaq/include/linux \
	$(XDAQ_ROOT)/daq/interface/shared/include \
	$(XDAQ_ROOT)/daq/interface/evb/include	\
	$(XDAQ_ROOT)/pixel/PixelUtilities/PixelI2OUtilities/include \
	$(XDAQ_ROOT)/pixel/PixelUtilities/PixelI2OUtilities/examples 

LibraryDirs = $(XDAQ_ROOT)/daq/xoap/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/toolbox/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/xdata/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/xcept/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/pt/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/extern/gmp/gmp-4.1.4/.libs 


include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
