//
// Test program that will read and write out 
// configuration files.
//
//
//
//
//

#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTBMSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTKFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortcardMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDelay25Calib.h"
#include "CalibFormats/SiPixelObjects/interface/PixelLTCConfig.h"
//Should really not depend on PixelConfigFile... Hm should move int
//PixelConfigDBInterface...
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTTCciConfig.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <iomanip> 

using namespace std;
using namespace pos;

pos::PixelConfigBase* getVersion(std::string path,int version){
  if (path=="nametranslation/") {
    pos::PixelNameTranslation* object=0;
    PixelConfigInterface::get(object,path,version);
    return object;
  } else if (path=="fedconfig/") {
    pos::PixelFEDConfig* object=0;
    PixelConfigInterface::get(object,path,version);
    return object;
  } else if (path=="fecconfig/") {
    pos::PixelFECConfig* object=0;
    PixelConfigInterface::get(object,path,version);
    return object;
  } else if (path=="tkfecconfig/") {
    pos::PixelTKFECConfig* object=0;
    PixelConfigInterface::get(object,path,version);
    return object;
  } else if (path=="detconfig/") {
    pos::PixelDetectorConfig* object=0;
    PixelConfigInterface::get(object,path,version);
    return object;
  } else if (path=="portcardmap/") {
    pos::PixelPortcardMap* object=0;
    PixelConfigInterface::get(object,path,version);
    return object;
  } else {
    cout << "Unknown path:"<<path<<endl;
    assert(0);
  }
  //Make compile happy...
  return 0;
}

void printHelp(){

  cout << "Usage:" <<endl;
  cout << "PixelConfigDBCmd.exe --printKey GlobalKey" <<endl;
  cout << "PixelConfigDBCmd.exe --printAlias" <<endl;
  cout << "PixelConfigDBCmd.exe --printAliasFull SCurve" <<endl;
  cout << "PixelConfigDBCmd.exe --printVersionAlias dac" <<endl;
  cout << "PixelConfigDBCmd.exe --insertData ltcconfig LTCConfiguration.txt" << endl;
  cout << "PixelConfigDBCmd.exe --insertDataSet fedcard fedcardlist.txt" << endl;
  cout << "PixelConfigDBCmd.exe --getVersion nametranslation/ 0" << endl;
  cout << "PixelConfigDBCmd.exe --insertConfigAlias PhysicsLowLumi 0" << endl;
  cout << "PixelConfigDBCmd.exe --insertVersionAlias dac 0 Default" << endl;
  cout << "PixelConfigDBCmd.exe --insertConfigAlias Physics dac Default"<<endl;
  cout << "                       detconfig Default nametranslation Default"<<endl;
  cout << "                       fecconfig Default fedconfig Default"<<endl;
  cout << "                       tkfecconfig Default portcardmap Default " << endl;
  cout << "PixelConfigDBCmd.exe --getFiles pixel/nametranslation/ 0" << endl;


}

