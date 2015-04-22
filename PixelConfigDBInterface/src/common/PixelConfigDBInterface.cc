/*
$Author: kreis $
$Date: 2012/01/19 16:44:26 $
$Revision: 1.94 $
*/
#include "xdaq/exception/Exception.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTimeFormatter.h"
#include "PixelConfigDBInterface/include/PixelConfigDBInterface.h"
#include <iomanip>
#include <occi.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

struct timeval start_time, end_time;
int    t_stat ;

//=====================================================================================
PixelConfigDBInterface::PixelConfigDBInterface(){
  timeToDecode_ = 0 ;
  comment_ = "" ;
}

//=====================================================================================
PixelConfigDBInterface::~PixelConfigDBInterface(){
  ;
}

//=====================================================================================
bool PixelConfigDBInterface::isConnected(){
  return database_.getNumberOfConnections();
}

//=====================================================================================
bool PixelConfigDBInterface::connect(){
  if(!isConnected()){
//    cout << __LINE__ << "]\t[PixelConfigDBInterface::connect()]\t\tTrying to connect to ..." << endl ;
    return database_.connect();
  }
//  cout << __LINE__ << "]\t[PixelConfigDBInterface::connect()]\t\tAlready connected..." << endl ;
  return true;
}

//=====================================================================================
void PixelConfigDBInterface::disconnect(){
  if(isConnected()){
    database_.disconnect();
  }
}

//=====================================================================================
void PixelConfigDBInterface::getAllPixelDACSettingsTable(vector< vector<string> >&  table, 
                                                      pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getAllPixelDACSettingsTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map<string , string  >  where;
  stringstream clause ;
  // modified by MR on 25-02-2008 09:13:16
  // COLUMN WISE ROC DAC SETTINGS TABLE

  //  Name                                         Null?       Type
  //------                                                      
  //  CONFIG_KEY                                  NOT NULL    NUMBER(38)
  //  CONFG_KEY                                      NOT NULL    VARCHAR2(80)
  //  VERSION                                                    VARCHAR2(40)
  //  KIND_OF_COND                                   NOT NULL    VARCHAR2(40)
  //  ROC_NAME                                                   VARCHAR2(187)
  //  HUB_ADDRS                                                  NUMBER(38)
  //  PORT_NUMBER                                                NUMBER(10)
  //  I2C_ADDR                                                   NUMBER
  //  GEOM_ROC_NUM                                               NUMBER(10)
  //  VDD                                            NOT NULL    NUMBER(38)
  //  VANA                                           NOT NULL    NUMBER(38)
  //  VSF                                            NOT NULL    NUMBER(38)
  //  VCOMP                                          NOT NULL    NUMBER(38)
  //  VLEAK                                          NOT NULL    NUMBER(38)
  //  VRGPR                                          NOT NULL    NUMBER(38)
  //  VWLLPR                                         NOT NULL    NUMBER(38)
  //  VRGSH                                          NOT NULL    NUMBER(38)
  //  VWLLSH                                         NOT NULL    NUMBER(38)
  //  VHLDDEL                                        NOT NULL    NUMBER(38)
  //  VTRIM                                          NOT NULL    NUMBER(38)
  //  VCTHR                                          NOT NULL    NUMBER(38)
  //  VIBIAS_BUS                                     NOT NULL    NUMBER(38)
  //  VIBIAS_SF                                      NOT NULL    NUMBER(38)
  //  VOFFSETOP                                      NOT NULL    NUMBER(38)
  //  VBIASOP                                        NOT NULL    NUMBER(38)
  //  VOFFSETRO                                      NOT NULL    NUMBER(38)
  //  VION                                           NOT NULL    NUMBER(38)
  //  VIBIAS_PH                                      NOT NULL    NUMBER(38)
  //  VIBIAS_DAC                                     NOT NULL    NUMBER(38)
  //  VIBIAS_ROC                                     NOT NULL    NUMBER(38)
  //  VICOLOR                                        NOT NULL    NUMBER(38)
  //  VNPIX                                          NOT NULL    NUMBER(38)
  //  VSUMCOL                                        NOT NULL    NUMBER(38)
  //  VCAL                                           NOT NULL    NUMBER(38)
  //  CALDEL                                         NOT NULL    NUMBER(38)
  //  TEMPRANGE                                      NOT NULL    NUMBER(38)
  //  WBC                                            NOT NULL    NUMBER(38)
  //  CHIPCONTREG                                    NOT NULL    NUMBER(38)

  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str() ;

  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << "]\t" << mthn << "Requested key    = " << clause.str() << "\t" << key.key() << std::endl ;
    }
    
  que.setPrefetch(50000) ;
  que.openTable("dac",where,false ,false, "ROC_NAME", true);
  
  getGeneralTable(table , que ) ;
//   dumpGeneralTable(table) ;
}//end getAllPixelDACSettingsTable

//=====================================================================================
void PixelConfigDBInterface::getPixelDACSettingsTable(vector< vector<string> >&  table, 
                                                      string module,
                                                      pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelDACSettingsTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map<string , string  >  where;
  stringstream clause ;
  // modified by MR on 25-02-2008 09:13:16
  // COLUMN WISE ROC DAC SETTINGS TABLE

  //  Name                                         Null?       Type
  //------                                                      
  //  CONFIG_KEY                                  NOT NULL    NUMBER(38)
  //  CONFG_KEY                                      NOT NULL    VARCHAR2(80)
  //  VERSION                                                    VARCHAR2(40)
  //  KIND_OF_COND                                   NOT NULL    VARCHAR2(40)
  //  ROC_NAME                                                   VARCHAR2(187)
  //  HUB_ADDRS                                                  NUMBER(38)
  //  PORT_NUMBER                                                NUMBER(10)
  //  I2C_ADDR                                                   NUMBER
  //  GEOM_ROC_NUM                                               NUMBER(10)
  //  VDD                                            NOT NULL    NUMBER(38)
  //  VANA                                           NOT NULL    NUMBER(38)
  //  VSF                                            NOT NULL    NUMBER(38)
  //  VCOMP                                          NOT NULL    NUMBER(38)
  //  VLEAK                                          NOT NULL    NUMBER(38)
  //  VRGPR                                          NOT NULL    NUMBER(38)
  //  VWLLPR                                         NOT NULL    NUMBER(38)
  //  VRGSH                                          NOT NULL    NUMBER(38)
  //  VWLLSH                                         NOT NULL    NUMBER(38)
  //  VHLDDEL                                        NOT NULL    NUMBER(38)
  //  VTRIM                                          NOT NULL    NUMBER(38)
  //  VCTHR                                          NOT NULL    NUMBER(38)
  //  VIBIAS_BUS                                     NOT NULL    NUMBER(38)
  //  VIBIAS_SF                                      NOT NULL    NUMBER(38)
  //  VOFFSETOP                                      NOT NULL    NUMBER(38)
  //  VBIASOP                                        NOT NULL    NUMBER(38)
  //  VOFFSETRO                                      NOT NULL    NUMBER(38)
  //  VION                                           NOT NULL    NUMBER(38)
  //  VIBIAS_PH                                      NOT NULL    NUMBER(38)
  //  VIBIAS_DAC                                     NOT NULL    NUMBER(38)
  //  VIBIAS_ROC                                     NOT NULL    NUMBER(38)
  //  VICOLOR                                        NOT NULL    NUMBER(38)
  //  VNPIX                                          NOT NULL    NUMBER(38)
  //  VSUMCOL                                        NOT NULL    NUMBER(38)
  //  VCAL                                           NOT NULL    NUMBER(38)
  //  CALDEL                                         NOT NULL    NUMBER(38)
  //  TEMPRANGE                                      NOT NULL    NUMBER(38)
  //  WBC                                            NOT NULL    NUMBER(38)
  //  CHIPCONTREG                                    NOT NULL    NUMBER(38)

  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str() ;
//   where["ROC_NAME"]= clause.str() ;

// 01234567890
// pixel/dac/
  module.erase(0,10) ;
  where["ROC_NAME"] = module ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << "]\t" << mthn << "Requested module = " << module << std::endl ;
      std::cout << __LINE__ << "]\t" << mthn << "Requested key    = " << clause.str() << "\t" << key.key() << std::endl ;
    }
    
  que.setPrefetch(50000) ;
  que.openTable("dac",where,false ,true, "ROC_NAME", true);
  
  getGeneralTable(table , que ) ;
//   dumpGeneralTable(table) ;
}//end getPixelDACSettingsTable

//=====================================================================================
void PixelConfigDBInterface::getPixelTrimBitsTable(vector< vector<string> >&  table,
                                                   string module,                   
                                                   pos::PixelConfigKey key)
{           
  std::string mthn = "[PixelConfigDBInterface::getPixelTrimBitsTable()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

  /**   // modified by MR on 18-06-2008 09:18:48
        CONF_KEY_ROC_TRIMS_V
        Name                                      Null?    Type
        ----------------------------------------- -------- ----------------------------
        CONFIG_KEY                                         NUMBER(38)
        CONFG_KEY                                          VARCHAR2(80)
        VERSION                                            VARCHAR2(40)
        KIND_OF_COND                                       VARCHAR2(40)
        ROC_NAME                                           VARCHAR2(187)
        HUB_ADDRS                                          NUMBER(38)
        PORT_NUMBER                                        NUMBER(10)
        ROC_I2C_ADDR                                       NUMBER
        GEOM_ROC_NUM                                       NUMBER(10)
        DATA_FILE                                          VARCHAR2(200)
        TRIM_CLOB                                          CLOB
  */
  //where["ROC_NAME"] = module+"_PLQ2" ;
   
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ;//clause.str() ;
//             1
//   01234567890
//   pixel/trim/
  module.erase(0,11) ;
  where["ROC_NAME"] = module ;

  que.openTable("trim",where,false ,true);

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Starting to get data for module: " << where["ROC_NAME"] << endl;
    }

  int nCol= que.getNumberOfColumns();
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Number of Columns:" << nCol << endl;
    }
  //cout<<"Max Size"<<col.max_size()<<endl;
  //cout<<"Max Size"<<pDSMa.max_size()<<endl;
  table.clear();
  //THIS MUST BE CHANGED WITH MEMORY SIZE TO AVOID HAVING TO BIG CHUNKS OF DATA
  que.setPrefetch(50000); // number of rows to prefetch

  for(int z = 1 ; z <= nCol ; z++)
    {
      col.push_back(que.getNameOfColumns(z));
    }
  table.push_back(col);

  int blobcnt = 0;
  gettimeofday(&start_time, (struct timezone *)0 );

  while(que.loadRow())
    {   
      col.clear();
      for(int c =1 ; c <= nCol ; c++ )
        {
        if(table[0][c-1] != "TRIM_CLOB")
          {
            col.push_back(que.getStringField(c));
            //cout<< __LINE__ << "]\t" << mthn << "Im passing thorugh column:"<<c<<endl;
          }
        else // CLOB decoding
          {
            oracle::occi::Clob clob ;
            que.getField(c, clob) ;
            oracle::occi::Stream * stream = clob.getStream(1,0) ;
//          cout << "Clob length(): " <<clob.length() << endl ;
            char * buffer = (char *)malloc(clob.length()*sizeof(char)+1); 
            memset(buffer, 0, clob.length()) ;
            buffer[clob.length()] = '\000';
            stream->readBuffer(buffer, clob.length()) ;
            col.push_back(buffer) ;
            free(buffer) ;
            //      cout << "Read buffer: |" << buffer << "|" << endl ;
            
            //      unsigned char * buffer = (unsigned char *)malloc(clob.length()*sizeof(char)); 
            //      clob.read(clob.length()/sizeof(char), buffer, clob.length()/sizeof(char), 1) ;
            /** Renaissance TRIMS ENCODING-DECODING
            stringstream sstrimbit;
            std::string trimbits;
            oracle::occi::Blob blob;
            que.getField(c , blob);
            if(blob.isNull())
              {
                cerr << mthn << "Null Blob" <<endl;
              }
            else
              {
                blob.open(oracle::occi::OCCI_LOB_READONLY);
                
                unsigned int headerLength ;
                unsigned int headerLengthSize = sizeof(unsigned int);
                blob.read(headerLengthSize,(unsigned char*)(&headerLength),headerLengthSize,1);
                cout << "Length: " << headerLength << endl;
                
                unsigned char numberOfElementsLeftChar[headerLength];
                blob.read(headerLength,numberOfElementsLeftChar,headerLength,1+headerLengthSize+headerLength);
//              cout << "NUMBEROFELEMENTSLEFT: " << numberOfElementsLeftChar << endl;
                
                string numberOfElementsLeftString = (const char*)numberOfElementsLeftChar;
                
                int semiColPos = numberOfElementsLeftString.find(":");
                int numberOfElementsLeft = atoi(numberOfElementsLeftString.substr(semiColPos+1,headerLength-semiColPos).c_str());
                cout << "NUMBER: " << numberOfElementsLeft << endl;
                
                //char val;
                int ROCNumberOfColumns = 52;
                int ROCNumberOfRows    = 80;
                
                const unsigned int bufSize=ROCNumberOfColumns*ROCNumberOfRows;
                unsigned char buffer[bufSize];
                unsigned int  readAmt = bufSize;
                unsigned int  offset  = 1+headerLengthSize+(numberOfElementsLeft+2)*headerLength;
                //     
//                         __ SYNTAX __
//                      unsigned int read(unsigned int amt,
//                      unsigned char *buffer,
//                      unsigned int bufsize,
//                      unsigned int offset = 1) const;
                        
//                      __ Parameters __
//                      amt:
//                      The number of consecutive bytes to be read from the BLOB.
//                      buffer:
//                      The buffer into which the BLOB data is to be read.
//                      bufsize:
//                      The size of the buffer. Valid values are: Numbers greater than or equal to amount.
//                      offset:
//                      The starting position at which to begin reading data from the BLOB. 
//                      If offset is not specified, the data is written from the beginning of the BLOB.
//                      Valid values are: Numbers greater than or equal to 1.

                blob.read(readAmt,buffer,bufSize,offset);
                
                //unsigned char trimbit[2080];
                //unsigned int y = 0 ;
                //int z = 0;
                for(unsigned int x = 1 ; x < bufSize ; x = x+2 )
                  {
                    // sstrimbit.flags(ios::hex);
                    // modified by MR on 18-06-2008 11:01:51
                    // Prende i trims della posizione x-1, quelli della posizione x 
                    // e li unisce in un unico char.
                    unsigned char a = ((buffer[x-1]<<4) | (buffer[x]&0x0f));
                    //            cout << "a:\t" << a << " hex: 0x";
                    //            for (int i=2*sizeof(char) - 1; i>=0; i--) {
                    //              cout << "0123456789ABCDEF"[((a >> i*4) & 0xF)];
                    //            }
                    //            cout << endl ;
                    
                    //            cout << "a:\t" << a << " binary:   ";
                    //            for( int i=sizeof(char)*8-1 ;i>=0; i--)
                    //              {
                    //                if(a&(1<<i)) cout << 1 ;
                    //                else cout << 0 ;
                    //              }
                    //            cout << endl ;
                    
                    sstrimbit<<a;
                    //trimbit[z] = ((buffer[y]<<4) | (buffer[x]&0x0f));
                    //y=y+2;
                    //z++;
                  }
                //sstrimbit<< trimbit;
                //              trimbits = sstrimbit.str() ;
                //              cout<<trimbits.size()<<endl;
                //              cout.flags(ios::hex);
                //              cout<<(short)trimbit[0]<<"\t"<<(short)trimbit[1]<<endl; 
                //              cout.flags(ios::hex);
                //              cout<<(short)buffer[0]<<"\t"<<(short)buffer[1]<<"\t"<<(short)buffer[2]<<"\t"<<(short)buffer[3]<<"\t";
                //                 unsigned char x[1] ;
                //              cout.flags(ios::hex);
                //x[0]= ((buffer[0]<<4) | (buffer[1]&0x0f));
                //cout<<((short)x[0])<<endl; 
                trimbits = sstrimbit.str();
                cout << "TrimBits.size():  " << trimbits.size() << endl;
                cout << "TrimBits:        |" << trimbits        << "|" <<endl;
                
                blob.close();
              } //end if ... else ...
            col.push_back(trimbits);
            */
          } // end of CLOB decoding
        
        }   // end loop over columns in a single row
      
      table.push_back(col);
      blobcnt++;
    } // end loop over retrived records
  gettimeofday(&end_time, (struct timezone *)0 );
  int total_usecs = (end_time.tv_sec - start_time.tv_sec)*1000000 + (end_time.tv_usec - start_time.tv_usec);
  if(DEBUG_CONFIG_DB)
    {  
      cout << __LINE__ << "]\t" << mthn                                                             << endl;
      cout << __LINE__ << "]\t" << mthn << "Number of BLOBs: " << blobcnt                           << endl;
      cout << __LINE__ << "]\t" << mthn << "Time taken     : " << total_usecs / 1000000  << " secs" << endl;
      cout << __LINE__ << "]\t" << mthn << "Table Size     : " << table.size()                      << endl;
    }
}// getPixelTrimBitsTable
 
