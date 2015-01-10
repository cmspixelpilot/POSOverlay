# $Id: Makefile,v 1.11 2012/02/17 15:42:19 mdunser Exp $

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

Project = pixel
Package = PixelUtilities/PixelDCSUtilities
PackageName = PixelDCSUtilities

Sources=$(wildcard src/common/*.cc)

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(XDAQ_ROOT)/include 

LibraryDirs = 

UserSourcePath = \
	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long 
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

Libraries = 
ExternalObjects = -L/usr/lib \

#
# Compile the source files and create a shared library
#

DependentLibraryDirs = $(BUILD_HOME)/pixel/PixelUtilities/PixelGUIUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
			$(BUILD_HOME)/pixel/PixelUtilities/PixelTKFECDataTools/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
			$(BUILD_HOME)/pixel/PixelConfigDBInterface/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
			$(BUILD_HOME)/pixel/CalibFormats/SiPixelObjects/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
			$(BUILD_HOME)/pixel/lib \
			$(XDAQ_ROOT)/lib

DependentLibraries = PixelGUIUtilities PixelTKFECDataTools PixelConfigDBInterface SiPixelObjects tstoreclient

DynamicLibrary= PixelDCSUtilities

ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
