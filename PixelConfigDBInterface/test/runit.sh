#!/bin/bash

export LD_LIBRARY_PATH=../lib/${XDAQ_OS}/${XDAQ_PLATFORM}:${XDAQ_ROOT}/daq/extern/oracle/${XDAQ_OS}${XDAQ_PLATFORM}:${BUILD_HOME}/${Project}/CalibFormats/${ExtDir}/lib/${XDAQ_OS}/${XDAQ_PLATFORM}
export ORACLE_HOME=/opt/xdaq/lib
export TNS_ADMIN=/opt/oracle/app/oracle/product/10.2.0/db_1/network/admin/

bin/${XDAQ_OS}/${XDAQ_PLATFORM}/MyTest.exe
