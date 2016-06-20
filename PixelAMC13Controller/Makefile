# $Id: Makefile,v 1.21 2012/01/20 19:49:49 kreis Exp $

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
Package=PixelAMC13Controller

Sources=PixelAMC13Controller.cc SimpleSOAPSender.cc

CACTUS_ROOT = /opt/cactus

IncludeDirs = \
        $(BUILD_HOME)/$(Project) \
        $(CACTUS_ROOT)/include

LibraryDirs = 

UserSourcePath = 

UserCFlags = -O0
UserCCFlags = -std=c++0x -g
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

DependentLibraryDirs =  $(BUILD_HOME)/pixel/lib \
                        $(XDAQ_ROOT)/lib	

DependentLibraries = PixeluTCAUtilities


Libraries = 

#
# Compile the source files and create a shared library
#

DynamicLibrary= PixelAMC13Controller


StaticLibrary=

ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
