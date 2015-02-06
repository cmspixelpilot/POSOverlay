# $Id: Makefile,v 1.15 2009/07/28 14:16:07 aryd Exp $

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
Package = PixelDCSInterface

Sources = PixelDCSDpInterface.cc \
	PixelDCSFSMInterface.cc \
	PixelDCSFSMNodeA4602.cc \
	PixelDCSFSMNodeA4603.cc \
	PixelDCSFSMNode.cc \
	PixelDCSFSMPartition.cc \
	PixelDCStoFEDDpCalibrationRow.cc \
	PixelDCStoFEDDpCalibrationTable.cc \
	PixelDCStoFEDDpConnectionRow.cc \
	PixelDCStoFEDDpFilterRow.cc \
	PixelDCStoFEDDpFilterTable.cc \
	PixelDCStoFEDDpInterface.cc \
	PixelDCStoFEDDpNameRow.cc \
	PixelDCStoFEDDpNameTable.cc \
	PixelDCStoFEDDpValueRow.cc \
	PixelDCStoFEDDpValueTable.cc \
	PixelDCStoTrkFECDpInterface.cc \
	version.cc 

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(XDAQ_ROOT)/$(Project)/$(Package)/include \
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
	$(XDAQ_ROOT)/daq/extern/i2o/include/ \
	$(XDAQ_ROOT)/daq/extern/i2o/include/i2o \
	$(XDAQ_ROOT)/daq/extern/i2o/include/i2o/shared \
	$(XDAQ_ROOT)/daq/xdaq/include \
	$(XDAQ_ROOT)/daq/xdaq/include/solaris \
	$(XDAQ_ROOT)/daq/xdaq/include/linux \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/soapdiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/xmldiagappender/include \
        $(ENV_CMS_TK_DIAG_ROOT)/tools/diagbag/include \
        $(ENV_CMS_TK_DIAG_ROOT)/toolbox \
        $(ENV_CMS_TK_DIAG_ROOT)/ 


LibraryDirs = $(XDAQ_ROOT)/daq/xoap/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/extern/xerces/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/extern/log4cplus/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(XDAQ_ROOT)/daq/toolbox/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/xdata/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/xcept/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/pt/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(XDAQ_ROOT)/daq/extern/mimetic/$(XDAQ_OS)$(XDAQ_PLATFORM)/lib \
	$(ENV_CMS_TK_DIAG_ROOT)/tools/lib 


UserSourcePath = \
	$(XDAQ_ROOT)/daq/xdaq/src/linux/common/

UserCFlags = -O
UserCCFlags = -g -O -Wno-long-long 
UserDynamicLinkFlags =
UserStaticLinkFlags =
UserExecutableLinkFlags =

#DependentLibraryDirs = $(BUILD_HOME)/pixel/PixelUtilities/PixelDCSUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
#	$(BUILD_HOME)/pixel/PixelUtilities/PixelGUIUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
#	$(BUILD_HOME)/pixel/PixelConfigDBInterface/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
#       $(BUILD_HOME)/pixel/PixelUtilities/PixelTestStandUtilities/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
#should be sufficient?
#DependentLibraryDirs = $(BUILD_HOME)/pixel/lib
DependentLibraryDirs =  $(BUILD_HOME)/pixel/lib \
			$(ENV_CMS_TK_DIAG_ROOT)/tools/lib \
                        $(XDAQ_ROOT)/lib	

DependentLibraries = PixelDCSUtilities PixelGUIUtilities PixelConfigDBInterface PixelTestStandUtilities TkDiagBagWizard xmldiagappender 

Libraries = 
ExternalObjects = -L/usr/lib \

#
# Compile the source files and create a shared library
#

DynamicLibrary= PixelDCSInterface

StaticLibrary=

ifdef Executable
Libraries=toolbox xdata xcept xoap xerces-c log4cplus mimetic
Executables= $(Executable).cc
endif

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
