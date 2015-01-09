//
// Test program that will read and write out 
// DAC settings, trim bitsa and maskbits for a
// readout link object.
//
//
//
//
//

#include "PixelConfigDataFormats/include/PixelROCName.h"
#include "PixelConfigDataFormats/include/PixelHdwAddress.h"
#include "PixelConfigDataFormats/include/PixelNameTranslation.h"
#include "PixelConfigDataFormats/include/PixelMaskAllPixels.h"
#include "PixelConfigDataFormats/include/PixelMaskOverride.h"
#include "PixelConfigDataFormats/include/PixelTrimAllPixels.h"
#include "PixelConfigDataFormats/include/PixelTrimOverride.h"
#include "PixelConfigDataFormats/include/PixelMaskBase.h"
#include "PixelConfigDataFormats/include/PixelMaskOverrideBase.h"
#include "PixelConfigDataFormats/include/PixelTrimBase.h"
#include "PixelConfigDataFormats/include/PixelTrimOverrideBase.h"
#include "PixelConfigDataFormats/include/PixelDACSettings.h"


//#ifdef PIXELFEC
#include "PixelFECInterface/include/PixelFECInterface.h"
//#endif
#include "PixelFECInterface/include/PixelFECConfigInterface.h"

#include <string>
#include <iostream>

#include "VMEAddressTable.hh"

#define NMODULES 1

