
TOOL_DIR:=$(BPIXELTOOLS)/tools

CRAZYFLAGS=-g -Wall  -DLINUX -Dx86 -Dlinux 

INCLUDES=-I./include \
	-I ${XDAQ_ROOT}/include \
	-I ${XDAQ_ROOT}/include/hal \
	-I $(PIXEL_INCLUDE) \
	-I ../tools/include


LIBDIRS=\
-L$(PIXEL_INCLUDE)/lib \
-L$(XDAQ_ROOT)/lib 

libs=-lPixelFEDInterface -lPixelTestStandUtilities -lSiPixelObjects -lpthread -ldl  -lGenericHAL -lCAENLinuxBusAdapter -lCAENVME  -lxerces-c  -lxcept -lconfig -ltoolbox -llog4cplus -lasyncresolv -luuid


all:	fed setupPixelFED

fed:	src/fed.cxx
	g++ $(CRAZYFLAGS) src/fed.cxx -o bin/fed $(INCLUDES) -L$(TOOL_DIR)/lib -lSockets $(LIBDIRS) $(libs)

setupPixelFED: src/setupPixelFED.cxx
	g++ $(CRAZYFLAGS) src/setupPixelFED.cxx -o bin/setupPixelFED $(INCLUDES)  -L$(TOOL_DIR)/lib -lSockets  $(LIBDIRS) $(libs)

clean:
	rm -f bin/fed
	rm -f bin/setupPixelFED
