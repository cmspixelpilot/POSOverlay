# XERCESROOT:=${XERCESROOT}

FECSOFTWARE_ROOT:=${ENV_CMS_TK_FEC_ROOT}

# SOCKETDIR=../tools
#XERCESLIB=xerces-c
#HALLIB = GenericHAL config

CRAZYFLAGS=-O0 -g -Wall -DDEFAULTXMLFILENAME=\"Unknown\" -DDCUADDRESS=0x0   -DLASERDRIVER42 -DDCUTHREAD -DDCUFILTER -DI2C100 -DTRACKER  -DXML_VALIDATION -DBT1003 -DLINUX -Wall -Werror -DCORETOOLS_VERSION=3.7.3 -Dx86 -DBUSVMECAENPCI -DBUSVMECAENUSB -DBUSPCIFEC -Dlinux -DLITTLE_ENDIAN__ -std=c++0x

INCLUDES=-I ./include \
	 -I $(FECSOFTWARE_ROOT)/generic/include \
         -I $(FECSOFTWARE_ROOT)/FecPciDeviceDriver/include \
         -I ${XDAQ_ROOT}/include\
         -I ${XDAQ_ROOT}/include/hal\
	 -I ../tools/include\
	 -I ${CACTUSINCLUDE} 
#         -I $(FECSOFTWARE_ROOT)/ThirdParty/APIConsoleDebugger/include \


LIBDIRS=\
-L$(XDAQ_ROOT)/lib \
-L$(FECSOFTWARE_ROOT)/lib \
-L$(CACTUSLIB)

Libraries = -lpthread -ldl -lxerces-c -lGenericHAL -lconfig -lxcept -lCAENLinuxBusAdapter -lCAENVME -lDeviceAccess -ltoolbox -llog4cplus -lasyncresolv -luuid -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal


ccu:	dirs src/ccu.cxx objects/ServerAccess.o ../tools/lib/libSockets.so
	g++ $(CRAZYFLAGS) src/ccu.cxx -std=c++0x objects/ServerAccess.o -o bin/ccu $(INCLUDES) -L../tools/lib -lSockets $(LIBDIRS) $(Libraries)

objects/ServerAccess.o: dirs src/ServerAccess.cc include/ServerAccess.h
	g++ $(CRAZYFLAGS) -c src/ServerAccess.cc -std=c++0x -o $@ $(INCLUDES)

../tools/lib/libSockets.so:
	(cd ../tools; make; cd ../ccu)
path:
	echo $(PATH)
dirs:
	mkdir -p lib objects bin

clean:
	rm -r bin/ccu objects/ServerAccess.o
