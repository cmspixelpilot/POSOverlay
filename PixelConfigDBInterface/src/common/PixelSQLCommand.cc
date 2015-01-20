/*
$Author: menasce $
$Date: 2011/01/21 11:29:20 $
$Revision: 1.44 $
*/

#include "PixelConfigDBInterface/include/PixelSQLCommand.h"
#include "PixelConfigDBInterface/include/PixelOracleDatabase.h"
#include <iostream>
#include <iomanip>
#include <sstream>

#define DEBUG_SQL 0

using namespace std;
using namespace oracle::occi;

//====================================================================================================================
PixelSQLCommand::PixelSQLCommand(PixelOracleDatabase *pdb){
  if(DEBUG_SQL)
    {
      std::cout << __LINE__ << "]\t[PixelSQLCommand::PixelSQLCommand()]"<<std::endl;
    }
  myPdb_ = pdb;
  init();
}

//====================================================================================================================
PixelSQLCommand::PixelSQLCommand(PixelOracleDatabase &db){
  if(DEBUG_SQL)
    {
      std::cout << __LINE__ << "]\t[PixelSQLCommand::PixelSQLCommand()]"<<std::endl;
    }
  myPdb_ = &db;
  init();
}

//====================================================================================================================
void PixelSQLCommand::init(){
  connection_ = myPdb_->getConnection();
  statement_ = 0;
  result_ = 0;
  m_bError = 0;
  load_variable("sql_connection_dev",tnsOracleName_);
  for (std::string::iterator i = tnsOracleName_.begin(); i != tnsOracleName_.end(); ++i)
    *i = tolower(*i);   
  fillTnsKocViewDictionary() ;
}

