########################################################################

BIN_DIR =	bin

EXE	=	$(BIN_DIR)/fed_ph1

EXE2    =	$(BIN_DIR)/checkfifo2s

SRC_DIR =	src

OBJS    =	$(SRC_DIR)/fed_ph1.o

OBJS2   =	$(SRC_DIR)/checkfifo2s.o

CXX	=	g++

COPTS	=	-fPIC -D LINUX -Wall
#COPTS	=	-g -fPIC -DLINUX -Wall

FLAGS	=	-Wall -s
#FLAGS	=	-Wall

LIBS	=	/opt/xdaq/lib/libCAENVME.so #/usr/lib64/libcurses.so
#LIBS	=	-L/opt/xdaq/lib -l CAENVME -L/usr/lib -l curses

INCLUDES=	-I. -I/opt/xdaq/include

#########################################################################

all	:	$(BIN_DIR) $(EXE) $(EXE2)

$(BIN_DIR):
		mkdir -p $(BIN_DIR)

$(EXE)	:	$(OBJS)
		$(CXX) $(FLAGS) -o $(EXE) $(OBJS) $(LIBS)

$(EXE2)	:	$(OBJS2)
		$(CXX) $(FLAGS) -o $(EXE2) $(OBJS2) $(LIBS)

.PHONY  :	clean

clean   :
		/bin/rm -f $(EXE); \
		/bin/rm -f $(EXE2); \
		/bin/rm -f $(OBJS); \
		/bin/rm -f $(OBJS2)


%.o	:	%.cpp
		$(CXX) $(COPTS) $(INCLUDES) -D LINUX -c -o $@ $<
