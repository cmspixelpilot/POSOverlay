// $Id: PixelCalibrationBase.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		       1                  *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelCalibrationBase.h"
// #include <toolbox/convertstring.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace pos;

//FIXME should be defined in some headerfile; or I should 
//understand how to use the DCSUtilities... (ryd)
#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"
const std::string dpeSuffix_get = ":_online.._value";

PixelCalibrationBase::PixelCalibrationBase(const PixelSupervisorConfiguration & tempConfiguration, const SOAPCommander& soapCommander) 
  : PixelSupervisorConfiguration(tempConfiguration),
    SOAPCommander(soapCommander)
{
  //FIXME this method should never be called and should be removed.
  std::cout << "Greetings from the PixelCalibrationBase copy constructor." << std::endl;

  //the copying of pointers is taken care of in PixSupConf
  event_=0;
  eventNumberOfLastReportedProgress_=0;

  percentageOfJob_=0;

  
  if(dynamic_cast <PixelCalibConfiguration*> (theCalibObject_)==0) 
    { 
      sendingMode_="yes";
    }
  else{
    
  sendingMode_=(dynamic_cast <PixelCalibConfiguration*> (theCalibObject_))->parameterValue("useSOAP");
  
  }

  ttcCalSyncThrottlingTimer_.start();
}


PixelCalibrationBase::~PixelCalibrationBase(){}


void PixelCalibrationBase::sendBeginCalibrationToFEDs(){

  std::string action="BeginCalibration";
  sendToFED(action);
}



void PixelCalibrationBase::sendEndCalibrationToFEDs(){
 
    std::string action="EndCalibration";
    sendToFED(action);
     
}


void PixelCalibrationBase::runBeginCalibration(){
  event_=0;
  nextEvent_=true;
  eventNumberOfLastReportedProgress_=0;
  
  cout<<"[PixelCalibrationBase::runBeginCalibration] theCalibObject_ = "<<theCalibObject_<<endl;  //for debugging
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  //assert(tempCalibObject!=0); //this fails for Delay25
  
  resetROC_=true;
  if (tempCalibObject!=0 && tempCalibObject->parameterValue("ROCReset") == "no" ) {
    cout << "PixelCalibrationBase:beginCalibration "
	 << "will not reset ROC." << endl;
    resetROC_=false;
  }
  //std::cout << "[PixelCalibrationBase::runBeginCalibration]: will call sendBeginCalibrationToFEDs"<<std::endl;
  sendBeginCalibrationToFEDs();  
  //std::cout << "[PixelCalibrationBase::runBeginCalibration]: done calling sendBeginCalibrationToFEDs"<<std::endl;
  beginCalibration();
  //std::cout << "[PixelCalibrationBase::runBeginCalibration]: done calling beginCalibration"<<std::endl;
  
  sendTTCTBMReset();
  
}


void PixelCalibrationBase::runEndCalibration(){
  sendEndCalibrationToFEDs();
  endCalibration();
}


void PixelCalibrationBase::beginCalibration(){}

void PixelCalibrationBase::endCalibration(){}

bool PixelCalibrationBase::runEvent(){

  bool cont=execute();

  if(nextEvent_) {
    event_++;
  }
    
  return cont;

}

void PixelCalibrationBase::reportProgress( double howOften, std::ostream& out, int NTriggersTotal )
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	unsigned int NCalls;
	if ( NTriggersTotal == -1 )
	{
		NCalls = tempCalibObject->nTriggersTotal();
	}
	else
	{
		assert( NTriggersTotal > 0 );
		NCalls = (unsigned int)NTriggersTotal;
	}
	
	if ( event_ == 0 || (double(event_ - eventNumberOfLastReportedProgress_))/(double(NCalls)) > howOften )
	{
		eventNumberOfLastReportedProgress_ = event_;
		
		double percentageOfJob=100.*(double(event_))/(double(NCalls));
		this->setPercentageOfJob(percentageOfJob);
		
		out << tempCalibObject->mode() << ": " << setprecision(2) << percentageOfJob<< std::setprecision(6) << "% complete" << endl;
	}
}

