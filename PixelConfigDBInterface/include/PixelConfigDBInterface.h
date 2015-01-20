/*************************************************************************
 * XDAQ Components for Pixel Online Software                             *
 * Copyright (C) 2007, 2009, I.N.F.N. Milano Bicocca                     *
 * All rights reserved.                                                  *
 * Authors: Dario Menasce, Marco Rovere                                  *
$Author: kreis $
$Date: 2012/01/19 16:45:55 $
$Revision: 1.72 $
 ************************************************************************/

#ifndef PixelConfigDBInterface_h
#define PixelConfigDBInterface_h
//
// OK, first of this is not a DB; this class will try to 
// define an interface to accessing the configuration data.
// 
// 
//
#include "PixelConfigDBInterface/include/PixelSQLCommand.h"
#include "PixelConfigDBInterface/include/PixelOracleDatabase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTrimAllPixels.h"
#include "CalibFormats/SiPixelObjects/interface/PixelMaskAllPixels.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"   
#include "CalibFormats/SiPixelObjects/interface/PixelFEDConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDCard.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTKFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortcardMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelLowVoltageMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTBMSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTTCciConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelLTCConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDTestDAC.h"
#include "CalibFormats/SiPixelObjects/interface/PixelMaxVsf.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDelay25Calib.h"
#include "CalibFormats/SiPixelObjects/interface/PixelGlobalDelay25.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDTestDAC.h"
#include <string>
#include <assert.h>
#include <iostream>
#include <typeinfo>
#include <sstream>
#include <vector>
// TO BE REMOVED -- TEMPORARY PATCH
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h"

#include "xgi/Method.h"

#define DEBUG_CONFIG_DB 0

using namespace std;
//typedef int PixelConfigKey;

/* class PixelConfigDBInterface : public VPixelConfig<PixelConfigDBInterface>{ */
/* class PixelConfigDBInterface : public Thread{ */

/*!  \ingroup AbstractConfigurationInterface "Configuration Objects Interface"
 *
 *   \class PixelConfigDBInterface PixelConfigDBInterface.h "interface/PixelConfigDBInterface.h"
 *   \brief The concrete class describing the "DB-based configuration" interface
 */
class PixelConfigDBInterface{

 public:
  PixelConfigDBInterface();
  virtual ~PixelConfigDBInterface();
  bool isConnected();
  bool connect();
  void disconnect();
  
  void dumpTableInfo                   (const char* table_name, const char* where=0, bool bForUpdate=false    );
  std::vector<std::pair<std::string, unsigned int> >  getAliases() ;
// modified by MR on 09-02-2009 10:24:53
  std::vector<std::vector<std::string> >  getAliasesWithCommentsAndDate() ;
  unsigned int clone(unsigned int oldkey, std::string path, unsigned int version) ;

  // modified by MR on 29-09-2008 10:03:37
  std::vector<std::pair< std::string, unsigned int> > getVersions(pos::PixelConfigKey key) ;
// modified by MR on 09-02-2009 15:16:55
  std::vector<std::vector< std::string> > getVersionsWithCommentsAndDate(pos::PixelConfigKey key) ;
  // modified by MR on 29-09-2008 23:20:51
  void addAlias(std::string alias, unsigned int key,
		std::vector<std::pair<std::string, std::string> > versionaliases);
  // Added by DM on 12-11-2009 14:12:22
  void addAuthor(std::string author) ;
  // modified by DM on 05-01-2009 12:10:51
  void addComment(std::string comment) ;
  // modified by MR on 30-09-2008 08:57:42
  unsigned int makeKey(std::vector<std::pair<std::string, unsigned int> > versions) ;
  // modified by MR on 30-09-2008 09:27:15
  void addVersionAlias(std::string path, unsigned int version, std::string alias) ;
  // modified by MR on 30-09-2008 11:43:22
  void addAlias(std::string alias, unsigned int key) ;
  // modified by MR on 30-09-2008 12:10:45
  bool getVersionAliases(std::string configAlias, unsigned int &key, std::vector<std::pair<std::string,std::string> > &versionAliases);
  // modified by MR on 16-10-2008 16:43:45
  std::string uploadStatus(std::string) ;
  // modified by MR on 10-11-2008 11:48:45
  std::vector<std::string> getVersionAliases(std::string path);

  std::string commitToDB(unsigned int globalKey) ;

  // modified by MR on 30-09-2008 14:30:52
  std::map<std::string, unsigned int> getAliases_map() ;
  // modified by MR on 30-09-2008 14:54:44
  std::map<std::string, std::vector<std::pair<unsigned int, std::string> > > getVersionData() ;
  // modified by MR on 13-10-2008 12:14:29
  std::map<std::string, std::vector<std::pair<unsigned int, std::string> > > getVersionData(std::string koc) ;
  // modified by MR on 13-10-2008 15:28:42
  // modified by MR on 26-02-2009 11:12:28
  std::vector<std::vector< std::string> >  getVersionDataWithComments(std::string koc) ;
  std::set<unsigned int > getExistingVersions(std::string koc) ;
  // modified by MR on 30-09-2008 23:46:34
  unsigned int getNextAvailableVersion(std::string sequence) ;
  // modified by MR on 08-10-2008 21:35:05
  void setKeyAlias(pos::PixelConfigKey key, std::string alias) ;
  // modified by MR on 09-10-2008 11:54:33
  std::map<std::string, std::vector<std::pair< std::string, unsigned int> > > getFullVersions() ;

  std::map<unsigned int, std::vector<std::vector< std::string> > > getConfGivenKoc(std::map<std::string, std::string> query,
                                                                                   int start, 
                                                                                   int howMany,
										   int from,
										   int to, 
										   int &total) ;
  // modified by MR on 10-10-2008 15:50:19
  std::map<std::string,std::vector<std::vector< std::string> > > getFullCfgs(int start, 
                                                                             int howMany,
									     int from, 
									     int to, 
									     int &total);
  // modified by MR on 14-10-2008 09:08:45
  std::map<std::string,std::vector<std::pair< std::string, std::string> > >   getKeyAliasVersionAliases(int start, int howMany, int &total) ;
  std::map<std::string,std::vector<std::string> >                             getKeyAliasVersionAliases(int &total) ;

