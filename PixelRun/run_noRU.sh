#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
LOGFN=Logs/log.$DTSTAMP
rm -f lastlog
ln -s $LOGFN lastlog
echo $LOGFN
echo
if [ "$1" == "gdb" ]; then
    gdb --args ${XDAQ_ROOT}/bin/xdaq.exe -z pixel -p 1973 -c ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS_DCS.xml
else
    ${XDAQ_ROOT}/bin/xdaq.sh -z pixel -p 1973 -c ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS_DCS.xml 2>&1 | tee $LOGFN
fi

