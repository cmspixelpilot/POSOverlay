# $Id: Makefile,v 1.10 2009/10/08 19:07:20 wjohns Exp $

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
Package=PixelFEDInterface

CACTUS_ROOT = /opt/cactus

Sources = $(wildcard src/common/*.cc)


IncludeDirs = \
        $(BUILD_HOME)/$(Project) \
        $(CACTUS_ROOT)/include \
        $(XDAQ_ROOT)/include/hal \
        $(XDAQ_ROOT)/include
#        /usr/local/CAEN/CAEN-VME/Linux/include 

LibraryDirs = 

#UserSourcePath = \
#	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -std=c++0x -g -O -Wno-long-long -DLINUX
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

DependentLibraryDirs =  $(BUILD_HOME)/pixel/lib
DependentLibraries = PixeluTCAUtilities

# These libraries can be platform specific and
# potentially need conditional processing
#

Libraries =
ExternalObjects = 

#
# Compile the source files and create a shared library
#
#ifdef Library
#StaticLibrary= PixelFEDInterface
DynamicLibrary= PixelFEDInterface
#endif



ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
