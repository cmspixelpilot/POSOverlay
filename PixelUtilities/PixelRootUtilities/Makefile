# $Id: Makefile,v 1.5 2012/01/21 17:36:09 kreis Exp $

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
Package=PixelUtilities/PixelRootUtilities
PackageName=PixelRootUtilities

Library=PixelRootUtilities
Sources=$(wildcard src/common/*.cc)
	

IncludeDirs = \
	      ${BUILD_HOME}/${Project} \
	      $(XERCESCROOT)/$(XDAQ_OS)$(XDAQ_PLATFORM)/include\
        $(shell root-config --incdir)
#$(ROOTSYS)/include
	
LibraryDirs = 
	 
UserCFlags =
UserCCFlags = -g
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

Libraries = 

DependentLibraryDirs =

DependentLibraries =

#
# Compile the source files and create a shared library
#
DynamicLibrary= $(Library)

StaticLibrary=


include $(XDAQ_ROOT)/config/Makefile.rules

_all:
	cd test; make; cd ..

clean:
	rm -rf *~ src/common/*~ include/*~

include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
