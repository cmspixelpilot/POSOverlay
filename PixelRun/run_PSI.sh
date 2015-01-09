#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
echo $DTSTAMP
${XDAQ_ROOT}/daq/xdaq/bin/linux/x86/xdaq.sh -p 1973 -c ${XDAQ_ROOT}/pixel/XDAQConfiguration/ConfigurationPSI.xml | tee log.$DTSTAMP
