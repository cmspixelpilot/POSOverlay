CFLAGS  = -Wall -std=c++0x -g -m64 $(DEBUG)
LDFLAGS =  -m64
SOFLAGS = -shared

TOOL_DIR:=$(BPIXELTOOLS)/tools
# The root folder for all the cactus
CACTUS_ROOT = /opt/cactus

# Set the project home directory to the current directory
MY_PROJECT_ROOT = $(shell pwd)

#PIXEL_INCLUDE:=$(PIXEL_INCLUDE)
#PIXEL_INCLUDE=$(BUILD_HOME)/pixel



INC = -I./include -I$(XDAQ_ROOT)/include  -I$(XDAQ_ROOT)/include/hal  -I$(PIXEL_INCLUDE) -I$(TOOL_DIR)/include -I$(TOOL_DIR) -I$(CACTUS_ROOT)/include
 
# XDAQ libraries needed for HAL
LibraryDirs = \
        -L$(PIXEL_INCLUDE)/lib \
        -L$(XDAQ_ROOT)/lib \
	-L${CACTUS_ROOT}/lib \
	-L${MY_PROJECT_ROOT}/lib 


Libraries=  -lPixelFECInterface -lCAENVME  -lGenericHAL -lCAENLinuxBusAdapter -lxerces-c -lxcept -lconfig -ltoolbox -llog4cplus -lasyncresolv -luuid    \
                -lpthread \
                \
                -lboost_filesystem \
                -lboost_regex \
                -lboost_system \
                -lboost_thread \
                \
                -lcactus_extern_pugixml \
                -lcactus_uhal_log \
                -lcactus_uhal_grammars \
                -lcactus_uhal_uhal

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

