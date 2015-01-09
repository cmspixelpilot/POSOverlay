#ifndef _DBREADER_
#define _DBREADER_

#include <string>
#include <sstream>
#include <vector>

class TSQLServer;
class TSQLResult;

using namespace std;

class DBReader
{
  public:
                      	  DBReader();
  virtual             	  ~DBReader();
  TSQLResult *        	  getSelectionResult		  (void);
  int                 	  makeSelection 		  (int,string,string);
  void                    dumpSelectionResult             (void);
  
  int                 	  connectToDB	    (void);
  int                 	  closeDBConnection (void);
  int                 	  queryDB	    (void);

  private:

  string              	  dbName  ;
  string              	  account ;
  string              	  pword   ;
  stringstream        	  selection;
  TSQLServer*         	  db;
  TSQLResult*         	  selectionResult;	

};

#endif
