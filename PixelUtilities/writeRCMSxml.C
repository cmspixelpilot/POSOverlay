#include "TString.h"
#include <iostream>
#include <fstream>

/*
update -- found a classic root bug where code is simply not correctly read by CINT.
Now one can and must run this by compiling with ACLIC (be sure to include the "++"!):

root -b -l -q writeRCMSxml.C++
*/


/*
This macro will generate an xml file for creating a new RCMS configuration.

At the moment, all parameters of the configuration are hard-coded into the script.
The overall structure is defined by the structure found in the go() and metaxml()
methods, while the details are mostly in XApp().

The various configuration details are scattered throughout the script.
*/

/*
Changes to move to RPMs (these are now controlled by global flag rpm_):

- undefine BUILD_HOME (set buildhome to "") - done
- PixelDCSFSMInterface xml is now in /opt/xdaq/htdocs/PixelDCSInterface/xml/interface.xml - done
- ConfigurationRootDirectory - done
- ROOTSYS - done 
- LD_LIBRARY_PATH - done
- .so files are now in /opt/xdaq/lib -done
- .set files for DiagSystem (will move to /opt/xdaq/dat/DiagSystem/) -done
- Profile.xml - done FEDProfile.xml - done

Also, we are now using the variable ttc_ in order to make the generated xml for the TTCciControls
substantially more 'minimal' than the for the other applications.
This is to eliminate all reference to libraries and files that don't exist on the TTC machine,
where we do not install our rpms.

UPDATE (27 Aug 2010) -- noticed that the Profile.xml will still be taken from ~pixelpro for the TTC!
This is ok for rpm_=false but should not be this way (someday) for rpm_=true
*/

//configuration options
const TString configName= "Pixels_noDB"; //name of the configuration
const bool rpm_=false;              //rpm mode or not
const bool usePrevSEURec = true; // use preventative SEU recovery, i.e. run SEU code if another subdetector detects one
//if both of these options are false, then will do whole pixels config
const bool fpixonly_=false;         //do fpix only config
const bool bpixonly_=false;          //do bpix only config
//don't set both of the above to true!

//use DB or not?
const bool confSourceIsDB_ = false;


//this is used internally
bool ttc_;

TString JobControlBlock() {
  //getting lazy here
  TString r= "  &lt;xc:Context url=\"http://vmepcs2b18-11.cms:9999\"&gt;\n    &lt;xc:Application class=\"jobcontrol\" id=\"10\" group=\"jc\" network=\"xmas\" /&gt;\n  &lt;/xc:Context&gt;\n  &lt;xc:Context url=\"http://vmepcs2b18-12.cms:9999\"&gt;\n    &lt;xc:Application class=\"jobcontrol\" id=\"10\" group=\"jc\" network=\"xmas\" /&gt;\n  &lt;/xc:Context&gt;\n  &lt;xc:Context url=\"http://vmepcs2b18-13.cms:9999\"&gt;\n    &lt;xc:Application class=\"jobcontrol\" id=\"10\" group=\"jc\" network=\"xmas\" /&gt;\n  &lt;/xc:Context&gt;\n  &lt;xc:Context url=\"http://vmepcs2b18-14.cms:9999\"&gt;\n";
  r+="    &lt;xc:Application class=\"jobcontrol\" id=\"10\" group=\"jc\" network=\"xmas\" /&gt;\n  &lt;/xc:Context&gt;\n  &lt;xc:Context url=\"http://vmepcs2b18-15.cms:9999\"&gt;\n    &lt;xc:Application class=\"jobcontrol\" id=\"10\" group=\"jc\" network=\"xmas\" /&gt;\n  &lt;/xc:Context&gt;\n  &lt;xc:Context url=\"http://vmepcs2b18-16.cms:9999\"&gt;\n    &lt;xc:Application class=\"jobcontrol\" id=\"10\" group=\"jc\" network=\"xmas\" /&gt;\n  &lt;/xc:Context&gt;\n  &lt;xc:Context url=\"http://vmepcs2b16-10.cms:9999\"&gt;\n    &lt;xc:Application class=\"jobcontrol\" id=\"10\" group=\"jc\" network=\"xmas\" /&gt;\n  &lt;/xc:Context&gt;";

  return r;
}

TString RCMS_xdaq7() {
  TString r="  &lt;xc:Context url=\"http://cmsrc-pixel.cms:17001/rcms/services/replycommandreceiver\"&gt;\n    &lt;xc:Application class=\"RCMSStateListener\" id=\"50\" instance=\"0\" network=\"xmas\" /&gt;\n  &lt;/xc:Context&gt;";
  return r;
}

TString RCMS_xdaq10() {
  TString r="  &lt;xc:Context url=\"http://cmsrc-pixel.cms:17001/rcms\"&gt;\n    &lt;xc:Application class=\"RCMSStateListener\" id=\"50\" instance=\"0\" network=\"xmas\" path=\"/services/replycommandreceiver\" /&gt;\n  &lt;/xc:Context&gt;";
  return r;
}

TString RCMS() {
  return RCMS_xdaq10();
}

TString MiscModule(TString path) {
  TString r;
  r.Form("    &lt;xc:Module&gt;%s&lt;/xc:Module&gt;",path.Data());
  return r;
}

TString MiscModuleTTC(TString path) {
  TString r;
  r.Form("&lt;xc:Module&gt;%s&lt;/xc:Module&gt;",path.Data());
  return r;
}

TString getFEDvme(int instance) {
  TString vme="BOGUSINSTANCE";
  if (instance==1) vme= "vmepcs2b18-13.cms";
  else if (instance==2) vme = "vmepcs2b18-14.cms";
  else if (instance==3) vme = "vmepcs2b18-12.cms";
  return vme;
}

TString getFECvme(int instance) {
  TString vme="BOGUSINSTANCE";
  if (instance==1) vme= "vmepcs2b18-16.cms";
  else if (instance==2) vme = "vmepcs2b18-15.cms";
  return vme;
}