  /* Returns a pointer to the data found in the path with configuration key. */
  template <class T>  
    void get(T* &data, std::string path, pos::PixelConfigKey key)
    {
      std::string mthn = "]\t[PixelConfigDBInterface::get()]\t\t\t\t    ";
      data = 0  ;

      if(DEBUG_CONFIG_DB)
        {
          cout << __LINE__ << mthn << "Type of data to read  : " << typeid(data).name() << " gKey: " << key.key() << " data-ptr:" << data << endl ;
        }
      vector< vector<string> > databaseTable;
      try{
        if (typeid(data)==typeid(pos::PixelDACSettings*)){                        // [1]  DONE - PixelDACSettings
          /*  cout << "Will return PixelDACSettings" << endl;                                   
              From key I will find the version to implement */                                  
          string version = "v_2_2_4";                                                           
          
	  getAndCacheData(databaseTable, path, key.key()) ;
	  if(checkDatabaseResults(databaseTable.size(),"DAC"))
	    data = (T*) new pos::PixelDACSettings(databaseTable);
        }                                                                                               
        else if (typeid(data)==typeid(pos::PixelTrimBase*)){                      // [2]  DONE - PixelTrimAllPixels
	  getAndCacheData(databaseTable, path, key.key()) ;
	  if(checkDatabaseResults(databaseTable.size(),"TRIM"))
	    data = (T*) new pos::PixelTrimAllPixels(databaseTable);                                       
        }                                                                                       
        else if (typeid(data)==typeid(pos::PixelMaskAllPixels*)){                 // [3]  DONE - PixelMaskAllPixels
	  getAndCacheData(databaseTable, path, key.key()) ;
	  if(checkDatabaseResults(databaseTable.size(),"MASK"))
	    data = (T*) new pos::PixelMaskAllPixels(databaseTable);                         	  
        }                                                                                 	  
        else if (typeid(data)==typeid(pos::PixelMaskBase*)){                      // [4]  DONE - PixelMaskBase
	  getAndCacheData(databaseTable, path, key.key()) ;
	  if(checkDatabaseResults(databaseTable.size(),"MASK") )                     
	    data = (T*) new pos::PixelMaskAllPixels(databaseTable);
        }                                                                                               
        else if (typeid(data)==typeid(pos::PixelFECConfig*)){                     // [5]  DONE - PixelFECConfig
          getPixelFECTable(databaseTable ,path,key);                                                    
	  if(checkDatabaseResults(databaseTable.size(),"FEC"))
	    data = (T*) new pos::PixelFECConfig(databaseTable);                                   
        }                                                                                               
        else if (typeid(data)==typeid(pos::PixelFEDConfig*)){                     // [6]  DONE - PixelFEDConfig
          getPixelFEDTable(databaseTable,path, key);                                            
	  if(checkDatabaseResults(databaseTable.size(),"FED"))
          data = (T*) new pos::PixelFEDConfig(databaseTable);                                   
        }                                                                                               
        else if (typeid(data)==typeid(pos::PixelFEDCard*)){                       // [7]  DONE - PixelFEDCard
          vector<string> empty ;                                                                        
          getPixelFEDCardTable(databaseTable,path, key);                                                
	  if(checkDatabaseResults(databaseTable.size(),"FEDCard"))
	    {
	      databaseTable.push_back(empty) ;
	      appendPixelFEDCardTBMLevelsTable(databaseTable, path, key) ;                          
	      databaseTable.push_back(empty) ;                                                      
	      checkDatabaseResults(databaseTable.size(),"FEDTBM") ;                         
	      appendPixelFEDCardROCLevelsTable(databaseTable, path, key) ;                          
	      databaseTable.push_back(empty) ;                                                      
	      checkDatabaseResults(databaseTable.size(),"FEDROC") ;                         
	      data = (T*) new pos::PixelFEDCard(databaseTable);
	    }
        }                                                                                               
        else if (typeid(data)==typeid(pos::PixelPortcardMap*)){                   // [8]  DONE - PixelPortcardMap
          getPixelPortcardMapTable(databaseTable,path, key);                                    
	  if(checkDatabaseResults(databaseTable.size(),"PORTCARD"))
	    data = (T*) new pos::PixelPortcardMap(databaseTable);                                 
        }                                                                                               
        else if (typeid(data)==typeid(pos::PixelDetectorConfig*)){                // [9]  DONE - PixelDetectorConfig
          getPixelDetectorConfigTable(databaseTable,path, key);                                         
	  if(checkDatabaseResults(databaseTable.size(),"DETCONFIG"))
	    data = (T*) new pos::PixelDetectorConfig(databaseTable);                              
        }                                                                                       
        else if (typeid(data)==typeid(pos::PixelLowVoltageMap*)){                 // [10] DONE - PixelLowVoltageMap
          getPixelLowVoltageTable(databaseTable,path, key);                              
	  if(checkDatabaseResults(databaseTable.size(),"LOWV"))
	    data = (T*) new pos::PixelLowVoltageMap(databaseTable);                        
        }                                                                        
        else if (typeid(data)==typeid(pos::PixelPortCardConfig*)){                // [11] DONE - PixelPortCardConfig
	  getAndCacheData(databaseTable, path, key.key());
	  if(checkDatabaseResults(databaseTable.size(),"PORTCARDCONFIG"))
	    data = (T*) new pos::PixelPortCardConfig(databaseTable);                 
        }                                                                                
        else if (typeid(data)==typeid(pos::PixelTBMSettings*)){                   // [12] DONE - PixelTBMSettings
	  getAndCacheData(databaseTable, path, key.key()) ;
	  if(checkDatabaseResults(databaseTable.size(),"TBM"))
	    data = (T*) new pos::PixelTBMSettings(databaseTable);                  
        }                                                                        
        else if (typeid(data)==typeid(pos::PixelNameTranslation*)){               // [13] DONE - PixelNameTranslation
          getPixelNameTranslationTable(databaseTable,path, key);  
	  if(checkDatabaseResults(databaseTable.size(),"NMT"))
	    data = (T*) new pos::PixelNameTranslation(databaseTable);
        } 
        else if (typeid(data)==typeid(pos::PixelTKFECConfig*)){                   // [14] DONE - PixelTKFECConfig
          getPixelTKFECConfig(databaseTable,path, key);  
	  if(checkDatabaseResults(databaseTable.size(),"TKFEC"))
	    data = (T*) new pos::PixelTKFECConfig(databaseTable);
        }
        else if (typeid(data)==typeid(pos::PixelTTCciConfig*)){                   // [15] DONE - PixelTTCciConfig
          getPixelTTCciConfig(databaseTable,path, key);  
	  if(checkDatabaseResults(databaseTable.size(),"TTC"))                    
	    data = (T*) new pos::PixelTTCciConfig(databaseTable);
        }
        else if (typeid(data)==typeid(pos::PixelLTCConfig*)){                     // [15] DONE - PixelTTCciConfig
          getPixelLTCConfig(databaseTable,path, key);  
	  if(checkDatabaseResults(databaseTable.size(),"LTC") )
	    data = (T*) new pos::PixelLTCConfig(databaseTable);
        }
        else if (typeid(data)==typeid(pos::PixelMaxVsf*)){                        // [16] DONE - PixelMaxVsf
          getPixelMaxVSFConfig(databaseTable,path, key);  
	  if(checkDatabaseResults(databaseTable.size(),"MAXVSF") )
	    data = (T*) new pos::PixelMaxVsf(databaseTable);
        }
        else if (typeid(data)==typeid(pos::PixelCalibBase*)) {                    // [17] DONE - PixelCalibBase
          calibType_ = getPixelCalibConfig(databaseTable,path, key);  
          if(calibType_ == "calib")
            {
              if(DEBUG_CONFIG_DB)
                {
                  cout << __LINE__ << mthn << "Creating Object of type PixelCalibConfiguration" << endl ;
                }
	      if(checkDatabaseResults(databaseTable.size(),"CALIB"))
		data = (T*) new pos::PixelCalibConfiguration(databaseTable);
            }
          else if(calibType_ == "delay25")
            {
              if(DEBUG_CONFIG_DB)
                {
                  cout << __LINE__ << mthn << "Creating Object of type PixelDelay25Calib" 	    << endl ;
                }
	      if(checkDatabaseResults(databaseTable.size(),"DEL25"))                     
		data = (T*) new pos::PixelDelay25Calib(databaseTable);
            }
          else if(calibType_ == "fedtestdac")
            {
              if(DEBUG_CONFIG_DB)
                {
                  cout << __LINE__ << mthn << "Creating Object of type PixelFEDTestDAC"   	    << endl ;
                }
	      if(checkDatabaseResults(databaseTable.size(),"FEDCALIB") )
		data = (T*) new pos::PixelFEDTestDAC(databaseTable);
            }
          else
            {
              if(DEBUG_CONFIG_DB)
                {
                  cout << __LINE__ << mthn << "Unknown calibration type: |" << calibType_ << "|" << std::endl;
                }
	      data = 0 ;
	      //jmt -- note: do *not* put an assert(0) here. In Physics running this causes a crash at configure.
            }
        }
	else if (typeid(data)==typeid(pos::PixelGlobalDelay25*)) {
	  getPixelGlobalDelay25(databaseTable, key) ;
	  if(checkDatabaseResults(databaseTable.size(),"GLOBALDELAY25") )
	    {
	      data = (T*) new pos::PixelGlobalDelay25(databaseTable);
	    }
	  else
	    {
	      data = 0 ;
	    }
	  //std::cout << "[pos::PixelConfigFile::get()]\t\t\tWill return PixelGlobalDelay25" << std::endl;
	  //pos::PixelConfigKey tmp(6057) ;
	  //pos::PixelConfigFile::get(data, path, tmp) ;
	}
        //std::cout << "Will return the PixelMaxVsf" << std::endl;
        /*       else if (typeid(data)==typeid(pos::PixelFEDCard*)){ */
        /*      getPixelNameTranslationTable(databaseTable,path, key);   */
        /*      data = (T*) new pos::PixelNameTranslation(databaseTable); */
        /*       }  */
        else{
          cout << __LINE__ << mthn << "No matching object with typeid " << typeid(data).name() << std::endl;
          assert(0);
          data=0;
        }
        if(DEBUG_CONFIG_DB)
          {
            cout << __LINE__ << mthn << "Just before return: " << typeid(data).name() << " data-ptr: " << data << endl ;
          }
        return;
      }//end try
      catch(std::bad_typeid)
        {
          cerr << mthn << "Bad typeid" << endl;
          assert(0) ;
        } 
      catch(std::bad_alloc)
        {
          cerr << mthn << "Error allocating memory." << endl;
          assert(0) ;
        }
      catch(std::exception& e)
        {
          cout << __LINE__ << mthn << "Exception raised: " << e.what() << endl;
          string err_message = string(e.what()) + string(typeid(data).name()) ;
          assert(0) ;
          /*    XCEPT_RAISE(xgi::exception::Exception, err_message) ; */
        }
    }//end get

