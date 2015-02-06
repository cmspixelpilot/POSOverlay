# $Id: Makefile,v 1.2 2009/09/23 12:31:31 joshmt Exp $

#########################################################################
# XDAQ Components for Distributed Data Acquisition                      #
# Copyright (C) 2000-2004, CERN.			                #
# All rights reserved.                                                  #
# Authors: J. Gutleber and L. Orsini					#
#                                                                       #
# For the licensing terms see LICENSE.		                        #
# For the list of contributors see CREDITS.   			        #
#########################################################################

#########################
# Environment Variables #
#########################

# XDAQ_ROOT  =	The root of your xdaq installation; should be /opt/xdaq
# BUILD_HOME = 	The directory that includes pixel.  For example:
#				Pixel in ~/build/pixel -> BUILD_HOME = ~/build
# ROOTSYS	 =	The root of your root install (should contain bin, lib, macros, etc)
# ENV_CMS_TK_DIAG_ROOT = 	The root of your diag system install.  Should include a tools directory.

#Include xdaq make structure
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelFEDMonitor

#Give the framework the sources for compilation
Sources= PixelFEDMonitor.cc 

#Include directories
IncludeDirs = \
        $(BUILD_HOME)/$(Project) \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/soapdiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/xmldiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/diagbag/include \
        $(ENV_CMS_TK_DIAG_ROOT)/toolbox \
        $(ENV_CMS_TK_DIAG_ROOT)

LibraryDirs = 

UserSourcePath = \
	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long -D_FILE_OFFSET_BITS=64 $(BUSADAPTERC++FLAG)
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

DependentLibraryDirs = $(BUILD_HOME)/pixel/lib \
       $(ENV_CMS_TK_DIAG_ROOT)/tools/lib \
       $(XDAQ_ROOT)/lib	

DependentLibraries = TkDiagBagWizard xmldiagappender 

DependentLibraries += 

ifeq ($(VMEDUMMY), yes)
UserCCFlags += -DVMEDUMMY
endif

Libraries=

DynamicLibrary= PixelFEDMonitor

StaticLibrary=


ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic 
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
