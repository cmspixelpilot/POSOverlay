#ifndef PixelSQLCommand_h
#define PixelSQLCommand_h
/*
$Author: rovere $
$Date: 2009/06/15 12:57:50 $
$Revision: 1.15 $
*/

#include "PixelConfigDBInterface/include/PixelDatabaseUtilities.h"
#include <occi.h>
#include <sstream>
#include <vector>
#include <map>
#include <string>

using namespace std;

class PixelOracleDatabase;

class PixelSQLCommand {
 public:
  PixelSQLCommand(PixelOracleDatabase &db);
  PixelSQLCommand(PixelOracleDatabase *pdb);
  ~PixelSQLCommand();
  operator bool () { return error(); }
  bool error() {
    return m_bError;
  }
  void init();
  PixelSQLCommand& startOver();  
  PixelSQLCommand& commit();
  void createStatement();
  void terminateStatement();
  void reCreateStatement();         

  template<class T> PixelSQLCommand& operator<<(const T &t) {
    command_ << t;
    return *this;
  }
  PixelSQLCommand& operator<<(const char* str) {
    command_ << str;
    return *this;
  }
  PixelSQLCommand& operator<<(PixelSQLCommand& (*manipulator)(PixelSQLCommand&)) {
    return manipulator(*this);
  } 
  void getField(int index, std::string &str);  
  void getField(int index, int &i);     
  std::string getStringField(int index);   
  int  getIntField(int index);  
  void getField(int index, oracle::occi::Blob &blob);   
  void getField(int index, oracle::occi::Clob &clob);   
  void setField(int index, std::string &str);  
  void setField(int index, int i); 
  void setField(int index, oracle::occi::Blob &blob); 
  unsigned int getNumArrayRows() ;
  PixelSQLCommand& setSql(); 
  PixelSQLCommand& exec(bool bAutoCommit=true);
  PixelSQLCommand& execUpdate(); 
  PixelSQLCommand& openTable(const char* table_name, std::map<std::string ,std::string> &where , bool bForUpdate ,bool like, string orderByColumn="", bool ordered=false );  
  PixelSQLCommand& openTable(const char* table_name, const char* where=0, bool bForUpdate=false); 
  PixelSQLCommand& openTable(const char* table_name, const std::string &where, bool bForUpdate=false); 
  PixelSQLCommand& openTable(const char* table_name, const std::ostringstream &where, bool bForUpdate=false); 
  PixelSQLCommand& openConditionDataAuditlog(std::string fileToSearchFor) ;
  PixelSQLCommand& openVersionAliasTable() ;
  PixelSQLCommand& openVersionAliasTable(std::string) ;
  PixelSQLCommand& openExistingVersionsTable(string koc) ;
  PixelSQLCommand& openKeyAliasKeyVersions() ;
  PixelSQLCommand& getNumberOfTotalCfgs() ;
  PixelSQLCommand& getNumberOfTotalKeyAliases();
  PixelSQLCommand& getTotalCfgs() ;
  int getNumberOfColumns();
  std::string getNameOfColumns(int index); 
  vector<string> getNamesOfColumns(int numCols) ;
  int loadRow();  
  int loadRow(int rows);  
  void setPrefetch(unsigned int prefetch);
  void setDataBuffer(int index, char * buffer, int size) ;
  int getNextAvailableVersion(std::string sequence="default") ;
  void dumpTableInfo() ;
  void fillTnsKocViewDictionary() ;
  
 private:
  
  PixelOracleDatabase *myPdb_;     
  std::ostringstream command_;   
  oracle::occi::Connection* connection_;  
  oracle::occi::Statement* statement_;   
  oracle::occi::ResultSet* result_;   
  std::vector<oracle::occi::MetaData> metadata_;  
  bool m_bSQLAlreadySet;
  bool m_bError;
  string printType (int type) ;
  string tnsOracleName_ ;
  map<string, map<string, string> > tnsKocViewDictionary_ ;
};

#endif
