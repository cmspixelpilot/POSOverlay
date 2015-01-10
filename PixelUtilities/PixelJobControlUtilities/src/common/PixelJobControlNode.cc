// $Id: PixelJobControlNode.cc,v 1.5 2009/09/03 09:32:32 joshmt Exp $

/**************************************************************************
 *                                                                        *
 * Author: Joshua Thompson, Cornell			 	          *
 * Copyright 2009 Cornell University                                      *
 **************************************************************************/

#include "PixelUtilities/PixelJobControlUtilities/include/PixelJobControlNode.h"

#include "xoap/MessageReference.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

using namespace std;

PixelJobControlNode::PixelJobControlNode(xdaq::ApplicationDescriptor* d, string machineName) :
  jc_(d),
  machineName_(machineName),
  jcstatus_("undefined")
{
}

PixelJobControlNode::~PixelJobControlNode()
{
}

void PixelJobControlNode::add( string jid, string name, unsigned int instance) {
  jidmap_[jid]="unknown";
  jidToName_[jid] = make_pair(name,instance);
}

bool PixelJobControlNode::isJobControlOK() const {
  if ( jcstatus_ == "responsive") return true;
  return false;
}

bool PixelJobControlNode::isJobControlInError() const {
  if ( jcstatus_ == "error") return true;
  return false;
}

bool PixelJobControlNode::isJobControlUnresponsive() const {
  if ( jcstatus_ == "unresponsive") return true;
  return false;
}

vector<pair<string,unsigned int> > PixelJobControlNode::getListNotInState(const string &state) const {

  vector<pair<string,unsigned int> > matchingJobs;

  for (map<string,string>::const_iterator ijid=jidmap_.begin(); ijid != jidmap_.end(); ++ijid) {
    if (ijid->second!=state) matchingJobs.push_back(jidToName_.find(ijid->first)->second);
  }

  return matchingJobs;
}

vector<pair<string,unsigned int> > PixelJobControlNode::getListInState(const string &state) const {

  vector<pair<string,unsigned int> > matchingJobs;

  for (map<string,string>::const_iterator ijid=jidmap_.begin(); ijid != jidmap_.end(); ++ijid) {
    if (ijid->second==state) matchingJobs.push_back(jidToName_.find(ijid->first)->second);
  }
  return matchingJobs;
}

vector<pair<string,unsigned int> >  PixelJobControlNode::getCrashedList() const {
  return getListNotInState("S");
}

vector<pair<string,unsigned int> >  PixelJobControlNode::getAliveList() const {
  return getListInState("S");
}

void PixelJobControlNode::checkStatus( SOAPCommander* soapcommander) {
      
  for (map<string,string>::iterator ijid=jidmap_.begin(); ijid != jidmap_.end(); ++ijid) {
    xoap::MessageReference jcreply ;
    
    //cout<<" -- starting --"<<endl;
    Attribute_Vector parameters(1); 
    parameters[0].name_="jid";
    parameters[0].value_=ijid->first;
    try {
      //      cout<<" -- sending SOAP to JobControl! -- "<<endl;
      jcreply=soapcommander->SendWithSOAPReply(jc_ , "getJobStatus",parameters);
      //      cout<<" -- SOAP reply from JobControl -- "<<endl;
      //      jcreply->writeTo(std::cout); cout<<endl;
      //      cout<<" -- end of SOAP reply from JobControl -- "<<endl;
      jcstatus_="responsive";
    }
    catch (xcept::Exception & e) {
      cout<<"Failure talking to node: "<<jc_->getContextDescriptor()->getURL()<<endl;
      jcstatus_="unresponsive";
    }

    if (jcstatus_=="responsive") {
      xoap::SOAPEnvelope responseEnvelope = jcreply->getEnvelope();
      xoap::SOAPBody responseBody = responseEnvelope.getBody();
      if ( !responseBody.hasFault() ) {
	
	//	xoap::SOAPName commandElement = responseEnvelope.createName("state");
	std::vector<xoap::SOAPElement> bodyElements = responseBody.getChildElements();
	for ( vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	      bodyElement != bodyElements.end(); ++bodyElement ) {

	  //std::vector<xoap::SOAPElement> myBodyElements = bodyElement->getChildElements(commandElement);

	  xoap::SOAPName sname("state", "", "");
	  std::vector<xoap::SOAPElement> myBodyElements = bodyElement->getChildElements(sname);
	  ijid->second = myBodyElements.at(0).getValue();
	  //	  cout<<ijid->first<<" state = "<<ijid->second<<endl;
	}
      }
      else {cout<<"SOAP response from JobControl has a fault!"<<endl; jcstatus_="error";}
    }
    //    else {cout<<"Not responsive!"<<endl;} //FIXME
    // cout<<" -- ending --"<<endl;
  }
}
