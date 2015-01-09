# $Id: Makefile,v 1.3 2012/01/21 17:36:10 kreis Exp $

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
Package = PixelCalibrations/macros

Sources = $(wildcard src/common/*.cc)

Executable = offlinePixelTemperatureCalibrationLastDAC

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(XDAQ_ROOT)/include \
  $(shell root-config --incdir)
#$(ROOTSYS)/include 

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

DependentLibraryDirs = 

DependentLibraries = 

DynamicLibrary = PixelCalibrations_macros

ifdef Executable
LibraryDirs += $(XDAQ_ROOT)/lib \
  $(shell root-config --libdir)
#$(ROOTSYS)/lib
Libraries += toolbox xdata xcept xoap xerces-c log4cplus mimetic config
Libraries += uuid
Libraries += asyncresolv
ROOTLIBS = $(shell $(ROOTSYS)/bin/root-config --libs)
UserExecutableLinkFlags += $(ROOTLIBS)
UserExecutableLinkFlags += -lMinuit
Executables = $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
