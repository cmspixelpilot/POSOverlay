TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f
COPY_FILE= $(COPY) -p
COPY_DIR = $(COPY) -pR
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
CHK_FILE_EXISTS= test -f
MKDIR    = mkdir -p
QMAKE    = qmake
ECHO     = echo


include FecHeader.linux

######## KH for make install ###########
ifeq ($(origin LIB_DEST), undefined)
  XDAQ_LIB_DEST = lib
else
  XDAQ_LIB_DEST = $(LIB_DEST)
endif
LIB_DEST ?= lib
BIN_DEST ?= bin
CONFIG_DEST ?= /tmp
INCLUDE_DEST ?= /tmp
INCLUDE_FILES = generic/include/*.h* \
        FecPciDeviceDriver/include/*.h* 
########################################


all: tracker totem preshower
#	( $(ECHO) "Please specify or 'standalone' or 'tracker' or 'totem' or 'preshower' tags" )

tracker: Generic 
	make DeviceFactoryTemplate 
	make I2CTemplate
	make APIConsoleDebugger 
	make APIXMLDebugger  
	make XDAQFecSupervisor 
	make XDAQDcuFilter 
	make XDAQCrateController 
	make XDAQTkConfigurationDb 
	make GUIDebugger  

#	VmeConsoleDebugger

standalone: Generic APIConsoleDebugger APIXMLDebugger 
# PciConsoleDebugger

totem:   Generic Totem

preshower:    Generic PreshowerMake

clean: FecDeviceDriver_clean FecLibFtdi_clean FecUsbDriver_clean  Generic_clean DeviceFactory_clean APIConsoleDebugger_clean XDAQFecSupervisor_clean APIXMLDebugger_clean TestControlLoop_clean GUIDebugger_clean PciConsoleDebugger_clean XDAQDcuFilter_clean XDAQCrateController_clean Totem_clean preshower_clean XDAQTkConfigurationDb_clean VmeConsoleDebugger_clean I2CTemplate_clean DeviceFactoryTemplate_clean	

FecDeviceDriver:
	( cd FecPciDeviceDriver ; $(MAKE) ; cd .. )

FecDeviceDriver_clean:
	( cd FecPciDeviceDriver ; $(MAKE) clean ; cd .. )

FecLibFtdi:
	( cd FecUsbDeviceDriver/libftdi-0.7 ; $(MAKE) clean; $(MAKE) ; cd ../.. )

FecLibFtdi_clean:
	( cd FecUsbDeviceDriver/libftdi-0.7 ; $(MAKE) clean ; cd ../.. )

# KH
# FecUsbDriver:
FecUsbDriver: FecLibFtdi
	( cd FecUsbDeviceDriver ; $(MAKE) ; cd .. )

#libftdi:
#	( cd FecUsbDeviceDriver/libftdi-0.7; $(MAKE) ; cd ../.. )

FecUsbDriver_clean:
	( cd FecUsbDeviceDriver ; $(MAKE) clean ; cd .. )
# KH
#Generic: 
Generic:  
	( cd generic ; $(MAKE) Library=DeviceAccess ; $(MAKE) Library=DeviceDescriptions ; cd .. )
# KH
GenericDeviceAccess: 
#GenericDeviceAccess:   
	( cd generic ; $(MAKE) Library=DeviceAccess ; cd .. )

DeviceFactory:
	( cd generic ; $(MAKE) Library=DeviceDescriptions ; cd .. )

DeviceFactory_clean: 
	( cd generic ; $(MAKE) Library=DeviceDescriptions clean ; cd .. )

Generic_clean:
	( cd generic ; $(MAKE) clean ; cd .. )

XDAQClean: XDAQFecSupervisor_clean XDAQDcuFilter_clean XDAQCrateController_clean XDAQTkConfigurationDb_clean

APIConsoleDebugger:   
	( cd ThirdParty/APIConsoleDebugger ; $(MAKE) ; cd ../.. )

APIConsoleDebugger_clean: 
	( cd ThirdParty/APIConsoleDebugger ; $(MAKE) clean ; cd ../.. )

APIXMLDebugger:   
	( cd ThirdParty/APIXMLDebugger ; $(MAKE) ; cd ../.. )

APIXMLDebugger_clean:
	( cd ThirdParty/APIXMLDebugger ; $(MAKE) clean ; cd ../.. )

XDAQFecSupervisor:
	( cd FecSupervisor ; $(MAKE) DynamicLibrary=XDaqFec ; $(MAKE) DynamicLibrary=FecSupervisor ;  cd .. ) 

XDAQFecSupervisor_clean:
	( cd FecSupervisor ; $(MAKE) clean ; cd .. ) 

# KH
XDAQCrateController: 
#XDAQCrateController: DeviceFactory XDAQFecSupervisor
	( cd CrateController ; $(MAKE) ; cd .. ) 

XDAQCrateController_clean:
	( cd CrateController ; $(MAKE) clean ; cd .. ) 

# KH
XDAQDcuFilter:
#XDAQDcuFilter: DeviceFactory
	( cd DcuFilter ; $(MAKE) ; cd .. ) 

XDAQDcuFilter_clean:
	( cd DcuFilter ; $(MAKE) clean ; cd .. ) 

XDAQTkConfigurationDb:
	( cd TkConfigurationDb ; $(MAKE) ; cd .. ) 

XDAQTkConfigurationDb_clean:
	( cd TkConfigurationDb ; $(MAKE) clean ; cd .. ) 

VmeConsoleDebugger:
	( cd ThirdParty/VMEConsoleDebugger ; $(MAKE) ; cd ../.. )

VmeConsoleDebugger_clean:
	( cd ThirdParty/VMEConsoleDebugger ; $(MAKE) clean ; cd ../.. )

# KH for distcc
PciConsoleDebugger: 
#PciConsoleDebugger:   GenericDeviceAccess
	( cd ThirdParty/PCIConsoleDebugger ; $(MAKE) ; cd ../.. )

PciConsoleDebugger_clean:
	( cd ThirdParty/PCIConsoleDebugger ; $(MAKE) clean ; cd ../.. )

TestControlLoop:
	( cd ThirdParty/TestControlLoop ; $(QMAKE) TestControlLoop.pro ; $(MAKE) ; cd ../.. )

TestControlLoop_clean:
	( cd ThirdParty/TestControlLoop ; $(QMAKE) TestControlLoop.pro ; $(MAKE) clean ; cd ../.. )

# KH
 GUIDebugger: 
#GUIDebugger: DeviceFactory
	( cd ThirdParty/GUIDebugger ; $(QMAKE) GUIDebugger.pro ; $(MAKE) -f GUIDebugger.mak; cd ../.. )

GUIDebugger_clean:
	( cd ThirdParty/GUIDebugger ; $(QMAKE) GUIDebugger.pro ; $(MAKE) clean ; cd ../.. )

DeviceFactoryTemplate:
#DeviceFactoryTemplate: DeviceFactory
	 ( cd ThirdParty/DeviceFactoryTemplate ; $(MAKE) ; cd ../.. )

DeviceFactoryTemplate_clean:
	 ( cd ThirdParty/DeviceFactoryTemplate ; $(MAKE) clean ; cd ../.. )

I2CTemplate:
#I2CTemplate: GenericDeviceAccess
	 ( cd ThirdParty/I2CTemplate ; $(MAKE) ; cd ../.. )

I2CTemplate_clean:
	 ( cd ThirdParty/I2CTemplate ; $(MAKE) clean ; cd ../.. )

Totem:
	 ( cd ThirdParty/Totem ; $(MAKE) ; cd ../.. )

Totem_clean:
	 ( cd ThirdParty/Totem ; $(MAKE) clean ; cd ../.. )

PreshowerMake:
	 ( cd Preshower/generic ; $(MAKE) ; cd ../.. )

preshower_clean:
	 ( cd Preshower/generic ; $(MAKE) clean ; cd ../.. )

install:
	echo "XDAQ_LIB_DEST: $(XDAQ_LIB_DEST)" 
	($(MKDIR) lib ; \
	  if $(CHK_FILE_EXISTS) FecPciDeviceDriver/libfec_glue.a; then $(COPY_FILE) FecPciDeviceDriver/libfec_glue.a $(LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) FecPciDeviceDriver/libfec_glue.so; then $(COPY_FILE) FecPciDeviceDriver/libfec_glue.so $(LIB_DEST); fi; \
          if $(CHK_FILE_EXISTS) FecUsbDeviceDriver/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libfec_usb_glue.a; then $(COPY_FILE) FecUsbDeviceDriver/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libfec_usb_glue.a $(LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) generic/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libDeviceAccess.so; then $(COPY_FILE) generic/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libDeviceAccess.so $(LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) generic/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libDeviceDescriptions.so; then $(COPY_FILE) generic/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libDeviceDescriptions.so $(LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) ThirdParty/APIConsoleDebugger/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libAPIFecVme.a; then $(COPY_FILE) ThirdParty/APIConsoleDebugger/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libAPIFecVme.a $(LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) ThirdParty/DeviceFactoryTemplate/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libAnalysisCommandLineParameterSet.so; then $(COPY_FILE) ThirdParty/DeviceFactoryTemplate/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libAnalysisCommandLineParameterSet.so $(LIB_DEST); fi; \
	  $(MKDIR) $(BIN_DEST) ; \
	  if $(CHK_FILE_EXISTS) ThirdParty/APIConsoleDebugger/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)/ProgramTest.exe; then $(COPY_FILE) ThirdParty/APIConsoleDebugger/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)/ProgramTest.exe $(BIN_DEST); fi; \
	  if $(CHK_FILE_EXISTS) ThirdParty/APIXMLDebugger/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)/FecProfiler.exe; then $(COPY_FILE) ThirdParty/APIXMLDebugger/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)/FecProfiler.exe $(BIN_DEST); fi; \
	  if $(CHK_FILE_EXISTS) ThirdParty/VMEConsoleDebugger/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)/VmeDebugger.exe; then $(COPY_FILE) ThirdParty/VMEConsoleDebugger/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)/VmeDebugger.exe $(BIN_DEST); fi; \
	  if $(CHK_FILE_EXISTS) ThirdParty/PCIConsoleDebugger/PciDebugger.exe; then $(COPY_FILE) ThirdParty/PCIConsoleDebugger/PciDebugger.exe $(BIN_DEST); fi; \
	  if $(CHK_FILE_EXISTS) ThirdParty/GUIDebugger/TestControlLoop/TestControlLoop; then $(COPY_FILE) ThirdParty/GUIDebugger/TestControlLoop/TestControlLoop $(BIN_DEST); fi; \
	  if $(CHK_FILE_EXISTS) ThirdParty/GUIDebugger/src/GUIDebugger; then $(COPY_FILE) ThirdParty/GUIDebugger/src/GUIDebugger $(BIN_DEST); fi; \
	  $(MKDIR) $(XDAQ_LIB_DEST); \
	  if $(CHK_FILE_EXISTS) FecSupervisor/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libXDaqFec.so; then $(COPY_FILE) FecSupervisor/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libXDaqFec.so $(XDAQ_LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) FecSupervisor/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libFecSupervisor.so; then $(COPY_FILE) FecSupervisor/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libFecSupervisor.so $(XDAQ_LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) DcuFilter/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libDcuFilter.so; then $(COPY_FILE) DcuFilter/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libDcuFilter.so $(XDAQ_LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) CrateController/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libCrateController.so ; then $(COPY_FILE) CrateController/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libCrateController.so $(XDAQ_LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) TkConfigurationDb/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libTkConfigurationDb.so ; then $(COPY_FILE) TkConfigurationDb/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libTkConfigurationDb.so $(XDAQ_LIB_DEST); fi; \
	  if $(CHK_FILE_EXISTS) TkConfigurationDb/scripts/*.sh ; then $(COPY_FILE) TkConfigurationDb/scripts/*.sh $(BIN_DEST); fi; \
	  if $(CHK_DIR_EXISTS) "$(CONFIG_DEST)"; then cd config; ls | grep -E "^[^(CVS)]" | xargs $(COPY_FILE) --target-directory=$(CONFIG_DEST) ; cd - ; fi; \
          if $(CHK_FILE_EXISTS) ThirdParty/Totem/generic/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libTotemDeviceAccess.so; then $(COPY_FILE) ThirdParty/Totem/generic/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libTotemDeviceAccess.so $(LIB_DEST); fi; \
          if $(CHK_FILE_EXISTS) ThirdParty/Totem/generic/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libTotemDeviceDescriptions.so; then $(COPY_FILE) ThirdParty/Totem/generic/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/libTotemDeviceDescriptions.so $(LIB_DEST); fi; \
	if [[ "$(INCLUDE_DEST)" != "" ]]; \
	then $(COPY_FILE) $(INCLUDE_FILES) $(INCLUDE_DEST); \
	fi; \
	) 