int main(){
 
  //First lets do some tests of the PixelROCName class

  std::string n1="FPix_BpI_D2_BLD11_PNL1_PLQ3_ROC0";
  PixelROCName r1(n1);
  std::cout << "Original name:"<<n1<<std::endl;
  std::cout << "Parsed name  :"<<r1.rocname()<<std::endl<<std::endl;
  assert(n1==r1.rocname());


  std::string n2="FPix_BpI_D2_BLD11_PNL2_PLQ3_ROC0";
  PixelROCName r2(n2);
  std::cout << "Original name:"<<n2<<std::endl;
  std::cout << "Parsed name  :"<<r2.rocname()<<std::endl<<std::endl;
  assert(n2==r2.rocname());

  std::string n3="FPix_BpI_D2_BLD11_PNL2_PLQ4_ROC0";
  PixelROCName r3(n3);
  std::cout << "Original name:"<<n3<<std::endl;
  std::cout << "Parsed name  :"<<r3.rocname()<<std::endl<<std::endl;
  assert(n3==r3.rocname());



  std::string n4="FPix_BmO_D2_BLD11_PNL2_PLQ4_ROC9";
  PixelROCName r4(n4);
  std::cout << "Original name:"<<n4<<std::endl;
  std::cout << "Parsed name  :"<<r4.rocname()<<std::endl<<std::endl;
  assert(n4==r4.rocname());


  std::string n5="BPix_BmO_SEC2_LYR2_LDR4F_MOD2_ROC9";
  PixelROCName r5(n5);
  std::cout << "Original name:"<<n5<<std::endl;
  std::cout << "Parsed name  :"<<r5.rocname()<<std::endl<<std::endl;
  assert(n5==r5.rocname());


  std::string n6="BPix_BmO_SEC8_LYR2_LDR7H_MOD4_ROC9";
  PixelROCName r6(n6);
  std::cout << "Original name:"<<n6<<std::endl;
  std::cout << "Parsed name  :"<<r6.rocname()<<std::endl<<std::endl;
  assert(n6==r6.rocname());

  //return 0;

  //In the real system we will get these back from the database

  std::string path(getenv("XDAQ_ROOT"));

  path+="/pixel/PixelFECSupervisor/test/";

  PixelNameTranslation table(path+"translation.dat");

  std::cout << "Name translation table:"<<std::endl;
  std::cout << table << std::endl;

  std::string aROCName="FPix_BpI_D1_BLD1_PNL1_PLQ1_ROC3";

  const PixelHdwAddress* aROCAddress=table.getHdwAddress(aROCName);

  if (aROCAddress==0){
      std::cout << "Could not find the ROC:"<<aROCName<<std::endl;
  }
  else{
      std::cout << "The ROC "<<aROCName<<" has the addresses:"<<std::endl;
      std::cout << "mfec       :"<<aROCAddress->mfec()<<std::endl;
      std::cout << "mfecchannel:"<<aROCAddress->mfecchannel()<<std::endl;
      std::cout << "hubaddress :"<<aROCAddress->hubaddress()<<std::endl;
      std::cout << "portaddress:"<<aROCAddress->portaddress()<<std::endl;
      std::cout << "rocid      :"<<aROCAddress->rocid()<<std::endl;
  }

  //create PixelMask object that contains information for all pixels
  PixelMaskBase* theReferenceMask[NMODULES];

  //create PixelTrim object that contains information for all pixels
  PixelTrimBase* theReferenceTrim[NMODULES];

  //create PixelDAC setting object that contains information for all pixels
  PixelDACSettings* theDACSettings[NMODULES];


  //create PixelMask object that contains information for all pixels
  PixelMaskBase* theReferenceMaskTest[NMODULES];

  //create PixelTrim object that contains information for all pixels
  PixelTrimBase* theReferenceTrimTest[NMODULES];

  //create PixelDAC setting object that contains information for all pixels
  PixelDACSettings* theDACSettingsTest[NMODULES];


  //Loop over modules and read (same) configuration

  for(unsigned int imodule=0;imodule<NMODULES;imodule++){

      theReferenceMask[imodule]=new PixelMaskAllPixels(path+"ROC_Masks_module.dat");
      theReferenceTrim[imodule]=new PixelTrimAllPixels(path+"ROC_Trims_module.dat");
      theDACSettings[imodule]=new PixelDACSettings(path+"ROC_DAC_module.dat");


      theReferenceMask[imodule]->writeBinary(path+"ROC_Masks_module.bin");
      theReferenceTrim[imodule]->writeBinary(path+"ROC_Trims_module.bin");
      theDACSettings[imodule]->writeBinary(path+"ROC_DAC_module.bin");

      theReferenceMask[imodule]->writeASCII(path+"ROC_Masks_module_temp.dat");
      theReferenceTrim[imodule]->writeASCII(path+"ROC_Trims_module_temp.dat");
      theDACSettings[imodule]->writeASCII(path+"ROC_DAC_module_temp.dat");


      std::cout << "Will read binary mask" <<std::endl;

      theReferenceMaskTest[imodule]=new PixelMaskAllPixels(path+"ROC_Masks_module.bin");

      std::cout << "Will read binary trim" <<std::endl;

      theReferenceTrimTest[imodule]=new PixelTrimAllPixels(path+"ROC_Trims_module.bin");

      std::cout << "Will read binary dac" <<std::endl;

      theDACSettingsTest[imodule]=new PixelDACSettings(path+"ROC_DAC_module.bin");


      std::cout << "Will write ascii" <<std::endl;


      theReferenceMaskTest[imodule]->writeASCII(path+"ROC_Masks_module_test.dat");
      theReferenceTrimTest[imodule]->writeASCII(path+"ROC_Trims_module_test.dat");
      theDACSettingsTest[imodule]->writeASCII(path+"ROC_DAC_module_test.dat");


      //std::cout << *theReferenceMask <<std::endl;
      //std::cout << *theReferenceTrim <<std::endl;
      //std::cout << *theDACSettings <<std::endl;

  }

  //create PixelMaskOverridek object
  //PixelMaskOverrideBase* theMaskOverride=new PixelMaskOverride("MaskOverride.dat")


  //Now create an instance of a PixelFECInterface so that we
  //can ask it to configure itself with the data above.

  /*

  HAL::VMEDevice *VMEPtr(0);

  PixelFECInterface aFECInterface(VMEPtr);

  */

//#ifdef PIXELFEC


  //Need to set these to something realistic
  unsigned long fedBase=0x1;
  unsigned long BHandle=0x1;



  PixelFECInterface aFECInterface(fedBase,BHandle);


  PixelFECConfigInterface *theConfigInterface=&aFECInterface;
  

  std::cout << "Starting to configure..."<<std::endl;

  for(unsigned int imodule=0;imodule<NMODULES;imodule++){
      theReferenceTrim[imodule]->generateConfiguration(theConfigInterface,
						       &table,
						       *(theReferenceMask[imodule]));
      theDACSettings[imodule]->generateConfiguration(theConfigInterface,&table);
  }

  std::cout << "Done configuring."<<std::endl;

//#endif

  return 0;

}
