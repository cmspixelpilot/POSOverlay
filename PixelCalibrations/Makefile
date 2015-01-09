# $Id: Makefile,v 1.19 2012/02/17 15:42:15 mdunser Exp $

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

include $(ENV_CMS_TK_ONLINE_ROOT)/FecHeader.linux
include $(ENV_CMS_TK_ONLINE_ROOT)/config/FecRules.linux

FECSOFTWARE_ROOT:=${ENV_CMS_TK_ONLINE_ROOT}

Project=pixel
Package=PixelCalibrations

#ifdef Library
#Sources= $(Library).cc
#endif
Sources = $(wildcard src/common/*.cc)

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
        $(XDAQ_ROOT)/include/hal \
        $(ENV_CMS_TK_DIAG_ROOT) \
	$(ENV_CMS_TK_DIAG_ROOT)/tools/xmldiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/diagbag/include \
        $(shell root-config --incdir) \
        $(FECSOFTWARE_ROOT)/generic/include \
        $(FECSOFTWARE_ROOT)/ThirdParty/APIConsoleDebugger/include \
        $(FECSOFT_INCLUDE) 
#$(ROOTSYS)/include 

LibraryDirs = 

UserSourcePath = \
	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long ${BUSADAPTERC++FLAG} -std=c++0x
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

DependentLibraryDirs = $(BUILD_HOME)/pixel/PixelSupervisorConfiguration/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/pixel/CalibFormats/SiPixelObjects/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/pixel/PixelConfigDBInterface/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/pixel/PixelUtilities/PixelDCSUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/pixel/PixelUtilities/PixelRootUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/pixel/PixelUtilities/PixelXmlUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/pixel/PixelUtilities/PixelFEDDataTools/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(BUILD_HOME)/pixel/lib \
	$(XDAQ_ROOT)/lib

DependentLibraries = PixelSupervisorConfiguration SiPixelObjects PixelConfigDBInterface tstoreclient PixelDCSUtilities PixelFEDDataTools PixelRootUtilities PixelXmlUtilities

Libraries =
ExternalObjects = -L$(ENV_CMS_TK_ONLINE_ROOT)/lib \
                  -lAPIFecVme -lDeviceAccess

#
# Compile the source files and create a shared library
#
#ifdef Library
DynamicLibrary= PixelCalibrations
#DynamicLibrary= $(Library)
#endif



ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