void PixelCalibrationBase::prepareFEDCalibrationMode(unsigned int nevents) {
  Attribute_Vector parameters(1);
  parameters[0].name_  = "NEvents";
  parameters[0].value_ = itoa(nevents); 

  // JMTBAD should this be careful about which feds it sends to?
  std::string cmd("prepareFEDCalibrationMode");
  sendToFED(cmd, parameters);
}

void PixelCalibrationBase::sendTTCCalSync(){
  // JMT will this work putting it here always? Sleep after?
  prepareFEDCalibrationMode(1);

  if (useTTC_){
    Attribute_Vector parametersToTTC(2);
    parametersToTTC[0].name_="xdaq:CommandPar";
    parametersToTTC[0].value_="Execute Sequence";
    parametersToTTC[1].name_="xdaq:sequence_name";
    parametersToTTC[1].value_="CalSync";
    
    Supervisors::iterator i_PixelTTCSupervisor;
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor)
      {
        if (Send(i_PixelTTCSupervisor->second, "userCommand", parametersToTTC)!="userTTCciControlResponse")
          {
            cout<<"TTCciControl supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be used!"<<endl;
          }
      }
  }
  
  
  if (useTCDS_) {
    //do some throttling
    ttcCalSyncThrottlingTimer_.stop();
    double tcalsync = ttcCalSyncThrottlingTimer_.tottime()*1000000;
    ttcCalSyncThrottlingTimer_.reset();
    ttcCalSyncThrottlingTimer_.start();

    //limit of open TCP sockets: 20000 (crashes have been observed for >28000 sockets)
    //time until a socket gets closed again: 240s (TCP connetion timeout 4 minutes)
    //max calsync frequency: 20000/240 Hz = 83.3Hz --> delta t = 12000us
    double tcalsync_target = 8000;

    if (tcalsync < tcalsync_target) {
      usleep(tcalsync_target-tcalsync);
      //std::cout << "TTCCalSync trigger throttling: dt(CalSyncs)=" << tcalsync << "us, dt(min)="<< tcalsync_target <<"us, sleep= " << tcalsync_target-tcalsync << "us" << std::endl;
    }


    Attribute_Vector paramToTTC(1);
    paramToTTC[0].name_="actionRequestorId";
    paramToTTC[0].value_=TCDSSessionID_;
    Variable_Vector varToTTC(1);
    varToTTC[0].name_="bgoNumber";
    varToTTC[0].type_="unsignedInt";
    varToTTC[0].payload_="12";
    
    Supervisors::iterator i_PixelTTCController;
    for (i_PixelTTCController=PixelTTCControllers_.begin();i_PixelTTCController!=PixelTTCControllers_.end();++i_PixelTTCController)
      {
        if (Send(i_PixelTTCController->second, "SendBgo", paramToTTC, varToTTC)!="SendBgoResponse")
          {
            std::cout<<"PixelTTCController #"<<(i_PixelTTCController->first)<<" could not be used! Maybe it is not yet configured?"<<std::endl;
            diagService_->reportError("PixelTTCController #"+stringF(i_PixelTTCController->first) + " could not send CalSync.",DIAGERROR);
          }
      }
  }
}

void PixelCalibrationBase::sendTTCROCReset(){

  if (useTTC_)
    {
      Attribute_Vector parametersToTTC(2);
      parametersToTTC[0].name_="xdaq:CommandPar";
      parametersToTTC[0].value_="Execute Sequence";
      parametersToTTC[1].name_="xdaq:sequence_name";
      parametersToTTC[1].value_="ResetROC";
      
      Supervisors::iterator i_PixelTTCSupervisor;
      for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
        if (Send(i_PixelTTCSupervisor->second, "userCommand", parametersToTTC)!="userTTCciControlResponse") {
          cout<<"TTCciControl supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be used!"<<endl;
        }
      }
    }

  if (useTCDS_)
    {
      Attribute_Vector paramToTTC(1);
      paramToTTC[0].name_="actionRequestorId";
      paramToTTC[0].value_=TCDSSessionID_;
      Variable_Vector varToTTC(1);
      varToTTC[0].name_="bgoNumber";
      varToTTC[0].type_="unsignedInt";
      varToTTC[0].payload_="15";
      
      Supervisors::iterator i_PixelTTCController;
      for (i_PixelTTCController=PixelTTCControllers_.begin();i_PixelTTCController!=PixelTTCControllers_.end();++i_PixelTTCController)
        {
          if (Send(i_PixelTTCController->second, "SendBgo", paramToTTC, varToTTC)!="SendBgoResponse")
            {
              std::cout<<"PixelTTCController #"<<(i_PixelTTCController->first)<<" could not be used! Maybe it is not yet configured?"<<std::endl;
              diagService_->reportError("PixelTTCController #"+stringF(i_PixelTTCController->first) + " could not Reset ROC.",DIAGERROR);
            }
        }
    }
}