//=====================================================================================
void PixelConfigDBInterface::getPixelMaskBitsTable(vector< vector<string> >&  table,
                                                   string module,                   
                                                   pos::PixelConfigKey key)         
{
  std::string mthn = "[PixelConfigDBInterface::getPixelMaskBitsTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  int clobcnt = 0;

  /**
     View's name:    CONF_KEY_ROC_MASKS_V
     CONFIG_KEY                                      NUMBER(38)
     CONFG_KEY                                          VARCHAR2(80)
     VERSION                                            VARCHAR2(40)
     KIND_OF_COND                                       VARCHAR2(40)
     ROC_NAME                                           VARCHAR2(187)
     HUB_ADDRS                                          NUMBER(38)
     PORT_NUMBER                                        NUMBER(10)
     ROC_I2C_ADDR                                       NUMBER
     GEOM_ROC_NUM                                       NUMBER(10)
     DATA_FILE                                          VARCHAR2(200)
     MASK_CLOB                                          CLOB
   */

   
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str() ;
//             11
//   012345678901
//   pixel/mask/
  module.erase(0,11) ;
  where["ROC_NAME"] = module ;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Starting to get data for module: " << where["ROC_NAME"] << endl;
    }

  que.openTable("mask",where,false ,true);

  int nCol= que.getNumberOfColumns();
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Number of Columns:" << nCol << endl;
    }
  //cout<<"Max Size"<<col.max_size()<<endl;
  //cout<<"Max Size"<<pDSMa.max_size()<<endl;
  table.clear();
  //THIS MUST BE CHANGED WITH MEMORY SIZE TO AVOID HAVING TO BIG CHUNKS OF DATA

  for(int z = 1 ; z <= nCol ; z++)
    {
      col.push_back(que.getNameOfColumns(z));
    }
  table.push_back(col);

  gettimeofday(&start_time, (struct timezone *)0 );

  while(que.loadRow())
    {   
      col.clear();
      for(int c =1 ; c <= nCol ; c++ )
        {
        if(table[0][c-1] != "MASK_CLOB")
          {
            col.push_back(que.getStringField(c));
//            cout<< __LINE__ << "]\t" << mthn << "Im passing thorugh column:"<<c<<endl;
          }
        else // CLOB decoding
          {
            oracle::occi::Clob clob ;
            que.getField(c, clob) ;
//          oracle::occi::Stream * stream = clob.getStream(1,0) ;
//          cout << "Clob length()  : " << clob.length()       << endl ;
//          cout << "Clob Chunk Size: " << clob.getChunkSize() << endl ;
//          char * buffer = (char *)malloc(clob.length()*sizeof(char)+1);
            unsigned char * buffer = (unsigned char *)malloc(clob.length()*sizeof(char)+1);
            memset(buffer, 0, clob.length()) ;
            buffer[clob.length()] = '\000';
            clob.read(clob.length(), buffer, clob.length(), 1) ;
//          stream->readBuffer(buffer, clob.length()) ;
            col.push_back((char*)buffer) ;
            free(buffer) ;
          } // end of CLOB decoding
        }   // end loop over columns in a single row
      table.push_back(col);
      clobcnt++;
    } // end loop over retrived records
  
  gettimeofday(&end_time, (struct timezone *)0 );
  timeToDecode_ += (end_time.tv_sec - start_time.tv_sec)*1000000 + (end_time.tv_usec - start_time.tv_usec);
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << endl;
      cout << __LINE__ << "]\t" << mthn << "Number of CLOBs : " << clobcnt                              << endl;
      cout << __LINE__ << "]\t" << mthn << "Total Time taken: " << timeToDecode_ / 1000000.  << " secs" << endl;
      cout << __LINE__ << "]\t" << mthn << "Table Size      : " << table.size()                         << endl;
    }
}// getPixelMaskBitsTable

//=====================================================================================
void PixelConfigDBInterface::getPixelMaxVSFConfig    (vector< vector<string> >& table, string module, pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelMaskBitsTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map<string , string  >  where;
  stringstream clause ;

  clause << key.key() ;
  where["KEY_ALIAS_ID"] = theTrueDBKey_ ; //clause.str() ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << "]\t" << mthn << "Requested key    = " << clause.str() << "\t" << key.key() << std::endl ;
    }
    
  que.setPrefetch(40000) ;
  que.openTable("maxvsf",where,false ,true);
  
  getGeneralTable(table , que ) ;

}

//=====================================================================================
std::string PixelConfigDBInterface::getPixelCalibConfig(vector< vector<string> >& table, string module, pos::PixelConfigKey key)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getPixelCalibConfig()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn <<std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  int clobcnt = 0;

  /**
     View's name:    CONF_KEY_PIXEL_CALIB_V
          Name                                     Null?    Type

       0 CONFIG_KEY                                NOT NULL VARCHAR2(80)
       1 KEY_TYPE                                  NOT NULL VARCHAR2(80)
       2 KEY_ALIAS                                 NOT NULL VARCHAR2(80)
       3 KEY_ALIAS_ID                              NOT NULL NUMBER(38)
       4 VERSION                                            VARCHAR2(40)
       5 KIND_OF_COND                              NOT NULL VARCHAR2(40)
       6 CALIB_TYPE                                         VARCHAR2(200)
       7 CALIB_OBJ_DATA_FILE                       NOT NULL VARCHAR2(200)
       8 CALIB_OBJ_DATA_CLOB                       NOT NULL CLOB
  */

   
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str() ;

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "Starting to get data" << endl ; 
    }

  que.openTable("calib",where,false ,false);

  int nCol= que.getNumberOfColumns();
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "Number of Columns:" << nCol << endl;
    }
  //cout<<"Max Size"<<col.max_size()<<endl;
  //cout<<"Max Size"<<pDSMa.max_size()<<endl;
  table.clear();
  //THIS MUST BE CHANGED WITH MEMORY SIZE TO AVOID HAVING TO BIG CHUNKS OF DATA
  que.setPrefetch(50000); // number of rows to prefetch

  for(int z = 1 ; z <= nCol ; z++)
    {
      col.push_back(que.getNameOfColumns(z));
    }
  table.push_back(col);

  while(que.loadRow())
    {   
      col.clear();
      for(int c =1 ; c <= nCol ; c++ )
        {
        if(table[0][c-1] != "CALIB_OBJ_DATA_CLOB")
          {
            col.push_back(que.getStringField(c));
            //cout<< __LINE__ << mthn << "Im passing thorugh column:"<<c<<endl;
          }
        else // CLOB decoding
          {
            oracle::occi::Clob clob ;
            que.getField(c, clob) ;
            oracle::occi::Stream * stream = clob.getStream(1,0) ;
//          cout << "Clob length(): " <<clob.length() << endl ;
            char * buffer = (char *)malloc(clob.length()*sizeof(char)+1); 
            memset(buffer, 0, clob.length()) ;
            stream->readBuffer(buffer, clob.length()) ;
            buffer[clob.length()] = '\000' ;
            if(DEBUG_CONFIG_DB)
              { 
                cout << __LINE__ << mthn << endl ;
                cout << __LINE__ << mthn << buffer << endl ;
                cout << __LINE__ << mthn << endl ;
              }
            col.push_back(buffer) ;
            free(buffer) ;
          } // end of CLOB decoding
        }   // end loop over columns in a single row
      table.push_back(col);
      clobcnt++;
    } // end loop over retrived records
  
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "Table Size:" << table.size() << std::endl;
    }
  
  if(table.size() == 1) return "NoCalibObject" ;

  return table[1][6] ;
}

//=====================================================================================
void PixelConfigDBInterface::getPixelTTCciConfig(vector< vector<string> >& table, string module, pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelTTCciConfig()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  int clobcnt = 0;

  /**
     View's name:    CONF_KEY_TTC_CONFIG_V
     CONFIG_KEY                             NOT NULL NUMBER(38)
     CONFG_KEY                                 NOT NULL VARCHAR2(80)
     VERSION                                            VARCHAR2(40)
     KIND_OF_COND                              NOT NULL VARCHAR2(40)
     RUN_TYPE                                           VARCHAR2(40)
     RUN_NUMBER                                         NUMBER(38)
     TTC_OBJ_DATA_FILE                         NOT NULL VARCHAR2(200)
     TTC_OBJ_DATA_CLOB                         NOT NULL CLOB
  */
   
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str() ;
//             111111111
//   0123456789012345678
//   pixel/ttcciconfig/

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Starting to get data " << endl ;
    }

  que.openTable("ttcciconfig",where,false ,false);

  int nCol= que.getNumberOfColumns();
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Number of Columns:" << nCol << endl;
    }
  //cout<<"Max Size"<<col.max_size()<<endl;
  //cout<<"Max Size"<<pDSMa.max_size()<<endl;
  table.clear();
  //THIS MUST BE CHANGED WITH MEMORY SIZE TO AVOID HAVING TO BIG CHUNKS OF DATA
  que.setPrefetch(50000); // number of rows to prefetch

  for(int z = 1 ; z <= nCol ; z++)
    {
      col.push_back(que.getNameOfColumns(z));
    }
  table.push_back(col);

//   gettimeofday(&start_time, (struct timezone *)0 );

  while(que.loadRow())
    {   
      col.clear();
      for(int c =1 ; c <= nCol ; c++ )
        {
        if(table[0][c-1] != "TTC_OBJ_DATA_CLOB")
          {
            col.push_back(que.getStringField(c));
            //cout<< __LINE__ << "]\t" << mthn << "Im passing thorugh column:"<<c<<endl;
          }
        else // CLOB decoding
          {
            oracle::occi::Clob clob ;
            que.getField(c, clob) ;
            oracle::occi::Stream * stream = clob.getStream(1,0) ;
//          cout << "Clob length(): " <<clob.length() << endl ;
            char * buffer = (char *)malloc(clob.length()*sizeof(char)+1);
            memset(buffer, 0, clob.length()) ;
            buffer[clob.length()] = '\000' ;
            stream->readBuffer(buffer, clob.length()) ;
            col.push_back(buffer) ;
            free(buffer) ;
          } // end of CLOB decoding
        }   // end loop over columns in a single row
      table.push_back(col);
      clobcnt++;
    } // end loop over retrived records
  

//   cout << "Number of CLOBs: " << clobcnt << endl;
//   gettimeofday(&end_time, (struct timezone *)0 );
//   int total_usecs = (end_time.tv_sec - start_time.tv_sec)*1000000 + (end_time.tv_usec - start_time.tv_usec);
//   cout << "Time taken : " << total_usecs / 1000000  << " secs" << endl;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" <<mthn<<"Table Size:"<<table.size()<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::getPixelLTCConfig(vector< vector<string> >& table, string module, pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelLTCConfig()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  int clobcnt = 0;

  /**
     View's name:    CONF_KEY_LTC_CONFIG_V

     CONFIG_KEY                             NOT NULL NUMBER(38)
     KEY_TYPE
     KEY_ALIAS_ID
     KEY_ALIAS
     VERSION
     KIND_OF_COND
     LTC_OBJ_DATA_FILE                         NOT NULL VARCHAR2(200)
     LTC_OBJ_DATA_CLOB                         NOT NULL CLOB

  */
   
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str() ;
//             111111111
//   0123456789012345678
//   pixel/ttcciconfig/

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Starting to get data " << endl ;
    }

  que.openTable("ltcconfig",where,false ,false);

  int nCol= que.getNumberOfColumns();
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Number of Columns:" << nCol << endl;
    }
  //cout<<"Max Size"<<col.max_size()<<endl;
  //cout<<"Max Size"<<pDSMa.max_size()<<endl;
  table.clear();
//   //THIS MUST BE CHANGED WITH MEMORY SIZE TO AVOID HAVING TO BIG CHUNKS OF DATA
//   que.setPrefetch(50000); // number of rows to prefetch

  for(int z = 1 ; z <= nCol ; z++)
    {
      col.push_back(que.getNameOfColumns(z));
    }
  table.push_back(col);

//   gettimeofday(&start_time, (struct timezone *)0 );

  while(que.loadRow())
    {   
      col.clear();
      for(int c =1 ; c <= nCol ; c++ )
        {
        if(table[0][c-1] != "LTC_OBJ_DATA_CLOB")
          {
            col.push_back(que.getStringField(c));
            //cout<< __LINE__ << "]\t" << mthn << "Im passing thorugh column:"<<c<<endl;
          }
        else // CLOB decoding
          {
            oracle::occi::Clob clob ;
            que.getField(c, clob) ;
            oracle::occi::Stream * stream = clob.getStream(1,0) ;
//          cout << "Clob length(): " <<clob.length() << endl ;
            char * buffer = (char *)malloc(clob.length()*sizeof(char)+1);
            memset(buffer, 0, clob.length()) ;
            buffer[clob.length()] = '\000' ;
            stream->readBuffer(buffer, clob.length()) ;
            col.push_back(buffer) ;
            free(buffer) ;
          } // end of CLOB decoding
        }   // end loop over columns in a single row
      table.push_back(col);
      clobcnt++;
    } // end loop over retrived records
  

//   cout << "Number of CLOBs: " << clobcnt << endl;
//   gettimeofday(&end_time, (struct timezone *)0 );
//   int total_usecs = (end_time.tv_sec - start_time.tv_sec)*1000000 + (end_time.tv_usec - start_time.tv_usec);
//   cout << "Time taken : " << total_usecs / 1000000  << " secs" << endl;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" <<mthn<<"Table Size:"<<table.size()<<std::endl;
    }
}


//=====================================================================================
void PixelConfigDBInterface::getPixelMaskBitsBase(vector< vector<string> >& table, string module){
}

//=====================================================================================
void PixelConfigDBInterface::getPixelFECTable(vector< vector<string> >& table, string fec, pos::PixelConfigKey key){
  std::string mthn = "[PixelConfigDBInterface::getPixelFECTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str();
  que.openTable("fecconfig",where,false,false);

  getGeneralTable(table , que ) ;
   
}// end getPixelFECTable

//=====================================================================================
void PixelConfigDBInterface::getPixelFECTableByVersion(vector< vector<string> >& table,
					      string fec,
					      unsigned int version){
  std::string mthn = "[PixelConfigDBInterface::getPixelFECTableByVersion()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  /** // modified by MR on 18-02-2009 14:12:59
      Summary for Table/View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_PIXEL_FEC_CONFIG_V
      Column Name:                   KIND_OF_COND    Data Type:  VARCHAR2
      Column Name:                        VERSION    Data Type:  VARCHAR2
      Column Name:                    PIXFEC_NAME    Data Type:  VARCHAR2
      Column Name:                   CRATE_NUMBER    Data Type:  NUMBER
      Column Name:                    SLOT_NUMBER    Data Type:  NUMBER
      Column Name:                       VME_ADDR    Data Type:  VARCHAR2
  */

  clause << version ;
  where["VERSION"] = clause.str() ;
  que.openTable("fecconfig",where,false,false);

  getGeneralTable(table , que ) ;
   
}// end getPixelFECTableByVersion

//=====================================================================================
void PixelConfigDBInterface::getPixelFEDTable(vector< vector<string> >& table, string fed, pos::PixelConfigKey key){
  
  std::string mthn = "[PixelConfigDBInterface::getPixelFEDTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
//           1111 
// 01234567890123
// pixel/fedcard/
  
  fed.replace(0,13,"") ;
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();
//   where["PIXEL_FED"] = "PxlFED_" + fed ; 

  que.openTable("fedconfig",where,false,false);
  getGeneralTable(table,que);
}

//=====================================================================================
void PixelConfigDBInterface::getPixelGlobalDelay25(vector< vector<string> >& table, pos::PixelConfigKey key){
  
  std::string mthn = "[PixelConfigDBInterface::getPixelGlobalDelay25()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();
//   where["PIXEL_FED"] = "PxlFED_" + fed ; 

  que.openTable("globaldelay25",where,false,false);
  getGeneralTable(table,que);
}

