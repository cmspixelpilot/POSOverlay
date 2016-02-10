#!/bin/csh
source ../setup.csh

setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${XDAQ_ROOT}/lib:${PIXEL_INCLUDE}/lib


if $HOST == pc4226 then
  setenv INITFILE data/pc4226.ini
else if $HOST == pc5524 then
  setenv INITFILE data/pc5524.ini
else
  # generic, unlikely to work
  setenv INITFILE data/d.ini
endif

