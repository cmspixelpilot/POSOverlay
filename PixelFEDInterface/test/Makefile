
# XDAQ_ROOT should be defined 
# define XDAQ_OS amd XDAQ_PLATRFORM
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)


CC_OPTIM=-fPIC -DLINUX 
#ADDED_CFLAGS=-g -DDEBUG
ADDED_CFLAGS=-DNDEBUG 

CAEN_INCLUDE = /usr/local/CAEN/CAEN-VME/Linux/include/

IncludeDirs = \
        -I$(BUILD_HOME)/pixel \
        -I$(XDAQ_ROOT)/include/hal \
        -I$(XDAQ_ROOT)/include

# XDAQ libraries needed for HAL
LibraryDirs = \
        -L$(XDAQ_ROOT)/lib \
        -L$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	-L$(BUILD_HOME)/pixel/lib \
        -L$(BUILD_HOME)/pixel/CalibFormats/SiPixelObjects/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)\
	-L$(BUILD_HOME)/pixel/PixelUtilities/PixelTestStandUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) 

# Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Libraries= -l xcept -l xerces-c -l SiPixelObjects -l toolbox -l log4cplus -l asyncresolv -l uuid -l PixelTestStandUtilities

HAL = -L$(XDAQ_ROOT)/lib -lCAENLinuxBusAdapter -lGenericHAL -lHALUtilities -lconfig

LIBS = $(HAL) -l CAENVME $(LibraryDirs) $(Libraries)  

PIXEL_INCLUDE = $(BUILD_HOME)/pixel/
INC = -I. -I$(CAEN_INCLUDE) $(IncludeDirs) -I$(PIXEL_INCLUDE) 

CFLAGS=$(CC_OPTIM) $(ADDED_CFLAGS) -Wall -Wstrict-prototypes $(INC)

#PixFEDTestSimple :	PixFEDTestSimple.o PixelFED.o
#	g++ -o PixFEDTestSimple PixFEDTestSimple.o PixelFED.o $(LIBS) 
#PixFEDTestSimple.o : PixFEDTestSimple.cc PixelFED.h
#	g++ -o PixFEDTestSimple.o -c PixFEDTestSimple.cc $(CFLAGS)

testPixelFED :	testPixelFED.o PixelFEDInterface.o PixelTTCviInterface.o \
	PixelFEDFifoData.o 
	g++ -o testPixelFED testPixelFED.o PixelFEDInterface.o \
	PixelTTCviInterface.o PixelFEDFifoData.o $(LIBS) 

testPixelFED.o : testPixelFED.cpp ../include/PixelFEDInterface.h PixelTTCviInterface.h ../../CalibFormats/SiPixelObjects/interface/PixelFEDCard.h
	g++ -o testPixelFED.o -c testPixelFED.cpp $(CFLAGS)

loadFPGA :	loadFPGA.o PixelFEDInterface.o \
	PixelFEDFifoData.o 
	g++ -o loadFPGA loadFPGA.o PixelFEDInterface.o \
	PixelFEDFifoData.o $(LIBS) 

loadFPGA.o : loadFPGA.cpp ../include/PixelFEDInterface.h ../../CalibFormats/SiPixelObjects/interface/PixelFEDCard.h
	g++ -o loadFPGA.o -c loadFPGA.cpp $(CFLAGS)

PixelFEDInterface.o : ../src/common/PixelFEDInterface.cc ../include/PixelFEDInterface.h ../../CalibFormats/SiPixelObjects/interface/PixelFEDCard.h
	g++ -o PixelFEDInterface.o -c ../src/common/PixelFEDInterface.cc $(CFLAGS)

PixelFEDFifoData.o : ../src/common/PixelFEDFifoData.cc ../include/PixelFEDFifoData.h
	g++ -o PixelFEDFifoData.o -c ../src/common/PixelFEDFifoData.cc $(CFLAGS)

PixelTTCviInterface.o : PixelTTCviInterface.cc PixelTTCviInterface.h
	g++ -o PixelTTCviInterface.o -c PixelTTCviInterface.cc $(CFLAGS)

FecStatus :	FecStatus.o
	g++ -o FecStatus FecStatus.o $(LIBS)

FecStatus.o : FecStatus.cpp
	g++ -o FecStatus.o -c FecStatus.cpp $(CFLAGS)

clean:
	rm *.o testPixelFED FecStatus
