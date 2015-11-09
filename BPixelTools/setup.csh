#!/bin/tcsh
# common initializations for BPIXELTOOLS
setenv PIXEL_HOME /nfshome0/pixeldev
setenv PORT_BASE 2000
setenv CAEN_PS $PIXEL_HOME/CAEN_PS_cpp

setenv TRIDAS_ROOT ${PIXEL_HOME}/TriDAS
setenv XDAQ_ROOT /opt/xdaq

setenv INTERFACE vmecaenpci
setenv BPIXELTOOLS ${TRIDAS_ROOT}/BPixelTools

setenv BUILD_HOME ${TRIDAS_ROOT}/pixel
# setenv BUILD_HOME ${TRIDAS_ROOT}/pixel_2_8_3

setenv PIXEL_INCLUDE ${BUILD_HOME}
setenv PIXELLIB ${BUILD_HOME}/lib

setenv PYTHONPATH ${PYTHONPATH}:${BPIXELTOOLS}/tools/python
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${BPIXELTOOLS}/tools/lib
