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
Package=PixelUtilities/PixeluTCAUtilities/test

Library=
Executables=$(wildcard src/common/*.cc)

CACTUS_ROOT=/opt/cactus

IncludeDirs = \
	$(BUILD_HOME)/${Project} \
	$(CACTUS_ROOT)/include

LibraryDirs = \
	$(BUILD_HOME)/${Project}/$(Package)/../lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(CACTUS_ROOT)/lib

UserCFlags =
UserCCFlags = -g
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

Libraries = PixeluTCAUtilities

DynamicLibrary=
StaticLibrary=

include $(XDAQ_ROOT)/config/Makefile.rules

clean:
	rm -rf *~ src/common/*~ core*

include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