//=====================================================================================
void PixelConfigDBInterface::getPixelFEDTableByVersion(
						       vector< vector<string> >& table, 
						       string fed, 
						       unsigned int version){
  
  std::string mthn = "[PixelConfigDBInterface::getPixelFEDTableByVersion()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  /**  modified by MR on 18-02-2009 13:57:42
       Summary for Table/View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_FED_CRATE_CONFIG_V
       Column Name:                   KIND_OF_COND    Data Type:  VARCHAR2
       Column Name:                        VERSION    Data Type:  VARCHAR2
       Column Name:                      PIXEL_FED    Data Type:  NUMBER
       Column Name:                   CRATE_NUMBER    Data Type:  NUMBER
       Column Name:                       VME_ADDR    Data Type:  VARCHAR2
  */
  clause << version ;
  where["VERSION"] = clause.str() ;

  que.openTable("fedconfig",where,false,false);
  getGeneralTable(table,que);
}

//=====================================================================================
void PixelConfigDBInterface::getPixelFEDCardTable(vector< vector<string> >& table, string fed, pos::PixelConfigKey key){
  
  std::string mthn = "[PixelConfigDBInterface::getPixelFEDCardTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
//           1111 
// 01234567890123
// pixel/fedcard/
  
  fed.replace(0,14,"") ;
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();
  where["PIXEL_FED"]  = fed ; 
  
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Looking for key " << key.key() << " and fed " << fed << endl ;
    }

  que.openTable("fedcard",where,false,false);
  getGeneralTable(table,que);
}

//=====================================================================================
void PixelConfigDBInterface::appendPixelFEDCardTBMLevelsTable(vector< vector<string> >& table, string fed, pos::PixelConfigKey key){
  
  std::string mthn = "[PixelConfigDBInterface::getPixelFEDCardTBMLevelsTable()]    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > toAppend ;
  stringstream clause ;
//           1111 
// 01234567890123
// pixel/fedcard/
  
  fed.replace(0,14,"") ;
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();
  where["PIXEL_FED"]    = fed ; 
  
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Retrieving TBM Levels for key " << key.key() << " and fed " << fed << endl ;
    }

  que.openTable("fedcard1",where,false,false);
  getGeneralTable(toAppend,que);
  for(vector< vector<string> >::iterator it = toAppend.begin() ; it != toAppend.end() ; it++)
    {
      table.push_back(*it) ;
    }
  //  dumpGeneralTable(table) ;
}

//=====================================================================================
void PixelConfigDBInterface::appendPixelFEDCardROCLevelsTable(vector< vector<string> >& table, string fed, pos::PixelConfigKey key)
{
  
  std::string mthn = "[PixelConfigDBInterface::getPixelFEDCardROCLevelsTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > toAppend ;
  stringstream clause ;
//           1111 
// 01234567890123
// pixel/fedcard/
  
  fed.replace(0,14,"") ;
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();
  where["PIXEL_FED"]    = fed ; 
  
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "\tRetrieving ROC Levels for key " << key.key() << " and fed " << fed << endl ;
    }

  que.openTable("fedcard2",where,false,false);
  getGeneralTable(toAppend,que);
  for(vector< vector<string> >::iterator it = toAppend.begin() ; it != toAppend.end() ; it++)
    {
      table.push_back(*it) ;
    }
  //  dumpGeneralTable(table) ;
}

//=====================================================================================
void PixelConfigDBInterface::getPixelPortcardMapTable(vector< vector<string> >& table, 
                                                      string portcardmap,
                                                      pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelPortcardMapTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
//  Name                                           Null?    Type
//  ----------------------------------------- -------- ----------------------------
//  CONFIG_KEY                             NOT NULL      NUMBER(38)
//  CONFG_KEY                                 NOT NULL      VARCHAR2(80)
//  VERSION                                                 VARCHAR2(40)
//  KIND_OF_COND                              NOT NULL      VARCHAR2(40)
//  SERIAL_NUMBER                                           VARCHAR2(40)
//  PORT_CARD                                 NOT NULL      VARCHAR2(200)
//  PANEL_NAME                                NOT NULL      VARCHAR2(200)
//  TBM_MODE                                                VARCHAR2(200)
//  AOH_CHAN                                  NOT NULL      NUMBER(38)

  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str();
   
  que.openTable("portcardmap",where,false,false );

  getGeneralTable(table , que ) ;
}

//=====================================================================================
void PixelConfigDBInterface::getPixelPortcardMapTableByVersion(vector< vector<string> >& table, 
							       string portcardmap,
							       unsigned int version)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelPortcardMapTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

  /** // modified by MR on 18-02-2009 14:19:32
      Summary for Table/View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_PORTCARD_MAP_V
      Column Name:                   KIND_OF_COND    Data Type:  VARCHAR2
      Column Name:                        VERSION    Data Type:  VARCHAR2
      Column Name:                      PORT_CARD    Data Type:  VARCHAR2
      Column Name:                     PANEL_NAME    Data Type:  VARCHAR2
      Column Name:                       TBM_MODE    Data Type:  VARCHAR2
      Column Name:                       AOH_CHAN    Data Type:  NUMBER
  */

  clause << version ;
  where["VERSION"] = clause.str() ; // theTrueDBKey_  ; //clause.str();
   
  que.openTable("portcardmap",where,false,false );

  getGeneralTable(table , que ) ;
}

//=====================================================================================
void PixelConfigDBInterface::getPixelDetectorConfigTable(vector< vector<string> >& table, 
                                                         string module, 
                                                         pos::PixelConfigKey key){
  std::string mthn = "[PixelConfigDBInterface::getPixelDetectorConfigTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

// modified by MR on 12-02-2008 09:40:45
  // 0 => CONFIG_KEY    (number(38))
  // 1 => CONFIG_KEY       (varchar2(80))
  // 2 => VERSION 
  // 3 => KIND_OF_COND
  // 4 => SERIAL_NUMBER
  // 5 => ROC_NAME
  // 6 => PANEL_NAME
  // 7 => ROC_STATUS

  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();

  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << "]\t" << mthn << "Requested module = " << module << std::endl ;
      std::cout << __LINE__ << "]\t" << mthn << "Requested key    = " << clause.str() << "\t" << key.key() << std::endl ;
    }
  que.openTable("detconfig",where,false, false);
  
  getGeneralTable(table , que ) ;
  //  dumpGeneralTable(table) ;
}

//=====================================================================================
void PixelConfigDBInterface::getPixelDetectorConfigTableByVersion(vector< vector<string> >& table, 
								  string module, 
								  unsigned int version){
  std::string mthn = "[PixelConfigDBInterface::getPixelDetectorConfigTableByVersion()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  /**

     Summary for Table/View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_DET_CONFIG_V
     Column Name:                   KIND_OF_COND    Data Type:  VARCHAR2
     Column Name:                        VERSION    Data Type:  VARCHAR2
     Column Name:                       ROC_NAME    Data Type:  VARCHAR2
     Column Name:                     ROC_STATUS    Data Type:  VARCHAR2
  */

  clause << version ;
  where["VERSION"] = clause.str() ;

  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << "]\t" << mthn << "Requested version    = " << version << std::endl ;
    }
  que.openTable("detconfig",where,false, false);
  
  getGeneralTable(table , que ) ;
  //  dumpGeneralTable(table) ;
}

//=====================================================================================
void PixelConfigDBInterface::getPixelLowVoltageTable(vector< vector<string> >& table, 
                                                     string module,
                                                     pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelLowVoltageTable()]     ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl;  
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

  clause << key.key() ;
  where["KEY_ALIAS_ID"] = theTrueDBKey_ ; // clause.str();
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Retrieving data for lowvoltagemap for global key " << key.key() << endl ;
    }
  que.openTable("lowvoltagemap", where, false, false);
  
  getGeneralTable(table , que ) ;
}

//=====================================================================================
void PixelConfigDBInterface::getAllPixelPortCardConfigTable(vector< vector<string> >& table, 
							    pos::PixelConfigKey key){
  std::string mthn = "[PixelConfigDBInterface::getPixelPortCardConfigTable()]     ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl;  
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

//   012345678901234
//   pixel/portcard/
//   portcard.replace(0,15, "") ;

  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();
//   where["PORT_CARD"]    = portcard ;
  if(DEBUG_CONFIG_DB)
    {
//       cout << __LINE__ << "]\t" << mthn << "Retrieving data for portcard " << portcard << " for global key " << key.key() << endl ;
    }
  que.openTable("portcard",where,false, false);
  
  getGeneralTable(table , que ) ;
}

//=====================================================================================
void PixelConfigDBInterface::getPixelPortCardConfigTable(vector< vector<string> >& table, 
                                                         string portcard,
                                                         pos::PixelConfigKey key){
  std::string mthn = "[PixelConfigDBInterface::getPixelPortCardConfigTable()]     ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl;  
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

//   012345678901234
//   pixel/portcard/
  portcard.replace(0,15, "") ;

  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();
  where["PORT_CARD"]    = portcard ;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Retrieving data for portcard " << portcard << " for global key " << key.key() << endl ;
    }
  que.openTable("portcard",where,false, false);
  
  getGeneralTable(table , que ) ;
}

//=====================================================================================
void PixelConfigDBInterface::getPixelTBMSettingsTable(vector< vector<string> >& table, 
                                                      string module,
                                                      pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelTBMSettingsTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl;  
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;


  //FPix_BmO_D2_BLD8_PNL2
  /**

View's name:     CONF_KEY_PIXEL_TBM_V

 ----------------------------------------- -------- ----------------------------
 CONFIG_KEY                                      NUMBER(38)
 CONFIG_KEY                                         VARCHAR2(80)
 VERSION                                            VARCHAR2(40)
 CONDITION_DATA_SET_ID                              NUMBER(38)
 KIND_OF_CONDITION_ID                               NUMBER(38)
 KIND_OF_COND                                       VARCHAR2(40)
 TBM_PART_ID                                        NUMBER(38)
 TBM_SER_NUM                                        VARCHAR2(40)
 MODULE_NAME                                        VARCHAR2(99)
 HUB_ADDRS                                          NUMBER(38)
 ANLG_INBIAS_ADDR                                   NUMBER(38)
 ANLG_INBIAS_VAL                                    NUMBER(38)
 ANLG_OUTBIAS_ADDR                                  NUMBER(38)
 ANLG_OUTBIAS_VAL                                   NUMBER(38)
 ANLG_OUTGAIN_ADDR                                  NUMBER(38)
 ANLG_OUTGAIN_VAL                                   NUMBER(38)
 TBM_MODE                                           VARCHAR2(200)

N.B.: YOU HAVE TO STRIP OUT THE pixel/tbm/ TRAILING PART OF THE MODULE THAT IS PRESENT BECAUSE THE 
FILES-BASED INTERFACE NEED IT!!!!
  */
  //01234567891
  //pixel/tbm/
  module.replace(0,10, "") ;

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "Looking for module " << module << endl ;
    }
  where["MODULE_NAME"] = module;
  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str();

//  cout << __LINE__ << "]\t" << __LINE__ << mthn << "Looking for module " << module << endl ;
  que.openTable("tbm",where,false, true);

  getGeneralTable(table , que ) ;
//   dumpGeneralTable(table) ;

}//end getPixelTBMSettings

//=====================================================================================
void PixelConfigDBInterface::getPixelNameTranslationTable(vector< vector<string> >& table, 
                                                          string module,
                                                          pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelNameTranslationTable()]    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

//  Name                                           Null?    Type
//  ----------------------------------------- -------- ----------------------------
//  CONFIG_KEY                             NOT NULL      NUMBER(38)
//  CONFG_KEY                                 NOT NULL      VARCHAR2(80)
//  VERSION                                                 VARCHAR2(40)
//  KIND_OF_COND                              NOT NULL      VARCHAR2(40)   
//  SERIAL_NUMBER                                           VARCHAR2(40)
//  ROC_NAME                                  NOT NULL      VARCHAR2(200)  
//  PXLFEC_NAME                               NOT NULL      VARCHAR2(200)  
//  MFEC_POSN                                 NOT NULL      NUMBER(38)     
//  MFEC_CHAN                                 NOT NULL      NUMBER(38)     
//  HUB_ADDRS                                 NOT NULL      NUMBER(38)     
//  PORT_NUM                                  NOT NULL      NUMBER(38)     
//  ROC_I2C_ADDR                              NOT NULL      NUMBER(38)       
//  PXLFED_NAME                               NOT NULL      VARCHAR2(200)    
//  FED_CHAN                                  NOT NULL      NUMBER(38)       
//  FED_ROC_NUM                               NOT NULL      NUMBER(38)       


  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; //clause.str();

  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << "]\t" << mthn << "Requested module = " << module << std::endl ;
      std::cout << __LINE__ << "]\t" << mthn << "Requested key    = " << clause.str() << "\t" << key.key() << std::endl ;
    }
  
  que.openTable("nametranslation",where,false ,false);

  getGeneralTable(table , que ) ;


}//end getPixelNameTranslationTable

//=====================================================================================
void PixelConfigDBInterface::getPixelNameTranslationTableByVersion(vector< vector<string> >& table, 
								   string module,
								   unsigned int version)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelNameTranslationTableByVersion()]    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  /** modified by MR on 18-02-2009 13:46:29
     Summary for Table/View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_NAME_TRANS_V
     Column Name:                   KIND_OF_COND    Data Type:  VARCHAR2
     Column Name:                        VERSION    Data Type:  VARCHAR2
     Column Name:                       ROC_NAME    Data Type:  VARCHAR2
     Column Name:                    PXLFEC_NAME    Data Type:  NUMBER
     Column Name:                      MFEC_POSN    Data Type:  NUMBER
     Column Name:                      MFEC_CHAN    Data Type:  NUMBER
     Column Name:                      HUB_ADDRS    Data Type:  NUMBER
     Column Name:                       PORT_NUM    Data Type:  NUMBER
     Column Name:                   ROC_I2C_ADDR    Data Type:  NUMBER
     Column Name:                    PXLFED_NAME    Data Type:  NUMBER
     Column Name:                       FED_CHAN    Data Type:  NUMBER
     Column Name:                    FED_ROC_NUM    Data Type:  NUMBER
     Column Name:                       TBM_MODE    Data Type:  VARCHAR2
  */
  clause << version ;
  where["VERSION"] = clause.str() ; // theTrueDBKey_  ; //clause.str();

  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << "]\t" << mthn << "Requested version = " << version << std::endl ;
    }
  
  que.openTable("nametranslation",where,false ,false);

  getGeneralTable(table , que ) ;


}//end getPixelNameTranslationTableByVersion

//=====================================================================================
void PixelConfigDBInterface::getPixelTKFECConfig(vector< vector<string> >& table,
                                                 string module,                  
                                                 pos::PixelConfigKey key)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelNameTranslationTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

  clause << key.key() ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ; // clause.str();
  
  que.openTable("tkfecconfig",where,false ,false);
  getGeneralTable(table , que ) ;

}// end getPixelTKFECConfig

//=====================================================================================
void PixelConfigDBInterface::getPixelTKFECConfigByVersion(vector< vector<string> >& table,
							  string module,                  
							  unsigned int version)
{
  std::string mthn = "[PixelConfigDBInterface::getPixelNameTranslationTable()]\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << "]\t" << mthn <<std::endl; 
    }
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
  /** // modified by MR on 18-02-2009 14:11:50
      Summary for Table/View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_TRACKER_FEC_CONFIG_V
      Column Name:                   KIND_OF_COND    Data Type:  VARCHAR2
      Column Name:                        VERSION    Data Type:  VARCHAR2
      Column Name:                    TRKFEC_NAME    Data Type:  VARCHAR2
      Column Name:                    CRATE_LABEL    Data Type:  VARCHAR2
      Column Name:                   CRATE_NUMBER    Data Type:  NUMBER
      Column Name:                           TYPE    Data Type:  VARCHAR2
      Column Name:                    SLOT_NUMBER    Data Type:  NUMBER
      Column Name:                       VME_ADDR    Data Type:  VARCHAR2
      Column Name:                       I2CSPEED    Data Type:  NUMBER
  */

  clause << version ;
  where["VERSION"] = clause.str() ;
  
  que.openTable("tkfecconfig",where,false ,false);
  getGeneralTable(table , que ) ;

}// end getPixelTKFECConfigByVersion

//=====================================================================================
void PixelConfigDBInterface::getGeneralTable(vector< vector<string> >& table, PixelSQLCommand& query){
  std::string mthn = "]\t[PixelConfigDBInterface::getGeneralTable()]\t \t    ";
  std::vector< string > col;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "Starting to get data" << endl;
    }

  int nCol= query.getNumberOfColumns();
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "Number of Columns: " << nCol << endl;
    }

  table.clear();
  query.setPrefetch(50000);
//   query.setPrefetch(1);

