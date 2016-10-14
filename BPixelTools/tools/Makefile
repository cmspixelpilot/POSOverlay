CC=		g++
FLAGS=		 -O0 -g -Wall -I./include -fPIC
#FLAGS=		 -g -Wall -DDEBUG -I./include 
LDLAGS= -m64 
SOFLAGS= -shared
OBJECTDIR=objects
SRCDIR=src
INCLUDEDIR=include
PIXELLIB=$(BUILD_HOME)/pixel/lib

OBJECTS=$(OBJECTDIR)/Getline.o $(OBJECTDIR)/MultiplexingServer.o $(OBJECTDIR)/SimpleCommand.o $(OBJECTDIR)/SimpleConnection.o $(OBJECTDIR)/VMELock.o $


lib:	dirs $(OBJECTS)
	$(CC) -shared -o lib/libSockets.so $(LDFLAGS) $(OBJECTS)
	rm -f $(PIXELLIB)/libSockets.so
	cp lib/libSockets.so  $(PIXELLIB)/libSockets.so

$(OBJECTDIR)/Getline.o:		dirs $(SRCDIR)/Getline.c
	$(CC) $(FLAGS) -c $(SRCDIR)/Getline.c -o $(OBJECTDIR)/Getline.o

$(OBJECTDIR)/VMELock.o:		dirs $(SRCDIR)/VMELock.cc
	$(CC) $(FLAGS) -c $(SRCDIR)/VMELock.cc -o $(OBJECTDIR)/VMELock.o

$(OBJECTDIR)/MultiplexingServer.o: dirs $(SRCDIR)/MultiplexingServer.cc $(INCLUDEDIR)/MultiplexingServer.h $(OBJECTDIR)/SimpleCommand.o $(OBJECTDIR)/SimpleConnection.o $(OBJECTDIR)/Getline.o
	$(CC) $(FLAGS) -c $(SRCDIR)/MultiplexingServer.cc -o $(OBJECTDIR)/MultiplexingServer.o	

$(OBJECTDIR)/SimpleCommand.o:	dirs $(SRCDIR)/SimpleCommand.cc $(INCLUDEDIR)/SimpleCommand.h
	$(CC) $(FLAGS) -c $(SRCDIR)/SimpleCommand.cc -o $(OBJECTDIR)/SimpleCommand.o

$(OBJECTDIR)/SimpleConnection.o:	dirs $(SRCDIR)/SimpleConnection.cc $(INCLUDEDIR)/SimpleConnection.h
	$(CC) $(FLAGS) -c $(SRCDIR)/SimpleConnection.cc -o $(OBJECTDIR)/SimpleConnection.o

dirs:
	mkdir -p lib objects

clean:
			rm -f $(OBJECTS); rm -f lib/libSockets.so

