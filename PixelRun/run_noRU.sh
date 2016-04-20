#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
LOGFN=Logs/log.$DTSTAMP
rm -f lastlog
ln -s $LOGFN lastlog
echo $LOGFN
echo
if [ "$1" == "gdb" ]; then
    gdb --args ${XDAQ_ROOT}/bin/xdaq.exe -p 1973 -e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml -c ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS_DCS.xml -z pixel
else
    ${XDAQ_ROOT}/bin/xdaq.sh -p 1973 -e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml -c ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS_DCS.xml 2>&1 -z pixel | tee $LOGFN
fi

