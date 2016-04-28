#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
LOGFN=Logs/log.$DTSTAMP
rm -f lastlog
ln -s $LOGFN lastlog
echo $LOGFN
echo

gdb=0
dcs=1

while test $# -gt 0
do
    case "$1" in
        --gdb) gdb=1
            ;;
        --nodcs) dcs=0
            ;;
        --*) echo "bad option $1"
            ;;
        *) echo "argument $1"
            ;;
    esac
    shift
done

cmd='-z pixel -p 1973 -c '

if [ "$dcs" == "1" ]; then
    cmd="$cmd ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS_DCS.xml"
else
    cmd="$cmd ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS.xml"
fi

if [ "$gdb" == "0" ]; then
    cmd="${XDAQ_ROOT}/bin/xdaq.sh $cmd"
else
    cmd="gdb --args ${XDAQ_ROOT}/bin/xdaq.exe $cmd"
fi

echo $cmd
$cmd
