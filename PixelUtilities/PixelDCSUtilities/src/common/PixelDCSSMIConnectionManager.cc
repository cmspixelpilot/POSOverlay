// $Id: PixelDCSSMIConnectionManager.cc,v 1.2 2009/07/30 12:28:29 joshmt Exp $

/**************************************************************************
 * Class used to manage a text file holding a list of connections         *
 * to the PSX server. Used by:                                            *
 *   PixelDCSFSMInterface                                                 *
 *                                                                        *
 * Author: Joshua Thompson, Cornell			 	          *
 *                                                                        *
 **************************************************************************/

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSMIConnectionManager.h"

const std::string filename ="/tmp/psx_smi_connection_list";

using namespace std;

PixelDCSSMIConnectionManager::PixelDCSSMIConnectionManager() : outputFile_(0)
{
}

PixelDCSSMIConnectionManager::~PixelDCSSMIConnectionManager()
{
  delete outputFile_;
}

void PixelDCSSMIConnectionManager::disconnectOldConnections( PixelDCSSMICommander* smiCommander ) {

  vector<string> connectionList = getOpenConnections();
  vector<string> stillOpen;

  vector<string>::const_iterator connection = connectionList.begin();
  for ( ; connection != connectionList.end() ; ++connection) {
    try {
      cout<<"[PixelDCSSMIConnectionManager::disconnectOldConnections] about to disconnect psx id "
	  <<*connection<<endl;
      smiCommander->disconnectFromFSM( *connection );
    }
    //in the case where we try to disconnect from a connection that does not exists, it seems
    //that the server does NOT complain at all
    catch ( xdaq::exception::Exception& e ) {
      //this handles the case where the connection exists but we fail to close it
      stillOpen.push_back(*connection);
    }
  }

  //delete the file (unless we have failed to read anything from it)
  if (connectionList.size() >0) clearConnectionFile();

  vector<string>::const_iterator iOpen = stillOpen.begin();
  for ( ; iOpen != stillOpen.end() ; ++iOpen) {
    cout<<"[PixelDCSSMIConnectionManager::disconnectOldConnections] failed to close connection so I will keep it open "<<*iOpen<<endl;
    addOpenConnection(*iOpen);
  }

}

void PixelDCSSMIConnectionManager::addOpenConnection( const string connectionId ) {

  //for now we'll do this the easy, wasteful way -- open and close the file each and every time

  if (outputFile_ == 0 ) {
    outputFile_ = new ofstream( filename.c_str() , ios::app);
    if (outputFile_==0 || !(outputFile_->is_open()) ) {
      cout<<"Failed to open file for output!"<<endl;
      return; //FIXME need better error recovery
    }
  }

  //  cout<<"writing to file connectionID = "<<connectionId<<endl; //DEBUG

  (*outputFile_)<<connectionId<<endl;

  //if we implement the logic in PixelDCSFSMInterface to know when to close the file,
  //then we can move this to a separate function
  outputFile_->close();
  delete outputFile_;
  outputFile_=0;

}

vector<string> PixelDCSSMIConnectionManager::getOpenConnections() {

  vector<string> connectionList;

  ifstream inputFile( filename.c_str() );

  //if we fail to open the file, we just won't close the open connections
  if ( !(inputFile.is_open()) ) {
    cout<<"[PixelDCSSMIConnectionManager::getOpenConnections] FAILED TO OPEN Connection FILE!"<<endl; //DEBUG
    return connectionList;
  }

  string connectionId;
  while (inputFile>>connectionId) {
    if (connectionId.length() > 0)  connectionList.push_back(connectionId);
  }

  inputFile.close();

  return connectionList;
}

void PixelDCSSMIConnectionManager::clearConnectionFile() {
  cout<<"[PixelDCSSMIConnectionManager::clearConnectionFile] Deleting connection file!"<<endl;  //DEBUG

  if (remove(filename.c_str())==0) return;

  cout<<"[PixelDCSSMIConnectionManager::clearConnectionFile] Had a problem deleting the connection file!"<<endl;

}
