# $Id: Makefile,v 1.7 2012/05/04 15:58:35 ecklund Exp $

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
# ROOTSYS	 =	The root of your root install (should contain bin, lib, macros, etc) // if you use $(shell root-config --incdir) this becomes all is fine
# ENV_CMS_TK_DIAG_ROOT = 	The root of your diag system install.  Should include a tools directory.

#Include xdaq make structure
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelMonitor

#Give the framework the sources for compilation
Sources= PixelMonitor.cc 

#Include directories
IncludeDirs = \
        $(BUILD_HOME)/$(Project) \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/soapdiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/xmldiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/diagbag/include \
        $(ENV_CMS_TK_DIAG_ROOT)/toolbox \
        $(ENV_CMS_TK_DIAG_ROOT) \
        $(shell root-config --incdir) \
        $(BUILD_HOME)/$(Project)/$(Package) \
#	$(shell curlpp-config --libs)
#	$(ROOTSYS)/include

LibraryDirs = 

UserSourcePath = \
	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long -D_FILE_OFFSET_BITS=64 $(BUSADAPTERC++FLAG)
#UserDynamicLinkFlags = $(ROOTLibs)
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

ROOTLibs = Tree

curlppLibs = curlpp curl  

DependentLibraryDirs = $(BUILD_HOME)/pixel/lib \
       $(ENV_CMS_TK_DIAG_ROOT)/tools/lib \
       $(XDAQ_ROOT)/lib	\
       $(shell root-config --libdir) \
       $(ROOTSYS)/lib 

#K       $(BUILD_HOME)/$(Project)/$(Package)/curlpp-0.7.3/curlpp/.libs

DependentLibraries = TkDiagBagWizard xmldiagappender 

DependentLibraries += $(ROOTLibs) $(curlppLibs)

ifeq ($(VMEDUMMY), yes)
UserCCFlags += -DVMEDUMMY
endif

Libraries=

DynamicLibrary= PixelMonitor

StaticLibrary=


ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic 
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