//   for(int z = 1 ; z <= nCol ; z++){
//     col.push_back(query.getNameOfColumns(z));
//     cout << mthn << "Inserted: " << col.back() << endl ;
//   }//end for 

  table.push_back(query.getNamesOfColumns(nCol));
  /** modified by MR on 03-07-2008 15:44:29
      Fetching rows array
      
  const int nRows = 50 ;
  char * buffer = (char*)malloc(nRows*nCol*500*sizeof(char)) ;
  for(int r = 0 ; r < nRows ; r++)
    {
      for(int c = 0 ; c < nCol ; c++)
        {
          query.setDataBuffer(c+1, &buffer[c*nRows*500], 500) ;
        }
    }
  while(query.loadRow(50)){
    if(DEBUG_CONFIG_DB)
      {
        cout << mthn << " Fetched rows: " << query.getNumArrayRows() << endl ;
      }
    for(int r = 0 ; r < query.getNumArrayRows() ; r++ )
      {
        col.clear();
        for(int c = 0 ; c <  nCol ; c++ )
          {
            col.push_back(string(&buffer[500*(r+nRows*c)], 500));
          }//end for
        table.push_back(col);
      }
  }//end while
  */

  while(query.loadRow()){
    col.clear();
    for(int c =1 ; c <= nCol ; c++ ){
      col.push_back(query.getStringField(c));
//       cout << "Column at " << c << " contains " << col.back()<< endl ;
    }//end for
    table.push_back(col);
  }//end while

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "Table Size: " << table.size() <<endl;
    }
}


//=====================================================================================
void PixelConfigDBInterface::dumpGeneralTable(vector< vector<string> >& table){
  std::string mthn = "[PixelConfigDBInterface::dumpGeneralTable()]\t    ";

  cout << __LINE__ << "]\t" << mthn << endl ;
  for(unsigned int r = 0 ; r < table.size() ; r++){    //Goes to every row of the Matrix
    if(table[r].size() == 0)
      {
	cout << __LINE__ << "]\t" << "__________________ NEW TABLE __________________\n"<< endl ;
        continue ;
      }
    for(vector<string>::iterator it = table[r].begin() ; it != table[r].end() ; it++)
      {
//      cout << *it <<"["<<&*it<<"]\t"  ;
	cout << setw(15) << *it <<"\t"  ;
      }
        cout << endl ;
  }
}

//=====================================================================================
// modified by MR on 30-09-2008 23:45:08
unsigned int PixelConfigDBInterface::getNextAvailableVersion(std::string sequence)
{
  std::string mthn = "[PixelConfigDBInterface::getNextVersion()]" ;
  
  //cout << __LINE__ << "]\t" << mthn << endl ;

  PixelSQLCommand que(database_);
  int nextKey ;
  nextKey = que.getNextAvailableVersion(sequence) ;
  return nextKey ;
}

//=====================================================================================
// modified by MR on 26-09-2008 10:26:13
unsigned int PixelConfigDBInterface::clone(unsigned int oldkey, std::string path, unsigned int version)
{
  assert(0) ;
//   std::string mthn = "[PixelConfigDBInterface::clone()] " ;
  
//   cout << __LINE__ << "]\t" << mthn << endl ;

//   PixelSQLCommand que(database_);
//   int nextKey ;
//   nextKey = que.getNextAvailableVersion() ;
//   cout << __LINE__ << "]\t" << mthn << "next key: " << nextKey << endl ;
//   /**
//      View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_ALIAS_KEY_DATASET_MAP_V
//      Name                                                                                                  Null?    Type

//      CONFIG_KEY_ALIAS_ID     NOT NULL NUMBER(38)
//      KEY_ALIAS_NAME          NOT NULL VARCHAR2(80)
//      CONFIG_KEY_ID           NOT NULL NUMBER(38)
//      KEY_NAME                NOT NULL VARCHAR2(80)
//      CONDITION_DATA_SET_ID   NOT NULL NUMBER(38)
//      KIND_OF_CONDITION_ID    NOT NULL NUMBER(38)
//      KIND_OF_CONDITION_NAME  NOT NULL VARCHAR2(40)
//      CONDITION_VERSION                VARCHAR2(40)
//   */
//   return 0 ;
}

//=====================================================================================
// modified by MR on 11-02-2008 15:32:54
std::vector<std::pair<std::string, unsigned int> > PixelConfigDBInterface::getAliases()
{
  /**

     CONFIG_KEY_ID         NOT NULL NUMBER(38)   // 0
     CONFIG_KEY            NOT NULL VARCHAR2(80) // 1 
     KEY_ALIAS_ID          NOT NULL NUMBER(38)   // 2
     CONFIG_ALIAS          NOT NULL VARCHAR2(80) // 3 
     CONFIG_KEY_TYPE       NOT NULL VARCHAR2(80) // 4 
  */
  std::string mthn = "]\t[PixelConfigDBInterface::getAliases]\t\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << "]\t" << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::vector<std::pair<std::string, unsigned int> > result ;

  que.openTable("config_keys",where,false ,true, "CONFIG_ALIAS", true);

  getGeneralTable(table , que ) ;

  pair<string, unsigned int> tmp ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      tmp.first  = it->operator[](3) ;
      tmp.second = atoi( (it->operator[](1)).c_str() ) ;
      result.push_back(tmp) ;
//      cout << __LINE__ << mthn << "Alias: " << tmp.first << "\tName: " << tmp.second << endl ;
    }
//   dumpGeneralTable(table) ;
//   cout << "col   " << &col   << endl ;
//   cout << "where " << &where << endl ;
//   cout << "table " << &table << endl ;
//   int j = 0 ;
//   for(vector< vector<string> >::iterator it = table.begin(); it != table.end() ; it++)
//     {
//       cout << "j "         << j << endl ;
//       cout << "it "        << &*it << endl ;
//       cout << "it.size() " << it->size() << endl ;
//       j++ ;
//       it->clear() ;
//     }
//   table.clear() ;
  return result ;
}

//=====================================================================================
// modified by MR on 09-02-2009 10:06:28
std::vector<std::vector<std::string > > PixelConfigDBInterface::getAliasesWithCommentsAndDate()
{
  /*
    CMS_PXL_PIXEL_VIEW_OWNER.CONF_KEY_CONFIG_KEYS_V
    
    Column Name:                  CONFIG_KEY_ID    Data Type:  NUMBER
    Column Name:                     CONFIG_KEY    Data Type:  VARCHAR2
    Column Name:                   KEY_ALIAS_ID    Data Type:  NUMBER
    Column Name:                   CONFIG_ALIAS    Data Type:  VARCHAR2
    Column Name:                CONFIG_KEY_TYPE    Data Type:  VARCHAR2
    Column Name:          RECORD_INSERTION_TIME    Data Type:  TIMESTAMP WITH TIMEZONE
    Column Name:            COMMENT_DESCRIPTION    Data Type:  VARCHAR2
    Column Name:                     PROVENANCE    Data Type:  VARCHAR2
    Column Name:                         AUTHOR    Data Type:  VARCHAR2
  */

  std::string mthn = "]\t[PixelConfigDBInterface::getAliasesWithCommentsAndDate()]\t\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout << __LINE__ << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;

  que.openTable("config_keys",where,false ,false);

  getGeneralTable(table , que ) ;

  //  dumpGeneralTable(table) ;
  return table ;
}

//=====================================================================================
std::vector<std::pair< std::string, unsigned int> > PixelConfigDBInterface::getVersions(pos::PixelConfigKey key)
{
  /**
     View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_KEY_DATASET_MAP_V
     Name                            Null?    Type
     0 CONFIG_KEY_ID           	    NOT NULL NUMBER(38)
     1 KEY_NAME			    NOT NULL VARCHAR2(80)
     2 CONDITION_DATA_SET_ID	    NOT NULL NUMBER(38)
     3 KIND_OF_CONDITION_ID	    NOT NULL NUMBER(38)
     4 KIND_OF_CONDITION_NAME	    NOT NULL VARCHAR2(40)
     5 CONDITION_VERSION		     VARCHAR2(40)
     6 IS_MOVED_TO_HISTORY	    NOT NULL CHAR(1)
     7 RECORD_INSERTION_TIME
     8 COMMENT_DESCRIPTION
     9 AUTHOR
 */
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::vector<std::pair<std::string, unsigned int> > result ;
  stringstream clause ;


  fillHashes() ;
  clause.str("") ;
  clause << key.key() ;
  where["KEY_NAME"] = clause.str()  ;
  std::string inHistory("F") ;
  where["IS_MOVED_TO_HISTORY"] = inHistory ;


  que.openTable("dataset_map",where,false ,false);

  getGeneralTable(table , que ) ;

  pair<string, unsigned int> tmp ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      if(conversionKOC[it->operator[](4)].find("roclevels")!=std::string::npos ||
	 conversionKOC[it->operator[](4)].find("tbmlevels")!=std::string::npos ) continue ;
      tmp.first  = conversionKOC[it->operator[](4)] ;
      tmp.second = atoi( (it->operator[](5)).c_str() ) ;
//       cout << __LINE__ << "] " << __PRETTY_FUNCTION__
//            << " it->operator[](4): " 
// 	   <<   it->operator[](4) 
// 	   << " tmp.first: "
// 	   <<   tmp.first
// 	   << " tmp.second: "
// 	   << tmp.second
//            << endl ;
      result.push_back(tmp) ;
    }
  return result ;
}

//=====================================================================================
std::vector<std::vector< std::string > > PixelConfigDBInterface::getVersionsWithCommentsAndDate(pos::PixelConfigKey key)
{
  /**
     View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_KEY_DATASET_MAP_V
     Name                            Null?    Type
     0 CONFIG_KEY_ID           	    NOT NULL NUMBER(38)
     1 KEY_NAME			    NOT NULL VARCHAR2(80)
     2 CONDITION_DATA_SET_ID	    NOT NULL NUMBER(38)
     3 KIND_OF_CONDITION_ID	    NOT NULL NUMBER(38)
     4 KIND_OF_CONDITION_NAME	    NOT NULL VARCHAR2(40)
     5 CONDITION_VERSION		     VARCHAR2(40)
     6 IS_MOVED_TO_HISTORY	    NOT NULL CHAR(1)
     7 RECORD_INSERTION_TIME
     8 COMMENT_DESCRIPTION
     9 AUTHOR
 */
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  vector< vector<string> > result ;
  stringstream clause ;


  fillHashes() ;
  clause.str("") ;
  clause << key.key() ;
  where["KEY_NAME"] = clause.str()  ;
  std::string inHistory("F") ;
  where["IS_MOVED_TO_HISTORY"] = inHistory ;

  que.openTable("dataset_map",where,false ,false);

  getGeneralTable(table , que ) ;

  vector<string> tmp ;
  // insert headers first!!!
  result.push_back(table[0]) ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      tmp.clear() ;
      if(conversionKOC[it->operator[](4)].find("roclevels")!=std::string::npos ||
	 conversionKOC[it->operator[](4)].find("tbmlevels")!=std::string::npos ) continue ;
      tmp.push_back(		  it->operator[](0))  ; // CONFIG_KEY_ID
      tmp.push_back(		  it->operator[](1))  ; // KEY_NAME
      tmp.push_back(		  it->operator[](2))  ; // CONDITION_DATA_SET_ID
      tmp.push_back(		  it->operator[](3))  ; // KIND_OF_CONDITION_ID
      tmp.push_back(conversionKOC[it->operator[](4)]) ; // KIND_OF_CONDITION_NAME
      tmp.push_back(		  it->operator[](5))  ; // CONDITION_VERSION
      tmp.push_back(		  it->operator[](6))  ; // IS_MOVED_TO_HISTORY
      tmp.push_back(		  it->operator[](7))  ; // INSERT_TIME
      tmp.push_back(		  it->operator[](8))  ; // COMMENT_DESCRIPTION
      tmp.push_back(		  it->operator[](9))  ; // AUTHOR
      result.push_back(tmp) ;
    }
//    dumpGeneralTable(result) ;
  return result ;
}

std::map<std::string, std::vector<std::pair< std::string, unsigned int> > > PixelConfigDBInterface::getFullVersions()
{
  /**
     View: CMS_PXL_PIXEL_VIEW_OWNER.CONF_KEY_DATASET_MAP_V
     Name                            Null?    Type
     0 CONFIG_KEY_ID           	    NOT NULL NUMBER(38)
     1 KEY_NAME			    NOT NULL VARCHAR2(80)
     2 CONDITION_DATA_SET_ID	    NOT NULL NUMBER(38)
     3 KIND_OF_CONDITION_ID	    NOT NULL NUMBER(38)
     4 KIND_OF_CONDITION_NAME	    NOT NULL VARCHAR2(40)
     5 CONDITION_VERSION		     VARCHAR2(40)
     6 IS_MOVED_TO_HISTORY	    NOT NULL CHAR(1)
 */
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::map<std::string, std::vector<std::pair< std::string, unsigned int> > > result ;
  stringstream clause ;


  fillHashes() ;
  clause.str("") ;
  std::string inHistory("F") ;
  where["IS_MOVED_TO_HISTORY"] = inHistory ;


  que.openTable("dataset_map",where,false ,true);

  getGeneralTable(table , que ) ;

  pair<string, unsigned int> tmp ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      if(conversionKOC[it->operator[](4)].find("roclevels")!=std::string::npos ||
	 conversionKOC[it->operator[](4)].find("tbmlevels")!=std::string::npos ) continue ;
      tmp.first  = conversionKOC[it->operator[](4)] ;
      tmp.second = atoi( (it->operator[](5)).c_str() ) ;
      result[it->operator[](1)].push_back(tmp) ;
//       result.push_back(tmp) ;
//       cout << endl ;
//       cout << it->operator[](0) << "  " ;
//       cout << it->operator[](1) << "  " ;
//       cout << it->operator[](2) << "  " ;
//       cout << it->operator[](3) << "  " ;
//       cout << it->operator[](4) << "  " ;
//       cout << it->operator[](5) << "  " ;
//       cout << it->operator[](6) << "  " ;
//       cout << it->operator[](7) << "  " ;
//       cout << it->operator[](8) << "  " ;
    }
  return result ;
}


//=====================================================================================
std::map<std::string,std::vector<std::vector< std::string> > > PixelConfigDBInterface::getFullCfgs(int start, 
                                                                                                   int howMany,
												   int from,
												   int to, 
												   int &total)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getFullCfgs()]\t\t\t    ";
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::map<std::string, std::vector<std::vector< std::string> > > result ;
  stringstream clause ;


  fillHashes() ;
  clause.str("") ;
  std::string inHistory("F") ;
  where["IS_MOVED_TO_HISTORY"] = inHistory ;

  que.getNumberOfTotalCfgs() ;
  getGeneralTable(table , que ) ;

  total = atoi(table[1][0].c_str()) ;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "Total number of Global Keys: " << total << endl ;
    }  
  que.getTotalCfgs() ;
  getGeneralTable(table , que ) ;
  vector<unsigned int> orderedTable ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      orderedTable.push_back((unsigned int)atoi((it->operator[](0)).c_str())) ;
    }
  std::sort(orderedTable.begin(), orderedTable.end());  

  int counter  = 0 ;
  int rSize    = 0 ;
  std::vector<int> keepIt ;

  for( vector<unsigned int>::iterator it = orderedTable.begin() ; it != orderedTable.end() ; it++)
    {
      if( (int)*it >= from && (int)*it <= to ) 
        {
	   keepIt.push_back((int)*it) ;
	}
    }

  for(std::vector<int>::iterator it=keepIt.begin(); it!=keepIt.end(); it++)
    {
      if( counter >= start && counter < start+howMany )    
	{
	  if( rSize <= howMany ) 
	    {
	      pos::PixelConfigKey globalKey( *it ) ;
	      std::vector<std::vector< std::string > > vList = getVersionsWithCommentsAndDate(globalKey) ;
	      stringstream toString("") ;
	      toString << *it ;
	      result[toString.str()] = vList ;
	      rSize++ ;
	    }
	}
      counter++ ;
    }
  return result ;

}

