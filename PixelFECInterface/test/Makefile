#Started with FED Makefile
#
# XDAQ_ROOT should be defined 
# define XDAQ_OS amd XDAQ_PLATRFORM
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

PACKAGE=pixel

CC_OPTIM=-fPIC 
#ADDED_CFLAGS=-g -DDEBUG
ADDED_CFLAGS=-DNDEBUG 

CAEN_INCLUDE = /usr/local/CAEN/CAEN-VME/Linux/include/
 
IncludeDirs = \
        -I$(BUILD_HOME)/$(PACKAGE) \
        -I$(XDAQ_ROOT)/include \
        -I$(XDAQ_ROOT)/include/hal

 
# XDAQ libraries needed for HAL
LibraryDirs = \
        -L$(XDAQ_ROOT)/lib \
        -L$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib 

 
# Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Libraries= -l xcept -l xerces-c 

HAL = -L$(XDAQ_ROOT)/daq/hal/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) -lCAENLinuxBusAdapter -lGenericHAL -lHALUtilities -lconfig

LIBS = $(HAL) -l CAENVME $(LibraryDirs) $(Libraries)  

PIXEL_INCLUDE = $(XDAQ_ROOT)/pixel/ 
INC = -I. -I$(CAEN_INCLUDE) $(IncludeDirs) -I$(PIXEL_INCLUDE) 

CFLAGS=$(CC_OPTIM) $(ADDED_CFLAGS) -Wall -Wstrict-prototypes $(INC)


testPFEC :	testPFEC.o PixelFECInterface.o
	g++ -o testPFEC testPFEC.o PixelFECInterface.o $(LIBS) 
#testPFEC.o : testPFEC.cc PixelFED.h
testPFEC.o : testPFEC.cc ../include/PixelFECInterface.h 
	g++ -o testPFEC.o -c testPFEC.cc $(CFLAGS)

PixelFECInterface.o: ../src/common/PixelFECInterface.cc ../include/PixelFECInterface.h
	g++ -o PixelFECInterface.o -c ../src/common/PixelFECInterface.cc $(CFLAGS)

#PixFEDTestSimple :	PixFEDTestSimple.o PixelFED.o
#	g++ -o PixFEDTestSimple PixFEDTestSimple.o PixelFED.o $(LIBS) 
#PixFEDTestSimple.o : PixFEDTestSimple.cc PixelFED.h
#	g++ -o PixFEDTestSimple.o -c PixFEDTestSimple.cc $(CFLAGS)

#testPixelFED :	testPixelFED.o PixelFEDInterface.o PixelTTCviInterface.o \
#	PixelFEDFifoData.o PixelFEDParameterReader.o
#	g++ -o testPixelFED testPixelFED.o PixelFEDInterface.o \
#	PixelTTCviInterface.o PixelFEDFifoData.o PixelFEDParameterReader.o $(LIBS) 

#testPixelFED.o : testPixelFED.cpp ../include/PixelFEDInterface.h PixelTTCviInterface.h
#	g++ -o testPixelFED.o -c testPixelFED.cpp $(CFLAGS)
  
#PixelFEDInterface.o : ../src/common/PixelFEDInterface.cc ../include/PixelFEDInterface.h
#	g++ -o PixelFEDInterface.o -c ../src/common/PixelFEDInterface.cc $(CFLAGS)

#PixelFEDFifoData.o : ../src/common/PixelFEDFifoData.cc ../include/PixelFEDFifoData.h
#	g++ -o PixelFEDFifoData.o -c ../src/common/PixelFEDFifoData.cc $(CFLAGS)

#PixelTTCviInterface.o : PixelTTCviInterface.cc PixelTTCviInterface.h
#	g++ -o PixelTTCviInterface.o -c PixelTTCviInterface.cc $(CFLAGS)

#PixelFEDParameterReader.o : ../src/common/PixelFEDParameterReader.cc ../include/PixelFEDParameterReader.h
#	g++ -o PixelFEDParameterReader.o -c ../src/common/PixelFEDParameterReader.cc $(CFLAGS)

 
clean:
	rm *.o testPFEC

