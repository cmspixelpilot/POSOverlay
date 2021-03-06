# $Id: Makefile,v 1.28 2012/01/21 17:36:15 kreis Exp $

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

include $(ENV_CMS_TK_ONLINE_ROOT)/FecHeader.linux
include $(ENV_CMS_TK_ONLINE_ROOT)/config/FecRules.linux

FECSOFTWARE_ROOT:=${ENV_CMS_TK_ONLINE_ROOT}

Project=pixel
Package=PixelSupervisor


Sources= PixelSupervisor.cc DiagWrapper.cc

CACTUS_ROOT = /opt/cactus

IncludeDirs = \
        $(BUILD_HOME)/$(Project) \
        $(CACTUS_ROOT)/include \
	$(XDAQ_ROOT)/include \
	$(XDAQ_ROOT)/include/hal \
        $(XDAQ_ROOT)/include/xdaq2rc \
        $(FECSOFTWARE_ROOT)/generic/include \
        $(FECSOFTWARE_ROOT)/ThirdParty/APIConsoleDebugger/include \
        ${FECSOFT_INCLUDE}

LibraryDirs = 

UserSourcePath = 

ROOTLibs =  -L$(shell root-config --libdir) \
	 -lAfterImage -lASImageGui -lASImage \
	 -lCore -lCint -lNew -lGed -lGeomPainter -lGeom -lGpad \
	 -lGui -lGuiBld -lGX11 -lGX11TTF -lfreetype -lGraf -lGraf3d -lHist -lHistPainter \
	 -lMatrix -lMinuit -lPostscript -lTree -lTreePlayer -lTreeViewer \
	 -lRint -lRGL -lX3d -lEG -lThread


UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long $(BUSADAPTERC++FLAG) -std=c++0x -DLINUX -Wl,--no-undefined
UserDynamicLinkFlags = $(ROOTLibs)
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#


DependentLibraryDirs = $(BUILD_HOME)/pixel/lib \
		       $(BUILD_HOME)/pixel/CalibFormats/SiPixelObjects/lib/linux/x86 \
		       $(BUILD_HOME)/pixel/PixelSupervisorConfiguration/lib/linux/x86 \
		       $(BUILD_HOME)/pixel/PixelCalibrations/lib/linux/x86 \
		       $(BUILD_HOME)/pixel/PixelConfigDBInterface/lib/linux/x86 \
		       $(BUILD_HOME)/pixel/PixelUtilities/PixelTestStandUtilities/lib/linux/x86 \
		       $(BUILD_HOME)/pixel/PixelUtilities/PixelJobControlUtilities/lib/linux/x86

DependentLibraries = SiPixelObjects PixelSupervisorConfiguration PixelCalibrations PixelConfigDBInterface occi nnz12 clntsh Pixelb2inUtilities PixelTestStandUtilities PixelJobControlUtilities GenericHAL CAENLinuxBusAdapter xdaq2rc

Libraries = 
ExternalObjects = -L$(XDAQ_ROOT)/lib  -L/usr/lib


#
# Compile the source files and create a shared library
#
DynamicLibrary= PixelSupervisor


StaticLibrary=

ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic 
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