  template <class T>  
    void getByVersion(T* &data, std::string path, unsigned int version)
    {
      std::string mthn = "]\t[PixelConfigDBInterface::getByVersion()]\t\t\t\t    ";
      data = 0  ;

      if(DEBUG_CONFIG_DB)
        {
          cout << __LINE__ << mthn << "Type of data to read  : " << typeid(data).name() << " version: " << version << " data-ptr:" << data << endl ;
        }
      vector< vector<string> > databaseTable;
      try{
        if (typeid(data)==typeid(pos::PixelDetectorConfig*))
	  {               
	    getPixelDetectorConfigTableByVersion(databaseTable,path, version);                                         
	    if(checkDatabaseResults(databaseTable.size(),"DETCONFIG"))
	      {
		data = (T*) new pos::PixelDetectorConfig(databaseTable);                              
	      }
	  }                                                                                       
        else if (typeid(data)==typeid(pos::PixelNameTranslation*))
	  {
	    getPixelNameTranslationTableByVersion(databaseTable,path, version);  
	    if(checkDatabaseResults(databaseTable.size(),"NMT"))
	      {
		data = (T*) new pos::PixelNameTranslation(databaseTable);
	      }
	  }
        else if (typeid(data)==typeid(pos::PixelFEDConfig*))
	  {
	    getPixelFEDTableByVersion(databaseTable,path, version);   
	    if(checkDatabaseResults(databaseTable.size(),"FED"))
	      {
		data = (T*) new pos::PixelFEDConfig(databaseTable);                                   
	      }
	  }
        else if (typeid(data)==typeid(pos::PixelFECConfig*))
	  {
	    getPixelFECTableByVersion(databaseTable ,path,version) ;
	    if(checkDatabaseResults(databaseTable.size(),"FEC")) 
	      {
		data = (T*) new pos::PixelFECConfig(databaseTable);
	      }
	  }
        else if (typeid(data)==typeid(pos::PixelTKFECConfig*))
	  {
	    getPixelTKFECConfigByVersion(databaseTable,path, version);
	    if(checkDatabaseResults(databaseTable.size(),"TKFEC"))
	      {
		data = (T*) new pos::PixelTKFECConfig(databaseTable);
	      }
	  }
        else if (typeid(data)==typeid(pos::PixelPortcardMap*))
	  {
	    getPixelPortcardMapTableByVersion(databaseTable,path, version);
	    if(checkDatabaseResults(databaseTable.size(),"PORTCARD"))
	      {
		data = (T*) new pos::PixelPortcardMap(databaseTable);
	      }
	  }                                                                                               
        else
	  {
	    cout << __LINE__ << mthn << "No matching object with typeid " << typeid(data).name() << std::endl;
	    assert(0);
	    data=0;
	  }
        if(DEBUG_CONFIG_DB)
          {
            cout << __LINE__ << mthn << "Just before return: " << typeid(data).name() << " data-ptr:" << data << endl ;
          }
        return;
      }//end try
      catch(std::bad_typeid)
        {
          cerr << mthn << "Bad typeid" << endl;
          assert(0) ;
        } 
      catch(std::bad_alloc)
        {
          cerr << mthn << "Error allocating memory." << endl;
          assert(0) ;
        }
      catch(std::exception& e)
        {
          cout << __LINE__ << mthn << "Exception raised: " << e.what() << endl;
          string err_message = string(e.what()) + string(typeid(data).name()) ;
          assert(0) ;
          /*    XCEPT_RAISE(xgi::exception::Exception, err_message) ; */
        }
    }//end getByVersion

	
  template <class T>
    void get(std::map<std::string, T*> &pixelObjects, pos::PixelConfigKey key)
    {
      std::stringstream s ; s << "[PixelConfigDBInterface::get(std::map<std::string, T*>)]    " ;
      std::string mthn = s.str() ;

/*       std::cout << "|" << typeid(pixelObjects.begin()->second).name() << "| vs. |" << typeid(pos::PixelDACSettings*).name() << "|" <<std::endl ; */
      if (typeid(pixelObjects.begin()->second ) == typeid(pos::PixelDACSettings*))
        {
          vector< vector<string> > databaseTable;
          getAllPixelDACSettingsTable(databaseTable,key); 
          if(DEBUG_CONFIG_DB)
            {
              std::cout << __LINE__ << mthn << "Database Table size: " << databaseTable.size() << std::endl ;
	    }
          std::map<std::string , int > colM;
          std::vector<std::string > colNames;
          
          //   colNames.push_back("CONFIG_KEY_ID");
          //   colNames.push_back("CONFIG_KEY");
          //   colNames.push_back("VERSION");
          //   colNames.push_back("KIND_OF_COND");
          colNames.push_back("ROC_NAME");
          //   colNames.push_back("HUB_ADDRS");
          //   colNames.push_back("PORT_NUMBER");
          //   colNames.push_back("I2C_ADDR");
          //   colNames.push_back("GEOM_ROC_NUM");
          colNames.push_back("VDD");
          colNames.push_back("VANA");
          colNames.push_back("VSF");
          colNames.push_back("VCOMP");
          colNames.push_back("VLEAK");
          colNames.push_back("VRGPR");
          colNames.push_back("VWLLPR");
          colNames.push_back("VRGSH");
          colNames.push_back("VWLLSH");
          colNames.push_back("VHLDDEL");
          colNames.push_back("VTRIM");
          colNames.push_back("VCTHR");
          colNames.push_back("VIBIAS_BUS");
          colNames.push_back("VIBIAS_SF");
          colNames.push_back("VOFFSETOP");
          colNames.push_back("VBIASOP");
          colNames.push_back("VOFFSETRO");
          colNames.push_back("VION");
          colNames.push_back("VIBIAS_PH");
          colNames.push_back("VIBIAS_DAC");
          colNames.push_back("VIBIAS_ROC");
          colNames.push_back("VICOLOR");
          colNames.push_back("VNPIX");
          colNames.push_back("VSUMCOL");
          colNames.push_back("VCAL");
          colNames.push_back("CALDEL");
          colNames.push_back("TEMPRANGE");
          colNames.push_back("WBC");
          colNames.push_back("CHIPCONTREG");
          
          // modified by MR on 25-02-2008 10:00:45
          // colM stores the index (referred to tableMat) where the specified dac setting is store!!!
          for(unsigned int c = 0 ; c < databaseTable[0].size() ; c++)
            {
              for(unsigned int n=0; n<colNames.size(); n++)
                {
                  if(databaseTable[0][c] == colNames[n])
                    {
                      colM[colNames[n]] = c;
                      break;
                    }
                }
            }//end for

          std::vector<std::vector<std::string> > moduleDACs ;
	  for(unsigned int r = 1 ; r < databaseTable.size() ; r++)
	    {
	      moduleDACs.clear() ;
	      moduleDACs.push_back(databaseTable[0]) ;
	      /*                0123456789  */
	      /*                pixel/dac/ */
	      
	      moduleDACs.push_back(databaseTable[r]) ;
	      string currentModule = databaseTable[r][colM["ROC_NAME"]] ;
/* 	      std::cout << "FIRST: " << currentModule << std::endl ; */
	      currentModule.replace(currentModule.find("_ROC"), currentModule.size() - currentModule.find("_ROC"), "") ;
/* 	      std::cout << "SECOND: " << currentModule << std::endl ; */
	      if(currentModule.find("_PLQ") != std::string::npos)
		{
		  currentModule.replace(currentModule.find("_PLQ"), currentModule.size() - currentModule.find("_PLQ"), "") ;
/* 		  std::cout << "THIRD: " << currentModule << std::endl ; */
		}
	      while(1)
		{
		  r++ ;
		  if(r == databaseTable.size()) break ;
		  string newCurrentModule = databaseTable[r][colM["ROC_NAME"]] ;
		  newCurrentModule.replace(newCurrentModule.find("_ROC"), newCurrentModule.size() - newCurrentModule.find("_ROC"), "") ;
		  if(newCurrentModule.find("_PLQ") != std::string::npos)
		    {
		      newCurrentModule.replace(newCurrentModule.find("_PLQ"), newCurrentModule.size() - newCurrentModule.find("_PLQ"), "") ;
/* 		      std::cout << "THIRD: " << newCurrentModule << std::endl ; */
		    }
/* 		  std::cout << "******* Comparing: |" << currentModule << "| vs |" << newCurrentModule << "|" << std::endl ; */
		  if(newCurrentModule == currentModule)
		    {
		      moduleDACs.push_back(databaseTable[r]) ;
		    }
		  else
		    {
		      r-- ;
		      break ;
		    }
		}
	      pixelObjects[currentModule] = (T*) new pos::PixelDACSettings(moduleDACs) ;
	    }
/*        data = (T*) new pos::PixelDACSettings(databaseTable);                                  */
        }
      else if (typeid(pixelObjects.begin()->second ) == typeid(pos::PixelPortCardConfig*))
        {
          vector< vector<string> > databaseTable;
          getAllPixelPortCardConfigTable(databaseTable,key); 
          if(DEBUG_CONFIG_DB)
            {
               std::cout << __LINE__ << mthn << "Database Table size: " << databaseTable.size() << std::endl ;
            }
          std::map<std::string , int > colM;
          std::vector<std::string > colNames;
          
	  colNames.push_back("PORT_CARD"   );
          // modified by MR on 25-02-2008 10:00:45
          // colM stores the index (referred to tableMat) where the specified dac setting is store!!!
          for(unsigned int c = 0 ; c < databaseTable[0].size() ; c++)
            {
              for(unsigned int n=0; n<colNames.size(); n++)
                {
                  if(databaseTable[0][c] == colNames[n])
                    {
                      colM[colNames[n]] = c;
                      break;
                    }
                }
            }//end for

          std::vector<std::vector<std::string> > portCards ;
	  for(unsigned int r = 1 ; r < databaseTable.size() ; r++)
	    {
	      portCards.clear() ;
	      portCards.push_back(databaseTable[0]) ;
	      portCards.push_back(databaseTable[r]) ;
	      pixelObjects[databaseTable[r][colM["PORT_CARD"]]] = (T*) new pos::PixelPortCardConfig(portCards) ;
	    }
/*        data = (T*) new pos::PixelDACSettings(databaseTable);                                  */
        }
      else
        {
          typename std::map<std::string, T* >::iterator iObject=pixelObjects.begin();
          for(;iObject!=pixelObjects.end();++iObject)
            {
              get(iObject->second,iObject->first,key);
            }
        }
    }
  

