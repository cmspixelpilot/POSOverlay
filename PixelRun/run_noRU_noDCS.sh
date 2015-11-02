#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
LOGFN=Logs/log.$DTSTAMP
rm -f lastlog
ln -s $LOGFN lastlog
echo $LOGFN
echo
#echo JMT turned off INFO level messages because the TTCciSupervisor is way too chatty...
#echo
${XDAQ_ROOT}/bin/xdaq.sh -p 1973 -e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml -c ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS.xml 2>&1 -z pixel | tee $LOGFN