//=====================================================================================
std::map<std::string,std::vector<std::pair< std::string, std::string> > > PixelConfigDBInterface::getKeyAliasVersionAliases(int start, int howMany, int &total)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getKeyAliasVersionAliases()]\t    " ;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::map<std::string , int > colM;
  std::vector<std::string > colNames;
  std::map<std::string, std::vector<std::pair< std::string, std::string> > > result ;
  stringstream clause ;

  que.getNumberOfTotalKeyAliases() ;
  getGeneralTable(table , que ) ;

  total = atoi(table[1][0].c_str()) ;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "Total number of Aliased Keys: " << total << endl ;
    }
  fillHashes() ;
  
  colNames.push_back("GK"       ) ;
  colNames.push_back("KEY_ALIAS") ;
  colNames.push_back("VA" 	) ;
  colNames.push_back("CD" 	) ;
  colNames.push_back("IT" 	) ;
  colNames.push_back("AU" 	) ;
  colNames.push_back("KOC"	) ;
  
  clause.str("") ;

  que.openKeyAliasKeyVersions() ;
  getGeneralTable(table , que ) ;
  
  for(unsigned int c = 0 ; c < table[0].size() ; c++){
    for(unsigned int n=0; n<colNames.size(); n++){
      if(table[0][c] == colNames[n]){
	colM[colNames[n]] = c;
	break;
      }
    }
  }//end for

  for(unsigned int n=0; n<colNames.size(); n++){
    if(colM.find(colNames[n]) == colM.end()){
      std::cerr << __LINE__ << mthn << "Couldn't find in the database the column with name " << colNames[n] << std::endl;
      assert(0);
    }
  }

  int counter = 0 ;
  std::string previous = table[1][colM["KEY_ALIAS"]] ;;
  for(unsigned int i = 1 ; i <table.size() ; i++ )
    {
      if(counter >= start && counter < (start+howMany) )
	{
  	  if(DEBUG_CONFIG_DB)
  	    {
	      cout << __LINE__              	  << mthn 
	           << table[i][colM["GK"]]  	  << "\t" 
	           << table[i][colM["KEY_ALIAS"]] << "\t" 
	           << table[i][colM["VA"]]  	  << "\t" 
	           << table[i][colM["CD"]]  	  << "\t" 
	           << table[i][colM["IT"]]  	  << "\t" 
	           << table[i][colM["AU"]]  	  << "\t" 
	           << table[i][colM["KOC"]] 	  << "\t"
		   << endl ;
	    }
	  result[table[i][colM["KEY_ALIAS"]]].push_back(std::make_pair(conversionKOC[table[i][colM["KOC"]]], 
								                     table[i][colM["VA"]]) ) ;
	}
      if(table[i][colM["KEY_ALIAS"]] != previous)
	{
	  previous = table[i][colM["KEY_ALIAS"]] ;
	  counter++ ;
	}
    }
  
  return result ;
}

//=====================================================================================
std::map<std::string,std::vector<std::string> > PixelConfigDBInterface::getKeyAliasVersionAliases(int &total)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getKeyAliasVersionAliases(int)]\t    " ;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::map<std::string , int > colM;
  std::vector<std::string > colNames;
  std::map<std::string,std::vector<std::string> > result ;
//  stringstream clause ;

  /**
     View's name: CONF_KEY_CONFIG_KEYS_V LEFT OUTER JOIN CONF_VERSION_ALIAS_V 

     Name                                      Null?    Type
     ----------------------------------------- -------- ----------------------------
     GK                                        NOT NULL
     KEY_ALIAS                                 NULL
     KOC                                       NULL
     VA                                        NULL
  */

  fillHashes() ;

  colNames.push_back("GK"	) ;    // Global Key			  (numeric) 
  colNames.push_back("KEY_ALIAS") ;    // Global KEY ALIAS
  colNames.push_back("KOC"	) ;    // Kind Of Condition		  (numeric)
  colNames.push_back("VA" 	) ;    // Kind of condition Version Alias
  colNames.push_back("IT" 	) ;    // Insertion Time
  colNames.push_back("CD" 	) ;    // Comment Description
  colNames.push_back("AU" 	) ;    // AUthor
  
//  clause.str("") ;

  que.openKeyAliasKeyVersions() ;
  getGeneralTable(table , que ) ;
  
  for(unsigned int c = 0 ; c < table[0].size() ; c++){
    for(unsigned int n=0; n<colNames.size(); n++){
      if(table[0][c] == colNames[n]){
	colM[colNames[n]] = c;
	break;
      }
    }
  }//end for

  for(unsigned int n=0; n<colNames.size(); n++){
    if(colM.find(colNames[n]) == colM.end()){
      std::cerr << __LINE__ << "]\t" << mthn << "Couldn't find in the database the column with name " << colNames[n] << std::endl;
      assert(0);
    }
  }
  
  
//  std::string previous = table[1][colM["GK"]] ;
  for(unsigned int i = 1 ; i <table.size() ; i++ )
    {
      if(DEBUG_CONFIG_DB)
	{
	  cout << __LINE__ << "]\t" << mthn <<               table[i][colM["GK"]       ] << "|\t|" 
	        			    <<  	     table[i][colM["KEY_ALIAS"]] << "|\t|"
	        			    << conversionKOC[table[i][colM["KOC"]]     ] << "|\t|"
	        			    <<  	     table[i][colM["VA"]       ] << "|\t|"
					    <<  	     table[i][colM["IT"]       ] << "|\t|"
					    <<  	     table[i][colM["CD"]       ] << "|\t|"
					    <<  	     table[i][colM["AU"]       ] << "|\t|"
					    << endl ;
	}
      result[table[i][colM["KEY_ALIAS"]]].push_back(conversionKOC[table[i][colM["KOC"]]]) ;
      result[table[i][colM["KEY_ALIAS"]]].push_back(table[i][colM["VA"]]) ;
      result[table[i][colM["KEY_ALIAS"]]].push_back(table[i][colM["IT"]]) ;
      result[table[i][colM["KEY_ALIAS"]]].push_back(table[i][colM["CD"]]) ;
      result[table[i][colM["KEY_ALIAS"]]].push_back(table[i][colM["AU"]]) ;
      result[table[i][colM["KEY_ALIAS"]]].push_back(table[i][colM["GK"]]) ;

//      if(table[i][colM["GK"]] != previous)
//	{
//	  previous = table[i][colM["GK"]] ;
//	}
    }
/*    
    cout << __LINE__ << "]\t" << mthn << "result.size() " << result.size() << endl ;
    for(std::map<std::string,std::vector<std::string> >::iterator i=result.begin(); i!=result.end(); i++)
    {
      if( i->first != "VcThrCalibration_P5_All" ) {continue;}
      cout << __LINE__ << "]\t" << mthn << "i->first: " << i->first << " Size: " << i->second.size() << endl ;
      for(std::vector<std::string>::iterator k=i->second.begin(); k!=i->second.end(); k++) 
      {
        cout << *k << "\t" ;
      }
      cout << endl ;
      cout << __LINE__ << "]\t" << mthn << "---------------------------------------------" << endl ;
    }
*/  
  return result ;
}

//=====================================================================================
void PixelConfigDBInterface::addAuthor(std::string author) 
{
  if( author == "" ) author = "Unknown author" ;
  author_ = author ;
}


//=====================================================================================
void PixelConfigDBInterface::addComment(std::string comment) 
{
  if( comment == "" ) comment = "Tm8gY29tbWVudCB3YXMgc3BlY2lmaWVkLg==" ;//base64 for "No comment was specified" ;
  comment_ = comment ;
}
//=====================================================================================
void PixelConfigDBInterface::addAlias(std::string alias, unsigned int key,
                                      std::vector<std::pair<std::string, std::string> > versionaliases)
{
  char * cpath = getenv("PIXELCONFIGURATIONSPOOLAREA") ;
  std::stringstream fullPath ;
  fullPath << cpath << "/addAlias_" << pos::PixelTimeFormatter::getmSecTime() << ".xma" ;
  std::ofstream aliasfile(fullPath.str().c_str()) ;

  fillHashes() ;

  aliasfile <<  "<?xml version=\"1.0\" encoding=\"utf-8\"?>" 									    << endl ;
  aliasfile <<  ""                                     									            << endl ;
  aliasfile <<  "<!-- " << __LINE__ << "] " << __PRETTY_FUNCTION__  << " -->" 						            << endl ;
  aliasfile <<  ""                                     									            << endl ;
  aliasfile <<  "<ROOT>"                                     									    << endl ;
  aliasfile <<  "  <MAPS>"                                   									    << endl ;
  aliasfile <<  "   <KEY_ALIAS>"                             									    << endl ;
  aliasfile <<  "     <NAME>" << alias << "</NAME>"          									    << endl ;
  aliasfile <<  "       <KEY mode='keep'>"                   									    << endl ;
  aliasfile <<  "         <NAME>" << key << "</NAME>"        									    << endl ;
  for(unsigned int i=0;i<versionaliases.size();i++)
    {
      aliasfile                                                                                                                     << endl ;
      aliasfile << "         <VERSION_ALIAS>"                                                                                       << endl ;
      aliasfile << "           <NAME>"                   << versionaliases[i].second                   << "</NAME>"                 << endl ;
      aliasfile << "           <KIND_OF_CONDITION>"                                                                                 << endl ;
      aliasfile << "             <EXTENSION_TABLE_NAME>" << conversionETNKOC[versionaliases[i].first]  << "</EXTENSION_TABLE_NAME>" << endl ;
      aliasfile << "             <NAME>"                 << conversionNameKOC[versionaliases[i].first] << "</NAME>"                 << endl ;
      aliasfile << "           </KIND_OF_CONDITION>" << endl ;
      aliasfile << "         </VERSION_ALIAS>" << endl ;
      if(versionaliases[i].first.find("fedcard") != std::string::npos)
        {
          aliasfile << "         <VERSION_ALIAS>"                                                                          	    << endl ;
          aliasfile << "           <NAME>"                   << versionaliases[i].second                   << "</NAME>"    	    << endl ;
          aliasfile << "           <KIND_OF_CONDITION>"                                                                    	    << endl ;
          aliasfile << "             <EXTENSION_TABLE_NAME>" << conversionETNKOC["fedcard1"]  << "</EXTENSION_TABLE_NAME>" 	    << endl ;
          aliasfile << "             <NAME>"                 << conversionNameKOC["fedcard1"] << "</NAME>"                 	    << endl ;
          aliasfile << "           </KIND_OF_CONDITION>"                                                                   	    << endl ;
          aliasfile << "         </VERSION_ALIAS>"                                                                         	    << endl ;
	  aliasfile                                                                                                        	    << endl ;
          aliasfile << "         <VERSION_ALIAS>"                                                                          	    << endl ;
          aliasfile << "           <NAME>"                   << versionaliases[i].second                   << "</NAME>"    	    << endl ;
          aliasfile << "           <KIND_OF_CONDITION>"                                                                    	    << endl ;
          aliasfile << "             <EXTENSION_TABLE_NAME>" << conversionETNKOC["fedcard2"]  << "</EXTENSION_TABLE_NAME>" 	    << endl ;
          aliasfile << "             <NAME>"                 << conversionNameKOC["fedcard2"] << "</NAME>"                 	    << endl ;
          aliasfile << "           </KIND_OF_CONDITION>"                                                                   	    << endl ;
          aliasfile << "         </VERSION_ALIAS>"                                                                         	    << endl ;
        }
  }

  aliasfile <<  "    </KEY>"            											    << endl ;
  aliasfile <<  "   </KEY_ALIAS>"       											    << endl ;
  aliasfile <<  "  </MAPS>"             											    << endl ;
  aliasfile <<  "</ROOT>"               	                                                                                    << endl ;


  aliasfile.close() ;
}

//=====================================================================================
unsigned int PixelConfigDBInterface::makeKey(std::vector<std::pair<std::string, unsigned int> > versions)
{
  char * cpath = getenv("PIXELCONFIGURATIONSPOOLAREA") ;
  std::stringstream fullPath ;
  fullPath << cpath << "/makeKey_" << pos::PixelTimeFormatter::getmSecTime() << ".xma" ;
  std::ofstream aliasfile(fullPath.str().c_str()) ;

  PixelSQLCommand que(database_);
  int nextKey ;
  nextKey = que.getNextAvailableVersion("default") ;
  cout << __LINE__ << "]\t[PixelConfigDBInterface::makeKey()]\t\t    New global key reserved: |" << nextKey << "|" << endl ;
  fillHashes() ;

  aliasfile <<  "<?xml version=\"1.0\" encoding=\"utf-8\"?>" 								    << endl ;
  aliasfile <<  ""                                     									    << endl ;
  aliasfile <<  "<!-- " << __LINE__ << "] " << __PRETTY_FUNCTION__  << " -->" 						    << endl ;
  aliasfile <<  ""                                     									    << endl ;
  aliasfile <<  "<ROOT>"                                     								    << endl ;
  aliasfile <<  "  <MAPS>"                                   								    << endl ;
  aliasfile <<  "    <KEY>"                                  								    << endl ;
  aliasfile <<  "      <NAME>" << nextKey << "</NAME>"                                                                      << endl ;
  aliasfile <<  "      <COMMENT_DESCRIPTION>" << comment_ << "</COMMENT_DESCRIPTION>"                                       << endl ;
  aliasfile <<  "      <CREATED_BY_USER>"     << author_  << "</CREATED_BY_USER>"                                           << endl ;
  for(unsigned int i=0;i<versions.size();i++)
    {
      aliasfile                                                                                                             << endl ;
      aliasfile << "      <CONFIGURATION>"                                                                                  << endl ;
      aliasfile << "       <PART>"                                                                                          << endl ;
      aliasfile << "         <PART_ID>1000</PART_ID>"                                                                       << endl ;
      aliasfile << "       </PART>"                                                                                         << endl ;
      aliasfile << "       <KIND_OF_CONDITION>"                                                                             << endl ;
      aliasfile << "           <EXTENSION_TABLE_NAME>" << conversionETNKOC[versions[i].first]  << "</EXTENSION_TABLE_NAME>" << endl ;
      aliasfile << "           <NAME>"                 << conversionNameKOC[versions[i].first] << "</NAME>"                 << endl ;
      aliasfile << "       </KIND_OF_CONDITION>"                                                                            << endl ;
      aliasfile << "       <DATA_SET>"                                                                                      << endl ;
      aliasfile << "         <VERSION>"                << versions[i].second                   << "</VERSION>"              << endl ;
      aliasfile << "         <CREATED_BY_USER>"        << author_                              << "</CREATED_BY_USER>"      << endl ;
      aliasfile << "         <COMMENT_DESCRIPTION>"    << comment_                             << "</COMMENT_DESCRIPTION>"  << endl ;
      aliasfile << "       </DATA_SET>"                                                                                     << endl ;
      aliasfile << "    </CONFIGURATION>"                                                                                   << endl ;
      if(versions[i].first.find("fedcard")!=std::string::npos)
        {
          aliasfile                                                                                                         << endl ;
	  aliasfile << "     <CONFIGURATION>"                                                                               << endl ;
          aliasfile << "       <PART>"                                                                                      << endl ;
	  aliasfile << "         <PART_ID>1000</PART_ID>"                                                                   << endl ;
          aliasfile << "       </PART>"                                                                                     << endl ;
          aliasfile << "       <KIND_OF_CONDITION>"                                                                         << endl ;
          aliasfile << "           <EXTENSION_TABLE_NAME>" << conversionETNKOC["fedcard1"]  << "</EXTENSION_TABLE_NAME>"    << endl ;
          aliasfile << "           <NAME>"                 << conversionNameKOC["fedcard1"] << "</NAME>"                    << endl ;
          aliasfile << "       </KIND_OF_CONDITION>"                                                                        << endl ;
          aliasfile << "       <DATA_SET>"                                                                                  << endl ;
          aliasfile << "         <VERSION>"                << versions[i].second            << "</VERSION>"                 << endl ;
          aliasfile << "       </DATA_SET>"                                                                                 << endl ;
	  aliasfile << "     </CONFIGURATION>"                                                                              << endl ;
          aliasfile                                                                                                         << endl ;
	  aliasfile << "     <CONFIGURATION>"                                                                               << endl ;
          aliasfile << "       <PART>"                                                                                      << endl ;
	  aliasfile << "         <PART_ID>1000</PART_ID>"                                                                   << endl ;
          aliasfile << "       </PART>"                                                                                     << endl ;
          aliasfile << "       <KIND_OF_CONDITION>"                                                                         << endl ;
          aliasfile << "           <EXTENSION_TABLE_NAME>" << conversionETNKOC["fedcard2"]  << "</EXTENSION_TABLE_NAME>"    << endl ;
          aliasfile << "           <NAME>"                 << conversionNameKOC["fedcard2"] << "</NAME>"                    << endl ;
          aliasfile << "       </KIND_OF_CONDITION>"                                                                        << endl ;
          aliasfile << "       <DATA_SET>"                                                                                  << endl ;
          aliasfile << "         <VERSION>"                << versions[i].second            << "</VERSION>"                 << endl ;
          aliasfile << "       </DATA_SET>"                                                                                 << endl ;
	  aliasfile << "     </CONFIGURATION>"                                                                              << endl ;
        } 
    }
  aliasfile <<  "    </KEY>"                                                                                                << endl ;
  aliasfile <<  "  </MAPS>"                                                                                                 << endl ;
  aliasfile <<  "</ROOT>"                                                                                                   << endl ;

  aliasfile.close() ;

  comment_ = "" ;

  return nextKey;
}