  /* Returns TRUE or FALSE according to presence of data into the DATABASE. */
  template <class T>  
    bool configurationDataExists(T* &data, std::string path, pos::PixelConfigKey key)
    {
      std::string mthn = "]\t[PixelConfigDBInterface::configurationDataExists()]\t\t    " ;

      vector< vector<string> > databaseTable;    
      try
        {
          if (typeid(data)==typeid(pos::PixelDACSettings*))
            {                 // [1]  DONE - PixelDACSettings
	      getAndCacheData(databaseTable, path, key.key()) ;
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelDACSettings(databaseTable);
              if(databaseTable.size() > 1) return true ;
            }                                                                                           
          else if (typeid(data)==typeid(pos::PixelTrimBase*))
            {                 // [2]  DONE - PixelTrimAllPixels
	      getAndCacheData(databaseTable, path, key.key()) ;
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelTrimAllPixels(databaseTable);                                   
              if(databaseTable.size() > 1) return true ;
            }                                                                                   
          else if (typeid(data)==typeid(pos::PixelMaskAllPixels*))
            {                 // [3]  DONE - PixelMaskAllPixels
	      getAndCacheData(databaseTable, path, key.key()) ;
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelMaskAllPixels(databaseTable);                                   
              if(databaseTable.size() > 1) return true ;
            }                                                                                           
          else if (typeid(data)==typeid(pos::PixelMaskBase*))
            {                 // [4]  DONE - PixelMaskBase
	      getAndCacheData(databaseTable, path, key.key()) ;
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelMaskAllPixels(databaseTable);                                   
              if(databaseTable.size() > 1) return true ;
            }                                                                                           
          else if (typeid(data)==typeid(pos::PixelFECConfig*))
            {                 // [5]  DONE - PixelFECConfig
              getPixelFECTable(databaseTable ,path,key);                                                        
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelFECConfig(databaseTable);                                       
              if(databaseTable.size() > 1) return true ;
            }                                                                                           
          else if (typeid(data)==typeid(pos::PixelFEDConfig*))
            {                 // [6]  DONE - PixelFEDConfig
              getPixelFEDTable(databaseTable,path, key);                                                
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelFEDConfig(databaseTable);                                       
              if(databaseTable.size() > 1) return true ;
            }                                                                                           
          else if (typeid(data)==typeid(pos::PixelFEDCard*))
            {                 // [7]  DONE - PixelFEDCard
              vector<string> empty ;                                                                    
              getPixelFEDCardTable(databaseTable,path, key);                                            
              if(databaseTable.size() == 1) return false ;
              databaseTable.push_back(empty) ;                                                  
              appendPixelFEDCardTBMLevelsTable(databaseTable, path, key) ;                              
              if(databaseTable.size() == 1) return false ;
              databaseTable.push_back(empty) ;                                                  
              appendPixelFEDCardROCLevelsTable(databaseTable, path, key) ;                              
              if(databaseTable.size() == 1) return false ;
              databaseTable.push_back(empty) ;                                                  
              data = (T*) new pos::PixelFEDCard(databaseTable);                                         
              if(databaseTable.size() > 1) return true ;
            }                                                                                           
          else if (typeid(data)==typeid(pos::PixelPortcardMap*))
            {                 // [8]  DONE - PixelPortcardMap
              /*        cout <<"[PixelConfigDBInterface()] data before: "<<data<<endl ; */                      
              getPixelPortcardMapTable(databaseTable,path, key);                                        
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelPortcardMap(databaseTable);                                     
              if(databaseTable.size() > 1) return true ;
              /*        cout <<"[PixelConfigDBInterface()] data after: "<<data<<endl ; */                       
            }                                                                                           
          else if (typeid(data)==typeid(pos::PixelDetectorConfig*))
            {                 // [9]  DONE - PixelDetectorConfig
              getPixelDetectorConfigTable(databaseTable,path, key);                                     
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelDetectorConfig(databaseTable);                          
              if(databaseTable.size() > 1) return true ;
            }                                                                                   
          else if (typeid(data)==typeid(pos::PixelLowVoltageMap*))
            {                 // [10] TO BE DONE - PixelLowVoltageMap
              getPixelLowVoltageTable(databaseTable,path,key);                           
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelLowVoltageMap(databaseTable);                    
              if(databaseTable.size() > 1) return true ;
            }                                                                    
          else if (typeid(data)==typeid(pos::PixelPortCardConfig*))
            {                 // [11] DONE - PixelPortCardConfig
              getPixelPortCardConfigTable(databaseTable,path, key);                      	      
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelPortCardConfig(databaseTable);                 
              if(databaseTable.size() > 1) return true ;
            }                                                                            
          else if (typeid(data)==typeid(pos::PixelTBMSettings*))
            {                 // [12] DONE - PixelTBMSettings
	      getAndCacheData(databaseTable, path, key.key()) ;
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelTBMSettings(databaseTable);                      
              if(databaseTable.size() > 1) return true ;
            }                                                                    
          else if (typeid(data)==typeid(pos::PixelNameTranslation*))
            {                 // [13] DONE - PixelNameTranslation
              getPixelNameTranslationTable(databaseTable,path, key);  
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelNameTranslation(databaseTable);
              if(databaseTable.size() > 1) return true ;
            } 
          else if (typeid(data)==typeid(pos::PixelTKFECConfig*))
            {                 // [14] DONE - PixelTKFECConfig
              getPixelTKFECConfig(databaseTable,path, key);  
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelTKFECConfig(databaseTable);
              if(databaseTable.size() > 1) return true ;
            }
          else if (typeid(data)==typeid(pos::PixelTTCciConfig*))
            {                   // [15] DONE - PixelTTCciConfig
              getPixelTTCciConfig(databaseTable,path, key);  
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelTTCciConfig(databaseTable);
              if(databaseTable.size() > 1) return true ;
            }
          else if (typeid(data)==typeid(pos::PixelMaxVsf*))
            {                        // [16] DONE - PixelMaxVsf
              getPixelMaxVSFConfig(databaseTable,path, key);  
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelMaxVsf(databaseTable);
              if(databaseTable.size() > 1) return true ;
            }
          else if (typeid(data)==typeid(pos::PixelGlobalDelay25*))
            {                        // [18] DONE - PixelGlobalDelay25
              getPixelGlobalDelay25(databaseTable, key);  
              if(databaseTable.size() == 1) return false ;
              data = (T*) new pos::PixelGlobalDelay25(databaseTable);
              if(databaseTable.size() > 1) return true ;
            }
          else if (typeid(data)==typeid(pos::PixelCalibBase*)) {                    // [17] DONE - PixelCalibBase
            calibType_ = getPixelCalibConfig(databaseTable,path, key);  
            if(calibType_ == "Gain Calibration")
              {
                cout << __LINE__ << mthn << "Creating Object of type PixelCalibConfiguration" << endl ;
		if(databaseTable.size() == 1) return false ;
		data = (T*) new pos::PixelCalibConfiguration(databaseTable);
                if(databaseTable.size() > 1) return true ;
              }
            else if(calibType_ == "delay25")
              {
                cout << __LINE__ << mthn << "Creating Object of type PixelDelay25Calib" << endl ;
		if(databaseTable.size() == 1) return false ;
                data = (T*) new pos::PixelDelay25Calib(databaseTable);
                if(databaseTable.size() > 1) return true ;
              }
            else if(calibType_ == "fedtestdac")
              {
                cout << __LINE__ << mthn << "Creating Object of type PixelFEDTestDAC" << endl ;
		if(databaseTable.size() == 1) return false ;
                data = (T*) new pos::PixelFEDTestDAC(databaseTable);
                if(databaseTable.size() > 1) return true ;
              }
            else
              {
                std::cout << __LINE__ << mthn << "Unknown calibration type: " << calibType_ << std::endl;
                assert(0) ;
              }
          }
          else
            {
              std::cout << __LINE__ << mthn << "No matching object with typeid " << typeid(data).name() << std::endl;
              assert(0);
              data=0;
            }
          if( DEBUG_CONFIG_DB )
	  {
            cout << __LINE__ << mthn << "Just before return" << endl ;
	  }
          return false;
        }//end try
      catch(std::bad_typeid)
        {
          std::cerr << __LINE__ << mthn << "Bad typeid"<<std::endl;
          assert(0) ;
        } 
      catch(std::bad_alloc)
        {
          std::cerr << mthn << "Error allocating memory." << std::endl;
          assert(0) ;
        }
      catch(std::exception& e)
        {
          cout << __LINE__ << mthn << "Exception raised: " << e.what() << endl;
          string err_message = string(e.what()) + string(typeid(data).name()) ;
          assert(0) ;
          /*    XCEPT_RAISE(xgi::exception::Exception, err_message) ; */
        }
    }//end configurationDataExists


