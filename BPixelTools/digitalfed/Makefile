########################################################################
TOOL_DIR:=$(BPIXELTOOLS)/tools

EXE	=	digitalfed

CC	=	g++

COPTS	=	-fPIC -D LINUX -Wall 
#COPTS	=	-g -fPIC -DLINUX -Wall 

FLAGS	=	-Wall -s
#FLAGS	=	-Wall

#DEPLIBS	=       /opt/xdaq/lib/libCAENVME.so /usr/lib/libcurses.a
DEPLIBS	=       /opt/xdaq/lib/libCAENVME.so

LIBS	=	-L/opt/xdaq/lib -lttcutils -lttcttcci -lpthread -ldl -lboost_program_options -lxcept -lxdaq -ltoolbox -lxdata -luuid -lpeer -lxgi -llog4cplus -lxerces-c -lcgicc -lxoap -lconfig -lasyncresolv -llogxmlappender -llogudpappender -lmimetic -lHALUtilities -lGenericHAL -lCAENVME -lCAENLinuxBusAdapter -lDummyBusAdapters -L$(TOOL_DIR)/lib -lSockets

INCLUDEDIR =	-I. -I/opt/xdaq/include -I/opt/xdaq/include/ttc/ttcci -I ../tools/include

OBJS	=	digitalfed.o

INCLUDES =	CAENVMElib.h CAENVMEtypes.h CAENVMEoslib.h 

#########################################################################

all	:	$(EXE)

clean	:
		/bin/rm -f $(OBJS) $(EXE)

$(EXE)	:	$(OBJS) 
		/bin/rm -f $(EXE)
		$(CC) $(FLAGS)  -o $(EXE) $(OBJS) $(DEPLIBS) $(LIBS)

$(OBJS)	:	digitalfed.cpp

%.o	:	%.cpp
		$(CC) $(COPTS) $(INCLUDEDIR) -D LINUX -c -o $@ $<

