// $Id: PixelJobControlMonitor.cc,v 1.6 2009/09/03 09:32:32 joshmt Exp $

/**************************************************************************
 * Monitors the status of POS processes by talking to JobControl          *
 *                                                                        *
 * Author: Joshua Thompson, Cornell			 	          *
 *                                                                        *
 * Copyright 2009 Cornell University                                      *
 **************************************************************************/

/*
The function doCheck() does the actual checking of the JobControl via SOAP.
The function getHtml() returns a string of HTML that can be displayed on a
web page.

These two functions can be safely called in parallel threads.
However, it is not safe to call doCheck() in more than one thread at a time.
*/

#include "PixelUtilities/PixelJobControlUtilities/include/PixelJobControlMonitor.h"

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"

using namespace std;

PixelJobControlMonitor::PixelJobControlMonitor( xdaq::ApplicationContext* context) :
  context_(context),
  lock_(new toolbox::BSem(toolbox::BSem::FULL,true)),
  htmlSummary_("")
{
  Init();
}

PixelJobControlMonitor::~PixelJobControlMonitor()
{
  delete lock_;
}

string PixelJobControlMonitor::getHtml() const {
  //a more elegant solution would be a scoped lock
  lock_->take(); 
  string htmlSummary=htmlSummary_;
  lock_->give();
  return htmlSummary;
}

void PixelJobControlMonitor::doCheck( SOAPCommander* soapcommander) {
  //  cout<<" --- JobControl Monitor Check ---"<<endl;

  ostringstream html;

  for (vector<PixelJobControlNode>::iterator inode=nodes_.begin(); inode!=nodes_.end(); ++inode) {
    //    cout<<"[PixelJobControlMonitor::doCheck()] checking node..."<<endl;
    inode->checkStatus(soapcommander); //send SOAP to jobcontrol
    if ( inode->isJobControlOK() ) {
      vector<pair<string,unsigned int> > deadJobs = inode->getCrashedList();
      vector<pair<string,unsigned int> > aliveJobs = inode->getAliveList();

      if (deadJobs.size()==0) html<<"<font color=GREEN>";
      else html<<"<font color=RED>";

      html << inode->getName() << ": "<<aliveJobs.size()<<" jobs OK ; " <<deadJobs.size()<<" jobs crashed</font><br>";

      //      cout<<" --- dead jobs ---"<<endl;
      //      printList(deadJobs);
      //      cout<<" --- alive jobs ---"<<endl;
      //      printList(aliveJobs);
    }
    else if (inode->isJobControlInError() ) {
      html << "<font color=RED>" << inode->getName() << " had a bad SOAP reply from JobControl</font><br>";
    }
    else if (inode->isJobControlUnresponsive() ) {
      html << "<font color=RED>" << inode->getName() << " has unresponsive JobControl</font><br>";
    }
  }

  lock_->take();
  htmlSummary_ = html.str();
  lock_->give();

}

void PixelJobControlMonitor::printList( const vector<pair<string,unsigned int> > & list) {

  for ( vector<pair<string,unsigned int> >::const_iterator i = list.begin() ; i!=list.end(); ++i) 
    cout<<i->first<<" "<<i->second<<endl;
}

void PixelJobControlMonitor::Init() {
  /*
Get the jobcontrols defined in the Configuration.xml. For each machine, get a list of the processes on that machine.
Fill the vector nodes_ with PixelJobControlNode objects (one object per machine)
  */


  try {
    set<xdaq::ApplicationDescriptor*> jclist = context_->getDefaultZone()->getApplicationGroup("jc")->getApplicationDescriptors("jobcontrol");
    
    set<xdaq::ApplicationDescriptor*> daqlist;
    set<xdaq::ApplicationDescriptor*> dcslist;
    
    try { daqlist = context_->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptors(); }
    catch (...) {daqlist.clear();}
    try { dcslist = context_->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptors(); }
    catch (...) {dcslist.clear();}
    //    cout<<"daq list + dcs list = total"<<endl;     //DEBUG
    //    cout<<daqlist.size()<<" + "<<dcslist.size();    //DEBUG
    if (dcslist.size()>0)    daqlist.insert( dcslist.begin(), dcslist.end() );
    //    cout<<" = "<<daqlist.size()<<endl; //DEBUG

    for (set<xdaq::ApplicationDescriptor*>::const_iterator ijclist=jclist.begin(); ijclist!=jclist.end(); ++ijclist) {
      const string jcMachine = getUrlWithoutPort( *ijclist );

      PixelJobControlNode jcnode( *ijclist, jcMachine );

      cout<<"Found JobControl on machine = "<<jcMachine<<endl;
      for (set<xdaq::ApplicationDescriptor*>::const_iterator iapplist=daqlist.begin(); iapplist!=daqlist.end(); ++iapplist) {
	const string appMachine = getUrlWithoutPort( *iapplist );
	if (jcMachine == appMachine ) {
	  //get job id
	  string jid = getJid(*iapplist);
	  jcnode.add(jid,(*iapplist)->getClassName(),(*iapplist)->getInstance());
	  cout<<"Found "<<(*iapplist)->getClassName()<<" instance="<<(*iapplist)->getInstance()<<" with jobid: "<<jid<<endl;
	}
      }
      nodes_.push_back(jcnode);
    }
  }
  catch (xcept::Exception & e) {
    cout<<"[PixelJobControlMonitor::Init] Unexpected error! Caught an exception while making list of monitored processes: "<<e.what()<<endl;
  }

}

string PixelJobControlMonitor::getUrlWithoutPort( xdaq::ApplicationDescriptor *d) {
  string URL=d->getContextDescriptor()->getURL();
  int portDelimiter=   URL.find_last_of(':');
  return URL.substr(0,portDelimiter );
}

string PixelJobControlMonitor::getJid( xdaq::ApplicationDescriptor *d) {
  string URL=d->getContextDescriptor()->getURL();
  if (URL.at( URL.length()-1 ) != '/') {URL+="/"; }
  URL += "urn:xdaq-application:lid=0"; //I wish there was a better way to do this

  //  cout<<"jid = "<<URL<<endl;

  return URL;
}