  // modified by MR on 17-09-2008 10:52:15
  // This method is used by the utility that populates the DB with already
  // existing configuration files.
  template <class T>  
    int put(T* &data, unsigned int configurationFileVersion)
    {
      std::stringstream s ; s << "[PixelConfigDBInterface::put()]\t\t\t\t    " ;
      std::string mthn = s.str() ;

      pos::PixelConfigKey key(-1) ;
      char * cpath = getenv("PIXELCONFIGURATIONSPOOLAREA") ;
      if( cpath == NULL )
        {
          cout << __LINE__ << mthn << "FATAL: PIXELCONFIGURATIONSPOOLAREA is undefined" << endl ;
          assert(0) ;
        }
      std::string path(cpath);
      cout << __LINE__ << mthn << "Type of data to write : " << typeid(data).name() << endl ;
//      cout << "Special case to handle: " << typeid(const pos::PixelFEDCard*).name() << endl ;

      if (typeid(data)==typeid(const pos::PixelFEDCard*)) 
        {
          ofstream *fedstream = new ofstream() ;                                                        // Create ofstream handle for FED
          ofstream *rocstream = new ofstream() ;                                                        // Create ofstream handle for ROC levels
          ofstream *tbmstream = new ofstream() ;                                                        // Create ofstream handle for TBM levels
          data->writeXMLHeader(key, configurationFileVersion, path, fedstream, rocstream, tbmstream) ;  // Open file and write header
          data->writeXML(fedstream, rocstream, tbmstream) ;                                             // Write data for each component
          data->writeXMLTrailer(fedstream, rocstream, tbmstream) ;                                      // Write trailer, close file
          delete fedstream ;    
          delete rocstream ;    
          delete tbmstream ;    
        }
      else
        {
          ofstream *outstream = new ofstream() ;                                                       // Create ofstream handle for FED
          data->writeXMLHeader(key, configurationFileVersion, path, outstream) ;                       // Open file and write header
          data->writeXML(outstream) ;                                                                  // Write data 
          data->writeXMLTrailer(outstream) ;                                                           // Write trailer, close file
          delete outstream ;    
        }
      cout << __LINE__ << mthn << "Data written!" << endl ;
      return 0 ;
    }
  /* Writes data back into the DataBase */