int main(int argc, char* argv[]){

  PixelConfigInterface Iconfig_ ;

  if (argc<2){
    printHelp();
    ::exit(0);
  }

  std::string cmd=argv[1];


  if (cmd=="--insertConfigAlias"&&argc==4){
    std::string alias=argv[2];
    unsigned int key=atoi(argv[3]);
    Iconfig_.addAlias(alias,key);
  }
  else if (cmd=="--getVersion"){
    assert(argc>3);
    std::string path=argv[2];
    unsigned int version=atoi(argv[3]);
    pos::PixelConfigBase* object=getVersion(path,version);
    object->writeASCII();
  }
  else if (cmd=="--insertVersionAlias"){
    assert(argc>4);
    std::string path=argv[2];
    unsigned int version=atoi(argv[3]);
    std::string alias=argv[4];
    Iconfig_.addVersionAlias(path,version,alias);
  }
  else if (cmd=="--insertConfigAlias"){
    assert(argc>4);
    std::string alias=argv[2];
    int argcounter=3;
    std::vector<std::pair<std::string, unsigned int> > versions;
    std::vector<std::pair<std::string, std::string> > versionAliases;
    while (argcounter+1<argc){
      std::string path=argv[argcounter];
      std::string alias=argv[argcounter+1];
      unsigned int ver;
      if (isdigit(*argv[argcounter+1])) {
	ver=atoi(argv[argcounter+1]);
      }
      else{
	std::pair<std::string, std::string> apair(path,alias);
	versionAliases.push_back(apair);
	ver=Iconfig_.getVersion(path,alias);
      }
      std::pair<std::string, unsigned int> apair(path,ver);
      versions.push_back(apair);
      argcounter+=2;
    }

    unsigned int key=Iconfig_.makeKey(versions);

    Iconfig_.addAlias(alias,key,versionAliases);

  }
  else if (cmd=="--printAlias"){
    std::vector<std::pair<std::string, unsigned int> > aliases=
                            Iconfig_.getAliases();
    std::cout << "Currently defined aliases:" << std::endl;
    unsigned int maxAliasLength=0;
    for(unsigned int i=0;i<aliases.size();i++){
      if (aliases[i].first.size()>maxAliasLength){
	maxAliasLength=aliases[i].first.size();
      }
    }
    for(unsigned int i=0;i<aliases.size();i++){
      std::cout << setw(maxAliasLength)<< aliases[i].first<<"  =  "<<aliases[i].second<<std::endl;
    }
  }
  else if (cmd=="--printAliasFull"){
    std::string alias=argv[2];
    unsigned int key;
    std::vector<std::pair<std::string, std::string> > versionAliases;
    bool exists=Iconfig_.getVersionAliases(alias,
					   key,
					   versionAliases);
    if (exists){
      cout << alias << " (key="<<key<<")";
      if (versionAliases.size()>0) {
	cout <<": ";
	for(unsigned int i=0;i<versionAliases.size();i++){
	  cout << versionAliases[i].first<<" "
	       << versionAliases[i].second<<" ";
	}	
      }
      std::cout << endl;
    } else {
      std::cout << "The configuration alias '"<<alias<<"' don't exist." 
		<< std::endl;      
    }
  }
  else if (cmd=="--printKey"){
    unsigned int key = atoi(argv[2]);
    std::vector<std::pair<std::string, unsigned int> > versions;
    versions = Iconfig_.getVersions(pos::PixelConfigKey(key));
    std::cout << "************ GLOBAL KEY " << key << " **************" << std::endl << std::endl ;
    int count = 1 ;
    std::cout << std::setw(23) << "KOC" << "\t" << "version" << std::endl << std::endl ;
    for(std::vector<std::pair<std::string, unsigned int> >::iterator it = versions.begin() ;
	it != versions.end() ; it++)
      {
	std::cout << std::setw(2) << count << "]" << std::setw(20) << (*it).first << "\t" << (*it).second << std::endl ;
	count++ ;
      }
    std::cout << "______________________________________________" << std::endl ;
  }
  else if (cmd=="--printVersionAlias"){
    std::string path=argv[2];
    std::vector<std::string> aliases = Iconfig_.getVersionAliases(path);

    std::cout << "Currently defined aliases for data type:"<<path << std::endl;
    for(unsigned int i=0;i<aliases.size();i++){
      std::cout << " " << aliases[i] << std::endl;
    }
  }
  else if (cmd=="--insertData"){
    std::string path=argv[2];
    std::string filename=argv[3];
    pos::PixelConfigBase* object=0;
    if (path=="nametranslation"){
      object=new pos::PixelNameTranslation(filename);
    } else if (path=="calib"){
      if (filename.find(std::string("calib.dat"))!=std::string::npos) {
	object=new pos::PixelCalibConfiguration(filename);
      } else if (filename.find(std::string("delay25.dat"))!=std::string::npos) {
	//pos::PixelDelay25Calib * tmp=new pos::PixelDelay25Calib(filename);
	//unsigned int version=Iconfig_.put(tmp,path);    
	Iconfig_.commit(0) ;
	return 0;
      } else {
	cout << "For calib path dont know filename:"<<filename<<endl;
	assert(0);
      }
			       
    } else if (path=="detconfig"){
      object=new pos::PixelDetectorConfig(filename);
    } else if (path=="ltcconfig"){
      object=new pos::PixelLTCConfig(filename);
    } else if (path=="ttcciconfig"){
      object=new pos::PixelTTCciConfig(filename);
    } else if (path=="fecconfig"){
      object=new pos::PixelFECConfig(filename);
    } else if (path=="tkfecconfig"){
      object=new pos::PixelTKFECConfig(filename);
    } else if (path=="fedconfig"){
      object=new pos::PixelFEDConfig(filename);
    } else if (path=="portcardmap"){
      object=new pos::PixelPortcardMap(filename);
    } else {
      std::cout << "Unknown path:"<<path<<std::endl;
      assert(0);
    }

    unsigned int version=Iconfig_.put(object,path);    
    Iconfig_.commit(0) ;

    std::cout << "Inserted as version:"<<version<< std::endl;
  }
  else if (cmd=="--insertDataSet"){
    std::string path=argv[2];
    std::string filename=argv[3];
    std::vector<pos::PixelConfigBase*> objects;
    ifstream in(filename.c_str());
    assert(in.good());
    std::string datafilename;
    in >> datafilename;
    while (!in.eof()){
      cout << "Will insert file:"<<datafilename<<endl;
      if (path=="dac"){
        objects.push_back((pos::PixelConfigBase*)new pos::PixelDACSettings(datafilename));
      } else if (path=="tbm"){
        objects.push_back((pos::PixelConfigBase*)new pos::PixelTBMSettings(datafilename));
      } else if (path=="portcard"){
        objects.push_back((pos::PixelConfigBase*)new pos::PixelPortCardConfig(datafilename));
      } else if (path=="trim"){
        objects.push_back((pos::PixelConfigBase*)new pos::PixelTrimAllPixels(datafilename));
      } else if (path=="mask"){
        objects.push_back((pos::PixelConfigBase*)new pos::PixelMaskAllPixels(datafilename));
      } else if (path=="fedcard"){
        objects.push_back((pos::PixelConfigBase*)new pos::PixelFEDCard(datafilename));
      } else {
        std::cout << "Unknown path:"<<path<<std::endl;
        assert(0);
      }
      in >> datafilename;
    }
    unsigned int version=Iconfig_.put(objects,path);    
    Iconfig_.commit(0) ;

    std::cout << "Inserted as version:"<<version<< std::endl;
  }
  else if (cmd=="--getFiles"){
    assert(argc>3);
    std::string path=argv[2];
    unsigned int version=atoi(argv[3]);
    PixelConfigKey key(version);
    std::string filePath=PixelConfigFile::getPath(path,key);
    cout << "filePath="<<filePath<<endl;
    std::string cmd="cp -p "+filePath+"*.dat .";
    cout << "cmd="<<cmd.c_str()<<endl;
    system(cmd.c_str());
  }
  else{
    std::cout << "Unknown option:" << argv[1] << std::endl;
  }

  return 0;

}

