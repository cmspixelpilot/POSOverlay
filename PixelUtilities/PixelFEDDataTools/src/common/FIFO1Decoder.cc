#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include <iostream>

using namespace pos;

namespace {
  const double cutFraction = 0.7;
  const bool printLocal = false;
}

// The original methods with common TBM&ROC UBs 
FIFO1Decoder::FIFO1Decoder(uint32_t *buffer, unsigned int ROCStringVectorSize, Moments &UB, Moments &B, AddressLevels &TBM_AddressLevels)
{
	valid_=false;
	unsigned int slot=transparentDataStart(buffer);
	unsigned int slotstart=slot;

	float recommended_UB=UB.mean()+3*UB.stddev();
	float recommended_BH=B.mean()+3*B.stddev();
	float recommended_BL=B.mean()-3*B.stddev();

	double maxUBLimit=UB.mean()*cutFraction+B.mean()*(1.-cutFraction);

	if (recommended_UB>maxUBLimit) {
	  if(printLocal) std::cout << "[FIFO1Decoder::FIFO1Decoder]"
		    << " UB.mean()="<<UB.mean()
		    << " UB.stddev()="<<UB.stddev()
		    << " B.mean()="<<B.mean()
		    << " B.stddev()="<<B.stddev()
		    << std::endl;
	  recommended_UB=maxUBLimit;
	}

	if (slot==0)	// there was no abrupt change to mark the beginning of data
	{
         	std::cout <<"FIFO1Decoder: did not find TBM header in transparent data" << std::endl;
		for (unsigned int i=0;i<=pos::fifo1TranspDataLength;++i)
		{
			std::cout<<"i="<<i<<" buffer[i]="<<((0xffc00000 & buffer[i])>>22)<<" ";
		}
		std::cout<<std::endl;
	}
	else 
	{
		for (unsigned int i=1;i<slot;++i)
		{	// Collect all Blacks before start of signal
			B.push_back((0xffc00000 & buffer[i])>>22);
		}

		UB.push_back((0xffc00000 & buffer[slot++])>>22);		// TBM Header
		UB.push_back((0xffc00000 & buffer[slot++])>>22);
		UB.push_back((0xffc00000 & buffer[slot++])>>22);
		B.push_back((0xffc00000 & buffer[slot++])>>22);

		TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);		// TBM Event Number
		TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);
		TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);
		TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);

		for (unsigned int ROC=0;ROC<ROCStringVectorSize;++ROC)
		{								// Collecting ROC Headers
		  UB.push_back((0xffc00000 & buffer[slot++])>>22);
		  B.push_back((0xffc00000 & buffer[slot++])>>22);
		  slot+=1;
		  if (slot>(pos::fifo1TranspDataLength-4)) {
		    static int counter=0;
		    counter++;
		    int base=1;
		    while (counter/base>10) {base*=10;}
		    if (counter%base==0){
		      std::cout<<"This error has happened "<<counter<<" times"
			       <<std::endl;
		      std::cout<<"Accessing beyond "
			       <<pos::fifo1TranspDataLength-4
			       <<" in transparent buffer!"<<std::endl;
		      std::cout << "UB.mean()+3*UB.stddev()+50 :"<<UB.mean()+3*UB.stddev()+50<<std::endl;
		      std::cout << "UB.mean()                  :"<<UB.mean()<<std::endl;
		      std::cout << "3*UB.stddev()              :"<<3*UB.stddev()<<std::endl;
		      for(unsigned int i=0;i<=pos::fifo1TranspDataLength;i++){
			if (i%10==0) {
			  if (i!=0) {
			    std::cout <<std::endl;
			  }
			  std::cout <<"buffer["<<i<<"]=";
			}
			std::cout <<( (0xffc00000 & buffer[i])>>22 )<<" ";
		      }
		      std::cout << std::endl;
		    }

		    return;
		  }
		}

		float recommended_UB=UB.mean()+3*UB.stddev()+50;
		float recommended_BH=B.mean()+3*B.stddev()+50;
		float recommended_BL=B.mean()-3*B.stddev()-50;


		double maxUBLimit=UB.mean()*cutFraction+B.mean()*(1.-cutFraction);

		if (recommended_UB>maxUBLimit) {
		  if(printLocal) std::cout << "[FIFO1Decoder::FIFO1Decoder 2 ]"
			    << " UB.mean()="<<UB.mean()
			    << " UB.stddev()="<<UB.stddev()
			    << " B.mean()="<<B.mean()
			    << " B.stddev()="<<B.stddev()
			    << std::endl;
		  recommended_UB=maxUBLimit;
		}


		if ( (((0xffc00000 & buffer[slot])>>22) < recommended_UB)
		&&   (((0xffc00000 & buffer[slot+1])>>22) < recommended_UB)
		&&   ((recommended_BL < ((0xffc00000 & buffer[slot+2])>>22)) && (((0xffc00000 & buffer[slot+2])>>22) < recommended_BH))
		&&   ((recommended_BL < ((0xffc00000 & buffer[slot+3])>>22)) && (((0xffc00000 & buffer[slot+3])>>22) < recommended_BH)))	valid_=true;
	}

	if (!valid_ && 0!=slotstart)
	{
		for (unsigned int i=0;i<=pos::fifo1TranspDataLength;++i)
		{
			std::cout<<"i="<<i<<" buffer[i]="<<((0xffc00000 & buffer[i])>>22)<<std::endl;
		}
		std::cout<<"Data started at "<<slotstart<<std::endl;
		std::cout<<"TBM Trailer... "<<((0xffc00000 & buffer[slot])>>22)<<" "<<((0xffc00000 & buffer[slot+1])>>22)<<" "<<((0xffc00000 & buffer[slot+2])>>22)<<" "<<((0xffc00000 & buffer[slot+3])>>22)<<std::endl;
		std::cout<<"recommended UB = "<<recommended_UB<<std::endl;
		std::cout<<"recommended BH = "<<recommended_BH<<std::endl;
		std::cout<<"recommended BL = "<<recommended_BL<<std::endl;
	}

}