  // modified by MR on 06-03-2009 09:22:25
  /**
     The following method is the one actualy used everywhere in Debbie, and possibly 
     in *ALL* the other supervisors.
     The use of the PixelConfigKey is obsolete: it should be removed because it
     is misleading.
     The other put method (T*&, unsigned int version, string) is used by the 
     populateDB.cc utility which *MUST* specify the version of each kind of
     condition without querying the DataBase (it is used to fill the DataBase with
     data stored in configuration files.)
   */
  template <class T>  
    int put(T* &data, pos::PixelConfigKey key)
    {
      std::stringstream s ; s << "[PixelConfigDBInterface::put()]\t\t\t\t    " ;
      std::string mthn = s.str() ;

      char * cpath = getenv("PIXELCONFIGURATIONSPOOLAREA") ;
      if( cpath == NULL )
        {
          cout << __LINE__ << mthn << "FATAL: PIXELCONFIGURATIONSPOOLAREA is undefined" << endl ;
          assert(0) ;
        }
      std::string path(cpath);
      int version = getNextAvailableVersion(getSequenceName(data)) ;
      cout << __LINE__ << mthn << "Type of data to write : " << typeid(data).name() << endl ;

      if (typeid(data)==typeid(const pos::PixelFEDCard*)) 
        {
          ofstream *fedstream = new ofstream() ;                                       // Create ofstream handle for FED
          ofstream *rocstream = new ofstream() ;                                       // Create ofstream handle for ROC levels
          ofstream *tbmstream = new ofstream() ;                                       // Create ofstream handle for TBM levels
          data->writeXMLHeader(key, version, path, fedstream, rocstream, tbmstream) ;  // Open file and write header
          data->writeXML(                          fedstream, rocstream, tbmstream) ;  // Write data for each component
          data->writeXMLTrailer(                   fedstream, rocstream, tbmstream) ;  // Write trailer, close file
          delete fedstream ;    
          delete rocstream ;    
          delete tbmstream ;    
        }
      else
        {
          ofstream *outstream = new ofstream() ;                                       // Create ofstream handle for FED
          data->writeXMLHeader(key, version, path, outstream, 0, 0) ;                  // Open file and write header
          data->writeXML(                          outstream, 0, 0) ;                  // Write data 
          data->writeXMLTrailer(                   outstream, 0, 0) ;                  // Write trailer, close file
          delete outstream ;    
        }
      cout << __LINE__ << mthn << "Data written!" << endl ;
      return version ;
    }