void PixelCalibrationBase::sendTTCTBMReset(){

  if (useTTC_){
    Attribute_Vector parametersToTTC(2);
    parametersToTTC[0].name_="xdaq:CommandPar";
    parametersToTTC[0].value_="Execute Sequence";
    parametersToTTC[1].name_="xdaq:sequence_name";
    parametersToTTC[1].value_="ResetTBM";
    
    Supervisors::iterator i_PixelTTCSupervisor;
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
      if (Send(i_PixelTTCSupervisor->second, "userCommand", parametersToTTC)!="userTTCciControlResponse") {
        cout<<"TTCciControl supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be used!"<<endl;
      }
    }
  }


  if (useTCDS_)
    {
      Attribute_Vector paramToTTC(1);
      paramToTTC[0].name_="actionRequestorId";
      paramToTTC[0].value_=TCDSSessionID_;
      Variable_Vector varToTTC(1);
      varToTTC[0].name_="bgoNumber";
      varToTTC[0].type_="unsignedInt";
      varToTTC[0].payload_="14";
      
      Supervisors::iterator i_PixelTTCController;
      for (i_PixelTTCController=PixelTTCControllers_.begin();i_PixelTTCController!=PixelTTCControllers_.end();++i_PixelTTCController)
        {
          if (Send(i_PixelTTCController->second, "SendBgo", paramToTTC, varToTTC)!="SendBgoResponse")
            {
              std::cout<<"PixelTTCController #"<<(i_PixelTTCController->first)<<" could not be used! Maybe it is not yet configured?"<<std::endl;
              diagService_->reportError("PixelTTCController #"+stringF(i_PixelTTCController->first) + " could not Reset TBM.",DIAGERROR);
            }
        }
    }  
}

int PixelCalibrationBase::sendLTCCalSync(unsigned int nTriggers){

  Attribute_Vector parameters(1);
  parameters[0].name_="Param";
  parameters[0].value_=itoa(nTriggers);

  int returnTriggers = 0;

  if (PixelLTCSupervisors_.begin()==PixelLTCSupervisors_.end()){
    cout << "[PixelCalibrationBase::sendLTCCalSync] don't have a LTC supervisor"<<endl;
  }

  Supervisors::iterator i_PixelLTCSupervisor;
  for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
    xoap::MessageReference reply=SendWithSOAPReply(i_PixelLTCSupervisor->second, "NTriggers", parameters);
    Attribute_Vector returnFromLTC(1);
    returnFromLTC.at(0).name_ = "CountTriggers";
    Receive(reply, returnFromLTC);
    returnTriggers = atoi(returnFromLTC.at(0).value_.c_str());
    //cout<<"[PixelCalibrationBase::sendLTCCalSync] reply="<<reply<<endl;;
  }

  //Assumes at most one LTC
  return returnTriggers;

}



void PixelCalibrationBase::nextFECConfig(unsigned int event){

    
  Attribute_Vector parameters(1);
  parameters[0].name_="Event";
  parameters[0].value_=itoa(event);       // Event#
  
  std::string cmd="CalibRunningThreshold";
  sendToFEC(cmd, parameters);
 
  ttcTimer_.start();
  if(resetROC_) {
    sendTTCROCReset();
  }
  ttcTimer_.stop();
  
}  