TString useDiagSystem(TString what) {
  TString r=  "       &lt;UseDiagSystem xsi:type=\"xsd:string\"&gt;tuned&lt;/UseDiagSystem&gt;\n";
  r +="        &lt;DiagSystemSettings xsi:type=\"xsd:string\"&gt;";

  if (rpm_) {
    r+="/opt/xdaq/dat/DiagSystem/";
  }
  else {
    r+="/nfshome0/pixelpro/XDAQConfigurations/May09/";
  }

  if (what=="supervisor")
    r+="SupervisorConfig.set";
  else if (what=="GED")
    r+= "GEDConfig.set";
  else if (what=="Ajax")
    r+="LogViewer.set";
  else r+="BOGUSWHAT";
  
  r+="&lt;/DiagSystemSettings&gt;\n";
  return r;

}

TString XApp(TString which, TString type="xapp", int instance=0) {

  TString buildhome="";
  if (!rpm_) buildhome="BUILD_HOME=/nfshome0/pixelpro/TriDAS";
  TString homehome="/nfshome0/pixelpro";
  if (rpm_) homehome="/tmp"; //does this work?
  TString rootsys="ROOTSYS=/nfshome0/pixelpro/root";
  if (rpm_) rootsys = "ROOTSYS=/opt/cmssw/root";
  TString ldlibrarypath="/nfshome0/pixelpro/root/lib:/nfshome0/pixelpro/TriDAS/FecSoftwareV3_0/lib:/nfshome0/pixelpro/TriDAS/DiagSystem/tools/lib:/nfshome0/pixelpro/TriDAS/pixel/lib:/opt/xdaq/lib";
  if (rpm_)  ldlibrarypath="/usr/local/CAEN/CAEN-VME/Linux/lib:/opt/cmssw/root/lib:/opt/xdaq/lib";
  if (ttc_) {
    //centrally provided:
    ldlibrarypath = "/opt/xdaq/lib"; 
    rootsys = "";
    buildhome = "";
  }
  TString reconf="RECONFIGURATIONFLAG=ALLOW";

  //the idea is that these defaults should always be overridden if they are going to be used
  TString vme="BOGUS";
  int port=88888888;
  int lid=port;
  TString so=vme;
  int id=port;
  TString network=vme;
  TString group=vme;
  TString properties=vme;
  TString service=vme;
  TString publish="false";
  TString profile="/nfshome0/pixelpro/XDAQConfigurations/XDAQConfiguration/Profile.xml";
  if (rpm_ && !ttc_) profile="/opt/xdaq/htdocs/PixelSupervisor/xml/Profile.xml";

  if (which == "PixelSupervisor") {
    vme = "vmepcs2b18-11.cms";
    port=1973;
    //    if (type=="endpoint") port = 1911; //default
    if (type=="endpoint") port = 1921; //test -- works!

    lid=51;
    so="${BUILD_HOME}/pixel/lib/libPixelSupervisor.so";
    if (rpm_) so="/opt/xdaq/lib/libPixelSupervisor.so";

    instance=0;

    id=0;

    network="xmas";
    group="daq";

    properties = "      &lt;properties xmlns=\"urn:xdaq-application:PixelSupervisor\" xsi:type=\"soapenc:Struct\"&gt;\n        &lt;UseRunInfo xsi:type=\"xsd:boolean\"&gt;true&lt;/UseRunInfo&gt;\n        &lt;DataBaseConnection xsi:type=\"xsd:string\"&gt;jdbc:oracle:thin:@cmsonr1-v:10121/cms_rcms.cern.ch&lt;/DataBaseConnection&gt;\n        &lt;DataBaseUsername xsi:type=\"xsd:string\"&gt;pixelpro&lt;/DataBaseUsername&gt;\n        &lt;RunSequence xsi:type=\"xsd:string\"&gt;cms.pixel.pib&lt;/RunSequence&gt;\n        ";
    properties+=useDiagSystem("supervisor");
    properties+="      &lt;/properties&gt;";

  }
  else if (which == "xplore::Application") {
    vme = getFEDvme(instance);
    port=1973;

    lid=9;
    network="local";
    so="/opt/xdaq/libxplore.so"; //FIXME..dupe info
    TString shortcutsxml = "/nfshome0/pixelpro/XDAQConfigurations/XDAQConfiguration/shortcuts.xml";
    if (rpm_) shortcutsxml = "/opt/xdaq/htdocs/PixelSupervisor/xml/shortcuts.xml";
    properties.Form("      &lt;properties xmlns=\"urn:xdaq-application:xplore::Application\" xsi:type=\"soapenc:Struct\"&gt;\n        &lt;settings xsi:type=\"xsd:string\"&gt;%s&lt;/settings&gt;\n        &lt;republishInterval xsi:type=\"xsd:string\"&gt;3600&lt;/republishInterval&gt;\n      &lt;/properties&gt;",shortcutsxml.Data());
  }
  else if (which=="sentinel::Application") {
    vme = getFEDvme(instance);
    port=1973;

    lid=22;
    network="local";
    group="exception";
    service="sentinel";
    so="/opt/xdaq/lib/libsentinel.so"; //FIXME..dupe info
    properties="      &lt;properties xmlns=\"urn:xdaq-application:Sentinel\" xsi:type=\"soapenc:Struct\"&gt;\n        &lt;useDiscovery xsi:type=\"xsd:boolean\"&gt;true&lt;/useDiscovery&gt;\n        &lt;publish xsi:type=\"xsd:string\"&gt;exception&lt;/publish&gt;\n        &lt;watchdog xsi:type=\"xsd:string\"&gt;PT5S&lt;/watchdog&gt;\n      &lt;/properties&gt;";
  }
  else if (which == "xmas::sensor::Application") {
    vme = getFEDvme(instance);
    port=1973;

    lid=10;
    service="sensor";
    group="pixel-monitor";
    network="local";
    so="/opt/xdaq/lib/libxmassensor.so"; //FIXME..dupe info
    properties =  "      &lt;properties xmlns=\"urn:xdaq-application:xmas::sensor::Application\" xsi:type=\"soapenc:Struct\"&gt;\n        &lt;useDiscovery xsi:type=\"xsd:boolean\" &gt;true&lt;/useDiscovery&gt;\n        &lt;useBroker xsi:type=\"xsd:boolean\" &gt;false&lt;/useBroker&gt;\n        &lt;brokerGroup xsi:type=\"xsd:string\" &gt;statistics&lt;/brokerGroup&gt;\n        &lt;brokerProfile xsi:type=\"xsd:string\" &gt;default&lt;/brokerProfile&gt;\n";
    TString prop_sensor;
    TString autoconfsearchpath="/nfshome0/pixelpro/XDAQConfigurations/XDAQConfiguration";
    if (rpm_) autoconfsearchpath="/opt/xdaq/share/cdaq"; //who knows....
    prop_sensor.Form("        &lt;autoConfigure xsi:type=\"xsd:boolean\" &gt;true&lt;/autoConfigure&gt;\n        &lt;autoConfSearchPath xsi:type=\"xsd:string\"&gt;%s&lt;/autoConfSearchPath&gt;\n            &lt;!-- url xsi:type=\"soapenc:Array\" soapenc:arrayType=\"xsd:ur-type[1]\"&gt;\n            &lt;item xsi:type=\"xsd:string\" soapenc:position=\"[0]\"&gt;/opt/xdaq/share/pixel/sensor/executive-monitor-l0.sensor&lt;/item&gt;\n            &lt;/url --&gt;\n        &lt;publish xsi:type=\"soapenc:Array\" soapenc:arrayType=\"xsd:ur-type[1]\"&gt;\n          &lt;item xsi:type=\"soapenc:Struct\" soapenc:position=\"[0]\"&gt;\n            &lt;tag xsi:type=\"xsd:string\"&gt;&lt;/tag&gt;\n            &lt;group xsi:type=\"xsd:string\"&gt;pixel-monitor&lt;/group&gt;\n          &lt;/item&gt;\n        &lt;/publish&gt;\n      &lt;/properties&gt;",autoconfsearchpath.Data());
    properties+=prop_sensor;
  }
  else if (which =="PixelFEDSupervisor") {
    profile="/nfshome0/pixelpro/XDAQConfigurations/XDAQConfiguration/FEDProfile.xml";
    if (rpm_) profile = "/opt/xdaq/htdocs/PixelFEDSupervisor/xml/FEDProfile.xml";

    vme = getFEDvme(instance);

    port=1973;
    if (type=="endpoint") {
      if (instance==1) port = 1913;
      else if (instance==2) port = 1914;
      else if (instance==3) port = 1912;
    }

    lid=70;
    id=1;
    network="xmas";
    group="daq";
    service=which;

    properties = "      &lt;properties xmlns=\"urn:xdaq-application:PixelFEDSupervisor\" xsi:type=\"soapenc:Struct\"&gt;\n";
    properties+=useDiagSystem("supervisor");
    properties+="&lt;/properties&gt;";

    so="${BUILD_HOME}/pixel/lib/libPixelFEDSupervisor.so";
    if (rpm_) so="/opt/xdaq/lib/libPixelFEDSupervisor.so";
  }
  else if (which =="PixelFECSupervisor") {
    vme = getFECvme(instance);

    port=1974;

    if (type=="endpoint") {
      if (instance==1) port = 1916;
      else if (instance==2) port = 1915;
    }

    id=2;
    lid=60;
    network="xmas";   
    group="daq";
    properties = "      &lt;properties xmlns=\"urn:xdaq-application:PixelFECSupervisor\" xsi:type=\"soapenc:Struct\"&gt;\n";
    properties+=useDiagSystem("supervisor");
    properties+="&lt;/properties&gt;";
    so="${BUILD_HOME}/pixel/lib/libPixelFECSupervisor.so";
    if (rpm_)     so="/opt/xdaq/lib/libPixelFECSupervisor.so";
  }
  else if (which =="PixelTKFECSupervisor") {
    vme= getFECvme(instance);
    id=3;
    port=1973;
    if (type=="endpoint") {
      if (instance==1) port = 1918; //in fact this one does not appear in my example file!
      else if (instance==2) port = 1917;
    }

    lid=100;
    network="xmas";
    group="daq";
    properties="      &lt;properties xmlns=\"urn:xdaq-application:PixelTKSupervisor\" xsi:type=\"soapenc:Struct\"&gt;\n";
    properties+=useDiagSystem("supervisor");
    if (instance==2)   properties+="        &lt;readDCU_workloop_usleep xsi:type=\"xsd:integer\"&gt;10000000&lt;/readDCU_workloop_usleep&gt;";
    properties+="\n      &lt;/properties&gt;";
    so = "${BUILD_HOME}/pixel/lib/libPixelTKFECSupervisor.so";
    if (rpm_)     so="/opt/xdaq/lib/libPixelTKFECSupervisor.so";
  }
  else if (which == "ttc::TTCciControl") {
    vme = "vmepcs2b16-10.cms";
    id =4;
    if (type=="endpoint") {
      if (instance==1) port = 1919;
      else if (instance==2) port = 1920;
    }
    else {
      if (instance==1) port=1975;
      else if (instance==2) port=1973;
    }

    lid=90;
    network="xmas";
    group="daq";
    service="ttc-ttcci";
    publish = "true";
    int location = (instance==1) ? 16 : 19;
    properties= "      &lt;properties xmlns=\"urn:xdaq-application:ttc::TTCciControl\" xsi:type=\"soapenc:Struct\"&gt;\n";
    TString ttcName;
    if( location == 16 ) // We are BPix
    	ttcName.Form( "&lt;name xsi:type=\"xsd:string\"&gt;TTCci-%d-BPIX&lt;/name&gt;\n        &lt;BusAdapter xsi:type=\"xsd:string\"&gt;CAENPCI&lt;/BusAdapter&gt;\n        &lt;group xsi:type=\"xsd:string\"&gt;Pixel&lt;/group&gt;\n        &lt;system xsi:type=\"xsd:string\"&gt;P5&lt;/system&gt;\n", location );
    else if( location == 19 ) // we are FPIX
	ttcName.Form( "&lt;name xsi:type=\"xsd:string\"&gt;TTCci-%d-FPIX&lt;/name&gt;\n        &lt;BusAdapter xsi:type=\"xsd:string\"&gt;CAENPCI&lt;/BusAdapter&gt;\n        &lt;group xsi:type=\"xsd:string\"&gt;Pixel&lt;/group&gt;\n        &lt;system xsi:type=\"xsd:string\"&gt;P5&lt;/system&gt;\n", location );
    properties += ttcName;
    TString loc;
    loc.Form("        &lt;Location xsi:type=\"xsd:integer\"&gt;%d&lt;/Location&gt;\n",location);
    properties+=loc;
    TString ttcconfigfile = "/nfshome0/pixelpro/TriDAS/pixel/XDAQConfiguration/TTCciConfiguration.txt";
    if (rpm_) ttcconfigfile = "/opt/xdaq/share/ttczone/sequences/ttc-lab-TTCciConfiguration.txt";
    TString someotherproperties;
//    someotherproperties.Form("        &lt;Is64XCompatible xsi:type=\"xsd:boolean\"&gt;true&lt;/Is64XCompatible&gt;\n	&lt;Configuration xsi:type=\"xsd:string\"&gt;[file=%s]&lt;/Configuration&gt;\n",ttcconfigfile.Data());
  
    someotherproperties.Form("        &lt;Configuration xsi:type=\"xsd:string\"&gt;[file=%s]&lt;/Configuration&gt;\n",ttcconfigfile.Data());
    properties+=someotherproperties;
    properties+="        &lt;BTimeCorrection xsi:type=\"xsd:unsignedInt\"&gt;100&lt;/BTimeCorrection&gt;\n        &lt;DelayT2Correction xsi:type=\"xsd:unsignedInt\"&gt;3&lt;/DelayT2Correction&gt;\n";
    //remove diagSystem from TTC
    //    properties+=useDiagSystem("supervisor");
    properties+="      &lt;/properties&gt;";
    //with new ttc configuration, we always run from the library provided by the TTC group
    so = "file:///opt/xdaq/lib/libttcttcci.so";
    //    if (rpm_) so="/opt/ttc-6.06.03/TTCSoftware/xdaq/libTTCciControl.so";
  }
  else if (which=="psx") {
    vme="cmspsx.cms";
    port=9923;
    id=6;
    lid=30;
    network="local";
    group="dcs";
    service="psx";
  }
  else if (which=="psxtk") {
    vme="cmspsx.cms";
    port=9922;
    id=6;
    lid=30;
    network="local";
    group="dcs";
    service="psxtk";
  }
  else if (which == "PixelDCSFSMInterface") {
    vme = "vmepcs2b18-11.cms";
    port=1974;
    if (type=="endpoint") port = 19111;

    lid=120;
    id=0;

    network="xmas";
    group="dcs";

    //FIXME should avoid this duplication
    if (!rpm_)  {properties = "      &lt;properties xmlns=\"urn:xdaq-application:PixelDCSFSMInterface\" xsi:type=\"soapenc:Struct\"&gt;\n        &lt;configuration xsi:type=\"xsd:string\"&gt;/nfshome0/pixelpro/TriDAS/pixel/PixelDCSInterface/xml/interface.xml&lt;/configuration&gt;\n";}
    else {properties = "      &lt;properties xmlns=\"urn:xdaq-application:PixelDCSFSMInterface\" xsi:type=\"soapenc:Struct\"&gt;\n        &lt;configuration xsi:type=\"xsd:string\"&gt;/opt/xdaq/htdocs/PixelDCSInterface/xml/interface.xml&lt;/configuration&gt;\n";}

    properties+=useDiagSystem("supervisor");
    properties+="      &lt;/properties&gt;";

    so="${BUILD_HOME}/pixel/lib/libPixelDCSInterface.so";
    if (rpm_)    so="/opt/xdaq/lib/libPixelDCSInterface.so";
    
  }
  else if (which == "PixelDCStoTrkFECDpInterface") {
    vme="vmepcs2b18-11.cms";
    port=1974;
    lid=121;
    id=0;
    
    network="xmas";
    group="dcs";
    
    so="${BUILD_HOME}/pixel/lib/libPixelDCSInterface.so";
    if (rpm_)    so="/opt/xdaq/lib/libPixelDCSInterface.so";

    properties = "  		&lt;properties xmlns=\"urn:xdaq-application:PixelDCStoTrkFECDpInterface\" xsi:type=\"soapenc:Struct\"&gt;\n  			&lt;dpValueUpdate_maxLength xsi:type=\"xsd:integer\"&gt;500&lt;/dpValueUpdate_maxLength&gt;\n  			&lt;dpName_status xsi:type=\"xsd:string\"&gt;cms_Pixel_DCSInterface&lt;/dpName_status&gt;\n  			&lt;oracleViewName_dpNames xsi:type=\"xsd:string\"&gt;urn:tstore-view-SQL:TrkFECdpNamesView&lt;/oracleViewName_dpNames&gt;\n  			&lt;oracleTableName_dpNames xsi:type=\"xsd:string\"&gt;CMS_PXL_PIXEL_VIEW.FPIX_DCU_DATAPOINTS_NEW_V&lt;/oracleTableName_dpNames&gt;\n  			&lt;oracleViewName_dpFilter xsi:type=\"xsd:string\"&gt;urn:tstore-view-SQL:TrkFECdpFilterView&lt;/oracleViewName_dpFilter&gt;\n";
    properties+="  			&lt;oracleTableName_dpFilter xsi:type=\"xsd:string\"&gt;CMS_PXL_PIXEL_VIEW.FPIX_DCU_FILTER_NEW_V&lt;/oracleTableName_dpFilter&gt;\n  			&lt;oracleViewName_dpCalibration xsi:type=\"xsd:string\"&gt;urn:tstore-view-SQL:TrkFECdpCalibrationView&lt;/oracleViewName_dpCalibration&gt;\n  			&lt;oracleTableName_dpCalibration xsi:type=\"xsd:string\"&gt;FECTODCSDPCALIBRATION&lt;/oracleTableName_dpCalibration&gt;\n  			&lt;oracleUserName xsi:type=\"xsd:string\"&gt;CMS_PXL_PIXEL_R&lt;/oracleUserName&gt;\n  			&lt;oraclePassword xsi:type=\"xsd:string\"&gt;everyoneknows_mv2CERN&lt;/oraclePassword&gt;\n  			&lt;version_dcu_calib_filter xsi:type=\"xsd:string\"&gt;V1&lt;/version_dcu_calib_filter&gt;\n  			&lt;psx_system_name xsi:type=\"xsd:string\"&gt;cms_trk_dcs_07:&lt;/psx_system_name&gt;\n  		&lt;/properties&gt;";


  }
  else if (which == "tstore::TStore") {
    vme="vmepcs2b18-11.cms";
    port=1974;
    lid=200;
    id=0;

    network="xmas";
    group="dcs";

    properties = "      &lt;properties xmlns=\"urn:xdaq-application:TStore\" xsi:type=\"soapenc:Struct\"&gt;\n        &lt;configurationRootDirectory xsi:type=\"xsd:string\"&gt;${BUILD_HOME}/pixel/PixelDCSInterface/xml/&lt;/configurationRootDirectory&gt;\n      &lt;/properties&gt;";
    if (rpm_)   properties = "      &lt;properties xmlns=\"urn:xdaq-application:TStore\" xsi:type=\"soapenc:Struct\"&gt;\n        &lt;configurationRootDirectory xsi:type=\"xsd:string\"&gt;/opt/xdaq/htdocs/PixelDCSInterface/xml/&lt;/configurationRootDirectory&gt;\n      &lt;/properties&gt;";
  }
  else if (which == "GlobalErrorDispatcher") {
    vme="vmepcs2b18-11.cms";
    port=1975;
    id=0;
    lid=40;
    instance=0;
    network="local";
    group="daq";
    
    properties = "&lt;properties xmlns=\"urn:xdaq-application:GlobalErrorDispatcher\" xsi:type=\"soapenc:Struct\"&gt;\n";
    properties+=useDiagSystem("GED");
    properties+="        &lt;LogFilePath xsi:type=\"xsd:string\"&gt;/tmp/&lt;/LogFilePath&gt;\n     &lt;/properties&gt;";
    
    so="${BUILD_HOME}/DiagSystem/tools/lib/libGlobalErrorDispatcher.so";
    if (rpm_)    so="/opt/xdaq/lib/libGlobalErrorDispatcher.so";

  }
  else if ( which == "AjaxLogReader") {
    vme="vmepcs2b18-11.cms";
    port=1975;
    lid=21;
    instance=0;
    network="local";
    group="daq";

    properties =  "     &lt;properties xmlns=\"urn:xdaq-application:AjaxLogReader\" xsi:type=\"soapenc:Struct\"&gt;\n";
    properties+=useDiagSystem("Ajax");
    properties+="      &lt;/properties&gt;";
    
    so="${BUILD_HOME}/DiagSystem/tools/lib/libAjaxLogReader.so";
    if (rpm_)    so="/opt/xdaq/lib/libAjaxLogReader.so";
  }
  else if (which == "xdaq2rc") {
    so="/opt/xdaq/lib/libxdaq2rc.so";
  }


  //a kludge -- for the ttc section (but not for the ttc app itself) -- blank out the properties
  if  (type == "app" || type=="appshort" ||type=="applong") {
    if (which!="ttc::TTCciControl" && ttc_) properties="";
  }

  TString r="";
  if (type=="xapp") {
    if ( which.Contains("tstore") ) {    //tstore has no modulePath or xdaqpath
      r.Form("<XdaqApplication className=\"%s\" hostname=\"%s\" port=\"%d\" urn=\"/urn:xdaq-application:lid=%d\" qualifiedResourceType=\"rcms.fm.resource.qualifiedresource.XdaqApplication\" instance=\"%d\" />",which.Data(),vme.Data(),port,lid,instance);
    }
    else if (  (which.Contains("xplore") || which.Contains("sentinel") || which.Contains("sensor"))) {//instance is hardcoded to 0
      r.Form("<XdaqApplication className=\"%s\" hostname=\"%s\" port=\"%d\" urn=\"/urn:xdaq-application:lid=%d\" qualifiedResourceType=\"rcms.fm.resource.qualifiedresource.XdaqApplication\" modulePath=\"%s\" xdaqPath=\"/opt/xdaq\" instance=\"0\" />",which.Data(),vme.Data(),port,lid,so.Data());
    }
    else //everything else
      r.Form("<XdaqApplication className=\"%s\" hostname=\"%s\" port=\"%d\" urn=\"/urn:xdaq-application:lid=%d\" qualifiedResourceType=\"rcms.fm.resource.qualifiedresource.XdaqApplication\" modulePath=\"%s\" xdaqPath=\"/opt/xdaq\" instance=\"%d\" />",which.Data(),vme.Data(),port,lid,so.Data(),instance);
    r.Prepend("   ");
  }
  else if (type=="exec") {
    TString configsource= confSourceIsDB_ ? "DB" : "/pixelscratch/pixelscratch/config/Pix";
    TString str1,str2;
    if( which == "ttc::TTCciControl" )
    {
     // special ttcci XDAQ_ZONE configuration here. Yes this is very hackish... 
     str1.Form("<XdaqExecutive hostname=\"%s\" port=\"%d\" urn=\"/urn:xdaq-application:lid=0\" qualifiedResourceType=\"rcms.fm.resource.qualifiedresource.XdaqExecutive\" instance=\"0\" logURL=\"xml://cmsrc-pixel.cms:17010\" logLevel=\"INFO\" pathToExecutive=\"/opt/xdaq/bin/xdaq.exe\" unixUser=\"pixelpro\" environmentString=\"HOME=%s XDAQ_ROOT=/opt/xdaq XDAQ_DOCUMENT_ROOT=/opt/xdaq/htdocs XDAQ_SETUP_ROOT=/opt/xdaq/share XDAQ_BASE=/opt/xdaq XDAQ_OS=linux XDAQ_PLATFORM=x86_64_slc5 %s XDAQ_ZONE=ttcp5 ",
     vme.Data(),port,homehome.Data(),buildhome.Data() );

    }
    else
    {
    str1.Form("<XdaqExecutive hostname=\"%s\" port=\"%d\" urn=\"/urn:xdaq-application:lid=0\" qualifiedResourceType=\"rcms.fm.resource.qualifiedresource.XdaqExecutive\" instance=\"0\" logURL=\"xml://cmsrc-pixel.cms:17010\" logLevel=\"INFO\" pathToExecutive=\"/opt/xdaq/bin/xdaq.exe -e %s\" unixUser=\"pixelpro\" environmentString=\"HOME=%s XDAQ_ROOT=/opt/xdaq XDAQ_DOCUMENT_ROOT=/opt/xdaq/htdocs XDAQ_SETUP_ROOT=/opt/xdaq/share XDAQ_BASE=/opt/xdaq XDAQ_OS=linux XDAQ_PLATFORM=x86_64_slc5 %s ",
	      vme.Data(),port,profile.Data(),homehome.Data(),buildhome.Data() );
    }
    if (rpm_) { //FIXME this is bad duplication
      //ENV_TTCSOFTWARE_ROOT=/opt/ttc-6.06.03/TTCSoftware 
      //could be that not having the ENV_CMS_TK_* variables defined won't work. we'll see
      str2.Form("PIXELCONFIGURATIONBASE=%s PIXELCONFIGURATIONSPOOLAREA=/nfshome0/pixelpro/PixelSpoolArea/ TNS_ADMIN=/pixel/config/private POS_OUTPUT_DIRS=/pixel/data1 %s LD_LIBRARY_PATH=%s JAVA_HOME=/usr/java/default  %s\">",
		configsource.Data(),rootsys.Data(),ldlibrarypath.Data(),reconf.Data());
    }
    else  {
      //ENV_TTCSOFTWARE_ROOT=/nfshome0/pixelpro/TriDAS/TTCSoftware
      if (fpixonly_) configsource=confSourceIsDB_ ? "DB" :"/pixelscratch/pixelscratch/config/FPix/PCDE_all";
      else if (bpixonly_) configsource=confSourceIsDB_ ? "DB" :"/pixelscratch/pixelscratch/config/BPix";
      str2.Form("PIXELCONFIGURATIONBASE=%s PIXELCONFIGURATIONSPOOLAREA=/nfshome0/pixelpro/PixelSpoolArea/ TNS_ADMIN=/nfshome0/pixelpro/private POS_OUTPUT_DIRS=/nfshome0/pixelpro/TriDAS/pixel/PixelRun/Runs %s ENV_CMS_TK_DIAG_ROOT=/nfshome0/pixelpro/TriDAS/DiagSystem ENV_CMS_TK_ONLINE_ROOT=/nfshome0/pixelpro/TriDAS/FecSoftwareV3_0 ENV_CMS_TK_FEC_ROOT=/nfshome0/pixelpro/TriDAS/FecSoftwareV3_0 LD_LIBRARY_PATH=%s CATALINA_HOME=/nfshome0/pixelpro/tomcat JAVA_HOME=/usr/java/default RCMS_HOME=/nfshome0/pixelpro/RunControl %s\">",
		configsource.Data(),rootsys.Data(),ldlibrarypath.Data(),reconf.Data());
    }
    r=str1;
    r+=str2;
  }
  else if (type=="context")
    r.Form("\n  &lt;xc:Context id=\"%d\" url=\"http://%s:%d/\"&gt;",id,vme.Data(),port);

  else if (type=="app") {

    if (which=="PixelFEDSupervisor") //this  is a bit of kludge
      r.Form("&lt;xc:Application class=\"%s\" id=\"%d\" instance=\"%d\" network=\"%s\" group=\"%s\" service=\"%s\"&gt;\n      %s\n    &lt;/xc:Application&gt;",
	     which.Data(),lid,instance,network.Data(),group.Data(),service.Data(),properties.Data());
    else if( which.Contains( "TTCciControl" ) )
      r.Form("&lt;xc:Application class=\"%s\" id=\"%d\" instance=\"%d\" network=\"%s\" group=\"%s\" publish=\"true\" service=\"%s\"&gt;\n      %s\n    &lt;/xc:Application&gt;", which.Data(),lid,instance,network.Data(),group.Data(),service.Data(), properties.Data());
    else
      r.Form("&lt;xc:Application class=\"%s\" id=\"%d\" instance=\"%d\" network=\"%s\" group=\"%s\"&gt;\n      %s\n    &lt;/xc:Application&gt;",
	     which.Data(),lid,instance,network.Data(),group.Data(),properties.Data());

    r.Prepend("\n   ");
  }
  else if (type=="appshort") {
    r.Form("\n&lt;xc:Application class=\"%s\" id=\"%d\" network=\"%s\"&gt;\n      %s\n    &lt;/xc:Application&gt;",
	   which.Data(),lid,network.Data(),properties.Data());
  }
  else if (type=="applong") {
    r.Form("\n&lt;xc:Application class=\"%s\" id=\"%d\"  network=\"%s\"  group=\"%s\" service=\"%s\"",
	   which.Data(),lid,network.Data(),group.Data(),service.Data());
    if (which.Contains("psx")) {
      r+= " instance=\"0\"/&gt;"; //special treatment for psx
    } 
    
    else {
      r+="&gt;\n";
      r+= properties;
      r+= "\n    &lt;/xc:Application&gt;";
    }
  }
  else if (type=="endpoint") {
    r.Form("    &lt;xc:Endpoint protocol=\"tcp\" service=\"b2in\" interface=\"eth0\" hostname=\"%s\" port=\"%d\" network=\"%s\"/&gt;",
	   vme.Data(),port,network.Data());
  }
  else if (type=="module") {
    r.Form("&lt;xc:Module&gt;%s&lt;/xc:Module&gt;",so.Data());
  }

  return r;
}