void PixelSQLCommand::fillTnsKocViewDictionary() 
{
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["globaldelay25"  ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_GLOBAL_DELAY25_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["lowvoltagemap"  ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_XDAQ_LOW_VOLTAGE_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["ltcconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_LTC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["portcardmap"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PORTCARD_MAP_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["trim"           ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_TRIMS_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["detconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_DET_CONFIG_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["portcard"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PORTCARD_SETTINGS_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["fecconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_FEC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["fedconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_FED_CRATE_CONFIG_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["nametranslation"] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_NAME_TRANS_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["ttcciconfig"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TTC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["dac"            ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROCDAC_COL_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["tkfecconfig"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TRACKER_FEC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["tbm"            ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_TBM_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["fedcard"        ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_FED_CONFIGURATION_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["fedcard1"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TBM_LEVELS_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["fedcard2"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_LEVELS_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["calib"          ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_CALIB_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["mask"           ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_MASKS_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["maxvsf"         ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_MAXVSF_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["config_keys"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_CONFIG_KEYS_V" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["dataset_map"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_DATASET_MAP_V";
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["ka_va"          ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ALIAS_VERSION_ALIAS_T";
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["conf_given_koc" ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_KINDS_OF_COND_V";
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["auditlog"       ] = "CMS_PXL_CORE_MANAGEMNT.CONDITIONS_DATA_AUDITLOG";
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["ver_alias"      ] = "CMS_PXL_CORE_IOV_MGMNT.CONFIG_VERSION_ALIASES" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["ver_alias_map"  ] = "CMS_PXL_CORE_IOV_MGMNT.CONFIG_VERSION_ALIAS_MAPS" ;
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["sequence"       ] = "CMS_PXL_CORE_IOV_MGMNT.get_sequence_nextval";
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["dataset"        ] = "CMS_PXL_CORE_CONDITION.COND_DATA_SETS";
  tnsKocViewDictionary_["cms_pxl_int2r_lb"]["koc"            ] = "CMS_PXL_CORE_CONDITION.KINDS_OF_CONDITIONS";

  tnsKocViewDictionary_["cms_omds_nolb"   ]["globaldelay25"  ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_GLOBAL_DELAY25_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["lowvoltagemap"  ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_XDAQ_LOW_VOLTAGE_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["ltcconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_LTC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["portcardmap"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PORTCARD_MAP_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["trim"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_TRIMS_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["detconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_DET_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["portcard"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PORTCARD_SETTINGS_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["fecconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_FEC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["fedconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_FED_CRATE_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["nametranslation"] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_NAME_TRANS_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["ttcciconfig"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TTC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["dac"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROCDAC_COL_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["tkfecconfig"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TRACKER_FEC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["tbm"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_TBM_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["fedcard"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_FED_CONFIGURATION_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["fedcard1"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TBM_LEVELS_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["fedcard2"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_LEVELS_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["calib"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_CALIB_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["mask"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_MASKS_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["maxvsf"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_MAXVSF_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["config_keys"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_CONFIG_KEYS_V" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["dataset_map"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_DATASET_MAP_V";
  tnsKocViewDictionary_["cms_omds_nolb"   ]["ka_va"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ALIAS_VERSION_ALIAS_T";
  tnsKocViewDictionary_["cms_omds_nolb"   ]["conf_given_koc" ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_KINDS_OF_COND_V";
  tnsKocViewDictionary_["cms_omds_nolb"   ]["auditlog"       ] = "CMS_PXL_CORE_MANAGEMNT.CONDITIONS_DATA_AUDITLOG";
  tnsKocViewDictionary_["cms_omds_nolb"   ]["ver_alias"      ] = "CMS_PXL_CORE_IOV_MGMNT.CONFIG_VERSION_ALIASES" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["ver_alias_map"  ] = "CMS_PXL_CORE_IOV_MGMNT.CONFIG_VERSION_ALIAS_MAPS" ;
  tnsKocViewDictionary_["cms_omds_nolb"   ]["sequence"       ] = "CMS_PXL_CORE_IOV_MGMNT.get_sequence_nextval";
  tnsKocViewDictionary_["cms_omds_nolb"   ]["dataset"	     ] = "CMS_PXL_CORE_COND.COND_DATA_SETS";
  tnsKocViewDictionary_["cms_omds_nolb"   ]["koc"	     ] = "CMS_PXL_CORE_COND.KINDS_OF_CONDITIONS";

  tnsKocViewDictionary_["cms_omds_lb"	  ]["globaldelay25"  ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_GLOBAL_DELAY25_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["lowvoltagemap"  ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_XDAQ_LOW_VOLTAGE_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["ltcconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_LTC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["portcardmap"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PORTCARD_MAP_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["trim"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_TRIMS_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["detconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_DET_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["portcard"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PORTCARD_SETTINGS_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["fecconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_FEC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["fedconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_FED_CRATE_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["nametranslation"] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_NAME_TRANS_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["ttcciconfig"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TTC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["dac"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROCDAC_COL_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["tkfecconfig"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TRACKER_FEC_CONFIG_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["tbm"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_TBM_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["fedcard"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_FED_CONFIGURATION_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["fedcard1"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TBM_LEVELS_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["fedcard2"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_LEVELS_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["calib"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_CALIB_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["mask"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_MASKS_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["maxvsf"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_MAXVSF_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["config_keys"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_CONFIG_KEYS_V" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["dataset_map"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_DATASET_MAP_V";
  tnsKocViewDictionary_["cms_omds_lb"	  ]["ka_va"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ALIAS_VERSION_ALIAS_T";
  tnsKocViewDictionary_["cms_omds_lb"	  ]["conf_given_koc" ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_KINDS_OF_COND_V";
  tnsKocViewDictionary_["cms_omds_lb"	  ]["auditlog"       ] = "CMS_PXL_CORE_MANAGEMNT.CONDITIONS_DATA_AUDITLOG";
  tnsKocViewDictionary_["cms_omds_lb"	  ]["ver_alias"      ] = "CMS_PXL_CORE_IOV_MGMNT.CONFIG_VERSION_ALIASES" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["ver_alias_map"  ] = "CMS_PXL_CORE_IOV_MGMNT.CONFIG_VERSION_ALIAS_MAPS" ;
  tnsKocViewDictionary_["cms_omds_lb"	  ]["sequence"       ] = "CMS_PXL_CORE_IOV_MGMNT.get_sequence_nextval";
  tnsKocViewDictionary_["cms_omds_lb"	  ]["dataset"	     ] = "CMS_PXL_CORE_COND.COND_DATA_SETS";
  tnsKocViewDictionary_["cms_omds_lb"	  ]["koc"	     ] = "CMS_PXL_CORE_COND.KINDS_OF_CONDITIONS";

  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["globaldelay25"  ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_GLOBAL_DELAY25_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["lowvoltagemap"  ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_XDAQ_LOW_VOLTAGE_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["ltcconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_LTC_CONFIG_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["portcardmap"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PORTCARD_MAP_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["trim"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_TRIMS_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["detconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_DET_CONFIG_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["portcard"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PORTCARD_SETTINGS_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["fecconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_FEC_CONFIG_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["fedconfig"      ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_FED_CRATE_CONFIG_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["nametranslation"] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_NAME_TRANS_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["ttcciconfig"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TTC_CONFIG_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["dac"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROCDAC_COL_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["tkfecconfig"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TRACKER_FEC_CONFIG_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["tbm"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_TBM_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["fedcard"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_FED_CONFIGURATION_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["fedcard1"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_TBM_LEVELS_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["fedcard2"       ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_LEVELS_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["calib"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_PIXEL_CALIB_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["mask"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_MASKS_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["maxvsf"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ROC_MAXVSF_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["config_keys"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_CONFIG_KEYS_V" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["dataset_map"    ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_DATASET_MAP_V";
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["ka_va"	     ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_ALIAS_VERSION_ALIAS_T";
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["conf_given_koc" ] = "CMS_PXL_PIXEL_VIEW.CONF_KEY_KINDS_OF_COND_V";
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["auditlog"       ] = "CMS_PXL_CORE_MANAGEMNT.CONDITIONS_DATA_AUDITLOG";
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["ver_alias"      ] = "CMS_PXL_CORE_IOV_MGMNT.CONFIG_VERSION_ALIASES" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["ver_alias_map"  ] = "CMS_PXL_CORE_IOV_MGMNT.CONFIG_VERSION_ALIAS_MAPS" ;
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["sequence"       ] = "CMS_PXL_CORE_IOV_MGMNT.get_sequence_nextval";
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["dataset"	     ] = "CMS_PXL_CORE_COND.COND_DATA_SETS";
  tnsKocViewDictionary_["cmsdevr_nolb"	  ]["koc"	     ] = "CMS_PXL_CORE_COND.KINDS_OF_CONDITIONS";
}

//====================================================================================================================
PixelSQLCommand::~PixelSQLCommand(){
  if(DEBUG_SQL)
    {
      std::cout<<std::endl;	
      std::cout << __LINE__ << "]\t[PixelSQLCommand::~PixelSQLCommand()]"<<std::endl;
    }
  terminateStatement();
  if(DEBUG_SQL)
    {
      std::cout<<std::endl;	
      std::cout << __LINE__ << "]\t[PixelSQLCommand::~PixelSQLCommand()]\t\t Releasing Connection to the Pool"<<std::endl;
    }
  myPdb_->releaseConnection(connection_);
}

//====================================================================================================================
void PixelSQLCommand::createStatement(){
  if (!statement_) {
    statement_ = connection_->createStatement();
    //m_st->setAutoCommit(1);
  }
}


//====================================================================================================================
void PixelSQLCommand::terminateStatement(){
  if(DEBUG_SQL)
    {
      std::cout<<std::endl;	
      std::cout << __LINE__ << "]\t[PixelSQLCommand::terminateStatement()]"<<std::endl;
    }
  if (statement_) {
	  if(DEBUG_SQL)
      {
        std::cout<<std::endl;	
        std::cout << __LINE__ << "]\t[PixelSQLCommand::terminateStatement()]\t\t Terminating statement"<<std::endl;
      }
    connection_->terminateStatement(statement_);
    statement_ = 0;
  }
}

//====================================================================================================================
void PixelSQLCommand::reCreateStatement(){
  terminateStatement();
  createStatement();
}


//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::setSql(){
  
  
  if (statement_) {
    if(DEBUG_SQL)
      {
	std::cout << __LINE__ << "]\t[PixelSQLCommand::setSql()]\t\t\t\t    sql exec: " << command_.str()  << std::endl;
      }
    statement_->setSQL(command_.str());
    command_.str("");
   
  }
  return *this;
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::commit(){
  if (myPdb_ && connection_)
    connection_->commit();
  return *this;
}


//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::exec(bool bAutoCommit){
  
  //reCreateStatement();
 
  try {
    
    if (command_.str() != ""){
      if(DEBUG_SQL)
	{
	  std::cout << __LINE__ << "]\t[PixelSQLCommand::exec()]\t\t\t\t    sql exec: " << command_.str() << std::endl;
	}
      result_ = statement_->executeQuery(command_.str()); 
     
    }
    else
      result_ = statement_->executeQuery();
    if (bAutoCommit)
      commit();

    
  } catch (oracle::occi::SQLException e) {
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::exec()]\t\t\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::exec()]\t\t\t\t    when trying to execute: " << command_.str() <<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::exec()]\t\t\t\t    rethrowing exception..." << std::endl;
    throw;
  }
  command_.str("");
  return *this;
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::execUpdate(){
  // reCreateStatement();
 
  try {
    if(DEBUG_SQL)
      {
	std::cout << __LINE__ << "]\t[PixelSQLCommand::execUpdate()]\t\t    sql exec_update: " << command_.str() << std::endl;
      }
    if (command_.str() != "")
      statement_->executeUpdate(command_.str());
    else
      statement_->executeUpdate();
    connection_->commit();
  } catch (oracle::occi::SQLException e) {
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::execUpdate()]\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::execUpdate()]\t\t    when trying to execute: " << command_.str() <<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::execUpdate()]\t\t    rethrowing exception..." << std::endl;
    throw;
  }
  command_.str("");
  // terminateStatement();
  return *this;
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::startOver(){
  command_.str("");
  return *this;
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openTable(const char* table_name, 
					    std::map<std::string ,std::string> &where , 
					    bool bForUpdate, 
					    bool like, 
					    string orderByColumn, 
					    bool ordered)
{
  startOver();
  
//  cout << __LINE__ << "]\t[PixelSQLCommand::openTable()]\t\t\t\t    tnsOracleName_: " << tnsOracleName_  				   << endl ;
//  cout << __LINE__ << "]\t[PixelSQLCommand::openTable()]\t\t\t\t    table_name    : " << table_name					   << endl ;
//  cout << __LINE__ << "]\t[PixelSQLCommand::openTable()]\t\t\t\t    map value     : " << tnsKocViewDictionary_[tnsOracleName_][table_name] << endl ;

  if(where.empty())
    { 
      command_ << "SELECT * FROM "     //part_id, channel_map_id, extension_table_name, name, value
	       << tnsKocViewDictionary_[tnsOracleName_][table_name] ;
      if(ordered)
	{
	  command_ << " ORDER BY " << orderByColumn ;
	}
      reCreateStatement();
    }
  else
    {
      command_ << "SELECT * FROM "     //part_id, channel_map_id, extension_table_name, name, value
	       <<  tnsKocViewDictionary_[tnsOracleName_][table_name] ;
      
      if(!like)
	{
	  int i = 1;
	  for(map<string , string>::iterator it =where.begin() ; it != where.end() ; it++)
	    {
	      if(it == where.begin())
		command_ << " WHERE " << it->first << "=:" << i ;
	      else
		{
		  command_ << " and " << it->first << "=:" << i ;
		}
	      i++;
	    }//end for
	  if(ordered)
	    {
	      command_ << " ORDER BY " << orderByColumn ;
	    }
	  
//          cout << __LINE__ << "]\t[PixelSQLCommand::openTable()]\t\t\t\t    command_: " << command_.str() << endl ;

	  reCreateStatement();
	  setSql();
	  
	  int j =1;
	  for(map<string , string>::iterator it2 =where.begin() ; it2 != where.end() ; it2++)
	    {
	      setField( j ,it2->second );
	      j++;
	    }
	  
	}//end if like
      else
	{
	for(map<string , string>::iterator it =where.begin() ; it != where.end() ; it++)
	  {
	    
	    if(it == where.begin())
	      command_ << " WHERE " << it->first << " like " << "'%" << it->second << "%'" ;
	    else
	      {
		command_ << " and " << it->first << " like " <<  "'%" << it->second << "%'" ;
	      }
	  }//end for
	if(ordered)
	  {
	    command_ << " ORDER BY " << orderByColumn ;
	  }
	reCreateStatement();
	}//end else like
    }//end else empty
   
  if (bForUpdate)
    {
      command_ << " FOR UPDATE ";
    }
  
//  cout << __LINE__ << "]\t[PixelSQLCommand::openTable()]\t\t\t\t    command_: " << command_.str() << endl ;
  
  bool bAutoCommit = ! bForUpdate;
  setPrefetch(50000) ;
  exec(bAutoCommit);
  //     std::cout << __LINE__ << "]\t[PixelSQLCommand::open_table()]\tDone!" << std::endl;
  return *this;	
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openTable(const char* table_name, const char* where, bool bForUpdate){
 
  startOver();
  
  command_ << "SELECT * FROM "     //part_id, channel_map_id, extension_table_name, name, value
	   << tnsKocViewDictionary_[tnsOracleName_][table_name] ;
  if (where){
    command_ << " WHERE " << where;
  }
  if (bForUpdate){
    command_ << " FOR UPDATE ";
  }

  cout << __LINE__ << "]\t[PixelSQLCommand::openTable()]\t\t\t\t   command_: " << command_.str() << endl ;

  reCreateStatement();
  bool bAutoCommit = ! bForUpdate;
  exec(bAutoCommit);
  //     std::cout << __LINE__ << "]\t[PixelSQLCommand::open_table()]\tDone!" << std::endl;
  return *this;	
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openTable(const char* table_name, const std::string &where, bool bForUpdate){
  return openTable(table_name, where.c_str(), bForUpdate);
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openTable(const char* table_name, const std::ostringstream &where, bool bForUpdate){
  return openTable(table_name, where.str().c_str(), bForUpdate);
}

//====================================================================================================================
int PixelSQLCommand::loadRow(){
  //   std::cout << __LINE__ << "]\t[PixelSQLCommand::load_row()]\tLoading!" << std::endl;
  int returnValue ;
  if (result_)
    {
      returnValue = result_->next() ;
//       cout << __LINE__ << "]\t[PixelSQLCommand::loadRow()]\tRows Loaded: " << result_->getNumArrayRows() << endl ;
      return returnValue ;
    }
  else
    {
      return false ;
    }
}

//====================================================================================================================
int PixelSQLCommand::loadRow(int rows){
  //   std::cout << __LINE__ << "]\t[PixelSQLCommand::load_row()]\tLoading!" << std::endl;
  int returnValue ;
  if (result_)
    {
      returnValue = result_->next(rows) ;
//       cout << __LINE__ << "]\t[PixelSQLCommand::loadRow()]\tRows Loaded: " << result_->getNumArrayRows() << endl ;
      return returnValue ;
    }
  else
    {
      return false;
    }
}

//====================================================================================================================
unsigned int PixelSQLCommand::getNumArrayRows()
{
  if (result_){
    return result_->getNumArrayRows();
  }
  else{
    return 0;
  }
}

//====================================================================================================================
void PixelSQLCommand::setPrefetch(unsigned int prefetch){
  if(DEBUG_SQL)
    {
      std::cout << __LINE__ << "]\t[PixelSQLCommand::setPrefetch()]\t\t\t    Setting prefetch to " << prefetch << std::endl ; 
    }
  if(!statement_) createStatement() ;
  //  statement_->setPrefetchRowCount(prefetch);
    statement_->setPrefetchRowCount(20000);
    statement_->setPrefetchMemorySize(2000000) ; // 1 Megabyte (in bytes)!!!
  //  statement_->setPrefetchMemorySize(10000000) ; // 10 Megabyte (in bytes)!!!
}

//====================================================================================================================
void PixelSQLCommand::getField(int index, std::string &str) {
  //     std::cout << __LINE__ << "]\t[PixelSQLCommand::get_field]\t" << std::endl;
  try{
    if (result_){
      str = result_->getString(index);
    }
  }catch(oracle::occi::SQLException e){
 
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setPrefetch()]\t\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setPrefetch()]\t\t\t    when trying to execute PixelSQLCommand::getString(int index) "<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setPrefetch()]\t\t\t    rethrowing exception..." << std::endl;
 
  }
}

//====================================================================================================================
void PixelSQLCommand::getField(int index, int &i) {
  
  try{ 
  
    if (result_){
      i = result_->getInt(index);
    }
  }catch(oracle::occi::SQLException e){
 
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    when trying to execute PixelSQLCommand::getInt(int index) "<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    rethrowing exception..." << std::endl;
 
  }
  
}

//====================================================================================================================
std::string PixelSQLCommand::getStringField(int index) {

  //std::cout << __LINE__ << "]\t[PixelSQLCommand::get_field]\t" << index << std::endl;
  try 
    {
      if (result_) 
	{
	  if(metadata_[index-1].getInt(MetaData::ATTR_DATA_TYPE) == OCCI_SQLT_TIMESTAMP_TZ )
	    {
	      // 	  cout << printType(metadata_[index-1].getInt(MetaData::ATTR_DATA_TYPE)) << endl ;
	      return result_->getTimestamp(index).toText(string("MM/DD/IYYY HH24:MI:SS"), 2) ;
	    }
	  else
	    {
	      return result_->getString(index);        //testing
	    }
	}  
      else 
	{
	  return "";
	}
      
    }
  catch(oracle::occi::SQLException e)
    {
      std::cerr << __LINE__ << "]\t[PixelSQLCommand::getStringField()]\t\t\t    " << e.getMessage()<<std::endl;
      std::cerr << __LINE__ << "]\t[PixelSQLCommand::getStringField()]\t\t\t    when trying to execute PixelSQLCommand::getString(int index) "<<std::endl;
      std::cerr << __LINE__ << "]\t[PixelSQLCommand::getStringField()]\t\t\t    rethrowing exception..." << std::endl;
    }
  return "";
}

//====================================================================================================================
int PixelSQLCommand::getIntField(int index) {
  try
    {
      if (result_)
	{
	  return result_->getInt(index);
	}    
      else
	{
	  return -1;
	}
    }
  catch(oracle::occi::SQLException e)
    {
      std::cerr << __LINE__ << "]\t[PixelSQLCommand::getIntField()]\t\t\t    " << e.getMessage()<<std::endl;
      std::cerr << __LINE__ << "]\t[PixelSQLCommand::getIntField()]\t\t\t    when trying to execute PixelSQLCommand::getInt(int index) "<<std::endl;
      std::cerr << __LINE__ << "]\t[PixelSQLCommand::getIntField()]\t\t\t    rethrowing exception..." << std::endl;
    }
  return 0 ;
}

//====================================================================================================================
void PixelSQLCommand::getField(int index, oracle::occi::Blob &blob) {
  try{
    if (result_){
      blob = result_->getBlob(index);
    }
  
  }catch(oracle::occi::SQLException e){
  
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    when trying to execute PixelSQLCommand::getBlob( int index) "<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    rethrowing exception..." << std::endl;
  
  }
}

//====================================================================================================================
void PixelSQLCommand::getField(int index, oracle::occi::Clob &clob) {
  try{
    if (result_){
      clob = result_->getClob(index);
    }
  
  }catch(oracle::occi::SQLException e){
  
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    when trying to execute PixelSQLCommand::getClob( int index) "<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::getField()]\t\t\t    rethrowing exception..." << std::endl;
  
  }
}


//====================================================================================================================
void PixelSQLCommand::setField(int index, std::string &str) {
 
  try{
  
    if (statement_){  
      statement_->setString(index, str);
   
    }
  
  }catch(oracle::occi::SQLException e){
  
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    when trying to execute PixelSQLCommand::setString( int index , string str) "<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    rethrowing exception..." << std::endl;
 
  }
  
}

//====================================================================================================================
void PixelSQLCommand::setField(int index, int i) {
  try{
  
    if (statement_){
      statement_->setInt(index, i);
    }
  }catch(oracle::occi::SQLException e){
 
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    when trying to execute PixelSQLCommand::setInt( int index , int i) "<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    rethrowing exception..." << std::endl;
 
 
  }
}

//====================================================================================================================
void PixelSQLCommand::setField(int index, oracle::occi::Blob &blob) {
  try{
 
    if (statement_){
      statement_->setBlob(index, blob);
    }
  
  }catch(oracle::occi::SQLException e){
 
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    " << e.getMessage()<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    when trying to execute PixelSQLCommand::setBlob( int index , Blob blob) "<<std::endl;
    std::cerr << __LINE__ << "]\t[PixelSQLCommand::setField()]\t\t\t    rethrowing exception..." << std::endl;
 
 
  }
}

//====================================================================================================================
PixelSQLCommand& exec(PixelSQLCommand& sql_cmd){
  return sql_cmd.exec();
}

//====================================================================================================================
PixelSQLCommand& execWithoutCommit(PixelSQLCommand& sql_cmd){
  return sql_cmd.exec(false);
}

//====================================================================================================================
PixelSQLCommand& execUpdate(PixelSQLCommand& sql_cmd){
  return sql_cmd.execUpdate();
}

//====================================================================================================================
PixelSQLCommand& startOver(PixelSQLCommand& sql_cmd){
  return sql_cmd.startOver();
}

//====================================================================================================================
PixelSQLCommand& setSql(PixelSQLCommand& sql_cmd){
  return sql_cmd.setSql();
}
//**********************************************************************************  

//====================================================================================================================
int PixelSQLCommand::getNextAvailableVersion(std::string sequence)
{
  //Stored Procedure 
  // CMS_PXL_CORE_IOV_MGMNT_OWNER.contiguous_sequences_nextval (sequence_name in varchar2, sequence_value out number) 
  //create anonymous block for executing stored proc/function
  command_.str("") ;
  command_ << "select "<< tnsKocViewDictionary_[tnsOracleName_]["sequence"] << "('" << sequence <<"') as val from dual" ;
  //  command_ << "select CMS_PXL_CORE_IOV_MGMNT_OWNER.CONFIG_VERSION_SEQ.nextval from dual" ;
  reCreateStatement() ;
  setSql();
  exec(true); //execute procedure
  
  loadRow() ;
  int offSet = 200000 ;
  int tmp = getIntField(1) ;
  if(DEBUG_SQL)
    {
      cout << __LINE__ << "]\t[PixelSQLCommand::getNextAvailableVersion()] Reserved Global Key number: |" << tmp << "|" << endl ;
    }
  return tmp+offSet ;
}

//====================================================================================================================
int PixelSQLCommand::getNumberOfColumns(){
   
  //  try{
   
  metadata_.clear();
  metadata_ = result_->getColumnListMetaData();
  if(DEBUG_SQL)
    {
      for(unsigned int i = 0 ; i < metadata_.size() ; i++)
	{
	  cout << " Column Name: " << metadata_[i].getString(MetaData::ATTR_NAME) ;
	  cout << "\tData Type  : " <<
	    (printType (metadata_[i].getInt(MetaData::ATTR_DATA_TYPE))) << endl ;
	}
      
    }
  return (int)metadata_.size();
    
  /*  }catch (oracle::occi::SQLException e)
      {
    
      std::cerr << e.getMessage()<<std::endl;
      std::cerr << "when trying to get number of columns "<<std::endl;
    
    
      }
  */
}    

//====================================================================================================================
void PixelSQLCommand::dumpTableInfo(){
   
  //  try{
   
  metadata_.clear();
  metadata_ = result_->getColumnListMetaData();
  for(unsigned int i = 0 ; i < metadata_.size() ; i++)
    {
      cout << " Column Name: " << setw(30) << metadata_[i].getString(MetaData::ATTR_NAME) ;
      cout << "\tData Type:  " <<
	(printType (metadata_[i].getInt(MetaData::ATTR_DATA_TYPE))) << endl ;
    }
}    
    
//====================================================================================================================
std::string PixelSQLCommand::getNameOfColumns(int index)
{
  string returnString = "" ;
  try
    {
      metadata_.clear();
      metadata_ = result_->getColumnListMetaData();
      MetaData mv = metadata_[index-1];
      returnString = mv.getString( MetaData::ATTR_NAME );
    }
  catch(oracle::occi::SQLException e)
    {
      std::cerr << e.getMessage()<<std::endl;
      std::cerr << "when trying to get name of columns "<<std::endl;
    }
  return returnString;
}

//====================================================================================================================
vector<std::string> PixelSQLCommand::getNamesOfColumns(int numCol)
{
  vector<string> returnVector ;
  try
    {
      metadata_.clear();
      metadata_ = result_->getColumnListMetaData();
      for(int j = 0; j < numCol ; j++)
	{
	  returnVector.push_back( metadata_[j].getString( MetaData::ATTR_NAME ) );
	}
    }
  catch(oracle::occi::SQLException e)
    {
      std::cerr << e.getMessage()<<std::endl;
      std::cerr << "when trying to get name of columns "<<std::endl;
    }
  return returnVector;
}

//====================================================================================================================
void PixelSQLCommand::setDataBuffer(int index, char * buffer, int size) 
{
  result_->setDataBuffer(index, buffer, OCCI_SQLT_STR, size) ;
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openConditionDataAuditlog(std::string fileToSearchFor)
{
  startOver();

  command_ << "SELECT C.ARCHVE_FILE_NAME," ;
  command_ << "       C.UPLOAD_STATUS  " ;
  command_ << " FROM  " << tnsKocViewDictionary_[tnsOracleName_]["auditlog"] << " C ";
  command_ << " WHERE C.ARCHVE_FILE_NAME LIKE  '" << fileToSearchFor << "%'" ;

  std::cout << command_.str() << std::endl ;
  reCreateStatement();
  setSql();
  setPrefetch(50000) ;
  exec(true);
  //     std::cout << __LINE__ << "]\t[PixelSQLCommand::open_table()]\tDone!" << std::endl;
  return *this;	
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openVersionAliasTable()
{
  startOver();

  command_ << "SELECT AL_VERS.NAME VERSION_ALIAS," ;
  command_ << "      CONDDS.VERSION," ;
  command_ << "      KOC.NAME KIND_OF_CONDITION" ;
  command_ << " FROM  " << tnsKocViewDictionary_[tnsOracleName_]["ver_alias"] << " AL_VERS";
  command_ << " INNER JOIN " << tnsKocViewDictionary_[tnsOracleName_]["ver_alias_map"] << "  AL_MAP" ;
  command_ << "	   ON AL_MAP.CONFIG_VERSION_ALIAS_ID = AL_VERS.CONFIG_VERSION_ALIAS_ID" ;
  command_ << " INNER JOIN " << tnsKocViewDictionary_[tnsOracleName_]["dataset"] << "  CONDDS" ;
  command_ << "    ON CONDDS.CONDITION_DATA_SET_ID = AL_MAP.CONDITION_DATA_SET_ID";
  command_ << " INNER JOIN " << tnsKocViewDictionary_[tnsOracleName_]["koc"] << " KOC";
  command_ << "	   ON CONDDS.KIND_OF_CONDITION_ID = KOC.KIND_OF_CONDITION_ID";
  command_ << "	 WHERE CONDDS.IS_RECORD_DELETED = 'F'";
  command_ << "   AND KOC.IS_RECORD_DELETED = 'F'";
  command_ << "	  AND AL_MAP.IS_MOVED_TO_HISTORY = 'F'";
  command_ << "	  AND AL_VERS.IS_RECORD_DELETED = 'F'";
  command_ << " ORDER BY KIND_OF_CONDITION,";
  command_ << "          VERSION_ALIAS,";
  command_ << "          VERSION" ;

  //cout << __LINE__ << "] [" << __PRETTY_FUNCTION__ << "]\tcommand_ = " << command_.str() << endl ; 

  reCreateStatement();
  setSql();
  setPrefetch(50000) ;
  exec(true);
  //     std::cout << __LINE__ << "]\t[PixelSQLCommand::open_table()]\tDone!" << std::endl;
  return *this;	
}


//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openVersionAliasTable(string koc)
{
  startOver();

  command_ << "SELECT AL_VERS.NAME VERSION_ALIAS," ;
  command_ << "      CONDDS.VERSION," ;
  command_ << "      CONDDS.COMMENT_DESCRIPTION," ;
  command_ << "      CONDDS.RECORD_INSERTION_TIME," ;
  command_ << "      CONDDS.CREATED_BY_USER," ;
  command_ << "      KOC.NAME KIND_OF_CONDITION" ;
  command_ << " FROM  " << tnsKocViewDictionary_[tnsOracleName_]["ver_alias"] << " AL_VERS";
  command_ << " INNER JOIN " << tnsKocViewDictionary_[tnsOracleName_]["ver_alias_map"] << " AL_MAP" ;
  command_ << "	   ON AL_MAP.CONFIG_VERSION_ALIAS_ID = AL_VERS.CONFIG_VERSION_ALIAS_ID" ;
  command_ << " INNER JOIN " << tnsKocViewDictionary_[tnsOracleName_]["dataset"] << " CONDDS" ;
  command_ << "    ON CONDDS.CONDITION_DATA_SET_ID = AL_MAP.CONDITION_DATA_SET_ID";
  command_ << " INNER JOIN " << tnsKocViewDictionary_[tnsOracleName_]["koc"] << " KOC";
  command_ << "	   ON CONDDS.KIND_OF_CONDITION_ID = KOC.KIND_OF_CONDITION_ID";
  command_ << "	 WHERE CONDDS.IS_RECORD_DELETED = 'F'";
  command_ << "   AND KOC.IS_RECORD_DELETED = 'F'";
  command_ << "	  AND AL_MAP.IS_MOVED_TO_HISTORY = 'F'";
  command_ << "	  AND AL_VERS.IS_RECORD_DELETED = 'F'";
  command_ << "   AND KOC.NAME LIKE '%" << koc << "%'" ;
  command_ << " ORDER BY KIND_OF_CONDITION,";
  command_ << "          VERSION_ALIAS,";
  command_ << "          VERSION" ;

  //cout << __LINE__ << "] [" << __PRETTY_FUNCTION__ << "]\tcommand_ = " << command_.str() << endl ; 

  reCreateStatement();
  setSql();
  setPrefetch(50000) ;
  exec(true);
  //     std::cout << __LINE__ << "]\t[PixelSQLCommand::open_table()]\tDone!" << std::endl;
  return *this;	
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openKeyAliasKeyVersions()
{
  startOver();
  command_ << "select " << tnsKocViewDictionary_[tnsOracleName_]["ka_va"] << ".key_name     	   gk,"        ;
  command_ << "       " << tnsKocViewDictionary_[tnsOracleName_]["ka_va"] << ".key_alias     	   key_alias," ;
  command_ << "       " << tnsKocViewDictionary_[tnsOracleName_]["ka_va"] << ".version_alias 	   va,"        ;
  command_ << "       " << tnsKocViewDictionary_[tnsOracleName_]["ka_va"] << ".comment_description cd,"        ;
  command_ << "       " << tnsKocViewDictionary_[tnsOracleName_]["ka_va"] << ".insert_time  	   it,"        ;
  command_ << "       " << tnsKocViewDictionary_[tnsOracleName_]["ka_va"] << ".author       	   au,"        ;
  command_ << "       " << tnsKocViewDictionary_[tnsOracleName_]["ka_va"] << ".kind_of_condition   koc"        ;
  command_ << "       from " << tnsKocViewDictionary_[tnsOracleName_]["ka_va"]                                 ;
  command_ << "       order by gk ASC" ;

//  cout << __LINE__ << "]\t[PixelSQLCommand::openKeyAliasKeyVersions(string)] " << command_.str()  << endl ;
  reCreateStatement();
  setSql();
  setPrefetch(50000) ;
  exec(true);
  //     std::cout << __LINE__ << "]\t[PixelSQLCommand::openKeyAliasKeyVersions(()]\tDone!" << std::endl;
  return *this;	
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::openExistingVersionsTable(string koc)
{
  startOver();

  command_ << "select distinct version from " << tnsKocViewDictionary_[tnsOracleName_]["dataset"] << " where extension_table_name = '" ;
  command_ << koc << "' and is_record_deleted = 'F'" ; 
  //  cout << __LINE__ << "]\t[PixelSQLCommand::openVersionAliasTable(string)] " << command_.str()  << endl ;
  reCreateStatement();
  setSql();
  setPrefetch(50000) ;
  exec(true);
  //     std::cout << __LINE__ << "]\t[PixelSQLCommand::open_table()]\tDone!" << std::endl;
  return *this;	
}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::getNumberOfTotalCfgs()
{
  startOver();

  command_ << "SELECT COUNT(UNIQUE KEY_NAME)  from " << tnsKocViewDictionary_[tnsOracleName_]["dataset_map"]<< " where is_moved_to_history = 'F'" ;

  reCreateStatement();
  setSql();
  setPrefetch(50000) ;
  exec(true);
  return *this;	

}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::getNumberOfTotalKeyAliases()
{
  startOver();

  command_ << "SELECT COUNT(*)  from " << tnsKocViewDictionary_[tnsOracleName_]["config_keys"] ;

  reCreateStatement();
  setSql();
  setPrefetch(50000) ;
  exec(true);
  return *this;	

}

//====================================================================================================================
PixelSQLCommand& PixelSQLCommand::getTotalCfgs()
{
  startOver();

  command_ << "SELECT UNIQUE KEY_NAME from " << tnsKocViewDictionary_[tnsOracleName_]["dataset_map"] << " where is_moved_to_history = 'F'" ;

  reCreateStatement();
  setSql();
  setPrefetch(50000) ;
  exec(true);
  return *this;	

}

//====================================================================================================================
string PixelSQLCommand::printType (int type)
{
  switch (type)
    {
    case OCCI_SQLT_CHR : return "VARCHAR2";
      break;
    case OCCI_SQLT_NUM : return "NUMBER";
      break;
    case OCCIINT : return "INTEGER";
      break;
    case OCCIFLOAT : return "FLOAT";
      break;
    case OCCI_SQLT_STR : return "STRING";
      break;
    case OCCI_SQLT_VNU : return "VARNUM";
      break;
    case OCCI_SQLT_LNG : return "LONG";
      break;
    case OCCI_SQLT_VCS : return "VARCHAR";
      break;
    case OCCI_SQLT_RID : return "ROWID";
      break;
    case OCCI_SQLT_DAT : return "DATE";
      break;
    case OCCI_SQLT_VBI : return "VARRAW";
      break;
    case OCCI_SQLT_BIN : return "RAW";
      break;
    case OCCI_SQLT_LBI : return "LONG RAW";
      break;
    case OCCIUNSIGNED_INT : return "UNSIGNED INT";
      break;
    case OCCI_SQLT_LVC : return "LONG VARCHAR";
      break;
    case OCCI_SQLT_LVB : return "LONG VARRAW";
      break;
    case OCCI_SQLT_AFC : return "CHAR";
      break;
    case OCCI_SQLT_AVC : return "CHARZ";
      break;
    case OCCI_SQLT_RDD : return "ROWID";
      break;
    case OCCI_SQLT_NTY : return "NAMED DATA TYPE";
      break;
      case OCCI_SQLT_REF : return "REF";
	break;
    case OCCI_SQLT_CLOB: return "CLOB";
      break;
    case OCCI_SQLT_BLOB: return "BLOB";
      break;
    case OCCI_SQLT_FILE: return "BFILE";
      break;
    case OCCI_SQLT_TIMESTAMP_LTZ : return "TIMESTAMP WITH LOCAL TIMEZONE";
      break ;
    case OCCI_SQLT_TIMESTAMP: return "TIMESTAMP";
      break ;	
    case OCCI_SQLT_TIMESTAMP_TZ: return "TIMESTAMP WITH TIMEZONE";
      break ;
    }
  return "Unknown data type" ;
} // End of printType (int)