FIFO1Decoder::FIFO1Decoder(uint32_t *buffer, std::vector<PixelROCName> ROCStringVector, Moments &UB, Moments &B, std::map<PixelROCName, AddressLevels> &ROC_AddressLevels, AddressLevels &TBM_AddressLevels)
{

	valid_=false;
	unsigned int slot=transparentDataStart(buffer);
	unsigned int startslot=slot;

        if(slot==0) {
          std::cout<<" TBM header not found, skip data"<<std::endl;
	  if(0) { // use only for debugging
	    for(unsigned int i=0;i<=pos::fifo1TranspDataLength;i++){
	      std::cout <<"buffer["<<i<<"]="<<( (0xffc00000 & buffer[i])>>22 )<<" ";
	    }
	    std::cout<<std::endl;
	  }
          return;
        } //d.k.

        for (unsigned int ii=1;ii<slot;ii++){
          B.push_back((0xffc00000 & buffer[ii])>>22);
        }

	UB.push_back((0xffc00000 & buffer[slot++])>>22);		// TBM Header
	UB.push_back((0xffc00000 & buffer[slot++])>>22);
	UB.push_back((0xffc00000 & buffer[slot++])>>22);
	B.push_back((0xffc00000 & buffer[slot++])>>22);

	TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);		// TBM Event Number
	TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);
	TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);
	TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);

	bool dump=false;

	for (unsigned int ROC=0;ROC<ROCStringVector.size();++ROC)				// HARDWIRE ALERT!
	{
		std::map<PixelROCName, AddressLevels>::const_iterator it=ROC_AddressLevels.find(ROCStringVector[ROC]);
		bool validROC=false;
		if (it!=ROC_AddressLevels.end()) validROC=true;

		/*
		static int count1=0;

		if (count1<100){
		  if ( validROC ) std::cout << "ROC="<<ROC<<" "<<ROCStringVector[ROC]<<" is valid"<<std::endl;
		  if ( !validROC ) std::cout << "ROC="<<ROC<<" "<<ROCStringVector[ROC]<<" is not valid"<<std::endl;
		  count1++;
		}
		*/

		UB.push_back((0xffc00000 & buffer[slot++])>>22);	// ROC Header UB
		B.push_back((0xffc00000 & buffer[slot++])>>22);		// ROC Header B
		slot+=1;						// Last DAC

		while (((0xffc00000 & buffer[slot])>>22) > UB.mean()+3*UB.stddev()+50) // Then it's a hit!
		{
			if (validROC)
			{
				ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint((0xffc00000 & buffer[slot++])>>22);
				ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint((0xffc00000 & buffer[slot++])>>22);
				ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint((0xffc00000 & buffer[slot++])>>22);
				ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint((0xffc00000 & buffer[slot++])>>22);
				ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint((0xffc00000 & buffer[slot++])>>22);
				++slot;		// Charge deposit
			}
			else
			{
				slot+=6;
				static int counter=0;
				if (counter<100){
					std::cout<<"ROC "<<ROCStringVector.at(ROC)<<" hit when not meant to be hit!"<<std::endl;
					counter++;
					dump=true;
				}
			}
		}

		if (slot>(pos::fifo1TranspDataLength-1)) {
		  static int counter=0;
		  counter++;
		  int base=1;
		  while (counter/base>10) {base*=10;}
		  if (counter%base==0){
		    std::cout<<"This error has happened "<<counter<<" times"
			     <<std::endl;
		    std::cout<<"Accessing beyond "
			   <<pos::fifo1TranspDataLength-1
			   <<" in transparent buffer!"<<std::endl;
		    std::cout << "UB.mean()+3*UB.stddev()+50 :"<<UB.mean()+3*UB.stddev()+50<<std::endl;
		    std::cout << "UB.mean()                  :"<<UB.mean()<<std::endl;
		    std::cout << "3*UB.stddev()              :"<<3*UB.stddev()<<std::endl;
		    std::cout << "I think data started in clock #"<<startslot<<" below ... "<<std::endl;
		    for(unsigned int i=0;i<=pos::fifo1TranspDataLength;i++){
		      if (i%10==0) {
			if (i!=0) {
			  std::cout <<std::endl;
			}
			std::cout <<"buffer["<<i<<"]=";
		      }
		      std::cout <<( (0xffc00000 & buffer[i])>>22 )<<" ";
		    }
		    std::cout << std::endl;
		  }

		  return;
		}
	}

	float recommended_UB=UB.mean()+3*UB.stddev()+50;
	float recommended_BH=B.mean()+3*B.stddev()+50;
	float recommended_BL=B.mean()-3*B.stddev()-50;


	double maxUBLimit=UB.mean()*cutFraction+B.mean()*(1.-cutFraction);
	
	if (recommended_UB>maxUBLimit) {
	  if(printLocal) std::cout << "[FIFO1Decoder::FIFO1Decoder 3 ]"
		    << " UB.mean()="<<UB.mean()
		    << " UB.stddev()="<<UB.stddev()
		    << " B.mean()="<<B.mean()
		    << " B.stddev()="<<B.stddev()
		    << std::endl;
	  recommended_UB=maxUBLimit;
	}



	//std::cout<<"TBM Trailer... "<<((0xffc00000 & buffer[slot])>>22)<<" "<<((0xffc00000 & buffer[slot+1])>>22)<<" "<<((0xffc00000 & buffer[slot+2])>>22)<<" "<<((0xffc00000 & buffer[slot+3])>>22)<<std::endl;
	//std::cout<<"recommended UB = "<<recommended_UB<<std::endl;
	//std::cout<<"recommended BH = "<<recommended_BH<<std::endl;
	//std::cout<<"recommended BL = "<<recommended_BL<<std::endl;

	if ( (((0xffc00000 & buffer[slot])>>22) < recommended_UB)
	&&   (((0xffc00000 & buffer[slot+1])>>22) < recommended_UB)
	&&   ((recommended_BL < ((0xffc00000 & buffer[slot+2])>>22)) && (((0xffc00000 & buffer[slot+2])>>22) < recommended_BH))
	&&   ((recommended_BL < ((0xffc00000 & buffer[slot+3])>>22)) && (((0xffc00000 & buffer[slot+3])>>22) < recommended_BH)))	valid_=true;

	if (!valid_) {

	            std::cout <<" Invalid TBM trailer"<<std::endl;
		    std::cout << "recommended_BH              :"<<recommended_BH<<std::endl;	           
		    std::cout << "recommended_BL              :"<<recommended_BL<<std::endl;	           
		    std::cout << "B.mean()                   :"<<B.mean()<<std::endl;
		    std::cout << "3*B.stddev()              :"<<3*B.stddev()<<std::endl;
		    std::cout << "recommended_UB              :"<<recommended_UB<<std::endl;
		    std::cout << "UB.mean()                  :"<<UB.mean()<<std::endl;
		    std::cout << "3*UB.stddev()              :"<<3*UB.stddev()<<std::endl;
		    std::cout << "I think the following are TBM trailer... "<<std::endl;
		    std::cout << "Clock "<<slot<<" = "<<((0xffc00000 & buffer[slot])>>22)<<std::endl;
		    std::cout << "Clock "<<slot+1<<" = "<<((0xffc00000 & buffer[slot+1])>>22)<<std::endl;
		    std::cout << "Clock "<<slot+2<<" = "<<((0xffc00000 & buffer[slot+2])>>22)<<std::endl;
		    std::cout << "Clock "<<slot+3<<" = "<<((0xffc00000 & buffer[slot+3])>>22)<<std::endl;

		    std::cout << "I think data started in clock #"<<startslot<<" below ... "<<std::endl;
		    for(unsigned int i=0;i<=pos::fifo1TranspDataLength;i++){
		      std::cout <<"buffer["<<i<<"]="<<( (0xffc00000 & buffer[i])>>22 )<<std::endl;
		    }

	}


}

