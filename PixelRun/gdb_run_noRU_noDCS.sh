#!/bin/sh
gdb --args /opt/xdaq/bin/xdaq.exe -p 1973 -e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml -c ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS.xml -z pixel

