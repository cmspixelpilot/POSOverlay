#ifndef PixelConfigInterface_h
#define PixelConfigInterface_h
/*
$Author: menasce $
$Date: 2009/11/17 15:12:22 $
$Revision: 1.77 $
*/

#include <stdlib.h>
#include <iostream>
#include <string>
#include <set>
#include "PixelConfigDBInterface/include/PixelConfigDBInterface.h" // was DB
#include "CalibFormats/SiPixelObjects/interface/PixelTimeFormatter.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h" 
#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h" 
#include "CalibFormats/SiPixelObjects/interface/PixelConfigDoc.info" 
#include <cstdlib>

//typedef int PixelConfigKey;

/*template <class K> */
/*!  \defgroup AbstractConfigurationInterface "Configuration Objects Interface"
 *    \brief The class describing an abstact interface to the "Configuration Objects".
 *
 *  Configuration Objects represent the knowledge of the whole CMS Pixel %Detector at
 *  any given time. This knowledge is made persistent in two alternative ways:<br/>
 *  <ul>
 *   <li> a file-based set of configurations, stored in an appropriate hierarchy of 
 *        folders, indexed by a main "configuration key"
 *   <li> a full fledged Oracle Database, were the %Detector status is represented by
 *        a complex set of relational tables.
 *  </ul>
 *  The "Configuration Objects Interface" encompasses both of these persistency methodologies.
 *  In particular ...
 *
 *  @{
 *   \class PixelConfigInterface PixelConfigInterface.h "interface/PixelConfigInterface.h"
 *   \brief The concrete class describing the "file-based configuration" interface
 */
class PixelConfigInterface {

 public:

  PixelConfigInterface(){getMode();}
  PixelConfigInterface(bool mode){setMode(mode);}
  
  //==============================================================================
  static void setMode(bool m)
  {
    //cout << __LINE__ << "]\t[PixelConfigInterface::setMode()]\t\t    " << endl ;
    if(getMode() != m)
      {
        getMode() = m;
        if(getMode() == true)
          {
            pixelConfigDB().connect();
          }
        else
          {
            pixelConfigDB().disconnect();
          }
       }
  }

  //==============================================================================
  static void setKeyAlias(pos::PixelConfigKey key, std::string theAlias)
  {
    if(getMode())
      {
	pixelConfigDB().setKeyAlias(key, theAlias);
      }
    else
      {;}
  }

  //==============================================================================
  static void setGlobalKey(pos::PixelConfigKey key)
  {
    if(getGlobalKey().key() != key.key())
      {
	getGlobalKey() = key;
      }
  }
  
  //==============================================================================
  template <class T>
    static void get(T* &pixelObject,
		    std::string path,
		    pos::PixelConfigKey key)
    {
      std::string mthn = "[PixelConfigInterface.h::get(scalar)]\t\t\t    " ;
      std::stringstream arg ; arg << "PixelConfigInterface::get(T*) T=" << typeid(pixelObject).name() ;
      pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter(arg.str()) ;
      setGlobalKey(key) ;
      if(getMode())
	{
	  pixelConfigDB().connect();
	  pixelConfigDB().get(pixelObject,path,key);
	}
      else
	{
	  pos::PixelConfigFile::get(pixelObject,path,key);
	}
      timer->stopTimer() ;
      delete timer ;
    }

  //==============================================================================
  template <class T>
    static void get(T* &pixelObject,std::string path,unsigned int version)
    {
      if(getMode())
        {
      	  //pixelConfigDB().connect();
	  pixelConfigDB().getByVersion(pixelObject,path,version);
      	}
      else{
      	  pos::PixelConfigFile::get(pixelObject,path,version);
      	}
    }

  //==============================================================================
  template <class T>
    static void get(std::map<std::string, T*> &objects,pos::PixelConfigKey key)
    {
      setGlobalKey(key) ;
      std::stringstream arg ; arg << "PixelConfigInterface::get(map<string,T*>) T=" << typeid(objects).name() ;
      pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter(arg.str()) ;
      if(getMode())
	{
	  pixelConfigDB().get(objects, key) ;
	}
      else
	{
	  pos::PixelConfigFile::get(objects,key);
	}
      timer->stopTimer() ;
      delete timer ;
    }
  
  //==============================================================================
  template <class T>
    static bool configurationDataExists(T* &pixelObject,
					std::string path,
					pos::PixelConfigKey key)
    {
      std::stringstream arg ; arg << "PixelConfigInterface::configurationDataExists(T*) T=" << typeid(pixelObject).name() ;
      pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter(arg.str()) ;
      bool retStatus = false ;
      if(getMode())
	{
	  retStatus = pixelConfigDB().configurationDataExists(pixelObject,path,key);
	} 
      else
	{
	  retStatus = pos::PixelConfigFile::configurationDataExists(pixelObject,path,key);
	}
      timer->stopTimer() ;
      delete timer ;
      return retStatus ;
    }
  
