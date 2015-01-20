# $Id: Makefile,v 1.13 2009/07/28 15:43:52 aryd Exp $

#########################################################################
# XDAQ Components for Distributed Data Acquisition                      #
# Copyright (C) 2000-2004, CERN.			                #
# All rights reserved.                                                  #
# Authors: J. Gutleber and L. Orsini					#
#                                                                       #
# For the licensing terms see LICENSE.		                        #
# For the list of contributors see CREDITS.   			        #
#########################################################################

##
#
# This is the example Makefile
#
# make Library=ExampleName
#
##

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelTTCSupervisor

#ifdef Library
Sources= PixelTTCSupervisor.cc \
	HTMLFieldElement.cc \
	HTMLTable.cc \
	TTCciXdaqUtils.cc \
	TTCXDAQBase.cc 
#endif


IncludeDirs = \
        $(BUILD_HOME)/$(Project) \
        $(XDAQ_ROOT)/include/hal \
	$(ENV_TTCSOFTWARE_ROOT)/common/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/soapdiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/xmldiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/diagbag/include \
        $(ENV_CMS_TK_DIAG_ROOT)/toolbox \
        $(ENV_CMS_TK_DIAG_ROOT)

LibraryDirs = 

UserSourcePath = \
	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

ifeq ($(VMEDUMMY), yes)
UserCCFlags += -DVMEDUMMY
BusAdapter:=VMEDummyBusAdapter
else
BusAdapter:=CAENLinuxBusAdapter
endif

DependentLibraryDirs = $(BUILD_HOME)/pixel/PixelUtilities/PixelTestStandUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/pixel/CalibFormats/SiPixelObjects/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        $(BUILD_HOME)/pixel/PixelConfigDBInterface/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/lib 

DependentLibraries = PixelTestStandUtilities SiPixelObjects PixelConfigDBInterface occi nnz10 clntsh TkDiagBagWizard xmldiagappender


Libraries =
#ExternalObjects = 
ExternalObjects = -L$(XDAQ_ROOT)/lib -L$(XDAQ_ROOT)/daq/hal/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)  -L$(ENV_TTCSOFTWARE_ROOT)/common \
	-l$(BusAdapter) -lGenericHAL -lHALUtilities -L/usr/lib -lCAENVME \
	-L$(XDAQ_ROOT)/TTCSoftware/common -lTTC 



UserCCFlags += -DIMAGE_DIR=$(ENV_TTCSOFTWARE_ROOT)/images

# -lCAENVME is needed when using the hardware!

#
# Compile the source files and create a shared library
#
#ifdef Library
DynamicLibrary= PixelTTCSupervisor
#endif

StaticLibrary=



ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
