#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
LOGFN=Logs/log.$DTSTAMP
rm -f lastlog
ln -s $LOGFN lastlog
echo $LOGFN
echo

gdb=0
dcs=1
xml=0
port=1973

while test $# -gt 0
do
    case "$1" in
        --gdb) gdb=1
            ;;
        --nodcs) dcs=0
            ;;
        --xml)
            shift
            xml=$1
            ;;
        --port)
            shift
            port=$1
            ;;
        --*) echo "bad option $1"
            ;;
        *) echo "argument $1 not parsed"
            ;;
    esac
    shift
done

cmd="-z pixel -p $port -c "

if [ "$xml" == "0" ]; then
    if [ "$dcs" == "1" ]; then
        cmd="$cmd ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS_DCS.xml"
    else
        cmd="$cmd ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS.xml"
    fi
else
    cmd="$cmd $xml"
fi

if [ "$gdb" == "0" ]; then
    cmd="${XDAQ_ROOT}/bin/xdaq.sh $cmd"
else
    cmd="gdb --args ${XDAQ_ROOT}/bin/xdaq.exe $cmd"
fi

echo $cmd
$cmd