  //==============================================================================
  static std::vector<std::pair<std::string, unsigned int> > getAliases()
  {
    std::string mthn = "]\t[PixelConfigInterface::getAliases()]\t\t\t    " ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getAliases()") ;
    std::vector<std::pair<std::string, unsigned int> > aliases ;
    if(getMode())
      {
        if( DEBUG_CONFIG_DB ) cout << __LINE__ << mthn << "Using DataBase connection" << endl ;
        aliases = pixelConfigDB().getAliases() ;
      }
    else
      {
        if( DEBUG_CONFIG_DB ) cout << __LINE__ << mthn << "Using files "              << endl ;
        aliases = pos::PixelConfigFile::getAliases();
      }
    timer->stopTimer() ;
    delete timer ;
    return aliases ;
  }

  //==============================================================================
  static std::vector<std::vector<std::string> > getAliasesWithCommentsAndDate()
  {
    std::string mthn = "]\t[PixelConfigInterface::getAliasesWithCommentsAndDate()]\t    " ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("[PixelConfigInterface::getAliasesWithCommentsAndDate()]") ;
    std::vector<std::vector<std::string > > aliases ;
    if(getMode())
      {
        if( DEBUG_CONFIG_DB ) cout << __LINE__ << mthn << "Using DataBase connection" << endl ;
        aliases = pixelConfigDB().getAliasesWithCommentsAndDate() ;
      }
    else
      {
        if( DEBUG_CONFIG_DB ) cout << __LINE__ << mthn << "Using files "              << endl ;
	vector<string>  tmp ;
	tmp.push_back("CONFIG_KEY_ID"        ) ; // 0
	tmp.push_back("CONFIG_KEY"           ) ; // 1
	tmp.push_back("KEY_ALIAS_ID"         ) ; // 2
	tmp.push_back("CONFIG_ALIAS"         ) ; // 3
	tmp.push_back("CONFIG_KEY_TYPE"      ) ; // 4
	tmp.push_back("RECORD_INSERTION_TIME") ; // 5
	tmp.push_back("COMMENT_DESCRIPTION"  ) ; // 6
	tmp.push_back("PROVENANCE"	     ) ; // 7
	tmp.push_back("AUTHOR"    	     ) ; // 8
        aliases.push_back(tmp) ;
	std::vector<std::pair<std::string, unsigned int> > aliasesNoCommentsAndDate = pos::PixelConfigFile::getAliases();
	for(std::vector<std::pair<std::string, unsigned int> >::iterator it = aliasesNoCommentsAndDate.begin();
	    it != aliasesNoCommentsAndDate.end() ; it++)
	  {
	    tmp.clear() ;
	    stringstream ss ;
	    ss.str("") ;
	    ss << it->second                       			      ; 		
	    tmp.push_back("Unavailable for files") 			      ; // 0 CONFIG_KEY_ID,		   fake for files 
	    tmp.push_back(ss.str())                			      ; // 1 CONFIG_KEY, GLOBAL KEY NUMBER
	    tmp.push_back("Unavailable for files") 			      ; // 2 KEY_ALIAS_ID,		   fake for files 
	    tmp.push_back(it->first)               			      ; // 3 CONFIG_KEY_ALIAS, Alias
	    tmp.push_back("Unavailable for files") 			      ; // 4 CONFIG_KEY_TYPE,		   fake for files 
	    tmp.push_back("01/01/1970 00:00:00")   			      ; // 5 RECORD_INSERTION_TYPE,	   fake for files
	    tmp.push_back("Q29tbWVudHMgYXJlIHVuYXZhaWxhYmxlIGZvciBmaWxlcw==") ; // 6 COMMENT_DESCRIPTION,   	   fake for files (base64 encoded)
	    tmp.push_back("Provenance unavailable for files")   	      ; // 7 PROVENANCE,		   fake for files
	    tmp.push_back("Author is unavailable for files")    	      ; // 8 AUTHOR,			   fake for files
	    aliases.push_back(tmp) ;
	  }
      }
    timer->stopTimer() ;
    delete timer ;
    return aliases ;
  }