void PixelCalibrationBase::enableFIFO3(){

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {

    std::vector<std::pair<unsigned int, std::vector<unsigned int> > > theFEDCardAndChannels=tempCalibObject->fedCardsAndChannels(i_PixelFEDSupervisor->first, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);
    for (unsigned int k=0;k<theFEDCardAndChannels.size(); ++k) {

      unsigned int fednumber=theFEDCardAndChannels.at(k).first;
	
      unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
      
      Attribute_Vector fedparameters(1);
      fedparameters[0].name_="VMEBaseAddress";
      fedparameters[0].value_=itoa(vmeBaseAddress); //FIXME should be
      // able to send one message to do all FEDs
      
      std::string reply;
      reply=Send(i_PixelFEDSupervisor->second,"EnableFIFO3", fedparameters);
      if (reply!="EnableFIFO3Done") {
	diagService_->reportError("Enabling the appropriate Spy FIFO on FED #" + stringF(fednumber) + " could not be done!",DIAGWARN);
	cout<<"Enabling the appropriate Spy FIFO on FED #"<<fednumber<<" could not be done! \n";
      }
    }
  }
}

void PixelCalibrationBase::readData(unsigned int event){
    
    Attribute_Vector parameters(1);
    parameters[0].name_="Event";
    parameters[0].value_=itoa(event);   
    std::string action="FEDCalibrations";
    
    sendToFED(action, parameters);
   
}

void PixelCalibrationBase::commandToAllTKFECCrates( std::string command, Attribute_Vector parameters )
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	std::set<unsigned int> tkfeccrates = tempCalibObject->getTKFECCrates(thePortcardMap_, *getmapNamePortCard(), theTKFECConfiguration_);
	
	for (std::set<unsigned int>::const_iterator itkfeccrate=tkfeccrates.begin();itkfeccrate!=tkfeccrates.end();++itkfeccrate)
	{
		if (Send(PixelTKFECSupervisors_[(*itkfeccrate)], command, parameters) != command+"Done")
		{
			diagService_->reportError(command+" in TKFEC crate # " + stringF((*itkfeccrate)) + " could not be done!",DIAGWARN);
		}
	}
}

void PixelCalibrationBase::commandToAllFECCrates( std::string command, Attribute_Vector parameters )
{
  sendToFEC(command, parameters);    

}

void PixelCalibrationBase::commandToAllFEDCrates( std::string command, Attribute_Vector parameters )
{
  sendToFED(command, parameters);     
}

