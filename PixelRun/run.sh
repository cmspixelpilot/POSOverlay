#!/bin/sh
DTSTAMP=`awk "BEGIN{print strftime(\"%y%b%d_%H%M\");}" `
LOGFN=${POS_LOG_DIR}/log.$DTSTAMP
rm -f lastlog
ln -s $LOGFN lastlog
echo $LOGFN
echo

gdb=0
gdbr=0
dcs=1
xml=0
hostn=$(hostname --long)
port=1973
b2inport=1914
ttcci=0

while test $# -gt 0
do
    case "$1" in
        --gdb) gdb=1
            ;;
        --gdbr) gdbr=1
            ;;
        --nodcs) dcs=0
            ;;
        --ttcci)
            shift
            ttcci=$1
            ;;
        --xml)
            shift
            xml=$1
            ;;
        --hostn)
            shift
            hostn=$1
            ;;
        --port)
            shift
            port=$1
            ;;
        --b2inport)
            shift
            b2inport=$1
            ;;
        --*) echo "bad option $1"
            ;;
        *) echo "argument $1 not parsed"
            ;;
    esac
    shift
done

cmd="-z pixel -p $port -c "

TTCSupervisorApplicationName=PixelAMC13Controller
SimTTC=true
if [ "$ttcci" != "0" ]; then
    TTCSupervisorApplicationName=ttc::TTCciControl
    SimTTC=false
fi

if [ "$xml" == "auto" ]; then
    cat > auto.xml <<EOF
<?xml version='1.0'?>
<xc:Partition xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"  xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" xmlns:xc="http://xdaq.web.cern.ch/xdaq/xsd/2004/XMLConfiguration-30">
	
  <xc:Context id="0" url="http://${hostn}:${port}/">

    <xc:Endpoint protocol="loopback" service="b2in" interface="eth0" hostname="${hostn}" port="${b2inport}" network="xmas"/>

    <xc:Module>${XDAQ_ROOT}/lib/libb2inutils.so</xc:Module>
    <xc:Module>${BUILD_HOME}/pixel/PixelSupervisor/lib/linux/x86_64_slc6/libPixelSupervisor.so</xc:Module>
    <xc:Module>${BUILD_HOME}/pixel/PixelTKFECSupervisor/lib/linux/x86_64_slc6/libPixelTKFECSupervisor.so</xc:Module>
    <xc:Module>${BUILD_HOME}/pixel/PixelFECSupervisor/lib/linux/x86_64_slc6/libPixelFECSupervisor.so</xc:Module>
    <xc:Module>${BUILD_HOME}/pixel/PixelFEDSupervisor/lib/linux/x86_64_slc6/libPixelFEDSupervisor.so</xc:Module>
    <xc:Module>${BUILD_HOME}/pixel/PixelAMC13Controller/lib/linux/x86_64_slc6/libPixelAMC13Controller.so</xc:Module>

    <xc:Application class="PixelSupervisor" id="51" instance="0" network="local" group="daq">
      <properties xmlns="urn:xdaq-application:PixelSupervisor" xsi:type="soapenc:Struct"> 
        <UseTCDS xsi:type="xsd:boolean">false</UseTCDS>
        <UseTTC xsi:type="xsd:boolean">true</UseTTC>
        <TTCSupervisorApplicationName xsi:type="xsd:string">${TTCSupervisorApplicationName}</TTCSupervisorApplicationName>
      </properties>
    </xc:Application>

    <xc:Application class="PixelTKFECSupervisor" id="100" instance="1" network="local" group="daq">
      <properties xmlns="urn:xdaq-application:PixelTKFECSupervisor" xsi:type="soapenc:Struct">
        <readDCU_workloop_usleep xsi:type="xsd:integer">10000000</readDCU_workloop_usleep>
      </properties>
    </xc:Application>

    <xc:Application class="PixelFECSupervisor" id="60" instance="1" network="local" group="daq"/>

    <xc:Application class="PixelFEDSupervisor" id="70" instance="1" network="local" group="daq"/>

    <xc:Application class="PixelAMC13Controller" id="90" instance="0" network="local" group="daq">
      <properties xmlns="urn:xdaq-application:PixelAMC13Controller" xsi:type="soapenc:Struct">
        <DoNothing xsi:type="xsd:boolean">false</DoNothing>
	<UriT1 xsi:type="xsd:string">chtcp-2.0://localhost:10203?target=amc13_T1:50001</UriT1>
	<AddressT1 xsi:type="xsd:string">/opt/cactus/etc/amc13/AMC13XG_T1.xml</AddressT1>
	<UriT2 xsi:type="xsd:string">chtcp-2.0://localhost:10203?target=amc13_T2:50001</UriT2>
	<AddressT2 xsi:type="xsd:string">/opt/cactus/etc/amc13/AMC13XG_T2.xml</AddressT2>
	<Mask xsi:type="xsd:string">1-12</Mask>
	<CalBX xsi:type="xsd:unsignedInt">381</CalBX>
	<L1ADelay xsi:type="xsd:unsignedInt">105</L1ADelay>
        <NewWay xsi:type="xsd:boolean">true</NewWay>
        <VerifyL1A xsi:type="xsd:boolean">true</VerifyL1A>
        <SimTTC xsi:type="xsd:boolean">${SimTTC}</SimTTC>
      </properties>
    </xc:Application>

  </xc:Context>