TString Module(TString which) {
  return XApp(which,"module");
}

TString Endpoint(TString which,int instance=0) {
  return XApp(which,"endpoint",instance);
}

TString CloseContext() {
  TString r="\n  &lt;/xc:Context&gt;";
  return r;
}

TString App(TString which, int instance=0) {
  return XApp(which,"app",instance);
}

TString Context(TString which,int instance=0) {
  return XApp(which,"context",instance);
}

TString XExec(TString which,int instance=0) {
  return XApp(which,"exec",instance);
}

TString ConfString(TString a) {

  TString r="<Configuration xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" user=\"pixelpro\" path=\"";
  r +=a;
  
r+= "\">";
 return r;
}

TString preventativeSEURecovery()
{
 TString r="<property name=\"PREVENTIVE_SOFT_ERROR_RECOVERY\" type=\"boolean\">\n";
 r+="<value>true</value>\n";
 r+="</property>\n";

 return r;
}

TString FMString() {

  TString a="  <FunctionManager name=\"PixelFunctionManager\" hostname=\"cmsrc-pixel.cms\" port=\"17000\" qualifiedResourceType=\"rcms.fm.resource.qualifiedresource.FunctionManager\" role=\"PIXEL\" sourceURL=\"http://cmsrc-pixel.cms:17000/functionmanagers/PixelFunctionManager.jar\" className=\"rcms.fm.app.pixelfm.PixelFunctionManager\">";
  return a;

}

