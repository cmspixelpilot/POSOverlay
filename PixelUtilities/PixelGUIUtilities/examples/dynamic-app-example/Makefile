# $Id: Makefile,v 1.1 2007/08/19 18:20:18 hollings Exp $

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

Project=pixel/PixelUtilities/PixelGUIUtilities
Package=examples/dynamic-app-example

Sources=MyDynamicApp.cpp AnotherGUI.cpp AGUI.cpp

IncludeDirs = \
	$(XDAQ_ROOT)/$(Project) \
    $(XDAQ_ROOT)/$(Project)/$(Package)/PixelGUIUtilities/examples/dynamic-app-example/include \
    $(XDAQ_ROOT)/daq/hal/generic/include \
    $(XDAQ_ROOT)/daq/hal/generic/include/$(XDAQ_OS) \
    $(XDAQ_ROOT)/daq/hal/busAdapter/include \
    /usr/local/CAEN/CAEN-VME/Linux/include \
	$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
    $(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/include/xercesc \
	$(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/extern/cgicc/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/toolbox/include \
	$(XDAQ_ROOT)/daq/toolbox/include/$(XDAQ_OS) \
	$(XDAQ_ROOT)/daq/toolbox/include/solaris \
	$(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/xoap/include \
	$(XDAQ_ROOT)/daq/xdata/include \
	$(XDAQ_ROOT)/daq/pt/include \
	$(XDAQ_ROOT)/daq/xcept/include \
	$(XDAQ_ROOT)/daq/xgi/include \
	$(XDAQ_ROOT)/daq/ \
    $(XDAQ_ROOT)/daq/i2o/include/ \
    $(XDAQ_ROOT)/daq/i2o/utils/include/ \
    $(XDAQ_ROOT)/daq/i2o/include/i2o \
    $(XDAQ_ROOT)/daq/i2o/include/i2o/shared \
    $(XDAQ_ROOT)/daq/extern/i2o/include/ \
    $(XDAQ_ROOT)/daq/extern/i2o/include/i2o \
    $(XDAQ_ROOT)/daq/extern/i2o/include/i2o/shared \
	$(XDAQ_ROOT)/daq/xdaq/include \
	$(XDAQ_ROOT)/daq/xdaq/include/solaris \
	$(XDAQ_ROOT)/daq/xdaq/include/linux \
	$(XDAQ_ROOT)/daq/interface/shared/include \
    $(XDAQ_ROOT)/daq/interface/evb/include  \
    $(XDAQ_ROOT)/daq/sentinel/include \
	$(XDAQ_ROOT)/config \
    $(XDAQ_ROOT)/daq/xrelay/include \
	$(XDAQ_ROOT)/pixel/PixelFEDInterface/include \
	$(XDAQ_ROOT)/pixel/PixelUtilities/PixelGUIUtilities/include

LibraryDirs = $(XDAQ_ROOT)/daq/xoap/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/toolbox/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/xdata/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/xcept/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/pt/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/pixel/PixelUtilities/PixelGUIUtilities/lib/linux/x86

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

DependentLibraryDirs = 	$(XDAQ_ROOT)/pixel/PixelFECInterface/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
			$(XDAQ_ROOT)/pixel/PixelConfigDataFormats/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
			$(XDAQ_ROOT)/pixel/PixelFEDInterface/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
			$(XDAQ_ROOT)/daq/toolbox/lib/linux/x86 \
			$(XDAQ_ROOT)/daq/xdata/lib/linux/x86 \
			$(XDAQ_ROOT)/daq/xcept/lib/linux/x86 \
			$(XDAQ_ROOT)/daq/xoap/lib/linux/x86 \
			$(XDAQ_ROOT)/daq/extern/xerces/linuxx86/lib \
			$(XDAQ_ROOT)/daq/extern/log4cplus/linuxx86/lib \
			$(XDAQ_ROOT)/daq/extern/mimetic/linuxx86/lib \
			$(XDAQ_ROOT)/pixel/PixelUtilities/PixelGUIUtilities/lib/linux/x86
#DependentLibraries = toolbox xdata xcept xoap xerces-c log4cplus mimetic
DependentLibraries = PixelGUIUtilities

#Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
ExternalObjects =
	



#
# Compile the source files and create a shared library
#

DynamicLibrary=DynamicAppTest


StaticLibrary=

ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