void PixelCalibrationBase::commandToAllFEDChannels( std::string command )
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	Attribute_Vector parametersToFED_command(2);
	parametersToFED_command[0].name_="VMEBaseAddress";
	parametersToFED_command[1].name_="FEDChannel";
	
	const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
	
	std::vector<int> messageIDs;
	bool flag=false;
	if(sendingMode_==""){
	  for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
	    {
	      const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
	      const unsigned int fednumber=channelHdwAddress.fednumber();
	      const unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
	      const unsigned int fedVMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
	      const unsigned int fedchannel=channelHdwAddress.fedchannel();
	      parametersToFED_command[0].value_=itoa(fedVMEBaseAddress);
	      parametersToFED_command[1].value_=itoa(fedchannel);
	     
	      int messageID=send(PixelFEDSupervisors_[fedcrate], command, flag, parametersToFED_command);

	      messageIDs.push_back(messageID);
	    }
      
	  for (std::vector<int>::iterator itr=messageIDs.begin(), itr_end=messageIDs.end(); itr!=itr_end; ++itr){ 
	    flag=waitForReply(*itr);
	  }
	}else if(sendingMode_=="yes"){	
	  for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
	    {
	      const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
	      const unsigned int fednumber=channelHdwAddress.fednumber();
	      const unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
	      const unsigned int fedVMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
	      const unsigned int fedchannel=channelHdwAddress.fedchannel();
	      parametersToFED_command[0].value_=itoa(fedVMEBaseAddress);
	      parametersToFED_command[1].value_=itoa(fedchannel);
	  
   
	      if (Send(PixelFEDSupervisors_[fedcrate], command, parametersToFED_command)!=command+"Done"){
		std::cout<< command+" couldn't be done for FED #"<<fednumber<<" situated in\ncrate "<<fedcrate<<" and VME base address = 0x"<<hex<<fedVMEBaseAddress<<dec<<std::endl;
	      }
	    }
	  
	}
}
void PixelCalibrationBase::setDAC(PixelROCName aROC,
				  unsigned int dacAddress,
				  unsigned int dac){

  //FIXME For now assume that there is only one pixel FEC to talk to.
  
  const PixelHdwAddress* hdw=theNameTranslation_->getHdwAddress(aROC);
  assert(hdw!=0);

  Attribute_Vector parsToFEC(8);
  parsToFEC[0].name_="mFEC";            parsToFEC[0].value_=itoa(hdw->mfec());
  parsToFEC[1].name_="mFECChannel";     parsToFEC[1].value_=itoa(hdw->mfecchannel());
  parsToFEC[2].name_="HubAddress";      parsToFEC[2].value_=itoa(hdw->hubaddress());
  parsToFEC[3].name_="PortAddress";     parsToFEC[3].value_=itoa(hdw->portaddress());
  parsToFEC[4].name_="ROCId";           parsToFEC[4].value_=itoa(hdw->rocid());
  parsToFEC[5].name_="DACAddress";      parsToFEC[5].value_=itoa(dacAddress);
  parsToFEC[6].name_="DACValue";        parsToFEC[6].value_=itoa(dac);
  unsigned int vmebaseaddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(hdw->fecnumber());
  parsToFEC[7].name_="VMEBaseAddress";  parsToFEC[7].value_=itoa(vmebaseaddress);

  unsigned int crate=theFECConfiguration_->crateFromFECNumber(hdw->fecnumber());

  std::string reply=Send(PixelFECSupervisors_[crate], "Prog_DAC", parsToFEC);

}

double PixelCalibrationBase::readIana(std::string dpeName){

  return readCAEN(dpeName+".actual.iMon");

}

double PixelCalibrationBase::readIdigi(std::string dpeName){

  return readCAEN(dpeName+".actual.iMon");

}

