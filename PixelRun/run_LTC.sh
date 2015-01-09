#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
echo $DTSTAMP
${XDAQ_ROOT}/bin/xdaq.sh -p 1974 -e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml -c ${BUILD_HOME}/pixel/XDAQConfiguration/ConfigurationP5_LTC.xml 2>&1 -z cms.pixel | tee logLTC.$DTSTAMP.$HOSTNAME

