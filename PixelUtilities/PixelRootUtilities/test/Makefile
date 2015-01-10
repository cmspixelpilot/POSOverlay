# $Id: Makefile,v 1.6 2012/01/21 17:36:10 kreis Exp $

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
Package=PixelUtilities/PixelRootUtilities/test
ExtDir=SiPixelObjects

Library=
Executables=$(wildcard src/common/*.cc)
	

IncludeDirs = \
	      $(BUILD_HOME)/${Project}/\
        $(shell root-config --incdir) \
	      $(XERCESCROOT)/$(XDAQ_OS)$(XDAQ_PLATFORM)/include
#$(ROOTSYS)/include
	
LibraryDirs = $(BUILD_HOME)/${Project}/$(Package)/../lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	      $(BUILD_HOME)/${Project}/CalibFormats/SiPixelObjects/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)\
	      $(BUILD_HOME)/${Project}/PixelUtilities/PixelXmlUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)\
	      $(BUILD_HOME)/${Project}/PixelUtilities/PixelFEDDataTools/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)\
        $(shell root-config --libdir) \
	      $(XERCESCROOT)/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib
#$(ROOTSYS)/lib

	 
UserCFlags =
UserCCFlags = -g
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

Libraries = PixelFEDDataTools PixelRootUtilities PixelXmlUtilities SiPixelObjects\
	    xerces-c Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics pthread m dl Thread

#DependentLibraryDirs = 	$(BUILD_HOME)/${Project}/$(Package)/../lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

#DependentLibraries = PixelRootUtilities \
#	Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics pthread m dl Thread

#
# Compile the source files and create a shared library
#
DynamicLibrary=

StaticLibrary=

include $(XDAQ_ROOT)/config/Makefile.rules

clean:
	rm -rf *~ src/common/*~ core*

include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
