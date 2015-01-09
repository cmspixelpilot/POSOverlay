#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
//#include "PixelAnalysisTools/include/PixelXmlReader.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace pos;
   
///////////////////////////////////////////////////////////////////////////////////
PixelCalibConfigurationExtended::PixelCalibConfigurationExtended(string calibFileName, PixelNameTranslation *pixelNameTranslation, 
                             PixelDetectorConfig *pixelDetectorConfig) : PixelCalibConfiguration(calibFileName){
   
   thePixelNameTranslation_  = pixelNameTranslation;
   buildROCAndModuleLists(pixelNameTranslation, pixelDetectorConfig);
   rocList_ = PixelCalibConfiguration::rocList();
   fillFedChannelHWandTypeInfo();
};

///////////////////////////////////////////////////////////////////////////////////
void PixelCalibConfigurationExtended::fillFedChannelHWandTypeInfo(){
  string mthn = "[PixelCalibConfigurationExtended::fillFedChannelHWandTypeInfo()]\t";
  fedChannelHWandTypeInfo_.clear();
  string panelType;
  vector<pair<vector<int>, string> > result ;
  std::vector<int> tmpV;				  	 	    
  tmpV.push_back(-1);				  	 	    
  tmpV.push_back(-1);				  	 	    
  tmpV.push_back(-1);
  tmpV.push_back(-1);
  tmpV.push_back(-1);
  int numberOfRocs=0;			  	 	    
  for(vector<PixelROCName>::iterator iter = rocList_.begin();iter!=rocList_.end() ; ++iter, ++numberOfRocs){
    if(iter->detsub() == 'F'){
      unsigned int hub  = thePixelNameTranslation_->getHdwAddress(*iter)->hubaddress();
      unsigned int port = thePixelNameTranslation_->getHdwAddress(*iter)->portaddress();
      unsigned int plaq = iter->plaquet();
      if( hub==12 || hub==13 || hub==14 ){
        //it is a 4 type
        if(plaq == 1 && port == 1 ||
           plaq == 2 && port == 0 ||
           plaq == 3 && port == 2 ||
           plaq == 4 && port == 3){
           //it is a 4L type
	         panelType = "4L";
         }
         else if(plaq == 1 && port == 2 ||
                 plaq == 2 && port == 3 ||
                 plaq == 3 && port == 1 ||
                 plaq == 4 && port == 0){
           //it is a 4R type
	         panelType = "4R";
         }
         else{
           cout << mthn << "Unrecognized 4 type panel: " << *iter << endl;
           continue;
         }
      }
      else if( hub==4 || hub==5 || hub==6 ){
         //it is a 3 type
        if(plaq == 1 && port == 1 ||
           plaq == 2 && port == 2||
           plaq == 3 && port == 3){
           //it is a 3L type
	         panelType = "3L";
         }
         else if(plaq == 1 && port == 2 ||
                 plaq == 2 && port == 1 ||
                 plaq == 3 && port == 0){
           //it is a 3R type
	         panelType = "3R";
         }
        else{
          cout << mthn << "Unrecognized 3 type panel: " << *iter << endl;
	        continue;
        }
      }
      else if( hub==31 ) { //(hub==31 is for Cornell teststand)
        if(plaq == 3 && port == 0) {
	        panelType = "3C";
        }
        else{
	        cout << mthn << "Unrecoginized panel on hub 31 (test stand?): " << *iter << endl;
	        continue;
        }
      }
      else{
        cout << mthn << "Unrecognized hub " << hub << " for panel " << *iter << endl;
        continue;
      }
      if(tmpV[0] != iter->disk()  || 
         tmpV[1] != iter->blade() ||
         tmpV[2] != iter->panel() ){
//          cout << mthn 																																					  
//  				     << "Changed name: " << *iter  																										  
//               << " Panel type: "  << panelType  																								  
//   	  	       << " Fed number: "  << thePixelNameTranslation_->getHdwAddress(*iter)->fednumber()  
//   	  	       << " Fed chan: "    << thePixelNameTranslation_->getHdwAddress(*iter)->fedchannel() 
//   	  	       << endl;																																					  
        tmpV[0] = iter->disk();					       
        tmpV[1] = iter->blade();					       
        tmpV[2] = iter->panel();
        pair<vector<int>,string> tmpPair = make_pair<vector<int>,string>(tmpV,panelType);
        bool alreadyThere = false;
        vector<pair<vector<int>, string> >::iterator it=result.begin();
				for(; it!=result.end();++it){
				  unsigned int iV = 0;
          for(iV=0; iV<tmpV.size(); ++iV){
					  if((*it).first[iV] != tmpV[iV]){
              break;
				    }
					}
					if(iV==tmpV.size()-1){
						alreadyThere = true;
					}
        }
        if(it==result.end()){					      
          vector<unsigned int> fedChannel;
          result.push_back(tmpPair);
       		fedChannel.push_back(iter->disk());
       		fedChannel.push_back(iter->blade());
       		fedChannel.push_back(iter->panel());
       		fedChannel.push_back(thePixelNameTranslation_->getHdwAddress(*iter)->fednumber());
       		fedChannel.push_back(thePixelNameTranslation_->getHdwAddress(*iter)->fedchannel());
       		fedChannelHWandTypeInfo_.push_back(make_pair<vector<unsigned int>,string>(fedChannel,panelType));
		  		panelTypeMap_[thePixelNameTranslation_->getHdwAddress(*iter)->fednumber()][thePixelNameTranslation_->getHdwAddress(*iter)->fedchannel()] = panelType;
        }    
      }
    }
    else{
      unsigned int fed = thePixelNameTranslation_->getHdwAddress(*iter)->fednumber();
      unsigned int channel = thePixelNameTranslation_->getHdwAddress(*iter)->fedchannel();
      if(tmpV[0] != iter->sec()  || 
         tmpV[1] != iter->layer() ||
         tmpV[2] != iter->ladder() || 
         tmpV[3] != iter->module() ||
         tmpV[4] != (int)channel
        ){
//          cout << mthn 																																					  
//  				     << "Changed name: " << *iter  																										  
//               << " Panel type: "  << panelType  																								  
//   	  	       << " Fed number: "  << thePixelNameTranslation_->getHdwAddress(*iter)->fednumber()  
//   	  	       << " Fed chan: "    << thePixelNameTranslation_->getHdwAddress(*iter)->fedchannel() 
//   	  	       << endl;																																					  
        panelType = iter->HF();
        if(panelType == "F"){
          if(iter->layer() == 1 || iter->layer() == 2){
            panelType += thePixelNameTranslation_->ChannelFromFEDChannel(fed,channel).TBMChannel().string();
          }
        }
        numberOfRocs = 0;
        tmpV[0] = iter->sec();					       
        tmpV[1] = iter->layer();					       
        tmpV[2] = iter->ladder();
        tmpV[3] = iter->module();
        tmpV[4] = channel;
        pair<vector<int>,string> tmpPair = make_pair<vector<int>,string>(tmpV,panelType);
        bool alreadyThere = false;
        vector<pair<vector<int>, string> >::iterator it=result.begin();
				for(; it!=result.end();++it){
				  unsigned int iV = 0;
          for(iV=0; iV<tmpV.size(); ++iV){
					  if((*it).first[iV] != tmpV[iV]){
              break;
				    }
					}
					if(iV==tmpV.size()-1){
						alreadyThere = true;
					}
        }
        if(it==result.end()){					      
          vector<unsigned int> fedChannel;
          result.push_back(tmpPair);
       		fedChannel.push_back(iter->sec());
       		fedChannel.push_back(iter->layer());
       		fedChannel.push_back(iter->ladder());
       		fedChannel.push_back(thePixelNameTranslation_->getHdwAddress(*iter)->fednumber());
       		fedChannel.push_back(thePixelNameTranslation_->getHdwAddress(*iter)->fedchannel());
       		fedChannelHWandTypeInfo_.push_back(make_pair<vector<unsigned int>,string>(fedChannel,panelType));
		  		panelTypeMap_[thePixelNameTranslation_->getHdwAddress(*iter)->fednumber()][thePixelNameTranslation_->getHdwAddress(*iter)->fedchannel()] = panelType;
        }    
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////////
void PixelCalibConfigurationExtended::getRowsAndCols(unsigned int state, const vector<unsigned int> *&rows, 
                                   const vector<unsigned int> *&cols) const{
  assert(state<nConfigurations());
  unsigned int i_ROC=state/(columnList().size()*rowList().size()*nScanPoints());
  unsigned int i_row=(state-i_ROC*columnList().size()*rowList().size()*nScanPoints())/
                     (columnList().size()*nScanPoints());
  unsigned int i_col=(state-i_ROC*columnList().size()*rowList().size()*nScanPoints()-
		      i_row*columnList().size()*nScanPoints())/(nScanPoints());
  assert(i_row<rowList().size());
  assert(i_col<columnList().size());
  rows= &(rowList()[i_row]);
  cols= &(columnList()[i_col]);
}

///////////////////////////////////////////////////////////////////////////////////
unsigned int PixelCalibConfigurationExtended::getNumberOfPixelInjected(){
  //   if(PixelCalibConfiguration::rowList().size()!=0 && PixelCalibConfiguration::columnList.size()!=0){
     return PixelCalibConfiguration::rowList()[0].size()*PixelCalibConfiguration::columnList()[0].size(); 
     //   }
     //return 0;
}
