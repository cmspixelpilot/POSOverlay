#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTrimAllPixels.h"
#include "CalibFormats/SiPixelObjects/interface/PixelMaskAllPixels.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortcardMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTBMSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
#include <iostream>

using namespace pos;

int main(){
  PixelConfigInterface base;
  PixelDACSettings * a = 0;
  PixelTrimAllPixels * b =0;
  PixelMaskAllPixels * c = 0; 
  PixelFECConfig *d = 0;
  PixelFEDConfig *e = 0;
  PixelPortcardMap *f = 0;
  PixelDetectorConfig *g = 0;
  PixelPortCardConfig *h = 0 ;
  PixelTBMSettings *i = 0;
  PixelNameTranslation *j = 0;
  int swt;
  PixelConfigKey theKey(5);


  cout << "Do you want to read from file(f) or database(d)?";
  string fileOrDb = "";
  cin >> fileOrDb;
  cout<<endl;
  if(fileOrDb == "d"){
    base.setMode(true);
  }
  else if(fileOrDb == "f"){
    base.setMode(false);
  }
  else{
    cout << "Don't know what to do with command --> " << fileOrDb << endl;
    return 0;
  }
  
  cout<<"XDAQ Configuration Classes:"<<endl<<endl;
  
  cout<<"1)  PixelDACSettings"<<endl;
  cout<<"2)  PixelTrimAllPixels"<<endl;
  cout<<"3)  PixelMaskAllPixels"<<endl;
  cout<<"4)  PixelFECConfig"<<endl;
  cout<<"5)  PixelFEDConfig"<<endl;
  cout<<"6)  PixelPortcardMap"<<endl;
  cout<<"7)  PixelDetectorConfig"<<endl;
  cout<<"8)  PixelPortCardConfig"<<endl;
  cout<<"9)  PixelTBMSetings"<<endl;
  cout<<"10) PixelNameTranslation"<<endl;
  cout<<"11) Run All"<<endl;
  cout<<"12) Exit"<<endl;
  
  cout<<"Choose which XDAQ class you want to Configure or Initialize"<<endl;
  cin>>swt;
  cout<<endl<<endl;
  
  switch(swt){
  case 1: base.get(a,"FPix_BmO_D2_BLD8_PNL2",theKey);
//   case 1: base.get(a,"FPix_BmI_D1_BLD8_PNL2",theKey);
    break;
  case 2: base.get(b,"FPix_BmO_D2_BLD8_PNL2",theKey);
    break; 
  case 3: base.get(c,"FPix_BmO_D2_BLD8_PNL2",theKey);			//The string and the int that im sending are not doing
    break;					                  //anything, the int is the future PixelConfigkey that
  case 4: base.get(d,"",theKey);			       	//is going to be implemented in the database.
    d->writeASCII();
    break;
  case 5: base.get(e,"",theKey);
    break;
  case 6: base.get(f,"FPix_BmO_D2_PRT1",theKey);
    break;
  case 7: base.get(g,"",theKey);
    g->writeASCII();
    break;
  case 8: base.get(h,"FPix_BmO_D2_PRT3",theKey);
    break;
  case 9: base.get(i,"FPix_BmO_D2_BLD8_PNL2",theKey);
          std::cout<<*i<<std::endl; 
    break;
  case 10: base.get(j,"FPix_BmO_D2_BLD8_PNL2",theKey);
    j->writeASCII();      
    break;
  case 11: base.get(a,"FPix_BmO_D2_BLD8_PNL2",theKey);    
    	   base.get(b,"FPix_BmO_D2_BLD8_PNL2",theKey);        
           base.get(c,"FPix_BmO_D2_BLD8_PNL2",theKey);    
           base.get(d,"",theKey);
           base.get(e,"",theKey);
    	   base.get(f,"FPix_BmO_D2_PRT1",theKey);
    	   base.get(g,"FPix_BmO_D2_BLD8_PNL2",theKey); 
    	   base.get(h,"FPix_BmO_D2_PRT3",theKey);
    	   base.get(i,"FPix_BmO_D2_BLD8_PNL2",theKey);
    	   base.get(j,"FPix_BmO_D2_BLD8_PNL2",theKey);
    break;
    
  case 12: break;
    
  default :
     cerr<<swt<<" Is not an option"<<endl; 
  }
  
  return 0;
}
