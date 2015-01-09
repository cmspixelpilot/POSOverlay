# $Id: Makefile,v 1.44 2012/02/17 15:42:18 mdunser Exp $

#########################################################################
# XDAQ Components for Distributed Data Acquisition                      #
# Copyright (C) 2000-2004, CERN.			                #
# All rights reserved.                                                  #
# Authors: J. Gutleber and L. Orsini					#
#                                                                       #
# For the licensing terms see LICENSE.		                        #
# For the list of contributors see CREDITS.   			        #
#########################################################################

#########################
# Environment Variables #
#########################

# XDAQ_ROOT  =	The root of your xdaq installation; should be /opt/xdaq
# BUILD_HOME = 	The directory that includes pixel.  For example:
#				Pixel in ~/build/pixel -> BUILD_HOME = ~/build
# ROOTSYS	 =	The root of your root install (should contain bin, lib, macros, etc) // if you use $(shell root-config --incdir) all is fine
# ENV_CMS_TK_DIAG_ROOT = 	The root of your diag system install.  Should include a tools directory.

#Include xdaq make structure
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

include $(ENV_CMS_TK_ONLINE_ROOT)/FecHeader.linux
include $(ENV_CMS_TK_ONLINE_ROOT)/config/FecRules.linux

FECSOFTWARE_ROOT:=${ENV_CMS_TK_ONLINE_ROOT}

Project=pixel
Package=PixelFEDSupervisor

#Give the framework the sources for compilation
Sources= PixelFEDSupervisor.cc 

#Include directories
IncludeDirs = \
        $(BUILD_HOME)/$(Project) \
        $(BUILD_HOME)/pixel/PixelFEDSpySupervisor/include \
        $(BUILD_HOME)/pixel/PixelUtilities/PixelFEDDataTools/include \
	$(XDAQ_ROOT)/include/hal \
        $(XDAQ_ROOT)/include/i2o \
        $(XDAQ_ROOT)/include/i2o/shared \
        $(XDAQ_ROOT)/include/interface/evb \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/soapdiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/xmldiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/diagbag/include \
        $(ENV_CMS_TK_DIAG_ROOT)/toolbox \
        $(ENV_CMS_TK_DIAG_ROOT) \
        $(FECSOFTWARE_ROOT)/generic/include \
        $(FECSOFTWARE_ROOT)/ThirdParty/APIConsoleDebugger/include \
        $(FECSOFT_INCLUDE) \
        $(shell root-config --incdir)
#$(ROOTSYS)/include

LibraryDirs = 

UserSourcePath = \
	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long -D_FILE_OFFSET_BITS=64 $(BUSADAPTERC++FLAG) -std=c++0x
#UserDynamicLinkFlags = $(ROOTLibs)
UserStaticLinkFlags =
UserExecutableLinkFlags =

# These libraries can be platform specific and
# potentially need conditional processing
#

#ROOTLibs = Geom Gpad Gui GuiBld GX11 GX11TTF freetype Graf Graf3d Hist HistPainter Matrix Minuit \
#					 Postscript Tree TreePlayer TreeViewer Rint RGL X3d EG \
#					 AfterImage ASImageGui ASImage Core Cint New Ged GeomPainter

ROOTLibs = AfterImage ASImageGui ASImage \
         Core Cint New Ged GeomPainter Geom Gpad \
         Gui GuiBld GX11 GX11TTF freetype Graf Graf3d Hist HistPainter \
         Matrix Minuit Postscript Tree TreePlayer TreeViewer \
         Rint RGL X3d EG Thread


DependentLibraryDirs = $(BUILD_HOME)/pixel/lib \
       $(ENV_CMS_TK_DIAG_ROOT)/tools/lib \
       $(XDAQ_ROOT)/lib	\
/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
       $(shell root-config --libdir)
#$(ROOTSYS)/lib

DependentLibraries = PixelFEDInterface TkDiagBagWizard xmldiagappender PixelCalibrations \
					 PixelSupervisorConfiguration PixelI2O PixelTestStandUtilities SiPixelObjects \
					 PixelFEDDataTools PixelSharedDataTools PixelGUIUtilities occi ociei ocijdbc11 GenericHAL \
					 HALUtilities CAENLinuxBusAdapter CAENVME DummyBusAdapters Pixelb2inUtilities

DependentLibraries += $(ROOTLibs)

ifeq ($(VMEDUMMY), yes)
UserCCFlags += -DVMEDUMMY
endif

Libraries=

DynamicLibrary= PixelFEDSupervisor

StaticLibrary=


ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic 
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
