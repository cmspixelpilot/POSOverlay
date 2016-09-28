#!/bin/sh
#DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
#echo $DTSTAMP

#dirbase=/nfshome0/pixelpro/TriDAS/pixel/PixelRun/Logs
dirbase=/pixelscratch/pixelscratch/log0

if [ `hostname` == "vmepc-s2b18-08-01" ]
then
  dirbase=/pixelscratch/pixelscratch/pilot/Logs
fi
echo my hostname is `hostname` and my dirbase is $dirbase

timestamp=`echo $1$2$3_$4_$5 | sed 's/:/-/g'`

if [ ! -d $dirbase/Log_$timestamp ]
then 
  mkdir $dirbase/Log_$timestamp
fi

#test if any log files exist in /tmp
ls -1 /tmp/xdaqjcPID*.log >& /dev/null

if [ "$?" == "0" ]
then
## find which log file is which
PIXSUP=`grep -c "PixelSupervisor::Initialize" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
DCSINT=`grep -c "PixelDCSFSMInterface::Initialize" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FEC1=`grep -c "FEC Crate=1" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FEC2=`grep -c "FEC Crate=2" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
TKFEC1=`grep -c "PixelTKFECSupervisor.instance(1)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
TKFEC2=`grep -c "PixelTKFECSupervisor.instance(2)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
TKFECP=`grep -c "PixelTKFECSupervisor::Initialize" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FED1=`grep -c "FEDID:15" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FED2=`grep -c "FEDID:31" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FED3=`grep -c "FEDID:39" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
FEDP=`grep -c "FEDID:40" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
ICI0=`grep -c "PixeliCISupervisor.instance(0)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
ICI1=`grep -c "PixeliCISupervisor.instance(1)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
ICI2=`grep -c "PixeliCISupervisor.instance(2)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
PI0=`grep -c "PixelPISupervisor.instance(0)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
PI1=`grep -c "PixelPISupervisor.instance(1)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`
PI2=`grep -c "PixelPISupervisor.instance(2)" /tmp/xdaqjcPID*.log | awk -F: '// {if ($2>0) print $1;}' |head -n1`


#move and rename log files
if [ ! -z $PIXSUP ]
then
mv --backup=numbered $PIXSUP $dirbase/Log_$timestamp/PixelSupervisor.log
fi

if [ ! -z $DCSINT ]
then
mv --backup=numbered $DCSINT $dirbase/Log_$timestamp/PixelDCSFSMInterface.log
fi

if [ ! -z $FEC1 ]
then
mv --backup=numbered $FEC1 $dirbase/Log_$timestamp/PixelFECSupervisor_1.log
fi

if [ ! -z $FEC2 ]
then
mv --backup=numbered $FEC2 $dirbase/Log_$timestamp/PixelFECSupervisor_2.log
fi

if [ ! -z $TKFEC1 ]
then
mv --backup=numbered $TKFEC1 $dirbase/Log_$timestamp/PixelTKFECSupervisor_1.log
fi

if [ ! -z $TKFEC2 ]
then
mv --backup=numbered $TKFEC2 $dirbase/Log_$timestamp/PixelTKFECSupervisor_2.log
fi

if [ ! -z $TKFECP ]
then
mv --backup=numbered $TKFECP $dirbase/Log_$timestamp/PixelTKFECSupervisor_P.log
fi

if [ ! -z $FED1 ]
then
mv --backup=numbered $FED1 $dirbase/Log_$timestamp/PixelFEDSupervisor_1.log
fi

if [ ! -z $FED2 ]
then
mv --backup=numbered $FED2 $dirbase/Log_$timestamp/PixelFEDSupervisor_2.log
fi

if [ ! -z $FED3 ]
then
mv --backup=numbered $FED3 $dirbase/Log_$timestamp/PixelFEDSupervisor_3.log
fi

if [ ! -z $FEDP ]
then
mv --backup=numbered $FEDP $dirbase/Log_$timestamp/PixelFEDSupervisor_P.log
fi

if [ ! -z $ICI0 ]
then
mv --backup=numbered $ICI0 $dirbase/Log_$timestamp/PixeliCISupervisor_0.log
fi

if [ ! -z $ICI1 ]
then
mv --backup=numbered $ICI1 $dirbase/Log_$timestamp/PixeliCISupervisor_1.log
fi

if [ ! -z $ICI2 ]
then
mv --backup=numbered $ICI2 $dirbase/Log_$timestamp/PixeliCISupervisor_2.log
fi

if [ ! -z $PI0 ]
then
mv --backup=numbered $PI0 $dirbase/Log_$timestamp/PixelPISupervisor_0.log
fi

if [ ! -z $PI1 ]
then
mv --backup=numbered $PI1 $dirbase/Log_$timestamp/PixelPISupervisor_1.log
fi

if [ ! -z $PI2 ]
then
mv --backup=numbered $PI2 $dirbase/Log_$timestamp/PixelPISupervisor_2.log
fi
fi

#catch any unrenamed logs
ls -1 /tmp/xdaqjcPID*.log >& /dev/null
if [ "$?" == "0" ]
then
mv /tmp/xdaqjcPID*.log $dirbase/Log_$timestamp
fi

#find if xml files exist in /tmp
ls -1 /tmp/errorsLogFile*.xml >& /dev/null
if [ "$?" == "0" ]
then
#diagSystem xml files
mv /tmp/errorsLogFile*.xml $dirbase/Log_$timestamp
fi
