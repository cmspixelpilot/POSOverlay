#!/bin/csh

source ../setup.csh

#setenv XDAQ_ROOT /opt/xdaq/
#setenv XDAQ_BASE ${XDAQ_ROOT}

#setenv XDAQ_OS linux
#setenv XDAQ_PLATFORM x86
#setenv XERCESROOT ${XDAQ_BASE}

# setenv ENV_CMS_TK_FEC_ROOT ${FECSOFTWARE_ROOT}
# setenv ENV_CMS_TK_CAEN_ROOT $CAENVME:h:h:h
# setenv ENV_CMS_TK_HAL_ROOT /opt/xdaq/include/hal



# setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${XERCESROOT}/lib:${XDAQ_ROOT}/lib:${ENV_CMS_TK_HAL_ROOT}/lib/linux/x86:${ENV_CMS_TK_HAL_ROOT}/busAdapter/lib/linux/x86:${XDAQ_ROOT}/daq/xcept/lib/linux/x86:

setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${XDAQ_ROOT}/lib

setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${PIXEL_INCLUDE}/lib

