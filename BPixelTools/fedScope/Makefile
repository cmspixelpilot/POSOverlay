# ----------------------------------------------------------------------
# IMPORTANT: To run runFedScope from subdirectory PixelFEDInterface/fedScope:
#   ln -s ../params_fed.dat
#   ln -s ../FEDAddressMap.dat
# ----------------------------------------------------------------------

ROOTCFLAGS    = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      = $(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS     = $(shell $(ROOTSYS)/bin/root-config  --glibs)

CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -Wno-deprecated
LD            = g++
LDFLAGS       = -g
SOFLAGS       = -shared


CXXFLAGS      += $(ROOTCFLAGS)
LIBS           = $(ROOTLIBS) 

NGLIBS         = $(ROOTGLIBS) 
#NGLIBS        += -lMinuit 
GLIBS          = $(NGLIBS)

# -- Default rule
$(addprefix obj/,%.o) : %.cc 
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ================================================================================
HISTFILES = FedProducer.o \
            FedConsumer.o FedConsumerDict.o 

# ================================================================================
FEDscope: $(HISTFILES)
	$(CXX) $(SOFLAGS) $(HISTFILES) -o libFEDscope.so
	@$(MAKE) runFedProducer
	@$(MAKE) runFedConsumer
	@$(MAKE) runFedScope

# ================================================================================
runFedProducer: runFedProducer.o 
# ------------------------------
	$(LD) $(LDFLAGS)  -o runFedProducer runFedProducer.o $(GLIBS) ./libFEDscope.so

# ================================================================================
runFedConsumer: runFedConsumer.o
# ------------------------------
	$(LD) $(LDFLAGS)  -o runFedConsumer runFedConsumer.o $(GLIBS) ./libFEDscope.so

FedConsumerDict.cc: FedConsumer.hh
	$(ROOTSYS)/bin/rootcint -f FedConsumerDict.cc -c FedConsumer.hh

# ================================================================================
all: 
# --
	@$(MAKE) clean
	@$(MAKE) FEDscope
	@$(MAKE) runFedProducer
	@$(MAKE) runFedConsumer
	@$(MAKE) runFedScope

cleanall:
	rm -f *.o  *Dict.* core*
	rm -f runFedProducer runFedConsumer fedScope libFEDscope.so
	rm -f histos.root  runFedScope

clean:
	rm -f *~ *.o  *Dict.*  


# ======================================================================
# The XDAQ specific part
# ======================================================================
IncludeDirs = -I$(XDAQ_ROOT)/include \
               -I$(XDAQ_ROOT)/include/hal
BUILD_HOME = ${TRIDAS_ROOT}/${POSVERSION}
INC = -I. $(IncludeDirs) -I${BUILD_HOME}/

#        -I$(XDAQ_ROOT)/include/xcept \

LibraryDirs = -L${BUILD_HOME}/lib \
              -L${BUILD_HOME}/BPixelTools/tools/lib \
              -L${XDAQ_ROOT}/lib   


#NEW:
Libraries= -l xcept -l xerces-c -l config -l PixelFEDInterface -l SiPixelObjects -lPixelTestStandUtilities -ltoolbox -llog4cplus -lasyncresolv -luuid -lSockets

#OLD:
# Libraries= -l xcept -l xerces-c -l config -l PixelConfigDataFormats -l PixelFEDInterface


HAL = -lCAENLinuxBusAdapter -lGenericHAL -lHALUtilities

LLIBS = $(HAL) -l CAENVME $(LibraryDirs) $(Libraries) 


runFedScope: runFedScope.o  FedProducer.o
	g++ -fPIC -DLINUX -o runFedScope runFedScope.o  \
	$(LLIBS) $(ROOTGLIBS) ./libFEDscope.so

runFedScope.o : runFedScope.cpp 
	g++ -fPIC -DLINUX $(INC) -o runFedScope.o -c runFedScope.cpp $(CFLAGS) $(ROOTCFLAGS)


tar:
	cd .. && tar cvf fedScope.tar --exclude fedScope/params_fed.dat --exclude fedScope/FEDAddressMap.dat fedScope

