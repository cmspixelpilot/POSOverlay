# $Id: Makefile,v 1.8 2012/02/17 15:42:19 mdunser Exp $

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
Package=PixelHistoViewer

Library=PixelHistoViewer
Sources= $(Library).cc JsonList.cc PixelHistoPicGen.cc PixelHistoPicGenDetectorNavigator.cc
	

IncludeDirs = \
	$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)/$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)/$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/extern/cgicc/$(XDAQ_OS)/$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/toolbox/include \
	$(XDAQ_ROOT)/daq/toolbox/include/$(XDAQ_OS) \
	$(XDAQ_ROOT)/daq/toolbox/include/solaris \
	$(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)/$(XDAQ_PLATFORM)/include \
	$(XDAQ_ROOT)/daq/xoap/include \
	$(XDAQ_ROOT)/daq/xdata/include \
	$(XDAQ_ROOT)/daq/pt/include \
	$(XDAQ_ROOT)/daq/xcept/include \
	$(XDAQ_ROOT)/daq/xgi/include \
	$(XDAQ_ROOT)/daq/ \
	$(XDAQ_ROOT)/daq/extern/i2o/include/ \
	$(XDAQ_ROOT)/daq/extern/i2o/include/i2o \
	$(XDAQ_ROOT)/daq/extern/i2o/include/i2o/shared \
	$(XDAQ_ROOT)/daq/xdaq/include \
	$(XDAQ_ROOT)/daq/xdaq/include/solaris \
	$(XDAQ_ROOT)/daq/xdaq/include/linux \
	$(BUILD_HOME)/$(Project) \
  $(shell root-config --incdir)
#$(ROOTSYS)/include
	
#LibraryDirs = $(XDAQ_ROOT)/daq/xoap/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/extern/asyncresolv/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/toolbox/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/xdata/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/xcept/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/pt/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib


#ROOTLibs =  -L$(ROOTSYS)/lib \
	    -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -pthread -lm -ldl
	 
UserCFlags =
UserCCFlags = -g
#UserDynamicLinkFlags = $(ROOTLibs)
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

Libraries = 

DependentLibraryDirs = \
	               $(BUILD_HOME)/$(Project)/PixelUtilities/PixelFEDDataTools/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	               $(BUILD_HOME)/$(Project)/PixelUtilities/PixelRootUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	               $(BUILD_HOME)/$(Project)/PixelUtilities/PixelXmlUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	               $(BUILD_HOME)/$(Project)/CalibFormats/SiPixelObjects/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
                 $(shell root-config --libdir)
#$(ROOTSYS)/lib

DependentLibraries = PixelRootUtilities PixelXmlUtilities PixelFEDDataTools SiPixelObjects\
                     Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread pthread m dl
#	             Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics pthread m dl Thread

#
# Compile the source files and create a shared library
#
DynamicLibrary= $(Library)

StaticLibrary=

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules

clean:
	rm -rf *~ Configuration/*~ src/common/*~ include/*~ core* lib src/$(XDAQ_OS) images/generated/*
