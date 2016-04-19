#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
LOGFN=Logs/log.$DTSTAMP
rm -f lastlog
ln -s $LOGFN lastlog
echo $LOGFN
echo
#echo JMT turned off INFO level messages because the TTCciSupervisor is way too chatty...
#echo
if [ "$1" == "gdb" ]; then
    gdb --args ${XDAQ_ROOT}/bin/xdaq.exe -p 1973 -e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml -c ${BUILD_HOME}/pixel/XDAQConfiguration/simple.xml -z pixel
else
    ${XDAQ_ROOT}/bin/xdaq.sh -p 1973 -e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml -c ${BUILD_HOME}/pixel/XDAQConfiguration/simple.xml -z pixel 2>&1 | tee $LOGFN
fi