  /* Writes data back into the DataBase */
  template <class T>  
    int put(std::vector<T*> &data, pos::PixelConfigKey key)
    {
      std::stringstream s ; s << "[PixelConfigDBInterface::put()]\t\t\t\t    " ;
      std::string mthn = s.str() ;

      char * cpath = getenv("PIXELCONFIGURATIONSPOOLAREA") ;
      if( cpath == NULL )
        {
          cout << __LINE__ << mthn << "FATAL: PIXELCONFIGURATIONSPOOLAREA is undefined" << endl ;
          assert(0) ;
        }
      std::string path(cpath);
      int version = getNextAvailableVersion(getSequenceName(data[0])) ;
      typename std::vector<T*>::iterator dataIt=data.begin();
      
      if (dynamic_cast<pos::PixelFEDCard*> (*dataIt)) 
        {
          ofstream *fedstream = new ofstream() ;                                            // Create ofstream handle for FED
          ofstream *rocstream = new ofstream() ;                                            // Create ofstream handle for ROC levels
          ofstream *tbmstream = new ofstream() ;                                            // Create ofstream handle for TBM levels
          (*dataIt)->writeXMLHeader(key, version, path, fedstream, rocstream, tbmstream) ;  // Open file and write header
          for(; dataIt!=data.end(); dataIt++)
            {
              (*dataIt)->writeXML(fedstream, rocstream, tbmstream) ;                        // Write data for each component
            }
          dataIt=data.begin();      
          (*dataIt)->writeXMLTrailer(fedstream, rocstream, tbmstream) ;                     // Write trailer, close file
          delete fedstream ;    
          delete rocstream ;    
          delete tbmstream ;    
        }
      else
        {
          ofstream *out = new ofstream() ;                      // Create ofstream handle
          (*dataIt)->writeXMLHeader(key, version, path, out, 0, 0) ;  // Open file and write header
          for(; dataIt!=data.end(); dataIt++)
            {
              (*dataIt)->writeXML(out, 0, 0) ;                        // Write data for each component
            }
          dataIt=data.begin();      
          (*dataIt)->writeXMLTrailer(out, 0, 0) ;                     // Write trailer, close file
          delete out ;  
        }                                                       // Delete dangling file pointer
      cout << __LINE__ << mthn << "Data written!" << endl ;
      return version ;
    }

  template <class T>  
    int put(std::vector<T*> &data, unsigned int configurationFileVersion)
    {
      std::stringstream s ; s << "[PixelConfigDBInterface::put()]\t\t\t\t    " ;
      std::string mthn = s.str() ;

      pos::PixelConfigKey key(-1) ;
      char * cpath = getenv("PIXELCONFIGURATIONSPOOLAREA") ;
      if( cpath == NULL )
        {
          cout << __LINE__ << mthn << "FATAL: PIXELCONFIGURATIONSPOOLAREA is undefined" << endl ;
          assert(0) ;
        }
      std::string path(cpath);
      typename std::vector<T*>::iterator dataIt=data.begin();
      
      if (dynamic_cast<pos::PixelFEDCard*> (*dataIt)) 
        {
          ofstream *fedstream = new ofstream() ;                                            		     // Create ofstream handle for FED
          ofstream *rocstream = new ofstream() ;                                            		     // Create ofstream handle for ROC levels
          ofstream *tbmstream = new ofstream() ;                                            		     // Create ofstream handle for TBM levels
          (*dataIt)->writeXMLHeader(key, configurationFileVersion, path, fedstream, rocstream, tbmstream) ;  // Open file and write header
          for(; dataIt!=data.end(); dataIt++)
            {
              (*dataIt)->writeXML(fedstream, rocstream, tbmstream) ;                        		     // Write data for each component
            }
          dataIt=data.begin();      
          (*dataIt)->writeXMLTrailer(fedstream, rocstream, tbmstream) ;                     		     // Write trailer, close file
          delete fedstream ;    
          delete rocstream ;    
          delete tbmstream ;    
        }
      else
        {
          ofstream *out = new ofstream() ;                      // Create ofstream handle
          (*dataIt)->writeXMLHeader(key, configurationFileVersion, path, out, 0, 0) ;  // Open file and write header
          for(; dataIt!=data.end(); dataIt++)
            {
              (*dataIt)->writeXML(out, 0, 0) ;                        // Write data for each component
            }
          dataIt=data.begin();      
          (*dataIt)->writeXMLTrailer(out, 0, 0) ;                     // Write trailer, close file
          delete out ;  
        }                                                       // Delete dangling file pointer
      
      return 0 ;
    }

    template <class T>  
    string getSequenceName(T* &data)
    {
      std::string mthn = "\t[PixelConfigDBInterface::getSequencesName(T*&)]\t\t    "; 
      try{
        if (dynamic_cast<pos::PixelDACSettings*> (data)){                        // [1]  DONE - PixelDACSettings
	  return "ROC DAC Settings Col" ;
        }                                                                                               
        else if (dynamic_cast<pos::PixelTrimBase*> (data)){                      // [2]  DONE - PixelTrimAllPixels
	  return "ROC Trim Bits" ;
        }                                                                                       
        else if (dynamic_cast<pos::PixelMaskBase*> (data)){                      // [4]  DONE - PixelMaskBase
	  return "ROC Mask Bits" ;
        }                                                                                               
        else if (dynamic_cast<pos::PixelFECConfig*> (data)){                     // [5]  DONE - PixelFECConfig
	  return "Pixel FEC Parameters" ;
        }                                                                                               
        else if (dynamic_cast<pos::PixelFEDConfig*> (data)){                     // [6]  DONE - PixelFEDConfig
	  return "Pixel FED Crate Configuration";
        }                                                                                               
        else if (dynamic_cast<pos::PixelFEDCard*> (data)){                       // [7]  DONE - PixelFEDCard
	  return "Pixel FED Configuration" ;
        }                                                                                              
        else if (dynamic_cast<pos::PixelPortcardMap*> (data)){                   // [8]  DONE - PixelPortcardMap
	  return "Pixel Port Card Map" ;
        }                                                                                               
        else if (dynamic_cast<pos::PixelDetectorConfig*> (data)){                // [9]  DONE - PixelDetectorConfig
	  return "Pixel Detector Configuration";
        }                                                                                       
        else if (dynamic_cast<pos::PixelLowVoltageMap*> (data)){                 // [10] DONE - PixelLowVoltageMap
	  return "XDAQ Low Voltage Map" ;
        }                                                                        
        else if (dynamic_cast<pos::PixelPortCardConfig*> (data)){                // [11] DONE - PixelPortCardConfig
	  return "Pixel Port Card Settings" ;
        }                                                                                
        else if (dynamic_cast<pos::PixelTBMSettings*> (data)){                   // [12] DONE - PixelTBMSettings
	  return "Pixel TBM Parameters" ;
        }                                                                        
        else if (dynamic_cast<pos::PixelNameTranslation*> (data)){               // [13] DONE - PixelNameTranslation
	  return "Pixel Name Translation" ;
        } 
        else if (dynamic_cast<pos::PixelTKFECConfig*> (data)){                   // [14] DONE - PixelTKFECConfig
	  return "Tracker FEC Parameters" ;
        }
        else if (dynamic_cast<pos::PixelTTCciConfig*> (data)){                   // [15] DONE - PixelTTCciConfig
	  return "TTC Configuration Parameters"; 
        }
        else if (dynamic_cast<pos::PixelLTCConfig*> (data)){                     // [15] DONE - PixelTTCciConfig
	  return "LTC Configuration Parameters" ;
        }
        else if (dynamic_cast<pos::PixelMaxVsf*> (data)){                        // [16] DONE - PixelMaxVsf
	  return "ROC MaxVsf Setting" ;
        }
        else if (dynamic_cast<pos::PixelCalibBase*> (data)) {                    // [17] DONE - PixelCalibBase
	  return "Calibration Object Clob" ;
        }
        else if (dynamic_cast<pos::PixelCalibConfiguration*> (data)) {           // [18] DONE - PixelCalibConfiguration
	  return "Calibration Object Clob" ;
        }
	else if (dynamic_cast<pos::PixelGlobalDelay25*> (data)) {
	  return "Pixel Global Delay25" ;
	}
        else{
          cout << __LINE__ << mthn << "No matching object with typeid " << typeid(data).name() << std::endl;
          assert(0);
          data=0;
        }
        if(DEBUG_CONFIG_DB)
          {
            cout << __LINE__ << mthn << "Just before return: " << typeid(data).name() << " data-ptr:" << data << endl ;
          }
        return "";
      }//end try
      catch(std::bad_typeid)
        {
          cerr << mthn << "Bad typeid" << endl;
          assert(0) ;
        } 
      catch(std::bad_alloc)
        {
          cerr << mthn << "Error allocating memory." << endl;
          assert(0) ;
        }
      catch(std::exception& e)
        {
          cout << __LINE__ << mthn << "Exception raised: " << e.what() << endl;
          string err_message = string(e.what()) + string(typeid(data).name()) ;
          assert(0) ;
          /*    XCEPT_RAISE(xgi::exception::Exception, err_message) ; */
        }
    }

