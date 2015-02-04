CFLAGS  = -Wall -g -m64 $(DEBUG)
LDFLAGS =  -m64
SOFLAGS = -shared

TOOL_DIR:=$(BPIXELTOOLS)/tools
#PIXEL_INCLUDE:=$(PIXEL_INCLUDE)
#PIXEL_INCLUDE=$(BUILD_HOME)/pixel

INC = -I./include -I$(XDAQ_ROOT)/include  -I$(XDAQ_ROOT)/include/hal  -I$(PIXEL_INCLUDE) -I$(TOOL_DIR)/include -I$(TOOL_DIR)
 
# XDAQ libraries needed for HAL
LibraryDirs = \
        -L$(PIXEL_INCLUDE)/lib \
        -L$(XDAQ_ROOT)/lib 


Libraries=  -lPixelFECInterface -lCAENVME  -lGenericHAL -lCAENLinuxBusAdapter -lxerces-c -lxcept -lconfig -ltoolbox -llog4cplus -lasyncresolv -luuid

LIBS = $(LibraryDirs) $(Libraries) -L$(TOOL_DIR)/lib -lSockets
OBJ=objects
OBJECTS=$(OBJ)/SysCommand.o $(OBJ)/Module.o $(OBJ)/TBM.o $(OBJ)/Roc.o $(OBJ)/ControlNetwork.o


pxfec:	src/pxfec.cxx $(OBJECTS)  ../tools/lib/libSockets.so
	g++ $(CFLAGS) -o bin/pxfec src/pxfec.cxx $(OBJECTS) $(INC) $(LIBS)  

objects/SysCommand.o: src/SysCommand.cc include/SysCommand.h
	g++ $(CFLAGS) -c  src/SysCommand.cc -o  $@ -I include -I $(PIXEL_INCLUDE)

objects/ControlNetwork.o:	src/ControlNetwork.cc include/ControlNetwork.h include/Module.h include/SysCommand.h
	g++ $(CFLAGS) -c src/ControlNetwork.cc -oobjects/ControlNetwork.o $(INC)

objects/Module.o:	src/Module.cc include/Module.h include/Roc.h include/TBM.h include/SysCommand.h
	g++ $(CFLAGS) -c src/Module.cc -o  $@ $(INC)

objects/TBM.o:	src/TBM.cc include/TBM.h include/SysCommand.h
	g++ $(CFLAGS) -c src/TBM.cc -o  $@ $(INC)

objects/Roc.o:	src/Roc.cc include/Roc.h include/SysCommand.h
	g++ $(CFLAGS) -c src/Roc.cc -o  $@ $(INC)

../tools/lib/libSockets.so:
	(cd ../tools; make; cd ../ccu)
clean:	
	rm -r $(OBJECTS) bin/pxfec 