//=====================================================================================
void PixelConfigDBInterface::addVersionAlias(std::string path, unsigned int version, std::string alias)
{
  char * cpath = getenv("PIXELCONFIGURATIONSPOOLAREA") ;
  std::stringstream fullPath ;
  fullPath << cpath << "/addVersionAlias_" << path << "_" << version << pos::PixelTimeFormatter::getmSecTime() << ".xma" ;
  std::ofstream aliasfile(fullPath.str().c_str()) ;

  fillHashes() ;

  aliasfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"						      		<< endl ;
  aliasfile <<  ""                                     									<< endl ;
  aliasfile <<  "<!-- " << __LINE__ << "] " << __PRETTY_FUNCTION__  << " -->" 						<< endl ;
  aliasfile <<  ""                                     									<< endl ;
  aliasfile << "<ROOT>"											      		<< endl ;
  aliasfile << "  <MAPS>"										      		<< endl ;
  aliasfile << "    <VERSION_ALIAS>"									      		<< endl ;
  aliasfile << "      <NAME>" << alias << "</NAME>"							      		<< endl ;
  aliasfile << "	<CONFIGURATION>"								      		<< endl ;
  aliasfile												      		<< endl ;
  aliasfile << "	  <PART>"									      		<< endl ;
  aliasfile << "              <PART_ID>1000</PART_ID>"                                                                  << endl ;
  aliasfile << "	  </PART>"									      		<< endl ;
  aliasfile << "	  <KIND_OF_CONDITION>"								      		<< endl ;
  aliasfile << "	      <EXTENSION_TABLE_NAME>" << conversionETNKOC[path]	 << "</EXTENSION_TABLE_NAME>" 		<< endl ;
  aliasfile << "	      <NAME>"		      << conversionNameKOC[path] << "</NAME>"		      		<< endl ;
  aliasfile << "	  </KIND_OF_CONDITION>"								      		<< endl ;
  aliasfile << "	  <DATA_SET>"									      		<< endl ;
  aliasfile << "	    <VERSION>"		      << version		   << "</VERSION>"	      		<< endl ;
  aliasfile << "	  </DATA_SET>"									      		<< endl ;
  aliasfile << "	</CONFIGURATION>"								      		<< endl ;
  aliasfile <<  "    </VERSION_ALIAS>"  << endl ;
  // if this is not fedcard koc close the XML, otherwise append ROC and TBM aliases!!!
  if(path.find("fedcard") == std::string::npos)
    {
      aliasfile <<  "  </MAPS>"             << endl ;
      aliasfile <<  "</ROOT>"               << endl ;
      aliasfile.close() ;
    }

  if(path.find("fedcard") != std::string::npos)
    {
      /*
      fullPath.str("") ;
      fullPath << cpath << "/addVersionAlias_rocAnalogLevels_" << version << pos::PixelTimeFormatter::getmSecTime() << ".xma" ;
      std::ofstream aliasfile(fullPath.str().c_str()) ;
      aliasfile                                                                                                         << endl ;
      aliasfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"						      		<< endl ;
      aliasfile << "<ROOT>"											      	<< endl ;
      aliasfile << "  <MAPS>"										      		<< endl ;
      */
      aliasfile << "    <VERSION_ALIAS>"									      	<< endl ;
      aliasfile << "      <NAME>" << alias << "</NAME>"							      		<< endl ;
      aliasfile << "        <CONFIGURATION>"                                                                            << endl ;
      aliasfile << "          <PART>"                                                                                   << endl ;
      aliasfile << "              <PART_ID>1000</PART_ID>"                                                              << endl ;
      aliasfile << "          </PART>"                                                                                  << endl ;
      aliasfile << "          <KIND_OF_CONDITION>"                                                                      << endl ;
      aliasfile << "              <EXTENSION_TABLE_NAME>" << conversionETNKOC["fedcard1"]  << "</EXTENSION_TABLE_NAME>" << endl ;
      aliasfile << "              <NAME>"                 << conversionNameKOC["fedcard1"] << "</NAME>"                 << endl ;
      aliasfile << "          </KIND_OF_CONDITION>"                                                                     << endl ;
      aliasfile << "          <DATA_SET>"                                                                               << endl ;
      aliasfile << "            <VERSION>"                << version                       << "</VERSION>"              << endl ;
      aliasfile << "          </DATA_SET>"                                                                              << endl ;
      aliasfile << "        </CONFIGURATION>"                                                                           << endl ;
      aliasfile <<  "    </VERSION_ALIAS>"  << endl ;
      /*
      aliasfile <<  "  </MAPS>"             << endl ;
      aliasfile <<  "</ROOT>"               << endl ;
      aliasfile.close() ;

      fullPath.str("") ;
      fullPath << cpath << "/addVersionAlias_tbmAnalogLevels_" << version << pos::PixelTimeFormatter::getmSecTime() << ".xma" ;
      aliasfile.open(fullPath.str().c_str()) ;
      aliasfile                                                                                                         << endl ;
      aliasfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"						      		<< endl ;
      aliasfile << "<ROOT>"											      	<< endl ;
      aliasfile << "  <MAPS>"										      		<< endl ;
      */
      aliasfile << "    <VERSION_ALIAS>"									      	<< endl ;
      aliasfile << "      <NAME>" << alias << "</NAME>"							      		<< endl ;
      aliasfile << "        <CONFIGURATION>"                                                                            << endl ;
      aliasfile << "          <PART>"                                                                                   << endl ;
      aliasfile << "              <PART_ID>1000</PART_ID>"                                                              << endl ;
      aliasfile << "          </PART>"                                                                                  << endl ;
      aliasfile << "          <KIND_OF_CONDITION>"                                                                      << endl ;
      aliasfile << "              <EXTENSION_TABLE_NAME>" << conversionETNKOC["fedcard2"]  << "</EXTENSION_TABLE_NAME>" << endl ;
      aliasfile << "              <NAME>"                 << conversionNameKOC["fedcard2"] << "</NAME>"                 << endl ;
      aliasfile << "          </KIND_OF_CONDITION>"                                                                     << endl ;
      aliasfile << "          <DATA_SET>"                                                                               << endl ;
      aliasfile << "            <VERSION>"                << version                       << "</VERSION>"              << endl ;
      aliasfile << "          </DATA_SET>"                                                                              << endl ;
      aliasfile << "        </CONFIGURATION>"                                                                           << endl ;
      aliasfile <<  "    </VERSION_ALIAS>"  << endl ;
      aliasfile <<  "  </MAPS>"             << endl ;
      aliasfile <<  "</ROOT>"               << endl ;
      aliasfile.close() ;
    } 
//   aliasfile <<  "    </VERSION_ALIAS>"  << endl ;
//   aliasfile <<  "  </MAPS>"             << endl ;
//   aliasfile <<  "</ROOT>"               << endl ;
  
//   aliasfile.close() ;
}

//=====================================================================================
void PixelConfigDBInterface::addAlias(std::string alias, unsigned int key)
{
  char * cpath = getenv("PIXELCONFIGURATIONSPOOLAREA") ;
  std::stringstream fullPath ;
  fullPath << cpath << "/addAlias_" << pos::PixelTimeFormatter::getmSecTime() << ".xma" ;
  std::ofstream aliasfile(fullPath.str().c_str()) ;

  fillHashes() ;  

  aliasfile <<  "<?xml version=\"1.0\" encoding=\"utf-8\"?>"                  << endl ;
  aliasfile <<  ""                                     			      << endl ;
  aliasfile <<  "<!-- " << __LINE__ << "] " << __PRETTY_FUNCTION__  << " -->" << endl ;
  aliasfile <<  ""                                     			      << endl ;
  aliasfile <<  "<ROOT>"                                     		      << endl ;
  aliasfile <<  "  <MAPS>"                                   		      << endl ;
  aliasfile <<  "   <KEY_ALIAS>"                             		      << endl ;
  aliasfile <<  "     <NAME>"     << alias << "</NAME>"      		      << endl ;
  aliasfile <<  "       <KEY mode='keep'>"                   		      << endl ;
  aliasfile <<  "         <NAME>" << key   << "</NAME>"      		      << endl ;
  aliasfile <<  "    </KEY>"                                 		      << endl ;
  aliasfile <<  "   </KEY_ALIAS>"                            		      << endl ;
  aliasfile <<  "  </MAPS>"                                  		      << endl ;
  aliasfile <<  "</ROOT>"                                    		      << endl ;


  aliasfile.close() ;
}

//=====================================================================================
std::vector<std::string> PixelConfigDBInterface::getVersionAliases(std::string koc)
{

  std::string mthn("[PixelConfigDBInterface::getVersionAliases(std::string koc)]\t") ;

  std::map<std::string, std::vector<std::pair<unsigned int, std::string> > > tmp = getVersionData(koc) ;

  std::vector<std::string> result ;

  for(std::map<std::string, std::vector<std::pair<unsigned int, std::string> > >::iterator it = tmp.begin() ;
      it != tmp.end() ; it++)
    {
      for(std::vector<std::pair<unsigned int, std::string> >::iterator it2 = it->second.begin() ;
	  it2 != it->second.end() ; it2++)
	{
	  result.push_back(it2->second) ;
	} 
    }

  return result ;
}

//=====================================================================================
std::map<unsigned int, std::vector<std::vector< std::string> > > PixelConfigDBInterface::getConfGivenKoc(std::map<std::string, std::string> query,
                                                                                 		         int start, 
                                                                                 		         int howMany,
										 		         int from,
										 		         int to, 
										 		         int &total) 
{
  std::string mthn = "]\t[PixelConfigDBInterface::getConfGivenKoc()]\t\t    " ;
  PixelSQLCommand que(database_);
  vector< vector<string> > table ;

  que.openTable("conf_given_koc",query,false,false,"",false);
  getGeneralTable(table , que ) ;
 
  std::map<unsigned int, std::vector<std::vector< std::string> > > result ;
  std::vector<std::string> fieldNames ;

  std::string GK = "" ;

  int counter = -1 ;
  
  total = table.size() ;

//  cout << __LINE__ << mthn << "total: " << total << endl ;

  for(vector< vector<string> >::iterator it = table.begin() ; it != table.end() ; it++)
    {
      if( it == table.begin() ) 
        {
          for(vector<string>::iterator jt = (*it).begin() ; jt != (*it).end() ; jt++)
            {
	      std::string fieldName = (*jt) ;
	      for(unsigned int i=0; i<fieldName.size(); i++)
	        {
		  fieldName[i] = std::tolower(fieldName[i]) ;
		}
	      fieldNames.push_back(fieldName) ;
	    }
	  continue  ;
	}
      counter++ ;
      if( counter < start || counter > start + howMany ) continue ;
      int c = 0 ;
      std::vector<std::vector<std::string> > newvList ;
      for(vector<string>::iterator jt = (*it).begin() ; jt != (*it).end() ; jt++)
        {
      	  if( jt == (*it).begin() )  GK = it->operator[](0) ;
          std::vector<std::string> tmp ;
          tmp.push_back("") ;		 // 0 CONFIG_KEY_ID	    
          tmp.push_back("") ;		 // 1 KEY_NAME  	    
          tmp.push_back("") ;		 // 2 CONDITION_DATA_SET_ID
          tmp.push_back("") ;		 // 3 KIND_OF_CONDITION_ID
          tmp.push_back(fieldNames[c]) ; // 4 KIND_OF_CONDITION_NAME
          tmp.push_back(*jt) ;		 // 5 CONDITION_VERSION  
          tmp.push_back("") ;		 // 6 IS_MOVED_TO_HISTORY
          tmp.push_back("") ;		 // 7 RECORD_INSERTION_TIME
          tmp.push_back("") ;            // 8 COMMENT_DESCRIPTION base46_encode("Comments are unavailable for files")
          tmp.push_back("") ;		 // 9 AUTHOR
          c++ ;
          newvList.push_back(tmp) ;
	}
      
      result[atoi(GK.c_str())] = newvList;
    }
  return result ;
}
										 
//=====================================================================================
bool PixelConfigDBInterface::getVersionAliases(std::string configAlias,
					       unsigned int &key,
					       std::vector<std::pair<std::string,std::string> > &versionAliases)
{

  std::string mthn("]\t[PixelConfigDBInterface::getVersionAliases()]\t\t    ") ;
  /**
     logic:
     1. find a global key by its alias
     1.1 if found go on
     1.1.1 find the key number and put it into variable key
     1.1.2 find version-aliases and put them into vector versionAliases
     1.2 if NOT found return false; exit
   */

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "BEGIN"<< endl ;
    }

  fillHashes() ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn <<std::endl; 
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::vector<std::pair<std::string, unsigned int> > result ;

  where["CONFIG_ALIAS"] = configAlias ;

  que.openTable("config_keys",where,false ,false);
  getGeneralTable(table , que ) ;

  if(table.size() == 1) return false ;
  std::string theKey     = table[1][1] ;
  std::string theTrueKey = table[1][2] ;
  key = atoi(theKey.c_str()) ;
  /**
     view's name : conf_key_alias_version_alias_v

     Name                                Null?    Type
     -------------------------------- -------- --------------------------------------------
     0  CONFIG_KEY_ID                   NUMBER
     1  KEY_NAME                        VARCHAR2
     2  KEY_ALIAS_ID                    NUMBER
     3  KEY_ALIAS                       VARCHAR2
     4  KEY_TYPE                        VARCHAR2
     5  VERSION_ALIAS                   VARCHAR2
     6  VERSION                         VARCHAR2
     7  KIND_OF_CONDITION               VARCHAR2
     8  INSERT_TIME                     TIMESTAMP WITH TIMEZONE
     9  COMMENT_DESCRIPTION             VARCHAR2
     10 AUTHOR                          VARCHAR2  

     view's name : conf_key_alias_version_alias_T

     0  CONFIG_KEY_ID                   NUMBER
     1  KEY_NAME                        VARCHAR2
     2  KEY_ALIAS_ID                    NUMBER
     3  KEY_ALIAS                       VARCHAR2
     4  KEY_TYPE                        VARCHAR2
     5  VERSION_ALIAS_ID                NUMBER
     6  VERSION_ALIAS                   VARCHAR2
     7  KIND_OF_CONDITION_ID            NUMBER
     8  KIND_OF_CONDITION               VARCHAR2
     9  VERSION                         VARCHAR2
     10 INSERT_TIME                     TIMESTAMP WITH TIMEZONE
     11 COMMENT_DESCRIPTION             VARCHAR2
     12 AUTHOR                          VARCHAR2  
  */
  
  table.clear() ;
  where.clear() ;
  
  where["KEY_ALIAS_ID"] = theTrueKey ;
  
  que.openTable("ka_va",where,false ,false);
  getGeneralTable(table , que ) ;

  std::pair<std::string, std::string> tmp ;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "Exploring " << configAlias << " connected to GK " << key << " using TrueDBKey " << theTrueKey << endl ;
    }
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      tmp.first  = conversionKOC[it->operator[](8)] ;
      tmp.second = it->operator[](6) ;
      if(DEBUG_CONFIG_DB)
        {
      	  cout << __LINE__ << mthn << "|" << it->operator[](8) << "|  --> |" << conversionKOC[it->operator[](8)] << "|" ;
      	  cout << " aliased to |" << it->operator[](6) << "|  " << endl ;
        }
      versionAliases.push_back(tmp) ;
    }

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "END"<< endl ;
    }

  return true;
}

//=====================================================================================
std::map<std::string, unsigned int> PixelConfigDBInterface::getAliases_map()
{
  std::map<std::string, unsigned int> result ;
  std::vector<std::pair<std::string, unsigned int> >  tmp = getAliases() ;
  for(std::vector<std::pair<std::string, unsigned int> >::iterator it = tmp.begin() ;
      it != tmp.end() ; it++)
    {
      result[(*it).first] = (*it).second ; 
    }
  return result ;
}

//=====================================================================================
std::map<std::string, std::vector<std::pair<unsigned int, std::string> > > PixelConfigDBInterface::getVersionData()
{
  std::string mthn("[PixelConfigDBInterface::getVersionData()]\t") ;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::map<std::string, std::vector<std::pair<unsigned int, std::string> > > result ;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "BEGIN"<< endl ;
    }
  fillHashes() ;

  /**
     view's name : conf_key_alias_version_alias_v

     Name                                Null?    Type
     -------------------------------- -------- --------------------------------------------
     0     VERSION_ALIAS
     1     VERSION
     2     KIND_OF_CONDITION
  */

  que.openVersionAliasTable() ;
  getGeneralTable(table , que ) ;
  std::pair<unsigned int, std::string> tmp ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      tmp.first  = atoi(it->operator[](1).c_str());
      tmp.second = it->operator[](0) ;
