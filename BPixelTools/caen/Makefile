#CAEN_PS=  /home/l_pixel/CAEN_PS_cpp
CAEN_PS:=$(CAEN_PS)
SOCKETDIR:=$(BPIXELTOOLS)/tools

CAENLIB =$(CAEN_PS)/lib

OBJDIR=$(CAEN_PS)/objects
OBJECTS=  $(OBJDIR)/PowerSupply.o $(OBJDIR)/PS_Channel.o $(OBJDIR)/PS_CrateMap.o $(OBJDIR)/PS_Parameter.o $(OBJDIR)/PS_Slot.o $(OBJDIR)/PS_SysProperty.o $(OBJDIR)/PSU_Finder.o $(OBJDIR)/tinyxml.o $(OBJDIR)/tinystr.o $(OBJDIR)/tinyxmlerror.o $(OBJDIR)/tinyxmlparser.o


INCLUDES=-I$(CAEN_PS)/include -I$(CAEN_PS)/sources -I$(CAEN_PS)/tinyxml  -I$(SOCKETDIR)/include
LIBS= -L $(CAENLIB) -lcaenhvwrapper -lncurses -lhscaenet -lpthread -ldl -lm -L$(SOCKETDIR)/lib -lSockets



caen:	src/caen.cxx
	g++ -g -DUNIX  src/caen.cxx -o bin/caen $(OBJECTS) $(INCLUDES) $(LIBS)

clean:
	rm bin/caen

stupid:
	@echo $(CAEN_PS)
	@echo "CAENLIB="
	@echo $(CAENLIB)
	@echo "OBJECTS="
	@echo $(OBJECTS)
	@echo "INCLUDES="
	@echo $(INCLUDES)
	@echo "LIBS="
	@echo $(LIBS)