TString JobControl(TString pc) {
  TString a="    <Service name=\"JobControl\" hostname=\"";
  a+=pc;
  a+= "\" port=\"9999\" urn=\"/urn:xdaq-application:lid=10\" qualifiedResourceType=\"rcms.fm.resource.qualifiedresource.JobControl\" />";
  return a;

}

void metaxml(ofstream * fout) {

  *fout<<"     <configFile>&lt;?xml version='1.0'?&gt;"<<endl;
  *fout<<"&lt;xc:Partition xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"  xmlns:soapenc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xc=\"http://xdaq.web.cern.ch/xdaq/xsd/2004/XMLConfiguration-30\"&gt;"<<endl;

  if (!ttc_) { //a (successful) test -- this is a modification from the working configurations in the past
    *fout<<Context("GlobalErrorDispatcher")<<endl;
    
    *fout<<App("GlobalErrorDispatcher")<<endl;
    *fout<<Module("GlobalErrorDispatcher")<<endl;
    *fout<<App("AjaxLogReader")<<endl;
    *fout<<Module("AjaxLogReader")<<endl;
    *fout<<CloseContext()<<endl;
  }

  *fout<<Context("PixelSupervisor")<<endl;
  *fout<<Endpoint("PixelSupervisor")<<endl;
  *fout<<App("PixelSupervisor")<<endl;
  if (!ttc_) *fout<<Module("xdaq2rc")<<endl;
  if (!ttc_)   *fout<<Module("PixelSupervisor")<<endl;
  *fout<<CloseContext()<<endl;

  *fout<<Context("PixelDCSFSMInterface")<<endl;
  *fout<<Endpoint("PixelDCSFSMInterface")<<endl;
  *fout<<App("PixelDCSFSMInterface")<<endl;

  *fout<<App("PixelDCStoTrkFECDpInterface")<<endl;
  if (!ttc_)   *fout<<Module("PixelDCSFSMInterface")<<endl;

  *fout<<App("tstore::TStore")<<endl;
  if (!ttc_) {
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libtstoreutils.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libxalan-c.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libxoapfilter.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libtstore.so")<<endl;
  }
  *fout<<CloseContext()<<endl;

  for (int jj=1; jj<=3; jj++) {

    if (fpixonly_ && jj!=3) continue;
    if (bpixonly_ && jj==3) continue;

    *fout<<Context("PixelFEDSupervisor",jj)<<endl;
    *fout<<Endpoint("PixelFEDSupervisor",jj)<<endl;
    
    *fout<<App("PixelFEDSupervisor",jj)<<endl;
  if (!ttc_)     *fout<<Module("PixelFEDSupervisor")<<endl;
    
    *fout<<XApp("xplore::Application","appshort")<<endl;
    if (!ttc_) {
      *fout<<MiscModule("/lib/libslp.so")<<endl;
      *fout<<MiscModule("${XDAQ_ROOT}/lib/libxslp.so")<<endl;
      *fout<<MiscModule("${XDAQ_ROOT}/lib/libxploreutils.so")<<endl;
      *fout<<MiscModule("${XDAQ_ROOT}/lib/libxplore.so")<<endl;
    }    

    *fout<<XApp("sentinel::Application","applong")<<endl;
    if (!ttc_) {
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libwsaddressing.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libwseventing.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libsentinelutils.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libsentinel.so")<<endl;
    }
    
    *fout<<XApp("xmas::sensor::Application","applong")<<endl;
    if (!ttc_) {
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libwsutils.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libxmasutils.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libwsbrokerutils.so")<<endl;
    *fout<<MiscModule("${XDAQ_ROOT}/lib/libxmassensor.so")<<endl;
    }
    *fout<<CloseContext()<<endl;
  }

  for (int jj=1; jj<=2; jj++) {

    if (fpixonly_ && jj!=2) continue;
    if (bpixonly_ && jj!=1) continue;

    *fout<<Context("PixelFECSupervisor",jj)<<endl;
    *fout<<Endpoint("PixelFECSupervisor",jj)<<endl;
    *fout<<App("PixelFECSupervisor",jj)<<endl;
    if (!ttc_)     *fout<<Module("PixelFECSupervisor")<<endl;
    *fout<<CloseContext()<<endl;
  }

  for (int jj=1; jj<=2; jj++) {

    if (fpixonly_ && jj!=2) continue;
    if (bpixonly_ && jj!=1) continue;

    *fout<<Context("PixelTKFECSupervisor",jj)<<endl;
    //for reasons that i do not understand, there is no endpoint in our model configuration for instance 1
    //it appears that our configurations are inconsistent in their use of endpoints
    if (jj==2) *fout<<Endpoint("PixelTKFECSupervisor",jj)<<endl;
    *fout<<App("PixelTKFECSupervisor",jj)<<endl;
    if (!ttc_)     *fout<<Module("PixelTKFECSupervisor")<<endl;
    *fout<<CloseContext()<<endl;
  }

  for (int jj=1; jj<=2; jj++) {

    if (fpixonly_ && jj!=2) continue;
    if (bpixonly_ && jj!=1) continue;

    *fout<<Context("ttc::TTCciControl",jj)<<endl;
    //it seems that some configs have no endpoints for the ttc , but others do
    *fout<<Endpoint("ttc::TTCciControl",jj)<<endl;
    *fout<<App("ttc::TTCciControl",jj)<<endl;
    //this 'if' is new. If we are running from rpm then we only want this included in the TTC xdaq application block

    if (ttc_)  {
		*fout<<Module("ttc::TTCciControl")<<endl;
    	// marc *fout<<MiscModule("${XDAQ_ROOT}/lib/libttcttcci.so")<<endl;
    	*fout<<MiscModuleTTC("file:///opt/xdaq/lib/libttcutils.so")<<endl;
    	*fout<<MiscModuleTTC("file:///opt/xdaq/lib/libttcmonitoring.so")<<endl;
	}
    *fout<<CloseContext()<<endl;
  }

  *fout<<Context("psx")<<endl;
  //this is going to leave the instance=0 out of the psx server def'n
  //i will have to test if this is ok
  *fout<<XApp("psx","applong")<<endl;
  *fout<<CloseContext()<<endl;

  //not using this anymore!
//   *fout<<Context("psxtk")<<endl;
//   *fout<<XApp("psxtk","applong")<<endl;
//   *fout<<CloseContext()<<endl;

  *fout<<RCMS()<<endl;
  *fout<<JobControlBlock()<<endl;
  *fout<<"&lt;/xc:Partition&gt;</configFile>\n   </XdaqExecutive>"<<endl<<endl;


}


