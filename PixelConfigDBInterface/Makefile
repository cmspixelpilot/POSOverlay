# $Id: Makefile,v 1.12 2010/02/02 11:51:13 joshmt Exp $

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
Package=PixelConfigDBInterface


Sources = $(wildcard src/common/*.cc)

IncludeDirs = \
        $(BUILD_HOME)/$(Project)

LibraryDirs =

UserSourcePath = 

UserCFlags = 
UserCCFlags = -O -Wno-deprecated 
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =


# These libraries can be platform specific and
# potentially need conditional processing
#

Libraries = 
ExternalObjects = 

DependentLibraryDirs = $(XDAQ_ROOT)/lib 
DependentLibraries = occi clntsh

#
# Compile the source files and create a shared library
#
#ifdef Library
#StaticLibrary= $(Package)
DynamicLibrary= $(Package)
#endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
