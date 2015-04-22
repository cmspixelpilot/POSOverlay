# $Id: Makefile,v 1.46 2012/06/16 14:13:19 mdunser Exp $

#########################################################################
# XDAQ Components for Distributed Data Acquisition                      #
# Copyright (C) 2000-2004, CERN.			                #
# All rights reserved.                                                  #
# Authors: J. Gutleber and L. Orsini                                    #
#          modified by ryd for pixels					#
#                                                                       #
# For the licensing terms see LICENSE.		                        #
# For the list of contributors see CREDITS.   			        #
#########################################################################

##
#
# Project level Makefile
#
##


include $(XDAQ_ROOT)/config/mfAutoconf.rules

Project=pixel

include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)


ifeq ($(Set), pixel)
Packages=\
	CalibFormats/SiPixelObjects \
	PixelUtilities/PixelGUIUtilities \
	PixelUtilities/PixelI2OUtilities \
	PixelUtilities/PixelFEDDataTools \
	PixelConfigDBInterface \
	PixelUtilities/PixelTKFECDataTools \
	PixelUtilities/PixelSharedDataTools \
	PixelUtilities/PixelTestStandUtilities \
        PixelUtilities/PixelXmlUtilities \
	PixelUtilities/PixelRootUtilities \
	PixelUtilities/PixelDCSUtilities \
	PixelUtilities/Pixelb2inUtilities \
	PixelUtilities/PixelJobControlUtilities \
	PixelFEDInterface \
	PixelFECInterface \
	PixelSupervisorConfiguration \
	PixelCalibrations \
	PixelFEDSupervisor \
        PixelMonitor   \
        PixelFEDMonitor \
	PixelFECSupervisor \
	PixelTKFECSupervisor \
	PixelSupervisor \
	PixelDCSInterface \
	PixelHistoViewer \
	PixelAnalysisTools \
	PixelConfigDBInterface/test \
	PixelTCDSSupervisor
endif

#	PixelConfigDBGUI \


#	RUBuilderExample/RUBController \
#	RUBuilderExample/EventGenerator \
#	RUBuilderExample/FilterUnit \

install:
	mkdir -p lib
	mkdir -p bin
	ln -s -f ../CalibFormats/SiPixelObjects/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libSiPixelObjects.so lib/.
	ln -s -f ../PixelCalibrations/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelCalibrations.so lib/.
	ln -s -f ../PixelConfigDBInterface/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelConfigDBInterface.so lib/.
	ln -s -f ../PixelConfigDBInterface/test/bin/${XDAQ_OS}/${XDAQ_PLATFORM}/PixelConfigDBCmd.exe bin/.
	ln -s -f ../PixelFECInterface/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelFECInterface.so lib/.
	ln -s -f ../PixelFECSupervisor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelFECSupervisor.so lib/.
	ln -s -f ../PixelFEDInterface/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelFEDInterface.so lib/.
	ln -s -f ../PixelFEDSupervisor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelFEDSupervisor.so lib/.
	ln -s -f ../PixelFEDMonitor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelFEDMonitor.so lib/.
	ln -s -f ../PixelMonitor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelMonitor.so lib/.
	ln -s -f ../PixelSupervisor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelSupervisor.so lib/.
	ln -s -f ../PixelSupervisorConfiguration/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelSupervisorConfiguration.so lib/.
	ln -s -f ../PixelTKFECSupervisor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelTKFECSupervisor.so lib/.
	ln -s -f ../PixelUtilities/PixelFEDDataTools/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelFEDDataTools.so lib/.
	ln -s -f ../PixelUtilities/PixelGUIUtilities/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelGUIUtilities.so lib/.
	ln -s -f ../PixelUtilities/PixelI2OUtilities/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelI2O.so lib/.
	ln -s -f ../PixelUtilities/PixelTestStandUtilities/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelTestStandUtilities.so lib/.
	ln -s -f ../PixelUtilities/PixelDCSUtilities/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelDCSUtilities.so lib/.
	ln -s -f ../PixelUtilities/Pixelb2inUtilities/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelb2inUtilities.so lib/.
	ln -s -f ../PixelUtilities/PixelRootUtilities/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelRootUtilities.so lib/.
	ln -s -f ../PixelUtilities/PixelTKFECDataTools/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelTKFECDataTools.so lib/.
	ln -s -f ../PixelUtilities/PixelSharedDataTools/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelSharedDataTools.so lib/.
	ln -s -f ../PixelUtilities/PixelXmlUtilities/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelXmlUtilities.so lib/.
	ln -s -f ../PixelUtilities/PixelJobControlUtilities/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelJobControlUtilities.so lib/.
	ln -s -f ../PixelDCSInterface/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelDCSInterface.so lib/.
	ln -s -f ../PixelAnalysisTools/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelAnalysisTools.so lib/.
	ln -s -f ../PixelHistoViewer/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelHistoViewer.so lib/.
	ln -s -f ../PixelConfigDBGUI/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelConfigDBGUI.so lib/.
	ln -s -f ../PixelTCDSSupervisor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libPixelTCDSSupervisor.so lib/.
#	ln -s -f ../PixelMonitor/curlpp-0.6.1/curlpp/.libs/libcurlpp.so.0 lib/.


include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