void go()
{

  ofstream* fout = new ofstream(TString(configName+".xml").Data());

  //==========================================================================
  ttc_=false;
  *fout<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;

  *fout<<ConfString(configName)<<endl;
  if( usePrevSEURec )
     *fout << preventativeSEURecovery() << endl;
  *fout<<FMString()<<endl;
  ///beginning of first jobControl
  *fout<<JobControl("vmepcs2b18-11.cms")<<endl;

  *fout<<XApp("PixelSupervisor","xapp")<<endl;
  *fout<<XExec("PixelSupervisor")<<endl;
  metaxml(fout);
  //
  *fout<<XApp("GlobalErrorDispatcher","xapp")<<endl;
  *fout<<XExec("GlobalErrorDispatcher")<<endl;
  metaxml(fout);

  *fout<<XApp("AjaxLogReader","xapp")<<endl;
  *fout<<XExec("AjaxLogReader")<<endl;
  metaxml(fout);

  *fout<<XApp("PixelDCSFSMInterface","xapp")<<endl;
  *fout<<XApp("PixelDCStoTrkFECDpInterface","xapp")<<endl;
  *fout<<XApp("tstore::TStore","xapp")<<endl;
  *fout<<XExec("PixelDCSFSMInterface")<<endl;
  metaxml(fout);
  //end of first jobControl
  
  for (int ii=1; ii<=3; ii++) {

    if (fpixonly_ && ii!=3) continue;
    if (bpixonly_ && ii==3) continue;

    *fout<<JobControl(getFEDvme(ii))<<endl;
    *fout<<XApp("PixelFEDSupervisor","xapp",ii)<<endl;
    *fout<<XApp("xplore::Application","xapp",ii)<<endl;
    *fout<<XApp("sentinel::Application","xapp",ii)<<endl;
    *fout<<XApp("xmas::sensor::Application","xapp",ii)<<endl;
    *fout<<XExec("PixelFEDSupervisor",ii)<<endl;
    metaxml(fout);
  }

  for (int ii=1; ii<=2; ii++) {

    if (fpixonly_ && ii!=2) continue;
    if (bpixonly_ && ii!=1) continue;

    *fout<<JobControl(getFECvme(ii))<<endl;
    *fout<<XApp("PixelFECSupervisor","xapp",ii)<<endl;
    *fout<<XExec("PixelFECSupervisor",ii)<<endl;
    metaxml(fout);
    *fout<<XApp("PixelTKFECSupervisor","xapp",ii)<<endl;
    *fout<<XExec("PixelTKFECSupervisor",ii)<<endl;
    metaxml(fout);
  }

  //  bool rpm=rpm_;
  //  rpm_=false; //not yet ready to deal with rpm installtion on ttc pc
  ttc_=true;
  *fout<<JobControl("vmepcs2b16-10.cms")<<endl;
  for (int ii=1; ii<=2; ii++) {

    if (fpixonly_ && ii!=2) continue;
    if (bpixonly_ && ii!=1) continue;

    *fout<<XApp("ttc::TTCciControl","xapp",ii)<<endl;
    *fout<<XExec("ttc::TTCciControl",ii)<<endl;
    metaxml(fout);
  }
  //  rpm_=rpm;
  ttc_=false;
  
  *fout<<"  </FunctionManager>\n</Configuration>"<<endl;

  //==========================================================================
  fout->close();

}

void writeRCMSxml() {
  go();
}