//       cout << "|" << it->operator[](6) << "|  --> |" << conversionKOC[it->operator[](6)] << "|" << endl ;
//       cout << "|" << it->operator[](4) << "|  " << endl ;

      result[conversionKOC[it->operator[](2)]].push_back(tmp) ;
    }

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << "]\t" << mthn << "END"<< endl ;
    }
  return result ;
  


  /**  
  que.openTable("CMS_PXL_PIXEL_VIEW_OWNER.CONF_KEY_ALIAS_VERSION_ALIAS_V",where,false ,false);
  getGeneralTable(table , que ) ;

  std::pair<unsigned int, std::string> tmp ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      tmp.first  = atoi(it->operator[](5).c_str());
      tmp.second = it->operator[](4) ;
//       cout << "|" << it->operator[](6) << "|  --> |" << conversionKOC[it->operator[](6)] << "|" << endl ;
//       cout << "|" << it->operator[](4) << "|  " << endl ;

      result[conversionKOC[it->operator[](6)]].push_back(tmp) ;
    }
  return result ;
  */
}

//=====================================================================================
std::map<std::string, std::vector<std::pair<unsigned int, std::string> > > PixelConfigDBInterface::getVersionData(std::string koc)
{
  std::string mthn("]\t[PixelConfigDBInterface::getVersionData(string)]\t    ") ;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::map<std::string, std::vector<std::pair<unsigned int, std::string> > > result ;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "BEGIN"<< endl ;
    }
  fillHashes() ;

  /**
     view's name : conf_key_alias_version_alias_v

     Name                                Null?    Type
     -------------------------------- -------- --------------------------------------------
     0     VERSION_ALIAS
     1     VERSION
     2     KIND_OF_CONDITION
  */
   
  que.openVersionAliasTable(conversionNameKOC[koc]) ;
  getGeneralTable(table , que ) ;
  std::pair<unsigned int, std::string> tmp ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      tmp.first  = atoi(it->operator[](1).c_str());
      tmp.second = it->operator[](0) ;
//       cout << "|" << it->operator[](6) << "|  --> |" << conversionKOC[it->operator[](6)] << "|" << endl ;
//       cout << "|" << it->operator[](4) << "|  " << endl ;

      result[conversionKOC[it->operator[](2)]].push_back(tmp) ;
    }

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "END"<< endl ;
    }
  return result ;
}

//=====================================================================================
std::vector<std::vector< std::string> >  PixelConfigDBInterface::getVersionDataWithComments(std::string koc)
{
  std::string mthn("]\t[PixelConfigDBInterface::getVersionDataWithComments()]\t    ") ;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "BEGIN"<< endl ;
    }
  fillHashes() ;

  que.openVersionAliasTable(conversionNameKOC[koc]) ;
  getGeneralTable(table , que ) ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      it->operator[](5) = conversionKOC[it->operator[](5)] ;
//      cout << __LINE__ << mthn << it->operator[](5)<< endl ;
    }

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "END"<< endl ;
    }
  return table ;
}

//=====================================================================================
std::string PixelConfigDBInterface::uploadStatus(std::string uploadedFile)
{
  std::string mthn("[PixelConfigDBInterface::uploadStatus(string)]\t") ;

  /**
     TABLE NAME: CMS_PXL_CORE_MANAGEMNT_OWNER.CONDITIONS_DATA_AUDITLOG

    0  RECORD_ID                   NUMBER(38)        NOT NULL,
    1  RECORD_INSERTION_TIME       TIMESTAMP(6) WITH TIME ZONE NOT NULL,
    2  RECORD_INSERTION_USER       VARCHAR2(50 BYTE) NOT NULL,
    3  RECORD_LASTUPDATE_TIME      TIMESTAMP(6) WITH TIME ZONE,
    4  RECORD_LASTUPDATE_USER      VARCHAR2(50 BYTE),
    5  COMMENT_DESCRIPTION         VARCHAR2(4000 BYTE),
    6  ARCHVE_FILE_NAME            VARCHAR2(4000 BYTE) NOT NULL,
    7  DATA_FILE_NAME              VARCHAR2(4000 BYTE) NOT NULL,
    8  DATA_FILE_CHECKSUM          VARCHAR2(200 BYTE) NOT NULL,
    9  UPLOAD_STATUS               VARCHAR2(50 BYTE) NOT NULL,
    10 UPLOAD_HOSTNAME             VARCHAR2(200 BYTE) NOT NULL,
    11 UPLOAD_SOFTWARE             VARCHAR2(50 BYTE) NOT NULL,
    12 UPLOAD_TIME_SECONDS         NUMBER(38)        NOT NULL,
    13 UPLOAD_LOG_TRACE            VARCHAR2(4000 BYTE),
    14 CREATE_TIMESTAMP            TIMESTAMP(6) WITH TIME ZONE,
    15 CREATED_BY_USER             VARCHAR2(50 BYTE),
    16 VERSION                     VARCHAR2(40 BYTE),
    17 SUBVERSION                  NUMBER(38),
    18 KIND_OF_CONDITION_NAME      VARCHAR2(40 BYTE),
    19 EXTENSION_TABLE_NAME        VARCHAR2(30 BYTE),
    20 SUBDETECTOR_NAME            VARCHAR2(20 BYTE),
    21 RUN_TYPE                    VARCHAR2(40 BYTE),
    22 RUN_NUMBER                  NUMBER(38),
    23 TAG_NAME                    VARCHAR2(40 BYTE),
    24 INTERVAL_OF_VALIDITY_BEGIN  NUMBER(38),
    25 INTERVAL_OF_VALIDITY_END    NUMBER(38),
    26 DATASET_COUNT               NUMBER(38),
    27 DATASET_RECORD_COUNT        NUMBER(38),
    28 DATA_RELATED_TO_LIST        VARCHAR2(4000 BYTE)
  */
    
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  bool result = 1 ;


//   where["ARCHVE_FILE_NAME"] = uploadedFile ;
//   que.openTable("auditlog",where,false ,true);

// The following ad-hoc query simply retrieves ARCHVE_FILE_NAME and UPLOAD_STATUS columns from the table!!
  que.openConditionDataAuditlog(uploadedFile) ;
  getGeneralTable(table , que ) ;
  if(table.size() == 1) return std::string("progress") ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      if((*it)[1].find("Success") != std::string::npos)
	{
	  result *= 1 ;
	}
      else
	{
	  result *= 0 ;
	}
    }
  // we have success match: check if we were complete or only partial...
  if(result==1)
    {
      for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
	{
	  if((*it)[1] ==  "Success" )
	    return std::string("true") ;
	}
      // if we are here it means that we got only Success[*], but no final
      // "Success" --> return "progress"
      return std::string("progress") ;
    }

  cout << __LINE__ << mthn << uploadedFile << " " << result << endl ;
  //  if(result) return std::string("true") ;
  return std::string("false") ;
}
//=====================================================================================
std::set<unsigned int > PixelConfigDBInterface::getExistingVersions(std::string koc)
{
  std::string mthn("]\t[PixelConfigDBInterface::getExistingVersions()]\t\t    ") ;
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  set< unsigned int> result ;

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "BEGIN"<< endl ;
    }
  fillHashes() ;

  que.openExistingVersionsTable(conversionETNKOC[koc]) ;
  getGeneralTable(table , que ) ;
  std::pair<unsigned int, std::string> tmp ;
  for(vector< vector<string> >::iterator it = table.begin()+1 ; it != table.end() ; it++)
    {
      result.insert( (unsigned int) atoi(it->operator[](0).c_str()));
    }

  if(DEBUG_CONFIG_DB)
    {
      cout << __LINE__ << mthn << "END"<< endl ;
    }
  return result ;
}

//=====================================================================================
void PixelConfigDBInterface::setKeyAlias(pos::PixelConfigKey key, std::string theAlias)
{
  PixelSQLCommand que(database_);
  map< string , string > where;
  vector< vector<string> > table ;
  std::stringstream whereBuilder ;
  /**
     view's name : CONF_KEY_CONFIG_KEYS_V

     Name                                Null?    Type
     -------------------------------- -------- --------------------------------------------
     0 CONFIG_KEY_ID                    NOT NULL NUMBER(38)
     1 CONFIG_KEY                       NOT NULL VARCHAR2(80)
     2 KEY_ALIAS_ID                     NOT NULL NUMBER(38)
     3 CONFIG_ALIAS                     NOT NULL VARCHAR2(80)
     4 CONFIG_KEY_TYPE                  NOT NULL VARCHAR2(80)
  */
  whereBuilder << key.key() ;
  where["CONFIG_KEY"]   = whereBuilder.str() ;
  where["CONFIG_ALIAS"] = theAlias ;
  que.openTable("config_keys",where,false ,false);
  getGeneralTable(table , que ) ;
  
  theTrueDBKey_ = table[1][2] ;
}

//=====================================================================================
void PixelConfigDBInterface::fillHashes()
{
  
  conversionKOC["LTC Configuration Parameters"  ] = "ltcconfig";
  conversionKOC["Pixel Port Card Map"           ] = "portcardmap";
  conversionKOC["ROC Trim Bits"                 ] = "trim";
  conversionKOC["Pixel Detector Configuration"  ] = "detconfig";
  conversionKOC["Pixel Port Card Settings"      ] = "portcard";
  conversionKOC["Pixel FEC Parameters"          ] = "fecconfig";
  conversionKOC["Pixel FED Crate Configuration" ] = "fedconfig";
  conversionKOC["Pixel Name Translation"        ] = "nametranslation";
  conversionKOC["TTC Configuration Parameters"  ] = "ttcciconfig";
  conversionKOC["ROC DAC Settings Col"          ] = "dac";
  conversionKOC["Tracker FEC Parameters"        ] = "tkfecconfig";
  conversionKOC["Pixel TBM Parameters"          ] = "tbm";
  conversionKOC["Pixel FED Configuration"       ] = "fedcard";
  conversionKOC["ROC Analog Levels"             ] = "roclevels";
  conversionKOC["TBM Analog Levels"             ] = "tbmlevels";
  conversionKOC["Calibration Object Clob"       ] = "calib";
  conversionKOC["ROC Mask Bits"                 ] = "mask";
  conversionKOC["ROC MaxVsf Setting"            ] = "maxvsf";
  conversionKOC["XDAQ Low Voltage Map"          ] = "lowvoltagemap" ;
  conversionKOC["Pixel Global Delay25"          ] = "globaldelay25"         ;

  conversionNameKOC["globaldelay25"  ] = "Pixel Global Delay25"         ;
  conversionNameKOC["lowvoltagemap"  ] = "XDAQ Low Voltage Map"         ;
  conversionNameKOC["ltcconfig"      ] = "LTC Configuration Parameters" ;      
  conversionNameKOC["portcardmap"    ] = "Pixel Port Card Map"          ;       
  conversionNameKOC["trim"           ] = "ROC Trim Bits"                ;       
  conversionNameKOC["detconfig"      ] = "Pixel Detector Configuration" ;       
  conversionNameKOC["portcard"       ] = "Pixel Port Card Settings"     ;       
  conversionNameKOC["fecconfig"      ] = "Pixel FEC Parameters"         ;       
  conversionNameKOC["fedconfig"      ] = "Pixel FED Crate Configuration";       
  conversionNameKOC["nametranslation"] = "Pixel Name Translation"       ;       
  conversionNameKOC["ttcciconfig"    ] = "TTC Configuration Parameters" ;       
  conversionNameKOC["dac"            ] = "ROC DAC Settings Col"         ;       
  conversionNameKOC["tkfecconfig"    ] = "Tracker FEC Parameters"       ;       
  conversionNameKOC["tbm"            ] = "Pixel TBM Parameters"         ;       
  conversionNameKOC["fedcard"        ] = "Pixel FED Configuration"      ;       
  conversionNameKOC["fedcard1"       ] = "ROC Analog Levels"            ;       
  conversionNameKOC["fedcard2"       ] = "TBM Analog Levels"            ;       
  conversionNameKOC["calib"          ] = "Calibration Object Clob"      ;       
  conversionNameKOC["mask"           ] = "ROC Mask Bits"                ;       
  conversionNameKOC["maxvsf"         ] = "ROC MaxVsf Setting"           ;       

  conversionETNKOC["globaldelay25"   ] = "PIXEL_GLOBAL_DELAY25"         ;
  conversionETNKOC["lowvoltagemap"   ] = "XDAQ_LOW_VOLTAGE_MAP"         ;
  conversionETNKOC["ltcconfig"       ] = "PIXEL_LTC_PARAMETERS"         ;      
  conversionETNKOC["portcardmap"     ] = "PIXEL_PORTCARD_MAP"           ;       
  conversionETNKOC["trim"            ] = "ROC_TRIMS"                    ;       
  conversionETNKOC["detconfig"       ] = "PIXEL_DETECTOR_CONFIG"        ;       
  conversionETNKOC["portcard"        ] = "PIXEL_PORTCARD_SETTINGS"      ;       
  conversionETNKOC["fecconfig"       ] = "PIXEL_FEC_PARAMETERS"         ;       
  conversionETNKOC["fedconfig"       ] = "FED_CRATE_CONFIG"             ;       
  conversionETNKOC["nametranslation" ] = "PIXEL_NAME_TRANSLATION"       ;       
  conversionETNKOC["ttcciconfig"     ] = "PIXEL_TTC_PARAMETERS"         ;       
  conversionETNKOC["dac"             ] = "ROC_DAC_SETTINGS_COL"         ;       
  conversionETNKOC["tkfecconfig"     ] = "TRACKER_FEC_PARAMETERS"       ;       
  conversionETNKOC["tbm"             ] = "PIXEL_TBM_PARAMETERS"         ;       
  conversionETNKOC["fedcard"         ] = "FED_CONFIGURATION"            ;       
  conversionETNKOC["fedcard1"        ] = "ROC_ANALOG_LEVELS"            ;       
  conversionETNKOC["fedcard2"        ] = "TBM_ANALOG_LEVELS"            ;       
  conversionETNKOC["calib"           ] = "PIXEL_CALIB_CLOB"             ;       
  conversionETNKOC["mask"            ] = "ROC_MASKS"                    ;       
  conversionETNKOC["maxvsf"          ] = "ROC_MAXVSF"                   ;       
}

//=====================================================================================
std::string PixelConfigDBInterface::commitToDB(unsigned int globalKey)
{
  string mthn = "[PixelConfigDBInterface::commitToDB()]\t\t\t    " ;
  std::stringstream s ;
  int time     = 0   ; // in seconds
  int maxTime  = 120  ; // in seconds
  int interval = 5   ; // in seconds 
  std::string timestamp(pos::PixelTimeFormatter::getmSecTime());
  
  std::string zipperfile;
  if (getenv("BUILD_HOME")==0) {    //for running from RPM
    zipperfile=std::string(getenv("XDAQ_ROOT"))+"/util/PixelConfigDBGUI/util/zipper.pl";
  }
  else { //for non-rpm running
    zipperfile=std::string(getenv("BUILD_HOME"))+"/pixel/PixelConfigDBGUI/util/zipper.pl";
  }
      
  s << zipperfile << " "<< globalKey << " " << timestamp ;

  unsigned int retStatus = system(s.str().c_str()) ;
  if( retStatus != 0 )
    {
      XCEPT_RAISE (xdaq::exception::Exception,"Database error: A fatal error occurred while shipping zip file to final spoolarea");
    }
  s.str("") ;
  s << "globalKey_" <<  globalKey << "_" << timestamp << ".zip" ;
  cout << __LINE__ << mthn << "Querying DB for success [" << time << "/" << maxTime << "]"<< endl ;
  string result = uploadStatus(s.str()) ;
  cout << __LINE__ << mthn << "Querying DB for success [" << time << "/" << maxTime << "]"<< result << endl ;
  while(result != "true" && time < maxTime)
    {
      sleep(interval)  ;
      time += interval ;
      result = uploadStatus(s.str()) ;
      cout << __LINE__ << mthn << "Querying DB for success [" << time << "/" << maxTime << "]"<< result << endl ;
    }
  /*
  if(result == "true" )
    {
      // sleep for an addition 2*interval time and test again the database, in case some XML file in the main zip file got processed later...
      // An additional check to see if result is indeed true!!
      sleep(2*interval) ;
      result = uploadStatus(s.str()) ;
      if(result != "true") assert(0) ;
    }
  */
  if(time >= maxTime)
    {
      XCEPT_RAISE (xdaq::exception::Exception,"Database error: Did not receive success from database within timeout ");
    }
  return s.str() ;
}

//=====================================================================================
bool PixelConfigDBInterface::checkDatabaseResults(int size, string koc)
{
  if(size == 1)
    {
      cout << __LINE__ << "]\t[PixelConfigDBInterface::checkDatabaseResults()]\t    "
	   << "DB request for " << koc << " produced 0 results from DataBase" << endl ;
      //           assert(0) ;
      return false;
    }
  return true;
}