double PixelCalibrationBase::readCAEN(std::string dpeName){

  //std::string dpeName = "CAEN/pixelTIF_SY1527/branchController15/easyCrate0/easyBoard16/channel001.actual.iMon"; 

  std::string dpeValue = "";

  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("dpGet", "psx", PSX_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName dpElement = envelope.createName("dp");
  xoap::SOAPElement childElement = bodyElement.addChildElement(dpElement);
  xoap::SOAPName nameElement = envelope.createName("name");
  childElement.addAttribute(nameElement, dpeName + dpeSuffix_get);

  static int count=1000;

  count++;

  if (count<5) {
    std::cout << " Request : ------------------------------------ "<< std::endl;
    psxRequest->writeTo(std::cout);
    std::cout << std::endl;
    std::cout << " ---------------------------------------------- "<< std::endl;
  }
  
  xoap::MessageReference psxResponse;
  
  //FIXME will pick only the first; will assert if more than one.
  assert(psxServers_.size()==1);
  xdaq::ApplicationDescriptor* psxServer=psxServers_.begin()->second;

  psxResponse = app_->getApplicationContext()->postSOAP(psxRequest, *app_->getApplicationDescriptor(), *psxServer);

  if (count<5){
    std::cout << " Response : ------------------------------------ "<< std::endl;
    psxResponse->writeTo(std::cout);
    std::cout << std::endl;
    std::cout << " ---------------------------------------------- "<< std::endl;
  }

  xoap::SOAPEnvelope returnenvelope = psxResponse->getSOAPPart().getEnvelope();
  xoap::SOAPBody returnbody = returnenvelope.getBody();
  xoap::SOAPName returncommandElement = returnenvelope.createName("dpGetResponse");
  std::vector<xoap::SOAPElement> commandElements = returnbody.getChildElements(returncommandElement);          	  

  std::vector<xoap::SOAPElement>::iterator p_commandElement = 
    commandElements.begin();

  for (;p_commandElement != commandElements.end(); ++p_commandElement ) {
      xoap::SOAPName dpElement = returnenvelope.createName("dp");
      std::vector<xoap::SOAPElement> dpElements = p_commandElement->getChildElements(dpElement);          	  
      
      if ( dpElements.size() == 1 ) {
	dpeValue = dpElements[0].getValue();
      }
  }
  
  if (count<5){
    cout << "dpeValue:"<<dpeValue<<endl;
  }

  istringstream in(dpeValue);
  double value;
  in >> value;
  return value;

  

}


void PixelCalibrationBase::sendToFED(std::string& action, Attribute_Vector parametersToFED)
{
   bool flag=false;
   
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  std::set<unsigned int> fedcrates=tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);

  if(sendingMode_!="yes"){

    std::vector<int> messageIDs;

    for(std::set<unsigned int>::iterator itr=fedcrates.begin(), itr_end=fedcrates.end(); itr!=itr_end; ++itr){

      //std::cout << "[PixelCalibrationBase::sendToFED]: sending to FED "<<std::endl;      
      int messageID=send(PixelFEDSupervisors_[(*itr)], action, flag, parametersToFED);
      //std::cout << "[PixelCalibrationBase::sendToFED]: done sending to FED messageID="<<messageID<<std::endl;      
      
      messageIDs.push_back(messageID);

      
    }
    for (std::vector<int>::iterator itr=messageIDs.begin(), itr_end=messageIDs.end(); itr!=itr_end; ++itr){

      //std::cout << "[PixelCalibrationBase::sendToFED]: wait for reply messageID="<<*itr<<std::endl;      
      flag=waitForReply(*itr);
      //std::cout << "[PixelCalibrationBase::sendToFED]: got reply"<<std::endl;      

    }
  }
  else if(sendingMode_=="yes"){
    for(std::set<unsigned int>::iterator itr=fedcrates.begin(), itr_end=fedcrates.end(); itr!=itr_end; ++itr){
      
      std::string reply=Send(PixelFEDSupervisors_[*itr],action, parametersToFED);
      
      if( reply.find("Done")==string::npos){
	
	diagService_->reportError("FED Calibration in FED crate# " + stringF(*(itr)) + " could not be used!",DIAGWARN);
      }
    }
  }
 
}

void PixelCalibrationBase::sendToFEC(std::string& action, Attribute_Vector parameters)
{
   bool flag=false;
 
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  std::set<unsigned int> feccrates=tempCalibObject->getFECCrates(theNameTranslation_, theFECConfiguration_);
  
  if(sendingMode_!="yes"){
    
    std::vector<int> messageIDs;

    for(std::set<unsigned int>::iterator itr=feccrates.begin(), itr_end=feccrates.end(); itr!=itr_end; ++itr){

      int messageID=send(PixelFECSupervisors_[(*itr)], action, flag, parameters);
      messageIDs.push_back(messageID);
    }
    for (std::vector<int>::iterator itr=messageIDs.begin(), itr_end=messageIDs.end(); itr!=itr_end; ++itr){
      flag=waitForReply(*itr);
    }
  }
  else if(sendingMode_=="yes"){
    
      for(std::set<unsigned int>::iterator itr=feccrates.begin(), itr_end=feccrates.end(); itr!=itr_end; ++itr){
	
	std::string reply=Send(PixelFECSupervisors_[*itr],action, parameters);
	if (reply.find("Done")==string::npos){
	  diagService_->reportError("Calib Running could not be done!",DIAGERROR);
	}
      }
    }
  

}  


void PixelCalibrationBase::sendToTKFEC(std::string& action, Attribute_Vector parameters)
{

  Supervisors::iterator i_PixelTKFECSupervisor;
  for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
    std::string reply = Send(i_PixelTKFECSupervisor->second, action,parameters);
    if (reply.find("Done")==string::npos){
      diagService_->reportError("Calib Running could not be done!",DIAGERROR);
    }
  }
  
  //////I should change this to be sendingMode-dependent....

}


