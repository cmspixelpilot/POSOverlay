#!/bin/sh
gdb --args /opt/xdaq/bin/xdaq.exe -l WARN -p 1973 \
-e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml \
-c ${BUILD_HOME}/pixel/XDAQConfiguration/ConfigurationNoRU_TIF.xml -z pixel