EOF
    if [ "$ttcci" != "0" ]; then
        cat >> auto.xml <<EOF
  <xc:Context id="1" url="http://${ttcci}:1973/">
    <xc:Endpoint protocol="loopback" service="b2in" interface="eth0" hostname="${ttcci}" port="1914" network="xmas"/>

    <xc:Application class="ttc::TTCciControl" id="90" instance="0" network="local" group="daq">
      <properties xmlns="urn:xdaq-application:ttc::TTCciControl" xsi:type="soapenc:Struct">
        <name xsi:type="xsd:string">Board 1</name>
        <BusAdapter xsi:type="xsd:string">CAENPCI</BusAdapter>
        <Location xsi:type="xsd:integer">2</Location>
        <Configuration xsi:type="xsd:string">[file=/home/fnaltest/TriDAS/Config/ttcciconfig/0/TTCciConfiguration.txt]</Configuration> 
        <group xsi:type="xsd:string">Pixel</group>
        <system xsi:type="xsd:string">CR</system>
        <BTimeCorrection xsi:type="xsd:unsignedInt">100</BTimeCorrection>
        <DelayT2Correction xsi:type="xsd:unsignedInt">3</DelayT2Correction>
        <DisableMonitoring xsi:type="xsd:boolean">true</DisableMonitoring>
      </properties>
    </xc:Application>
    <xc:Module>${XDAQ_ROOT}/lib/libttcttcci.so</xc:Module>  
    <xc:Module>${XDAQ_ROOT}/lib/libttcutils.so</xc:Module>  
  </xc:Context>
EOF
    fi
    echo "</xc:Partition>" >> auto.xml
    cmd="$cmd $(pwd)/auto.xml"
elif [ "$xml" == "0" ]; then
    if [ "$dcs" == "1" ]; then
        cmd="$cmd ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS_DCS.xml"
    else
        cmd="$cmd ${BUILD_HOME}/pixel/XDAQConfiguration/XDAQ_ConfigurationPilotPix_AIO_TCDS.xml"
    fi
else
    cmd="$cmd $xml"
fi

if [ "$gdb" == "1" ]; then
    cmd="gdb --args ${XDAQ_ROOT}/bin/xdaq.exe $cmd"
elif [ "$gdbr" == "1" ]; then
    cmd="gdb --ex run --args ${XDAQ_ROOT}/bin/xdaq.exe $cmd"
else
    cmd="${XDAQ_ROOT}/bin/xdaq.sh $cmd"
fi

cmd="$cmd | tee $LOGFN"
echo $cmd
eval $cmd 
