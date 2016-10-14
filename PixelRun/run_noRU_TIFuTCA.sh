#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
LOGFN=Logs/log.$DTSTAMP
rm -f lastlog
ln -s $LOGFN lastlog
echo $LOGFN
echo
${XDAQ_ROOT}/bin/xdaq.sh -l WARN -p 1973 -e ${BUILD_HOME}/pixel/XDAQConfiguration/Profile.xml -c ${BUILD_HOME}/pixel/XDAQConfiguration/ConfigurationNoRU_TIFuTCA.xml 2>&1 -z pixel | tee $LOGFN