  //==============================================================================
  static bool getVersionAliases(std::string configAlias,
				unsigned int &key,
				std::vector<std::pair<std::string,std::string> > &versionAliases)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getVersionAliases()") ;
    bool retStatus = false ;
    if(getMode())
      {
        retStatus = pixelConfigDB().getVersionAliases(configAlias, key, versionAliases) ;
      }
    else
      {
        retStatus = pos::PixelConfigFile::getVersionAliases(configAlias,
        					            key,
        					            versionAliases);
      }
    timer->stopTimer() ;
    delete timer ;
    return retStatus ;
  }

  //==============================================================================
  static void forceAliasesReload(bool mode) 
  {
    if(getMode())
      {
        cout << __LINE__ << "]\t[PixelConfigInterface::forceAliasesReload()]\t\t    No such function implemented yet for DB access" << endl ;
      }
    else
      {
        pos::PixelConfigFile::forceAliasesReload(mode);
      }
  }

  //==============================================================================
  static std::map<std::string, unsigned int> getAliases_map()
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getAliases_map()") ;
    std::map<std::string, unsigned int> aliasMap ;
    if(getMode())
      {
        aliasMap = pixelConfigDB().getAliases_map() ;
      }
    else
      {
        aliasMap = pos::PixelConfigFile::getAliases_map();
      }
    timer->stopTimer() ;
    delete timer ;
    return aliasMap ;
  }

  //==============================================================================
  static void addAlias(std::string alias, unsigned int key)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::addAlias(string,unsigned int)") ;
    if(getMode())
      {
        pixelConfigDB().addAlias(alias, key) ;
      }
    else
      {
        pos::PixelConfigFile::addAlias(alias,key);
      }
    timer->stopTimer() ;
    delete timer ;
    return ;
  }

  //==============================================================================
  static void addAlias(std::string alias, unsigned int key,
		       std::vector<std::pair<std::string, std::string> > versionaliases)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::addAlias(string,unsigned int,vector)") ;
    if(getMode())
      {
        pixelConfigDB().addAlias(alias,key,versionaliases);
      }
    else
      {
        pos::PixelConfigFile::addAlias(alias,key,versionaliases);
      }
    timer->stopTimer() ;
    delete timer ;
    return ;
  }

  //==============================================================================
  static void addVersionAlias(std::string path, unsigned int version, std::string alias)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::addVersionAlias()") ;
    if(getMode())
      {
        pixelConfigDB().addVersionAlias(path, version, alias) ;
      }
    else
      {
        pos::PixelConfigFile::addVersionAlias(path,version,alias);
      }
    timer->stopTimer() ;
    delete timer ;
    return ;
  }

  //==============================================================================
  static void addComment(std::string comment)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::addComment(string)") ;
    if(getMode())
      {
        pixelConfigDB().addComment( comment) ;
      }
    else
      {
        std::cout << __LINE__ << "]\t[PixelConfigInterface::addComment()]\t\t    Not implemented for file-based repository" << std::endl ;
      }
    timer->stopTimer() ;
    delete timer ;
    return ;
  }


  //==============================================================================
  static void addAuthor(std::string author)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::addAuthor(string)") ;
    if(getMode())
      {
        pixelConfigDB().addAuthor( author) ;
      }
    else
      {
        std::cout << __LINE__ << "]\t[PixelConfigInterface::addAuthor()]\t\t    Not implemented for file-based repository" << std::endl ;
      }
    timer->stopTimer() ;
    delete timer ;
    return ;
  }
  //==============================================================================
  static unsigned int clone(unsigned int oldkey, std::string path, unsigned int version)
  {
    unsigned int newkey ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::clone()") ;
    if(getMode())
      {
	newkey =  pixelConfigDB().clone(oldkey, path, version) ;
      }
    else
      {
	pos::PixelConfigList iList = pos::PixelConfigFile::getConfig() ;
	newkey = iList.clone(oldkey,path,version);
	iList.writefile() ;
      }
    timer->stopTimer() ;
    delete timer ;
    return newkey ;
  }

  //==============================================================================
  static unsigned int makeKey(std::vector<std::pair<std::string, unsigned int> > versions)
  {
    unsigned int newkey ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::makeKey()") ;
    if(getMode())
      {
        newkey = pixelConfigDB().makeKey(versions);
      }
    else
      {
        newkey = pos::PixelConfigFile::makeKey(versions);
      }
    timer->stopTimer() ;
    delete timer ;
    return newkey ;
  }

  //==============================================================================
  static unsigned int getVersion(std::string path,std::string alias)
  {
    if(getMode())
      {
	pos::pathVersionAliasMmap vData ;
	vData = pixelConfigDB().getVersionData() ;
	for(pos::pathVersionAliasMmap::iterator it = vData.begin() ; it != vData.end() ; it++)
	  {
/* 	    cout << "|" << it->first << "|\t" */
/* 		 << "|" << path      << "|" << endl ; */
	    if(it->first == path)
	      {
		for(pos::vectorVAPairs::iterator va = it->second.begin() ; va != it->second.end() ; va++)
		  {
/* 		    cout << "|" << va->second << "|\t" */
/* 			 << "|" << alias      << "|" << endl ; */
		    if(va->second == alias)
		      return va->first ;
		  }
	      }
	  }
	std::cout << __LINE__ << "]\t[PixelConfigInterface::getVersion(path, alias)]\t Fatal, no version found for path "
		  << "|" << path << "|" << " and alias |" << alias << "|" << std::endl ;
	assert(0) ;
      }
    else
      {
        return pos::PixelConfigFile::getVersion(path, alias);
      }
  }

  //==============================================================================
  //Returns the different objects and their versions for a given configuration
  static std::vector<std::pair< std::string, unsigned int> > getVersions(pos::PixelConfigKey key)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getVersions()") ;
    std::vector<std::pair< std::string, unsigned int> > versions ;
    if(getMode())
      {
        versions = pixelConfigDB().getVersions(key) ;
      }
    else
      {
        versions = pos::PixelConfigFile::getVersions(key);
      }
    timer->stopTimer() ;
    delete timer ;
    return versions ;
  }

  //==============================================================================
  static std::vector<std::vector< std::string> > getVersionsWithCommentsAndDate(pos::PixelConfigKey key)
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
       7 INSERTI_TIME
       8 COMMENT_DESCRIPTION
       9 AUTHOR
    */
    std::string mthn = "]\t[PixelConfigInterface::getVersionsWithCommentsAndDate()]\t    " ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getVersionsWithCommentsAndDate()") ;
    std::vector<std::vector< std::string> > versions ;
    if(getMode())
      {
        versions = pixelConfigDB().getVersionsWithCommentsAndDate(key) ;
      }
    else
      {
	std::vector<std::pair< std::string, unsigned int> > versionsPartial ;
	vector<string> header,tmp ;
	header.push_back("CONFIG_KEY_ID");
	header.push_back("KEY_NAME");    
	header.push_back("CONDITION_DATA_SET_ID");
	header.push_back("KIND_OF_CONDITION_ID");
	header.push_back("KIND_OF_CONDITION_NAME");
	header.push_back("CONDITION_VERSION");
	header.push_back("IS_MOVED_TO_HISTORY");  
	header.push_back("INSERT_TIME");
	header.push_back("COMMENT_DESCRIPTION");
	header.push_back("AUTHOR");
	versions.push_back(header) ;
	versionsPartial  = pos::PixelConfigFile::getVersions(key);
	for(std::vector<std::pair<std::string, unsigned int> >::iterator it = versionsPartial.begin();
	    it != versionsPartial.end() ; it++)
	  {
	    tmp.clear() ;
	    stringstream ss ;
	    ss.str("") ;
	    tmp.push_back("fake")                               	      ; // CONFIG_KEY_ID	 
	    tmp.push_back("fake")			                      ; // KEY_NAME		 
	    tmp.push_back("fake")			        	      ; // CONDITION_DATA_SET_ID 
	    tmp.push_back("fake")			        	      ; // KIND_OF_CONDITION_ID  
	    tmp.push_back(it->first)			        	      ; // KIND_OF_CONDITION_NAME
	    ss << it->second				        	      ; //  
	    tmp.push_back(ss.str())			        	      ; // CONDITION_VERSION	 
	    tmp.push_back("fake")			        	      ; // IS_MOVED_TO_HISTORY   
	    tmp.push_back("01/01/1970 00:00:00")	        	      ; // RECORD_INSERTION_TIME 
	    tmp.push_back("Q29tbWVudHMgYXJlIHVuYXZhaWxhYmxlIGZvciBmaWxlcw==") ; // COMMENT_DESCRIPTION base64_encode("Comments are unavailable for files")
	    tmp.push_back("Author is unavailable for files")                  ; // AUTHOR              "QXV0aG9yIGlzIHVuYXZhaWxhYmxlIGZvciBmaWxlcw=="
	    versions.push_back(tmp) ;
	  }
      }
    timer->stopTimer() ;
    delete timer ;
    return versions ;
  }

  //==============================================================================
  //Returns the different objects and their versions for a given configuration
  static std::map<std::string,std::vector<std::pair< std::string, unsigned int> > > getFullVersions()
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getFullVersions()") ;
    std::map<std::string,std::vector<std::pair< std::string, unsigned int> > > result ;
    if(getMode())
      {
        return pixelConfigDB().getFullVersions() ;
      }
    else
      {
        std::vector <std::pair<std::string, unsigned int> > aList = pos::PixelConfigFile::getAliases(); 
        for(std::vector<std::pair<std::string, unsigned int> >::iterator itKey=aList.begin(); itKey!=aList.end(); itKey++ )
          {
              {
        	pos::PixelConfigKey globalKey((*itKey).second) ;
        	vector<pair<string, unsigned int> > vList = pos::PixelConfigFile::getVersions(globalKey) ;
        	std::stringstream keyString ;
        	keyString.str("") ;
        	keyString << globalKey.key() ;
        	result[keyString.str()] = vList ;
              }
          }
      }
    timer->stopTimer() ;
    delete timer ;
    return result ;
  }

  //==============================================================================
  static std::map<std::string,std::vector<std::pair< std::string, std::string> > > getKeyAliasVersionAliases(int start, int howMany, int &total)
  {
    std::string mthn = "]\t[PixelConfigInterface::getKeyAliasVersionAliases()]    " ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getKeyAliasVersionAliases()") ;
    std::map<std::string,std::vector<std::pair< std::string, std::string> > > result ;
    if(getMode())
      {
        result = pixelConfigDB().getKeyAliasVersionAliases(start, howMany, total) ;
      }
    else
      {
	pos::PixelAliasList aList = pos::PixelConfigFile::getAlias() ;
	total = aList.nAliases() ;
        cout << __LINE__ << mthn << "Total aliases found: " << total << endl ;
	std::map<std::string, std::vector<std::pair<std::string, std::string> > > serviceMap ;
	for(int i = 0 ; i < total ; i++)
	  {
	    serviceMap[aList.name(i)] =  aList.operator[](i).versionAliases() ;
            std::cout << __LINE__ << mthn << i << "\t" << aList.name(i) << "\t"  << aList.key(i) << std::endl ;
	  }
	std::map<std::string, std::vector<std::pair<std::string, std::string> > >::iterator kakv_it = serviceMap.begin() ;
	int count = 0 ;
        for(; kakv_it != serviceMap.end() ; kakv_it++)
          {
            std::cout << __LINE__ << mthn << "    " << (*kakv_it).first << " <- " << std::endl ;
	    if(count >= start && count < (start+howMany))
              {
		  result[(*kakv_it).first] = (*kakv_it).second ;
              }
	    count++ ;
          }
      }
    timer->stopTimer() ;
    delete timer ;
    return result ;
  }

  //==============================================================================
  static std::map<std::string,std::vector<std::string> > getKeyAliasVersionAliases(int &total)
  {
    std::string mthn = "]\t[PixelConfigInterface::getKeyAliasVersionAliases()]    " ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getKeyAliasVersionAliases(int&)") ;
    std::map<std::string,std::vector<std::string> > result ;
    if(getMode())
      {
        result = pixelConfigDB().getKeyAliasVersionAliases(total) ;
      }
    else
      {
	pos::PixelAliasList aList = pos::PixelConfigFile::getAlias() ;
	total = aList.nAliases() ;
	for(int i = 0 ; i < total ; i++)
	  {
            std::vector<std::pair<std::string, std::string> > vaPair = aList.operator[](i).versionAliases() ;
	    for(std::vector<std::pair<std::string, std::string> >::iterator it=vaPair.begin(); it!=vaPair.end(); it++)
	    {
	      stringstream ss ;
              result[aList.name(i)].push_back((*it).first 	        			) ;
              result[aList.name(i)].push_back((*it).second	        			) ;
              result[aList.name(i)].push_back("1970, Jan 01 - 00:00:00" 			) ;
              result[aList.name(i)].push_back("Q29tbWVudHMgYXJlIHVuYXZhaWxhYmxlIGZvciBmaWxlcw==") ;
              result[aList.name(i)].push_back("Author is not available for files"               ) ;
	      ss.str("") ;
	      ss << aList.key(i) ;
	      result[aList.name(i)].push_back(ss.str()) ;
	    }
	  }
      }
    timer->stopTimer() ;
    delete timer ;
    return result ;
  }


  //==============================================================================
  static std::map<unsigned int, std::vector<std::vector< std::string> > > getConfGivenKoc(std::map<std::string, std::string> kocs,
                                                                                          int start,   
                                                                                          int howMany, 
										          int from,    
										          int to,      
											  int &total)  
  {
    std::string mthn = "]\t[PixelConfigInterface::getConfGivenKoc()]\t\t    " ;

    std::string mode = kocs["MODE"] ;

    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getConfGivenKoc()") ;
    std::map<unsigned int, std::vector<std::vector< std::string> > > result ;
    if(getMode())
      {
       std::map<std::string, std::string> query;
       for(std::map<std::string, std::string>::iterator it=kocs.begin(); it!=kocs.end(); it++)
       {
        if( (*it).first   == "start" || 
	    (*it).first   == "limit" || 
	    (*it).first   == "MODE"  || 
	    (*it).second  == "ANY"   ) continue ;
        query[(*it).first] = (*it).second ;
       }
       result = pixelConfigDB().getConfGivenKoc(query, start, howMany, from, to, total) ;
      }
    else
      {
        unsigned int expected = 0 ;
        for(std::map<std::string, std::string>::iterator it=kocs.begin(); it!=kocs.end(); it++)
	  {
	    if( (*it).first  != "start" && 
	        (*it).first  != "limit" && 
	        (*it).first  != "MODE"  && 
		(*it).second != "ANY" ) expected++ ;
	  }

	static pos::PixelConfigList &list = pos::PixelConfigFile::getConfig() ;
        total = list.size();
        int counter = 0 ;	
	for(unsigned int key=0; key<(unsigned int)total; key++)
	  {
	     pos::PixelConfigKey globalKey(key) ;
	     vector<pair<string, unsigned int> > vList = pos::PixelConfigFile::getVersions(globalKey) ;
	     std::vector<std::vector< std::string> > newvList ;
             bool accept        = true ;
             unsigned int found = 0 ;
	     for(vector<pair<string, unsigned int> >::iterator i=vList.begin(); i!=vList.end(); i++)
	       {
	     	 std::string fieldName = (*i).first ;
	     	 for(unsigned int k=0; k<fieldName.size(); k++)
	     	   {
	     	     fieldName[k] = std::toupper(fieldName[k]) ;
	     	   }

             	 if( kocs[fieldName] == "ANY" || ( kocs[fieldName].size() > 0 && 
	     	     (unsigned int)atoi(kocs[fieldName].c_str()) == (*i).second) ) 
	     	   {
		     if( kocs[fieldName] != "ANY" && (unsigned int)atoi(kocs[fieldName].c_str()) == (*i).second) found++ ;
	     	   }
	     	 else
	     	   {
	     	     accept = false ;
             	   }
//if( key == 6753 ) cout << __LINE__ << mthn << key << "]\t" << (*i).first << " --> " << kocs[fieldName] << endl ;
	     	 std::vector< std::string> tmp ;
             	 std::stringstream s ; s << (*i).second ;
             	 tmp.push_back("") ;				    // 0 CONFIG_KEY_ID         
             	 tmp.push_back("") ;				    // 1 KEY_NAME	       
             	 tmp.push_back("") ;				    // 2 CONDITION_DATA_SET_ID
             	 tmp.push_back("") ;				    // 3 KIND_OF_CONDITION_ID
             	 tmp.push_back((*i).first) ;			    // 4 KIND_OF_CONDITION_NAME
             	 tmp.push_back(s.str()) ;			    // 5 CONDITION_VERSION  
             	 tmp.push_back("") ;				    // 6 IS_MOVED_TO_HISTORY
             	 tmp.push_back("") ;				    // 7 RECORD_INSERTION_TIME
             	 tmp.push_back("") ;				    // 8 COMMENT_DESCRIPTION base46_encode("Comments are unavailable for files")
             	 tmp.push_back("") ;				    // 9 AUTHOR
             	 if( i == vList.begin() ) newvList.push_back(tmp) ; // Add a dummy record: this is to comply with DB version,
	     	 newvList.push_back(tmp) ;			    // where first vector element is actually a list of FIELD names
	       }  

             if( accept ) 
	       {
//		       cout << __LINE__ << mthn << key << "]\tmode: " << mode << endl ;
	         if( mode == "STRICT" )
		   {
		     if( found != expected ) 
		     {
//		       cout << __LINE__ << mthn << key << "]\tExpected: " << expected << " found: " << found << endl ;
		       continue ;
		     } 
		   }
	     	 counter++ ;
	     	 if( counter < start || counter > start + howMany ) continue ;
//	     	 std::stringstream keyString ;
//	     	 keyString.str("") ;
//	     	 keyString << globalKey.key() ;
	     	 result[globalKey.key()] = newvList ;
	       }
	  }
      }
    timer->stopTimer() ;
    delete timer ;

    return result ;
  }
  
  //==============================================================================
  static std::map<std::string,std::vector<std::vector< std::string> > > getFullCfgs(int start, 
                                                                                    int howMany,
										    int from,
										    int to, 
										    int &total)
  {
    std::string mthn = "]\t[PixelConfigInterface::getFullCfgs()]\t\t\t    " ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getFullCfgs()") ;
    std::map<std::string,std::vector<std::vector< std::string> > > result ;
    if(getMode())
      {
       result = pixelConfigDB().getFullCfgs(start, howMany, from, to, total) ;
      }
    else
      {
//        if( from < start )	   {from = start	;} 
//        if( to   > start+howMany ) {to   = start+howMany;} 

	static pos::PixelConfigList &list = pos::PixelConfigFile::getConfig() ;
        total = list.size();
	
	for(unsigned int key=0; key<(unsigned int)total; key++)
	  {
            if( (from == -2147483647 && (key >= (unsigned int)start && key < (unsigned int)(start+howMany))) ||
	        (from != -2147483647 && (key >= (unsigned int)from  && key <= (unsigned int)to)) ) 
	      {
	        pos::PixelConfigKey globalKey(key) ;
	        vector<pair<string, unsigned int> > vList = pos::PixelConfigFile::getVersions(globalKey) ;
	        std::vector<std::vector< std::string> > newvList ;
	        for(vector<pair<string, unsigned int> >::iterator i=vList.begin(); i!=vList.end(); i++)
	          {
	            std::vector< std::string> tmp ;
                    std::stringstream s ; s << (*i).second ;
                    tmp.push_back("") ; 						// 0 CONFIG_KEY_ID	   
                    tmp.push_back("") ; 						// 1 KEY_NAME		   
                    tmp.push_back("") ; 						// 2 CONDITION_DATA_SET_ID
                    tmp.push_back("") ; 						// 3 KIND_OF_CONDITION_ID
                    tmp.push_back((*i).first) ; 					// 4 KIND_OF_CONDITION_NAME
                    tmp.push_back(s.str()) ;						// 5 CONDITION_VERSION  
                    tmp.push_back("") ; 						// 6 IS_MOVED_TO_HISTORY
                    tmp.push_back("1970, Jan 01 - 00:00:00") ;  			// 7 RECORD_INSERTION_TIME
                    tmp.push_back("Q29tbWVudHMgYXJlIHVuYXZhaWxhYmxlIGZvciBmaWxlcw==") ; // 8 COMMENT_DESCRIPTION base46_encode("Comments are unavailable for files")
                    tmp.push_back("Author is unavailable for files") ;  		// 9 AUTHOR
	            newvList.push_back(tmp) ;
	          }  
	        std::stringstream keyString ;
	        keyString.str("") ;
	        keyString << globalKey.key() ;
	        result[keyString.str()] = newvList ;
	      }  
	  }
      }
    timer->stopTimer() ;
    delete timer ;
    return result ;
  }

  //==============================================================================
  static std::string uploadStatus(std::string uploadedFile)
  {
    if(getMode())
      {
	return pixelConfigDB().uploadStatus(uploadedFile) ;
      }
    else
      {
	return std::string("true") ;
      }
  }
  //==============================================================================
  template <class T>
    static int put( T* pixelObject,int configurationFileVersion, std::string path)
  {
    std::stringstream arg ; arg << "PixelConfigInterface::put(T*,int,string) T=" << typeid(pixelObject).name() ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter(arg.str()) ;
    int retStatus = 0 ;
    if(getMode())
      {
        retStatus = pixelConfigDB().put(pixelObject,configurationFileVersion) ;
      }
    else
      {
        retStatus = put(pixelObject, path) ;
      }
    timer->stopTimer() ;
    delete timer ;
    return retStatus;
  }

  //==============================================================================
  template <class T>
    static int put(T* pixelObject,std::string label)
  {
    std::stringstream arg ; arg << "PixelConfigInterface::put(T*,string) T=" << typeid(pixelObject).name() ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter(arg.str()) ;
    int retStatus = 0 ;
    if(getMode())
      {
        retStatus = pixelConfigDB().put(pixelObject,getGlobalKey()) ;
      }
    else
      {
        retStatus = pos::PixelConfigFile::put(pixelObject, label);
      }
    timer->stopTimer() ;
    delete timer ;
    return retStatus;
  }

  //==============================================================================
  static std::vector<std::string> getVersionAliases(std::string path)
  {
    if(getMode())
      {
        return pixelConfigDB().getVersionAliases(path);
      }
    else
      {
        //assert(0);
        return pos::PixelConfigFile::getVersionAliases(path);
      }

  }

  //==============================================================================
  template <class T>
    static int put(std::vector<T*> objects,std::string label)
  {
    std::stringstream arg ; arg << "PixelConfigInterface::put(vector<T*>,string) T=" << typeid(objects).name() ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter(arg.str()) ;
    int retStatus = 0 ;
    if(getMode())
      {
        retStatus = pixelConfigDB().put(objects,getGlobalKey()) ;
      }
    else
      {
        retStatus = pos::PixelConfigFile::put(objects, label);
      }
    timer->stopTimer() ;
    delete timer ;
    return retStatus;
  }

  //==============================================================================
  template <class T>
    static int put(std::vector<T*> objects, int configurationFileVersion, std::string path)
  {
    std::stringstream arg ; arg << "PixelConfigInterface::put(vector<T*>,int,string) T=" << typeid(objects).name() ;
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter(arg.str()) ;
    int retStatus = 0 ;
    if(getMode())
      {
        retStatus = pixelConfigDB().put(objects,configurationFileVersion) ;
      }
    else
      {
        retStatus = put(objects, path) ;
      }
    timer->stopTimer() ;
    delete timer ;
    return retStatus;
  }

  //==============================================================================
  static pos::pathVersionAliasMmap getVersionData()
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getVersionData()") ;
    pos::pathVersionAliasMmap vData ;
    if(getMode())
      {
	vData = pixelConfigDB().getVersionData() ;
      }
    else
      {
	vData = pos::PixelConfigFile::getVersionData();
      }
    timer->stopTimer() ;
    delete timer ;
    return vData ;
  }

  //==============================================================================
  static pos::pathVersionAliasMmap getVersionData(string koc)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getVersionData(string)") ;
    pos::pathVersionAliasMmap vData ;
    if(getMode())
      {
	vData = pixelConfigDB().getVersionData(koc) ;
      }
    else
      {
	vData = pos::PixelConfigFile::getVersionData(koc);
      }
    timer->stopTimer() ;
    delete timer ;
    return vData ;
  }

  //==============================================================================
  static std::vector<std::vector<std::string> > getVersionDataWithComments(string koc)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getVersionDataWithComments(string)") ;
    std::map<std::string, std::vector<std::pair< unsigned int, string> > > vData ;
    std::vector<std::vector<std::string> > result ;
    if(getMode())
      {
	result =  pixelConfigDB().getVersionDataWithComments(koc) ;
      }
    else
      {
	std::vector<std::string> tmp ;
	tmp.push_back("VERSION_ALIAS");
	tmp.push_back("VERSION");
	tmp.push_back("COMMENT_DESCRIPTION");
	tmp.push_back("RECORD_INSERTION_TIME");
	tmp.push_back("RECORD_INSERTION_USER");
	tmp.push_back("KIND_OF_CONDITION");
	result.push_back(tmp) ;
	tmp.clear() ;
	stringstream ss ;
	pos::PixelConfigFile::forceAliasesReload(true) ;
	vData = pos::PixelConfigFile::getVersionData(koc);
	for(std::map<std::string, std::vector<std::pair< unsigned int, string> > >::iterator it = vData.begin() ;
	    it != vData.end() ; it++)
	  {
	    for(std::vector<std::pair< unsigned int, string> >::iterator itt = it->second.begin() ;
		itt != it->second.end() ; itt++)
	      {
		tmp.clear() ;
		tmp.push_back(itt->second) ;
		ss.str("") ;
		ss << itt->first ;
		tmp.push_back(ss.str()) ;
		tmp.push_back("Q29tbWVudHMgYXJlIHVuYXZhaWxhYmxlIGZvciBmaWxlcw==") ;
		tmp.push_back("01/01/1970 00:00:00") ;
		tmp.push_back("Author is unavailable for files") ;
		tmp.push_back(it->first) ;
		result.push_back(tmp) ;
	      }
	  }
	return  result ;
      }
    timer->stopTimer() ;
    delete timer ;
    return  result;
  }

  //==============================================================================
  static std::set<unsigned int> getExistingVersions(string koc)
  {
    pos::PixelTimeFormatter * timer = new pos::PixelTimeFormatter("PixelConfigInterface::getExistingVersions(string)") ;
    std::set<unsigned int> result ;
    if(getMode())
      {
	result = pixelConfigDB().getExistingVersions(koc) ;
      }
    else
      {
	pos::PixelConfigList iList = pos::PixelConfigFile::getConfig();
	for(unsigned int i = 0 ; i < iList.size() ; i++)
	  {
	    unsigned int version ;
	    if(iList[i].find(koc, version) != -1)
	      {
		result.insert(version) ;
	      }
	  }
      }
    timer->stopTimer() ;
    delete timer ;
    return result ;
  }

  //==============================================================================
  static std::vector<pos::pathAliasPair> getConfigAliases(std::string path) 
  {
    //    if(getMode())
    //      {
    //        return PixelConfigDBInterface::getConfigAliases(path);
    //      }
    //    else
    //      {
    return pos::PixelConfigFile::getConfigAliases(path);
    //      }
  }

  //==============================================================================
  static bool& getMode()
  {
    static bool mode = std::string(getenv("PIXELCONFIGURATIONBASE"))=="DB"; 
//    cout << __LINE__ << "]\t[PixelconfigInterface::getMode()]\t\t    " << "Setting mode to: " << mode << endl ;
    if(mode) pixelConfigDB().connect();
    //    static bool mode = false ; 
    return mode;
  }

  //==============================================================================
  static pos::PixelConfigKey& getGlobalKey()
  {
    static pos::PixelConfigKey lastUsedKey_(0); //for now make -1 initial value
    return lastUsedKey_;
  }

  //==============================================================================
  static std::string commit(int newKey)
  {
    if(getMode())
      {
	return pixelConfigDB().commitToDB(newKey) ;
      }
    else
      {
        std::cout << __LINE__ << "]\t[PixelConfigFile::commit()]\t\t\t    Not implemented for file-based repository" << std::endl ;
	return ("") ;
      }
  }  

 private:

  static PixelConfigDBInterface& pixelConfigDB(){
    static PixelConfigDBInterface aPixelConfigDB;
    return aPixelConfigDB;
  }

};

/* @} */

#endif