// modified by MR on 22-12-2008 10:02:10
// New caching mechanism server-side. Not the best choice, but this requires no modification to POS Supervisors...

//=====================================================================================
void PixelConfigDBInterface::getAndCacheData(vector< vector<string> > & databaseTable_, 
					     string moduleName_,
					     unsigned int GK)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getAndCacheData()]\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << moduleName_ << std::endl;
    }
  databaseTable_.clear() ;  
  string koc = getProperKOC(moduleName_) ;
  
  if(dataIsInCache(koc, GK))
    {
      findAndFillTable(databaseTable_,koc, moduleName_) ;
    }
  else
    {
      resetCache(koc) ;
      getDataAndFillCache(koc, GK, databaseTable_) ;
      findAndFillTable(databaseTable_,koc, moduleName_) ;
    }
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
bool PixelConfigDBInterface::dataIsInCache(string koc, unsigned int GK)
{
  std::string mthn = "]\t[PixelConfigDBInterface::dataIsInCache()]\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }

  if (cacheIndex_.find(koc) != cacheIndex_.end() )
    {
      if(cacheIndex_[koc] == GK) return true ;
        std::cout << __LINE__ << "]\t" << mthn << "Data found in Cache!" << std::endl ;
    }

  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }

  return false ;
}

//=====================================================================================
void PixelConfigDBInterface::findAndFillTable(vector<vector<string> > & dt, string koc, string moduleName_) 
{
  std::string mthn = "]\t[PixelConfigDBInterface::findAndFillTable()]\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << moduleName_ << " on " << koc << std::endl;
    }
  dt.clear() ;
  if(koc.find("TRIM") != std::string::npos)
    {
      //   01234567890
      //   pixel/trim/
      moduleName_.erase(0,11) ;
      if (cacheTRIM_.find(moduleName_) != cacheTRIM_.end())
	dt = cacheTRIM_[moduleName_];
    }

  if(koc.find("MASK") != std::string::npos)
    {
      //   01234567890
      //   pixel/mask/
      moduleName_.erase(0,11) ;
      if (cacheMASK_.find(moduleName_) != cacheMASK_.end())
	dt = cacheMASK_[moduleName_];
    }

  if(koc.find("DAC") != std::string::npos)
    {
      //   01234567890
      //   pixel/dac/
      moduleName_.erase(0,10) ;
      if (cacheDAC_.find(moduleName_) != cacheDAC_.end())
	dt = cacheDAC_[moduleName_];
    }

  if(koc.find("TBM") != std::string::npos)
    {
      //   01234567890
      //   pixel/tbm/
      moduleName_.erase(0,10) ;
      if (cacheTBM_.find(moduleName_) != cacheTBM_.end())
	dt = cacheTBM_[moduleName_];
    }

    if(koc.find("PORTCARDCONFIG") != std::string::npos)
    {
      //   012345678901234
      //   pixel/portcard/
      moduleName_.erase(0,15);
      if (cachePTCARDCFG_.find(moduleName_) != cachePTCARDCFG_.end())
        dt = cachePTCARDCFG_[moduleName_];
    }


  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "Returning a table of size: " << dt.size() <<std::endl;  
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::resetCache(string koc)
{
  std::string mthn = "]\t[PixelConfigDBInterface::resetCache()]\t\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }
  if(koc.find("TRIM") != std::string::npos)
    {
      cacheTRIM_.clear() ;
    }
  if(koc.find("MASK") != std::string::npos)
    {
      cacheMASK_.clear() ;
    }
  if(koc.find("DAC") != std::string::npos)
    {
      cacheDAC_.clear() ;
    }
  if(koc.find("TBM") != std::string::npos)
    {
      cacheTBM_.clear() ;
    }

  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::getDataAndFillCache(string koc, unsigned int GK, vector< vector<string> > & databaseTable_) 
{
  std::string mthn = "]\t[PixelConfigDBInterface::getDataAndFillCache()]\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }

  if(koc.find("TRIM") != std::string::npos)
    {
      getAllTRIMDataSets(koc, GK, databaseTable_) ;
      fillTRIMCache(databaseTable_) ;
      fillGeneralCacheTable(koc, GK) ;
    }
  if(koc.find("MASK") != std::string::npos)
    {
      getAllMASKDataSets(koc, GK, databaseTable_) ;
      fillMASKCache(databaseTable_) ;
      fillGeneralCacheTable(koc, GK) ;
    }
  if(koc.find("DAC") != std::string::npos)
    {
      getAllDACDataSets(koc, GK, databaseTable_) ;
      fillDACCache(databaseTable_) ;
      fillGeneralCacheTable(koc, GK) ;
    }
  if(koc.find("TBM") != std::string::npos)
    {
      getAllTBMDataSets(koc, GK, databaseTable_) ;
      fillTBMCache(databaseTable_) ;
      fillGeneralCacheTable(koc, GK) ;
    }
  if(koc.find("PORTCARDCONFIG") != std::string::npos)
    {
      getAllPTCARDCFGDataSets(koc, GK, databaseTable_) ;
      fillPTCARDCFGCache(databaseTable_) ;
      fillGeneralCacheTable(koc, GK) ;
    }


  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::fillTRIMCache(vector< vector<string> > & dt)
{
  /** We changed a bit the logic of the caching to try and make it
      faster. We now store the base name of each ROC as the main
      map-cache key, not the full ROC name. For the Forward
      detector, this means going from
      FPix_BmO_D2_BLD8_PNL2_PLQ1_ROC0 to FPix_BmO_D2_BLD8_PNL2,
      while for the Barrel case this means going from
      BPix_BpO_SEC8_LYR3_LDR21F_MOD4_ROC0 to
      BPix_BpO_SEC8_LYR3_LDR21F_MOD4. This means that whenever a
      Supervisor is asking data we do not have to loop over the
      full cache and match the requested baseName against the ROC
      name stored in the cache, but we simply address the cache by
      baseName. It will be slower in filling the cache but much
      faster, we hope, in getting data out of it. Care must be
      taken in properly dropping the unused part of the ROC name
      to obtain the correct baseName. Due to the way they are
      assembled, first we need to remove the common _ROC part and
      only after the additional PLQ part for the FPix case. We
      also add the METADATA row for each basename entry, so that
      we can void copying around the vectors and use just
      references tot he cache, which will have everything
      needed.*/

  std::string mthn = "]\t[PixelConfigDBInterface::fillTRIMCache()]\t\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin()"<<std::endl;
    }
  /**   // modified by MR on 18-06-2008 09:18:48
        CONF_KEY_ROC_TRIMS_V
        Name                                      Null?    Type
        ----------------------------------------- -------- ----------------------------
        0 CONFIG_KEY                                         VARCHAR2(80)
	1 KEY_TYPE                                           VARCHAR2(80)
	2 KEY_ALIAD_ID
	3 KEY_ALIAS
	4 VERSION
	5 KIND_OF_CONDITION
	6 ROC_NAME
	7 TRIM_BITS
  */
  std::vector< std::vector<std::string > > empty;
  for(unsigned int i = 1 ; i < dt.size() ; i++)
    {
      //       std::cout << __LINE__ << "]\t" << mthn << "Filling Cache for " << dt[i][6]<< std::endl ;
      std::string name(dt[i][6]);
      baseNameFromFullName(name);
      if (cacheTRIM_.find(name) != cacheTRIM_.end())
	cacheTRIM_[name].push_back(dt[i]);
      else
      {
	cacheTRIM_[name] = empty;
	cacheTRIM_[name].push_back(dt[0]) ;
	cacheTRIM_[name].push_back(dt[i]) ;
      }
    }
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::fillMASKCache(vector< vector<string> > & dt)
{
  std::string mthn = "]\t[PixelConfigDBInterface::fillMASKCache()]\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin()"<<std::endl;
    }
  /**   // modified by MR on 18-06-2008 09:18:48
        CONF_KEY_ROC_MASKS_V
        Name                                      Null?    Type
        ----------------------------------------- -------- ----------------------------
        0 CONFIG_KEY                                         VARCHAR2(80)
	1 KEY_TYPE                                           VARCHAR2(80)
	2 KEY_ALIAD_ID
	3 KEY_ALIAS
	4 VERSION
	5 KIND_OF_CONDITION
	6 ROC_NAME
	7 KILL_MASK
  */
  std::vector< std::vector<std::string > > empty;
  for(unsigned int i = 1 ; i < dt.size() ; i++)
    {
//       std::cout << __LINE__ << "]\t" << mthn << "Filling Cache for " << dt[i][6]<< std::endl ;
      std::string name(dt[i][6]);
      baseNameFromFullName(name);
      if (cacheMASK_.find(name) != cacheMASK_.end())
	cacheMASK_[name].push_back(dt[i]);
      else
      {
	cacheMASK_[name] = empty;
	cacheMASK_[name].push_back(dt[0]) ;
	cacheMASK_[name].push_back(dt[i]) ;
      }
    }
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::fillDACCache(vector< vector<string> > & dt)
{
  std::string mthn = "]\t[PixelConfigDBInterface::fillDACCache()]\t\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin()"<<std::endl;
    }
  std::vector< std::vector<std::string > > empty;
  for(unsigned int i = 1 ; i < dt.size() ; i++)
    {
//       std::cout << __LINE__ << "]\t" << mthn << "Filling Cache for " << dt[i][6]<< std::endl ;
      std::string name(dt[i][6]);
      baseNameFromFullName(name);
      if (cacheDAC_.find(name) != cacheDAC_.end())
	cacheDAC_[name].push_back(dt[i]);
      else
      {
	cacheDAC_[name] = empty;
	cacheDAC_[name].push_back(dt[0]) ;
	cacheDAC_[name].push_back(dt[i]) ;
      }
    }
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::fillTBMCache(vector< vector<string> > & dt)
{
  std::string mthn = "]\t[PixelConfigDBInterface::fillTBMCache()]\t\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin()"<<std::endl;
    }
  std::vector< std::vector<std::string > > empty;
  for(unsigned int i = 1 ; i < dt.size() ; i++)
    {
//       std::cout << __LINE__ << "]\t" << mthn << "Filling Cache for " << dt[i][7]<< std::endl ;
      std::string name(dt[i][7]);
      baseNameFromFullName(name);
      if (cacheTBM_.find(name) != cacheTBM_.end())
	cacheTBM_[name].push_back(dt[i]);
      else
      {
	cacheTBM_[name] = empty;
	cacheTBM_[name].push_back(dt[0]) ;
	cacheTBM_[name].push_back(dt[i]) ;
      }
    }
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::fillPTCARDCFGCache(vector< vector<string> > & dt)
{
  std::string mthn = "]\t[PixelConfigDBInterface::fillPTCARDCFGCache()]\t\t\t    " ;
  unsigned int index = 0;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin()"<<std::endl;
    }
  /** Extra care must be taken in order to properly select the column
   * over which we would like to index the cached data. In this case
   * we want to be able to quickly select data based on the portcard's
   * name: that's why we loop over all columns' names and store the
   * appropriate index, whose value is used as the key in the mapping
   * cache.  TODO implement the same logic in all the similar
   * methods.  */
  std::vector< std::vector<std::string > > empty;
  for (; index < dt[0].size(); ++index)
    if (dt[0][index] == "PORT_CARD")
      break;
  
  for(unsigned int i = 1 ; i < dt.size() ; i++)
    {
//       std::cout << __LINE__ << "]\t" << mthn << "Filling Cache for " << dt[i][6]<< std::endl ;
      std::string name(dt[i][index]);
      if (cachePTCARDCFG_.find(name) != cachePTCARDCFG_.end())
        cachePTCARDCFG_[name].push_back(dt[i]);
      else
      {
        cachePTCARDCFG_[name] = empty;
        cachePTCARDCFG_[name].push_back(dt[0]) ;
        cachePTCARDCFG_[name].push_back(dt[i]) ;
      }
    }
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::fillGeneralCacheTable(string koc, unsigned int GK)
{
  std::string mthn = "]\t[PixelConfigDBInterface::fillGeneralCacheTable()]\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }
  cacheIndex_[koc] = GK ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
string PixelConfigDBInterface::getProperKOC(string moduleName_)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getProperKOC()]\t\t    " ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << moduleName_ << std::endl;
    }
  if(moduleName_.find("mask") != std::string::npos) {return "MASK" ;}
  if(moduleName_.find("trim") != std::string::npos) {return "TRIM" ;}
  if(moduleName_.find("dac")  != std::string::npos) {return "DAC" ;}
  if(moduleName_.find("tbm")  != std::string::npos) {return "TBM" ;}
  if(moduleName_.find("portcard")  != std::string::npos) {return "PORTCARDCONFIG" ;}
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
  return "" ;
}

//=====================================================================================
void PixelConfigDBInterface::getAllTRIMDataSets(string koc, unsigned int GK, vector< vector<string> > &databaseTable_)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getAllTRIMDataSets()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

  /**   // modified by MR on 18-06-2008 09:18:48
        CONF_KEY_ROC_TRIMS_V
        Name                                      Null?    Type
        ----------------------------------------- -------- ----------------------------
        0 CONFIG_KEY                                         VARCHAR2(80)
	1 KEY_TYPE                                           VARCHAR2(80)
	2 KEY_ALIAD_ID
	3 KEY_ALIAS
	4 VERSION
	5 KIND_OF_CONDITION
	6 ROC_NAME
	7 TRIM_BITS
  */
  //where["ROC_NAME"] = module+"_PLQ2" ;
   
  clause << GK ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ;//clause.str() ;
  que.openTable("trim",where,false ,false);
  getGeneralTable(databaseTable_ , que ) ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::getAllMASKDataSets(string koc, unsigned int GK, vector< vector<string> > &databaseTable_)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getAllMASKDataSets()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;

  /**   // modified by MR on 18-06-2008 09:18:48
        CONF_KEY_ROC_MASKS_V
        Name                                      Null?    Type
        ----------------------------------------- -------- ----------------------------
        0 CONFIG_KEY                                         VARCHAR2(80)
	1 KEY_TYPE                                           VARCHAR2(80)
	2 KEY_ALIAD_ID
	3 KEY_ALIAS
	4 VERSION
	5 KIND_OF_CONDITION
	6 ROC_NAME
	7 KILL_MASK
  */
   
  clause << GK ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ;//clause.str() ;
  que.openTable("mask",where,false ,false);
  getGeneralTable(databaseTable_ , que ) ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::getAllDACDataSets(string koc, unsigned int GK, vector< vector<string> > &databaseTable_)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getAllDACDataSets()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
   
  clause << GK ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ;//clause.str() ;
  que.openTable("dac",where,false ,false);
  getGeneralTable(databaseTable_ , que ) ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::getAllTBMDataSets(string koc, unsigned int GK, vector< vector<string> > &databaseTable_)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getAllTBMDataSets()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
   
  clause << GK ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ;//clause.str() ;
  que.openTable("tbm",where,false, false );
  getGeneralTable(databaseTable_ , que ) ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

void PixelConfigDBInterface::getAllPTCARDCFGDataSets(string koc, unsigned int GK, vector< vector<string> > &databaseTable_)
{
  std::string mthn = "]\t[PixelConfigDBInterface::getAllPTCARDCFGDataSets()]\t\t    ";
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "begin() for " << koc << std::endl;
    }
  std::vector< string > col;
  PixelSQLCommand que(database_);
  map< string , string > where;
  stringstream clause ;
   
  clause << GK ;
  where["CONFIG_KEY"] = clause.str() ; // theTrueDBKey_  ;//clause.str() ;
  que.openTable("portcard",where,false ,false);
  getGeneralTable(databaseTable_ , que ) ;
  if(DEBUG_CONFIG_DB)
    {
      std::cout<< __LINE__ << mthn << "end()"<<std::endl;
    }
}

//=====================================================================================
void PixelConfigDBInterface::dumpTableInfo(const char* table_name, const char* where, bool bForUpdate)
{
  std::string mthn = "]\t[PixelConfigDBInterface::dumpTableInfo()]\t\t    ";
  PixelSQLCommand que(database_);
  que.openTable(table_name, where, bForUpdate);
  std::cout << __LINE__ << mthn << "Summary for Table/View: " << table_name << std::endl ;
  que.dumpTableInfo() ;
  std::cout << __LINE__ << mthn << std::endl << std::endl ;
}

void PixelConfigDBInterface::baseNameFromFullName(std::string & fullName)
{
  size_t found;
  fullName.replace(fullName.rfind("_ROC"), fullName.length(), "");
  if ((found=fullName.rfind("_PLQ")) != std::string::npos)
    fullName.replace(found, fullName.length(), "");
}
