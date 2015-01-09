#include "DBReader.h"
#include <iostream>
#include <map>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>


//============================================================================
DBReader::DBReader()
{
//   dbName	  = "oracle://uscmsdb03.fnal.gov:1521/cmscald.fnal.gov";
//   account	  = "JOSHI";
//   pword 	  = "umesh_404";
  dbName	  = "oracle://int2r1-v.cern.ch:10121/int2r_lb.cern.ch";
  account	  = "CMS_PXL_CORE_PVSS_OWNER";
  pword 	  = "dam_ftT7";
  db		  = NULL;
  selectionResult = NULL;
  selection.str("");
}

//============================================================================
DBReader::~DBReader()
{
  closeDBConnection();
}

//============================================================================
int DBReader::connectToDB()
{
  string mthn = "[DBReader::connectToDB()]\t" ;
  cout << mthn << "Connecting to " << dbName << endl ;

  if(db == NULL || !db->IsConnected())
  {
    db = TSQLServer::Connect(dbName.c_str(),account.c_str(),pword.c_str());
  }
  
  if(db == NULL || !db->IsConnected()) 
  {
     cout << mthn << "Connection failed" << endl ;
     return 0;
  }
  
  cout << mthn << "Connection established" << endl ;
  return 1;
}
//============================================================================
int DBReader::makeSelection(int tempID ,string allDateBegin,string allDateEnd)
{
  selection.str("");
  selection << "SELECT to_char(CHANGE_DATE, 'MON DD HH24:MI:SS yyyy'), READINGS_CALIBRATEDTEMPERATURE FROM cms_pixel_temperature_siemens WHERE DPID=" << tempID <<" AND CHANGE_DATE>to_timestamp('"<< allDateBegin <<"', 'MON DD HH24:MI:SS yyyy') AND CHANGE_DATE<to_timestamp('"<< allDateEnd << "', 'MON DD HH24:MI:SS yyyy')";
  return 1;
}
//============================================================================
int DBReader::queryDB()
{
  string mthn = "[DBReader::queryDB()]\t" ;
  cout << mthn << "Quering database =>sel---" << selection.str() << "---" << endl ;
  selectionResult = db->Query(selection.str().c_str());
  if(selectionResult == NULL)
  {
    cout << mthn << "Your query:" << endl;
    cout << selection.str() << endl;
    cout << "was wrong!" << endl ;
    return 0;
  }
  else if(!selectionResult)
  {
    cout << mthn << "Oracle: Query " << selection.str() << " returned nothing" << endl ;
    return 0;
  }
  
  cout << mthn << "Query result retrieved" << endl ;
  return 1;
}
//============================================================================
int DBReader::closeDBConnection()
{
  if(db != NULL && db->IsConnected())
  {
    db->Close();
    return 1;
  }
  return 0;
}
//============================================================================
TSQLResult * DBReader::getSelectionResult()
{

  string mthn = "[DBReader::getSelectionResult()]\t" ;

  if(!connectToDB())
  {
    return NULL;
  }
  if(!queryDB())
  {
    return NULL;
  }
  return selectionResult;
}
//============================================================================
void DBReader::dumpSelectionResult()
{
  string mthn = "[DBReader::dumpSelectionResult()]\t" ;
  int nRows = selectionResult->GetRowCount();
  int nCols = selectionResult->GetFieldCount();
  cout << mthn << "Found " << nRows << " elements" << endl ;
  int nLines = 10;  
  if (nRows > 0)
  {
     TSQLRow* row = NULL;
     while ((row = selectionResult->Next()) && nLines > 0)
     {
       nLines--;
       cout << mthn;
       for(int col=0; col<nCols; col++)
       {
         cout << selectionResult->GetFieldName(col) << ": " << row->GetField(col) << " " ;
       }
       cout << endl;
     }
  }
  else
  {
    cout << "There are no entries for your selection!!!" << endl;
  }
}
