#!/bin/sh
#DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
#echo $DTSTAMP
timestamp=`echo $1$2$3_$4_$5 | sed 's/:/-/g'`
if [ ! -d /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp ]
then 
  mkdir /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp
fi

#test if any log files exist in /tmp
ls -1 /tmp/xdaqjcPID*.log >& /dev/null

if [ "$?" == "0" ]
then
## find which log file is which
PIXSUP=`grep -c "PixelSupervisor::Initialize" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
DCSINT=`grep -c "PixelDCSFSMInterface::Initialize" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FEC1=`grep -c "PixelFECSupervisor.instance(1)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FEC2=`grep -c "PixelFECSupervisor.instance(2)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
TKFEC1=`grep -c "PixelTKFECSupervisor.instance(1)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
TKFEC2=`grep -c "PixelTKFECSupervisor.instance(2)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FED1=`grep -c "PixelFEDSupervisor.instance(1)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FED2=`grep -c "PixelFEDSupervisor.instance(2)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FED3=`grep -c "PixelFEDSupervisor.instance(3)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`

#move and rename log files
if [ ! -z $PIXSUP ]
then
mv --backup=numbered $PIXSUP /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelSupervisor.log
fi

if [ ! -z $DCSINT ]
then
mv --backup=numbered $DCSINT /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelDCSFSMInterface.log
fi

if [ ! -z $FEC1 ]
then
mv --backup=numbered $FEC1 /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelFECSupervisor_1.log
fi

if [ ! -z $FEC2 ]
then
mv --backup=numbered $FEC2 /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelFECSupervisor_2.log
fi

if [ ! -z $TKFEC1 ]
then
mv --backup=numbered $TKFEC1 /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelTKFECSupervisor_1.log
fi

if [ ! -z $TKFEC2 ]
then
mv --backup=numbered $TKFEC2 /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelTKFECSupervisor_2.log
fi

if [ ! -z $FED1 ]
then
mv --backup=numbered $FED1 /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelFEDSupervisor_1.log
fi

if [ ! -z $FED2 ]
then
mv --backup=numbered $FED2 /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelFEDSupervisor_2.log
fi

if [ ! -z $FED3 ]
then
mv --backup=numbered $FED3 /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp/PixelFEDSupervisor_3.log
fi
fi

#catch any unrenamed logs
ls -1 /tmp/xdaqjcPID*.log >& /dev/null
if [ "$?" == "0" ]
then
mv /tmp/xdaqjcPID*.log /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp
fi

#find if xml files exist in /tmp
ls -1 /tmp/errorsLogFile*.xml >& /dev/null
if [ "$?" == "0" ]
then
#diagSystem xml files
mv /tmp/errorsLogFile*.xml /nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs/Log_$timestamp
fi
