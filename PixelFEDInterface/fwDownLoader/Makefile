# $Id: Makefile,v 1.2 2007/09/19 18:23:33 wjohns Exp $

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
Package=PixelFEDInterface/fwDownLoader

Sources = $(wildcard src/common/*.cc) 

ifeq ($(Set), fed_jam1)
Sources = $(wildcard test/*.cpp)
Executable = test/fed_jam1
endif

IncludeDirs = \
        $(BUILD_HOME)/$(Project)/$(Package)/include \
        $(XDAQ_ROOT)/include/hal \
        $(XDAQ_ROOT)/include
#        /usr/local/CAEN/CAEN-VME/Linux/include 

LibraryDirs =  \
	$(XDAQ_ROOT)/lib/ \
	$(BUILD_HOME)/$(Project)/$(Package)/lib/linux/x86 \
	$(BUILD_HOME)/$(Project)/$(testDep)/lib/linux/x86 \
	/usr/lib
	

#UserSourcePath = \
#	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long
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
StaticLibrary=fwDownLoader 
#DynamicLibrary= 
#endif



ifdef Executable
Libraries=CAENVME 
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