 private:

  PixelOracleDatabase database_;
  void getAllPixelDACSettingsTable     (vector< vector<string> >& table,                     pos::PixelConfigKey key); 
  void getPixelDACSettingsTable        (vector< vector<string> >& table, string module,      pos::PixelConfigKey key); 
  void getPixelTrimBitsTable           (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelMaskBitsTable           (vector< vector<string> >& table, string module,      pos::PixelConfigKey key); 
  void getPixelTTCciConfig             (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelLTCConfig               (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelMaxVSFConfig            (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelLowVoltageTable         (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelMaskBitsBase            (vector< vector<string> >& table, string module                              ); 
  string getPixelCalibConfig           (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelFECTable                (vector< vector<string> >& table, string fec,         pos::PixelConfigKey key);
  void getPixelFEDTable                (vector< vector<string> >& table, string fed,         pos::PixelConfigKey key);
  void getPixelGlobalDelay25           (vector< vector<string> >& table,                     pos::PixelConfigKey key);
  void getPixelFEDCardTable            (vector< vector<string> >& table, string fed,         pos::PixelConfigKey key);
  void appendPixelFEDCardTBMLevelsTable(vector< vector<string> >& table, string path,        pos::PixelConfigKey key);
  void appendPixelFEDCardROCLevelsTable(vector< vector<string> >& table, string path,        pos::PixelConfigKey key);
  void getPixelPortcardMapTable        (vector< vector<string> >& table, string portcardmap, pos::PixelConfigKey key);
  void getPixelDetectorConfigTable     (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelPortCardConfigTable     (vector< vector<string> >& table, string portcard,    pos::PixelConfigKey key);
  void getAllPixelPortCardConfigTable  (vector< vector<string> >& table,                     pos::PixelConfigKey key);
  void getPixelTBMSettingsTable        (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelNameTranslationTable    (vector< vector<string> >& table, string module,      pos::PixelConfigKey key);
  void getPixelTKFECConfig             (vector< vector<string> >& table, string path,        pos::PixelConfigKey key);
  void getGeneralTable                 (vector< vector<string> >& table, PixelSQLCommand& query                     );
  void dumpGeneralTable                (vector< vector<string> >& table                                             );
  void fillHashes() ;
  bool checkDatabaseResults            (int ,std::string) ;                         

  void getPixelDetectorConfigTableByVersion  (vector< vector<string> >& table, string module     , unsigned int version);
  void getPixelNameTranslationTableByVersion (vector< vector<string> >& table, string module     , unsigned int version);
  void getPixelFEDTableByVersion             (vector< vector<string> >& table, string fed        , unsigned int version);
  void getPixelFECTableByVersion             (vector< vector<string> >& table, string fec        , unsigned int version);
  void getPixelTKFECConfigByVersion          (vector< vector<string> >& table, string path       , unsigned int version);
  void getPixelPortcardMapTableByVersion     (vector< vector<string> >& table, string portcardmap, unsigned int version);


  int timeToDecode_ ;
  std::map<std::string, std::string> 		 	      conversionKOC ;
  std::map<std::string, std::string> 		 	      conversionNameKOC ;
  std::map<std::string, std::string> 		 	      conversionETNKOC ;
  std::string theAlias_ ;
  std::string             		 	              author_  ;
  std::string             		 	              comment_ ;
  std::string                       			      theTrueDBKey_ ; // filled by the method setKeyAlias(key, alias) ;
  std::string                       		              calibType_ ;  

  map<string, unsigned int   >      			      cacheIndex_ ;
  map<string, vector<vector<string> > >      			      cacheTRIM_  ;
  map<string, vector<vector<string> > >     			      cacheMASK_  ;
  map<string, vector<vector<string> > >     			      cacheDAC_   ;
  map<string, vector<vector<string> > >     			      cacheTBM_   ;
  map<string, vector<vector<string> > >     			      cachePTCARDCFG_;
  
  void getAndCacheData(vector< vector<string> > & dt, string , unsigned int GK ) ;
  bool dataIsInCache(string koc, unsigned int GK ) ;
  void findAndFillTable(vector<vector<string> > & dt , string koc , string moduleName_)  ;
  void resetCache(string ) ;
  void getDataAndFillCache(string koc , unsigned int GK, vector< vector<string> > & dt) ;
  void fillTRIMCache(vector< vector<string> > & dt) ;
  void fillMASKCache(vector< vector<string> > & dt) ;
  void fillDACCache (vector< vector<string> > & dt) ;
  void fillTBMCache (vector< vector<string> > & dt) ;
  void fillPTCARDCFGCache (vector< vector<string> > & dt) ;
  void fillGeneralCacheTable(string koc, unsigned int GK) ;
  string getProperKOC(string koc) ;
  void getAllTRIMDataSets(string koc, unsigned int GK, vector< vector<string> > & dt) ;
  void getAllMASKDataSets(string koc, unsigned int GK, vector< vector<string> > & dt) ;
  void getAllDACDataSets (string koc, unsigned int GK, vector< vector<string> > & dt) ;
  void getAllTBMDataSets (string koc, unsigned int GK, vector< vector<string> > & dt) ;
  void getAllPTCARDCFGDataSets(string koc, unsigned int GK, vector< vector<string> > & dt);
  void baseNameFromFullName(std::string &);
};

#endif
