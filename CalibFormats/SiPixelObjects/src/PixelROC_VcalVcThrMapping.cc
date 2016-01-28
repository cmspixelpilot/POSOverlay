
/*************************************************************************
 * Auxiliary class for mapping the relation bewteen VCal and VcThr per  *
 * each ROC                                                                   *
 *                                                                       *
 * Author: Annapaola de Cosa, UZH                                        *
 *                                                                       *
 * Last update: $Date: 2014/04/04                                        *
 *************************************************************************/


#include "CalibFormats/SiPixelObjects/interface/PixelROC_VcalVcThrMapping.h"
#include <unistd.h>
//std::map<std::string, std::vector<float> > PixelROC_VcalVcThrMapping::rocmap;

//PixelROC_VcalVcThrMapping* PixelROC_VcalVcThrMapping::mapInstance=NULL;

PixelROC_VcalVcThrMapping* PixelROC_VcalVcThrMapping::mapInstance=0;


bool PixelROC_VcalVcThrMapping::inited = false;

PixelROC_VcalVcThrMapping* PixelROC_VcalVcThrMapping::Instance()
{
  if(!mapInstance){
    mapInstance = new PixelROC_VcalVcThrMapping();
  }
  return mapInstance;
}


PixelROC_VcalVcThrMapping::PixelROC_VcalVcThrMapping() 
{
  if(!inited)
    {
      // Delete mapRocOffset.txt if it already exists
            int res = access((std::string(getenv("POS_OUTPUT_DIRS")) + "/mapRocOffset.txt").c_str(), R_OK);
	    /*      if (res == 0) {
	unlink((std::string(getenv("POS_OUTPUT_DIRS")) + "/mapRocOffset.txt").c_str());
	std::cout<<"---> removing mapRocOffset.txt"<<std::endl;
      }
      else {
	std::cout<<"---> mapRocOffset.txt does not exist yet"<<std::endl;
	}*/
      ////////////////////////////////////////////////////////////////////////////////////

      std::ifstream mapfile;

      std::cout<<"*******************"<<std::endl;
      std::cout<<getenv("POS_OUTPUT_DIRS")<<std::endl;
      std::cout<<"*******************"<<std::endl;
      mapfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      try{
	
	//	std::string filename = "mapRocVcalVcThr.txt";
	std::string filename = std::string(getenv("POS_OUTPUT_DIRS"))+"/mapRocVcalVcThr.txt";
	std::cout<<"File to open "<<filename<<std::endl;
	mapfile.open(filename.c_str());
	if (mapfile.is_open())
	  {
	    std::cout<<"==> File opened <=="<<std::endl;
	  }
	else
	  {
	    std::cout << "Error opening file";
	  }

	std::string line;
	while(mapfile.good())
	  {
	    
	    std::getline(mapfile,line);
	    //	    std::cout<< "LINE: "<<line<<std::endl;
	    
	    if(line[0]=='B' || line[0] == 'F'){
	      std::istringstream iss(line);
	      std::string label;  
	      float a, b;
	      iss>>label;
	      iss>>a;
	      iss>>b; 
	      //std::cout<< "label a  b "<<label<< " "<< a << " " << b<<std::endl;
	      std::vector<float> params;
	      params.push_back(a);
	      params.push_back(b);
	      rocmap.insert(std::pair<std::string, std::vector<float> >(label,params) );
	      //std::cout<<"Value stored "<<rocmap[label][0]<<std::endl;
	      
	    }//end of if statement on line
	    
	  }//end of while loop
	
	mapfile.close();
	
      }//end of try statement
      catch (std::ifstream::failure e){
	std::cout<<"Exception opening/reading/closing file"<<std::endl;
	std::cout<<mapfile.good()<<std::endl;
	std::cout<<mapfile.bad()<<std::endl;
	std::cout<<mapfile.fail()<<std::endl;
	std::cout<<mapfile.eof()<<std::endl;
	
      }
      
      //      mapfile.close();
      inited = true;
      //      std::cout<<"INITIALIZATION "<<inited<<std::endl;
    }// if statement 
  else std::cout<<"Initialization already done "<<inited<<std::endl;
  
  std::cout<<"PixelROC_VcalVcThrMapping Constructor"<<std::endl;
  
}

//PixelROC_VcalVcThrMapping::~PixelROC_VcalVcThrMapping() {
//  delete mapInstance;
//  instance = NULL;
//  }



std::vector<float> PixelROC_VcalVcThrMapping::getValue(std::string const &key) { 
  return mapInstance->rocmap[key]; 
}

