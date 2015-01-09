# $Id: Makefile,v 1.6 2009/07/28 15:38:13 aryd Exp $

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
Package=PixelSupervisorConfiguration

#ifdef Library
#Sources= $(Library).cc
#endif
Sources = $(wildcard src/common/*.cc)

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
        $(XDAQ_ROOT)/include/hal \
	$(ENV_CMS_TK_DIAG_ROOT) \
        $(ENV_CMS_TK_DIAG_ROOT)/toolbox \
	$(ENV_CMS_TK_DIAG_ROOT)/tools/xmldiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/diagbag/include \
        $(FECSOFTWARE_ROOT)/generic/include \
        $(FECSOFTWARE_ROOT)/ThirdParty/APIConsoleDebugger/include \
        $(FECSOFT_INCLUDE)

LibraryDirs = 

UserSourcePath = \
	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long $(BUSADAPTERC++FLAG) -std=c++0x
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

Libraries =
ExternalObjects = 

#
# Compile the source files and create a shared library
#
#ifdef Library
#StaticLibrary= PixelSupervisorConfiguration
DynamicLibrary= $(Package)
#endif



ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
