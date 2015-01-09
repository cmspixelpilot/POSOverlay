
# XDAQ_ROOT should be defined 
# define XDAQ_OS amd XDAQ_PLATRFORM
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)


CC_OPTIM=-fPIC -DLINUX 
#ADDED_CFLAGS=-g -DDEBUG
ADDED_CFLAGS=-DNDEBUG 

CAEN_INCLUDE = /usr/local/CAEN/CAEN-VME/Linux/include/
 
IncludeDirs = \
        -I$(XDAQ_ROOT)/daq/xcept/include \
        -I$(XDAQ_ROOT)/daq/hal/generic/include \
        -I$(XDAQ_ROOT)/daq/hal/generic/include/linux \
        -I$(XDAQ_ROOT)/daq/hal/busAdapter/include \
        -I/usr/local/CAEN/CAEN-VME/Linux/include 
#        $(XDAQ_ROOT)/daq/xgi/include \
#        $(XDAQ_ROOT)/daq/ \
#        $(XDAQ_ROOT)/daq/extern/i2o/include/ \
#        $(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
#        $(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/include/xercesc \
#        $(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
#        $(XDAQ_ROOT)/daq/extern/cgicc/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
#        $(XDAQ_ROOT)/daq/toolbox/include \
#        $(XDAQ_ROOT)/daq/toolbox/include/$(XDAQ_OS) \
#        $(XDAQ_ROOT)/daq/toolbox/include/solaris \
#        $(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
#        $(XDAQ_ROOT)/daq/xoap/include \
#        $(XDAQ_ROOT)/daq/xdata/include \
#        $(XDAQ_ROOT)/daq/pt/include \
#        $(XDAQ_ROOT)/daq/extern/i2o/include/i2o \
#        $(XDAQ_ROOT)/daq/extern/i2o/include/i2o/shared \
#        $(XDAQ_ROOT)/daq/xdaq/include \
#        $(XDAQ_ROOT)/daq/xdaq/include/solaris \
#        $(XDAQ_ROOT)/daq/xdaq/include/linux \
#        $(XDAQ_ROOT)/$(Package)/include \
 
# XDAQ libraries needed for HAL
LibraryDirs = \
        -L$(XDAQ_ROOT)/pixel/PixelConfigDataFormats/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        -L$(XDAQ_ROOT)/pixel/PixelFECInterface/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        -L$(XDAQ_ROOT)/daq/xcept/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        -L$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
        -L/usr/local/CAEN/CAEN-VME/Linux/lib/
        # $(XDAQ_ROOT)/daq/hal/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
        #$(XDAQ_ROOT)/daq/xoap/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        #$(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
        #$(XDAQ_ROOT)/daq/toolbox/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        #$(XDAQ_ROOT)/daq/xdata/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        #$(XDAQ_ROOT)/daq/pt/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        #$(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
        # $(ENV_CMS_ORACLE_HOME)/lib \
        # $(ENV_CMS_FEC_ROOT)/generic/lib/linux/x86 \
        # $(ENV_CMS_FEC_ROOT)/lib \
        # $(CAENLIBDIR) $(HALLIBDIR) $(FEDLIBDIR)

# Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Libraries= -l xcept -l xerces-c -l PixelConfigDataFormats

#HAL = -L$(XDAQ_ROOT)/daq/hal/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) -lCAENLinuxBusAdapter -lGenericHAL -lHALUtilities 
HAL = -L$(XDAQ_ROOT)/daq/hal/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)  -lGenericHAL -lHALUtilities -lPixelFECInterface -lCAENVME

#LIBS = $(HAL) -l CAENVME $(LibraryDirs) $(Libraries)  
LIBS = $(HAL) $(LibraryDirs) $(Libraries)  

PIXEL_INCLUDE = $(XDAQ_ROOT)/pixel/ 
INC = -I. -I$(CAEN_INCLUDE) $(IncludeDirs) -I$(PIXEL_INCLUDE) 

CFLAGS=$(CC_OPTIM) $(ADDED_CFLAGS) -Wall -Wstrict-prototypes $(INC)

#PixFEDTestSimple :	PixFEDTestSimple.o PixelFED.o
#	g++ -o PixFEDTestSimple PixFEDTestSimple.o PixelFED.o $(LIBS) 
#PixFEDTestSimple.o : PixFEDTestSimple.cc PixelFED.h
#	g++ -o PixFEDTestSimple.o -c PixFEDTestSimple.cc $(CFLAGS)

testPixelConfigRead:	testPixelConfigRead.cc
	g++ -o testPixelConfigRead -I$(XDAQ_ROOT)/pixel $(IncludeDirs) testPixelConfigRead.cc $(LIBS) 

testPixelFEC:	testPixelFEC.cc
	g++ -o testPixelFEC -I$(XDAQ_ROOT)/pixel $(IncludeDirs) testPixelFEC.cc $(LIBS) 