//------------------------------------------------------------------------------------------------
// Find the 1st TBM UB
unsigned int FIFO1Decoder::transparentDataStart (uint32_t *buffer)
{
  float B_mean=0, B_stddev=0, B_sum=0, B_squares=0;
  unsigned int i=1;

  do
    {
      B_sum+=((0xffc00000 & buffer[i])>>22);
      B_squares+=pow(float((0xffc00000 & buffer[i])>>22),2);
      B_mean=B_sum/i;
      B_stddev=(B_squares/i-B_mean*B_mean);
      if(0>B_stddev)
        {
          B_stddev=0;
        }
      else
        {
          B_stddev=sqrt(B_stddev);
        }
      //                cout<<"Mean of black so far at position i="<<dec<<i<<" is "<<B_mean<<" with std dev = "<<B_stddev<<endl;
      i+=1;
    } while ((
              !(
                ((0xffc00000 & buffer[i])>>22)<(B_mean-3*B_stddev-50) &&
                ((0xffc00000 & buffer[i+1])>>22)<(B_mean-3*B_stddev-50) &&
                ((0xffc00000 & buffer[i+2])>>22)<(B_mean-3*B_stddev-50)
                ) || i<8)
             && i<512);

  if ( (i>=512) && printLocal ) {
    std::cout<<"[FIFO1Decoder::transparentDataStart] - Cannot find beginning of data!"<<std::endl;
    for (unsigned int j=0; j<256; ++j) {
      std::cout<<"[FIFO1Decoder::transparentDataStart] - buffer["<<j<<"] = "<<((0xffc00000 & buffer[j])>>22)<<std::endl;
    }
  }

  return i;
}
//-----------------------------------------------------------------------------------------------------------
// Add the individual TBM&ROC UB
// FIFO1 packets without any hits (first 2 events)
FIFO1Decoder::FIFO1Decoder(uint32_t *buffer, unsigned int ROCStringVectorSize, 
			   Moments &UB, Moments &B, AddressLevels &TBM_AddressLevels, 
			   Moments &UB_TBM, Moments &UB_ROC) {

  valid_=false;
  unsigned int slot=transparentDataStart(buffer);
  unsigned int slotstart=slot;
  
  float recommended_UB=UB.mean()+3*UB.stddev();
  float recommended_BH=B.mean()+3*B.stddev();
  float recommended_BL=B.mean()-3*B.stddev();
  double maxUBLimit=UB.mean()*cutFraction+B.mean()*(1.-cutFraction);
  
  if (recommended_UB>maxUBLimit) {
    if(printLocal) std::cout << "[FIFO1Decoder::FIFO1Decoder]"
	      << " UB.mean()="<<UB.mean()
	      << " UB.stddev()="<<UB.stddev()
	      << " B.mean()="<<B.mean()
	      << " B.stddev()="<<B.stddev()
	      << std::endl;
    recommended_UB=maxUBLimit;
  }
  
  if (slot==0) {	// there was no abrupt change to mark the beginning of data
    std::cout <<"FIFO1Decoder: did not find TBM header in transparent data" << std::endl;
    for (unsigned int i=0;i<=pos::fifo1TranspDataLength;++i)
      {
	std::cout<<"i="<<i<<" buffer[i]="<<((0xffc00000 & buffer[i])>>22)<<" ";
      }
    std::cout<<std::endl;

  } else {
    for (unsigned int i=1;i<slot;++i)
      {	// Collect all Blacks before start of signal
	B.push_back((0xffc00000 & buffer[i])>>22);
      }
    
      
    double tmp = (0xffc00000 & buffer[slot++])>>22;		// TBM Header
    UB.push_back(tmp);	
    UB_TBM.push_back(tmp);	   
    tmp = (0xffc00000 & buffer[slot++])>>22;	     
    UB.push_back(tmp);
    UB_TBM.push_back(tmp);	   
    tmp = (0xffc00000 & buffer[slot++])>>22;	     
    UB.push_back(tmp);
    UB_TBM.push_back(tmp);	   
    B.push_back((0xffc00000 & buffer[slot++])>>22);
    
    TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);		// TBM Event Number
    TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);
    TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);
    TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot++])>>22);
    
    for (unsigned int ROC=0;ROC<ROCStringVectorSize;++ROC)
      {								// Collecting ROC Headers
	tmp = (0xffc00000 & buffer[slot++])>>22;     // ROC UB
	UB.push_back(tmp);
	UB_ROC.push_back(tmp);	   
	B.push_back((0xffc00000 & buffer[slot++])>>22);
	slot+=1;
	if (slot>(pos::fifo1TranspDataLength-4)) {
	  static int counter=0;
	  counter++;
	  int base=1;
	  while (counter/base>10) {base*=10;}
	  if (counter%base==0){
	    std::cout<<"This error has happened "<<counter<<" times"
		     <<std::endl;
	    std::cout<<"Accessing beyond "
		     <<pos::fifo1TranspDataLength-4
		     <<" in transparent buffer!"<<std::endl;
	    std::cout << "UB.mean()+3*UB.stddev()+50 :"<<UB.mean()+3*UB.stddev()+50<<std::endl;
	    std::cout << "UB.mean()                  :"<<UB.mean()<<std::endl;
	    std::cout << "3*UB.stddev()              :"<<3*UB.stddev()<<std::endl;
	    for(unsigned int i=0;i<=pos::fifo1TranspDataLength;i++){
	      if (i%10==0) {
		if (i!=0) {
		  std::cout <<std::endl;
		}
		std::cout <<"buffer["<<i<<"]=";
	      }
	      std::cout <<( (0xffc00000 & buffer[i])>>22 )<<" ";
	    }
	    std::cout << std::endl;
	  }
	  
	  return;  // error exit
	}
      }
    
    float recommended_UB=UB.mean()+3*UB.stddev()+50;
    float recommended_BH=B.mean()+3*B.stddev()+50;
    float recommended_BL=B.mean()-3*B.stddev()-50; 
    
    double maxUBLimit=UB.mean()*cutFraction+B.mean()*(1.-cutFraction);      
    if (recommended_UB>maxUBLimit) {
      if(printLocal) std::cout << "[FIFO1Decoder::FIFO1Decoder 4 ]"
		<< " UB.mean()="<<UB.mean()
		<< " UB.stddev()="<<UB.stddev()
		<< " B.mean()="<<B.mean()
		<< " B.stddev()="<<B.stddev()
		<< std::endl;
      recommended_UB=maxUBLimit;
    }
    
    if ( (((0xffc00000 & buffer[slot])>>22) < recommended_UB) // valid trailer?
	 &&   (((0xffc00000 & buffer[slot+1])>>22) < recommended_UB)
	 &&   ((recommended_BL < ((0xffc00000 & buffer[slot+2])>>22)) && 
	       (((0xffc00000 & buffer[slot+2])>>22) < recommended_BH))
	 &&   ((recommended_BL < ((0xffc00000 & buffer[slot+3])>>22)) && 
	       (((0xffc00000 & buffer[slot+3])>>22) < recommended_BH)))	{
      valid_=true;
    } else {
      std::cout<<" [FIFO1Decoder] Invalid TBM trailer"<<std::endl;
    } // end if trailer 
    
  } // if valid header 
  
  if (!valid_ && 0!=slotstart)
    {
      for (unsigned int i=0;i<=pos::fifo1TranspDataLength;++i)
	{
	  std::cout<<"i="<<i<<" buffer[i]="<<((0xffc00000 & buffer[i])>>22)<<std::endl;
	}
      std::cout<<"Data started at "<<slotstart<<std::endl;
      std::cout<<"TBM Trailer... "<<((0xffc00000 & buffer[slot])>>22)<<" "<<((0xffc00000 & buffer[slot+1])>>22)<<" "<<((0xffc00000 & buffer[slot+2])>>22)<<" "<<((0xffc00000 & buffer[slot+3])>>22)<<std::endl;
      std::cout<<"recommended UB = "<<recommended_UB<<std::endl;
      std::cout<<"recommended BH = "<<recommended_BH<<std::endl;
      std::cout<<"recommended BL = "<<recommended_BL<<std::endl;
    }

}
//---------------------------------------------------------------------------------------------------------
// Add the individual TBM&ROC UB
// FIFO1 packets with hits (after 2 events)
FIFO1Decoder::FIFO1Decoder(uint32_t *buffer, std::vector<PixelROCName> ROCStringVector, 
			   Moments &UB, Moments &B, std::map<PixelROCName, AddressLevels> &ROC_AddressLevels, 
			   AddressLevels &TBM_AddressLevels,Moments &UB_TBM, Moments &UB_ROC) {

  valid_=false;
  unsigned int slot=transparentDataStart(buffer);
  unsigned int startslot=slot;
  
  if(slot==0) {
    std::cout<<" TBM header not found, skip data"<<std::endl;
    if(0) { // use only for debugging
      for(unsigned int i=0;i<=pos::fifo1TranspDataLength;i++){
	std::cout <<"buffer["<<i<<"]="<<( (0xffc00000 & buffer[i])>>22 )<<" ";
      }
      std::cout<<std::endl;
    }
    return;
  } //d.k.
  
  for (unsigned int ii=1;ii<slot;ii++){
    B.push_back((0xffc00000 & buffer[ii])>>22);
  }
  
  // Look at the TBM header
  double tmp = (0xffc00000 & buffer[slot++])>>22;		// TBM Header
  UB.push_back(tmp);	
  UB_TBM.push_back(tmp);	   
  tmp = (0xffc00000 & buffer[slot++])>>22;	     
  UB.push_back(tmp);
  UB_TBM.push_back(tmp);	   
  tmp = (0xffc00000 & buffer[slot++])>>22;	     
  UB.push_back(tmp);
  UB_TBM.push_back(tmp);	   
  B.push_back((0xffc00000 & buffer[slot++])>>22);
  
  float recommended_UB=UB.mean()+3*UB.stddev()+50;
  float recommended_BH=B.mean()+3*B.stddev()+50;
  float recommended_BL=B.mean()-3*B.stddev()-50;    
  double maxUBLimit=UB.mean()*cutFraction+B.mean()*(1.-cutFraction);
  if (recommended_UB>maxUBLimit) {
    if(printLocal) std::cout << "[FIFO1Decoder::FIFO1Decoder 5]"
	      << " UB.mean()="<<UB.mean()
	      << " UB.stddev()="<<UB.stddev()
	      << " B.mean()="<<B.mean()
	      << " B.stddev()="<<B.stddev()
	      << std::endl;
    recommended_UB=maxUBLimit;
  }

  // Look at the TBM event number
  unsigned int tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
  if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, TBM address level is below the UB cut "<<
    tmp0<<" "<<recommended_UB<<std::endl;
  TBM_AddressLevels.addPoint(tmp0);		// TBM Event Number
  tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
  if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, TBM address level is below the UB cut "<<
    tmp0<<" "<<recommended_UB<<std::endl;
  TBM_AddressLevels.addPoint(tmp0);		// TBM Event Number
  tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
  if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, TBM address level is below the UB cut "<<
    tmp0<<" "<<recommended_UB<<std::endl;
  TBM_AddressLevels.addPoint(tmp0);		// TBM Event Number
  tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
  if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, TBM address level is below the UB cut "<<
    tmp0<<" "<<recommended_UB<<std::endl;
  TBM_AddressLevels.addPoint(tmp0);		// TBM Event Number
  
  bool dump=false;  
  // Loop over ROCs
  for (unsigned int ROC=0;ROC<ROCStringVector.size();++ROC) {				// HARDWIRE ALERT!

    std::map<PixelROCName, AddressLevels>::const_iterator it=ROC_AddressLevels.find(ROCStringVector[ROC]);
    bool validROC=false;
    if (it!=ROC_AddressLevels.end()) validROC=true;
    
    /*
      static int count1=0;
      if (count1<100){
      if ( validROC ) std::cout << "ROC="<<ROC<<" "<<ROCStringVector[ROC]<<" is valid"<<std::endl;
      if ( !validROC ) std::cout << "ROC="<<ROC<<" "<<ROCStringVector[ROC]<<" is not valid"<<std::endl;
      count1++;
      }
    */
      
    tmp = (0xffc00000 & buffer[slot++])>>22;	 // ROC Header UB
    UB.push_back(tmp);
    UB_ROC.push_back(tmp);	   
    B.push_back((0xffc00000 & buffer[slot++])>>22);		// ROC Header B
    // Last DAC
    tmp = (0xffc00000 & buffer[slot++])>>22;	     
    if(tmp<recommended_UB) std::cout<<" FIFO1Decoder: Error, last DAC is below the UB cut "<<
      tmp<<" "<<recommended_UB<<std::endl;
    
    // Loop over hits
    while (((0xffc00000 & buffer[slot])>>22) > recommended_UB) { // Then it's a hit!
      if (validROC) {

	// Look at ROC address levels
	tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
	if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, ROC address level is below the UB cut "<<
	  tmp0<<" "<<recommended_UB<<std::endl;
	ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint(tmp0);
	tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
	if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, ROC address level is below the UB cut "<<
	  tmp0<<" "<<recommended_UB<<std::endl;
	ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint(tmp0);
	tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
	if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, ROC address level is below the UB cut "<<
	  tmp0<<" "<<recommended_UB<<std::endl;
	ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint(tmp0);
	tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
	if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, ROC address level is below the UB cut "<<
	  tmp0<<" "<<recommended_UB<<std::endl;
	ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint(tmp0);
	tmp0 = (0xffc00000 & buffer[slot++])>>22;	     
	if(tmp0<recommended_UB) std::cout<<" FIFO1Decoder: Error, ROC address level is below the UB cut "<<
	  tmp0<<" "<<recommended_UB<<std::endl;
	ROC_AddressLevels[ROCStringVector.at(ROC)].addPoint(tmp0);

	// Charge deposit
	tmp = (0xffc00000 & buffer[slot++])>>22;	     
	if(tmp<recommended_UB) std::cout<<" FIFO1Decoder: Error, Charge is below the UB cut "<<
	  tmp<<" "<<recommended_UB<<std::endl;

      }  else { // not valid ROC
	slot+=6;
	static int counter=0;
	if (counter<100){
	  std::cout<<"ROC "<<ROCStringVector.at(ROC)<<" hit when not meant to be hit!"<<std::endl;
	  counter++;
	  dump=true;
	}
      } // if valid ROC
    } // loop over hits
    
    if (slot>(pos::fifo1TranspDataLength-1)) { // is the buffer still valid
      static int counter=0;
      counter++;
      int base=1;
      while (counter/base>10) {base*=10;}
      if (counter%base==0){
	std::cout<<"This error has happened "<<counter<<" times"
		 <<std::endl;
	std::cout<<"Accessing beyond "
		 <<pos::fifo1TranspDataLength-1
		 <<" in transparent buffer!"<<std::endl;
	std::cout << "UB.mean()+3*UB.stddev()+50 :"<<UB.mean()+3*UB.stddev()+50<<std::endl;
	std::cout << "UB.mean()                  :"<<UB.mean()<<std::endl;
	std::cout << "3*UB.stddev()              :"<<3*UB.stddev()<<std::endl;
	std::cout << "I think data started in clock #"<<startslot<<" below ... "<<std::endl;
	for(unsigned int i=0;i<=pos::fifo1TranspDataLength;i++){
	  if (i%10==0) {
	    if (i!=0) {
	      std::cout <<std::endl;
	    }
	    std::cout <<"buffer["<<i<<"]=";
	  }
	  std::cout <<( (0xffc00000 & buffer[i])>>22 )<<" ";
	}
	std::cout << std::endl;
      }
      
      return;
    } // if valid buffer
  } // 
  
  //std::cout<<"TBM Trailer... "<<((0xffc00000 & buffer[slot])>>22)<<" "<<((0xffc00000 & buffer[slot+1])>>22)<<" "<<((0xffc00000 & buffer[slot+2])>>22)<<" "<<((0xffc00000 & buffer[slot+3])>>22)<<std::endl;
  //std::cout<<"recommended UB = "<<recommended_UB<<std::endl;
  //std::cout<<"recommended BH = "<<recommended_BH<<std::endl;
  //std::cout<<"recommended BL = "<<recommended_BL<<std::endl;
  
  tmp = (0xffc00000 & buffer[slot])>>22;	   // TBM UB trailer
  double tmp1 = (0xffc00000 & buffer[slot+1])>>22; // TBM UB trailer
  double tmp2 = (0xffc00000 & buffer[slot+2])>>22; // TBM B trailer
  double tmp3 = (0xffc00000 & buffer[slot+3])>>22; // TBM B trailer
  if (      (tmp < recommended_UB)  
       &&   (tmp1 < recommended_UB)
       &&   (recommended_BL < tmp2) 
       &&   (tmp2 < recommended_BH)
       &&   (recommended_BL < tmp3) 
       &&   (tmp3 < recommended_BH) ) { // is it a valid TBM trailer
    valid_=true;  
    UB.push_back(tmp);
    UB_TBM.push_back(tmp);	   
    UB.push_back(tmp1);
    UB_TBM.push_back(tmp1);	   
    B.push_back(tmp2);
    B.push_back(tmp3);
    // Do we want to store also the TBM status?
    //TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot+4])>>22); // TBM Status
    //TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot+5])>>22);
    //TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot+6])>>22);
    //TBM_AddressLevels.addPoint((0xffc00000 & buffer[slot+7])>>22);

  } else {
    std::cout<<" [FIFO1Decoder] Invalid TBM trailer"<<std::endl;
  } // end if trailer 
  
  if (!valid_) {
    std::cout <<" Invalid TBM trailer"<<std::endl;
    std::cout << "recommended_BH              :"<<recommended_BH<<std::endl;	           
    std::cout << "recommended_BL              :"<<recommended_BL<<std::endl;	           
    std::cout << "B.mean()                   :"<<B.mean()<<std::endl;
    std::cout << "3*B.stddev()              :"<<3*B.stddev()<<std::endl;
    std::cout << "recommended_UB              :"<<recommended_UB<<std::endl;
    std::cout << "UB.mean()                  :"<<UB.mean()<<std::endl;
    std::cout << "3*UB.stddev()              :"<<3*UB.stddev()<<std::endl;
    std::cout << "I think the following are TBM trailer... "<<std::endl;
    std::cout << "Clock "<<slot<<" = "<<((0xffc00000 & buffer[slot])>>22)<<std::endl;
    std::cout << "Clock "<<slot+1<<" = "<<((0xffc00000 & buffer[slot+1])>>22)<<std::endl;
    std::cout << "Clock "<<slot+2<<" = "<<((0xffc00000 & buffer[slot+2])>>22)<<std::endl;
    std::cout << "Clock "<<slot+3<<" = "<<((0xffc00000 & buffer[slot+3])>>22)<<std::endl;
    
    std::cout << "I think data started in clock #"<<startslot<<" below ... "<<std::endl;
    for(unsigned int i=0;i<=pos::fifo1TranspDataLength;i++){
      std::cout <<"buffer["<<i<<"]="<<( (0xffc00000 & buffer[i])>>22 )<<std::endl;
    }
   
  }
 
  return;
}

