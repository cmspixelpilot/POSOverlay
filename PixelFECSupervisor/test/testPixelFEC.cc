//
// Test program that will read and write out 
// DAC settings, trim bitsa and maskbits for a
// readout link object.
//
// Modified for hardware test at SiDet
// October 7, 2006 (kme)
//
//

#include "PixelConfigDataFormats/include/PixelROCName.h"
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

#include <unistd.h>
#include "PixelFECInterface/include/PixelFECInterface.h"
#include "PixelFECInterface/include/PixelFECConfigInterface.h"

#include <string>
#include <iostream>
#include <unistd.h>

#include "VMEAddressTable.hh"

#define NMODULES 1

int main(){

  //In the real system we will get these back from the database
 
  /*

  std::string path(getenv("XDAQ_ROOT"));

  path+="/pixel/PixelFECSupervisor/test/";

  PixelNameTranslation table(path+"translation.dat");

  std::cout << "Name translation table:"<<std::endl;
  std::cout << table << std::endl;

  std::string aROCName="FPix_BpI_D1_BLD1_PNL1_PLQ2_ROC3";

  const PixelROCName* aROC=table.getROCName(aROCName);

  if (aROC==0){
      std::cout << "Could not find the ROC:"<<aROCName<<std::endl;
  }
  else{
      std::cout << "The ROC "<<aROCName<<" has the addresses:"<<std::endl;
      std::cout << "mfec       :"<<aROC->mfec()<<std::endl;
      std::cout << "mfecchannel:"<<aROC->mfecchannel()<<std::endl;
      std::cout << "hubaddress :"<<aROC->hubaddress()<<std::endl;
      std::cout << "portaddress:"<<aROC->portaddress()<<std::endl;
      std::cout << "rocid      :"<<aROC->rocid()<<std::endl;
  }

  //create PixelMask object that contains information for all pixels
  PixelMaskBase* theReferenceMask[NMODULES];

  //create PixelTrim object that contains information for all pixels
  PixelTrimBase* theReferenceTrim[NMODULES];

  //create PixelDAC setting object that contains information for all pixels
  PixelDACSettings* theDACSettings[NMODULES];

  //Loop over modules and read (same) configuration

  for(unsigned int imodule=0;imodule<NMODULES;imodule++){

      theReferenceMask[imodule]=new PixelMaskAllPixels(path+"ROC_Masks_module.dat");
      theReferenceTrim[imodule]=new PixelTrimAllPixels(path+"ROC_Trims_module.dat");
      theDACSettings[imodule]=new PixelDACSettings(path+"ROC_DAC_module.dat");

      //std::cout << *theReferenceMask <<std::endl;
      //std::cout << *theReferenceTrim <<std::endl;
      //std::cout << *theDACSettings <<std::endl;

  }

  //create PixelMaskOverridek object
  //PixelMaskOverrideBase* theMaskOverride=new PixelMaskOverride("MaskOverride.dat")


  //Now create an instance of a PixelFECInterface so that we
  //can ask it to configure itself with the data above.

  */

  /*

  HAL::VMEDevice *VMEPtr(0);

  PixelFECInterface aFECInterface(VMEPtr);

  */


  //Need to set these to something realistic
  //unsigned long fecBase=0x80000024;
  unsigned long fecBase=0x68000000;
  //unsigned long fecBase=0xffffe000;
  // unsigned long fecBase=0xffff8000;
  
  long aBHandle=0;

#define LINK (0)
 short Link=LINK;

#define VMEDEVICE (0)
// define VMEDEVICE (0x80000000)
//#define VMEDEVICE (0xffff8000)

#define VMEBOARD (cvV2718)
  CVBoardTypes VMEBoard=VMEBOARD;
  short Device=0;

  cout<<"VMEBoard="<<VMEBoard<<" Device="<<Device<<" Link="<<Link<<" aBHandle="<<aBHandle<<" cvSuccess="<<cvSuccess<<endl;
  int aa=CAENVME_Init(VMEBoard, Device, Link, &aBHandle);
  if (aa!=cvSuccess)
   {
      cout<<"aa="<<aa<<endl;
      printf("\n\n Error opening the device\n");
      exit(1);
    }

cout<<"END OF STEP 1!!!!!"<<endl;
  PixelFECInterface aFECInterface(fecBase,aBHandle);


  aFECInterface.setssid(4);

  //aFECInterface.test(9);

//int injectrstroc(const int mfec, const int bitstate);
//  int injecttrigger(const int mfec, const int bitstate);
//  int injectrsttbm(const int mfec, const int bitstate);

//	aFECInterface.injectrstroc(4, 1);
//	aFECInterface.injectrsttbm(4, 1);

  //clrcal(int mfec, int fecchannel, 
  //	     int hubaddress, int portaddress, int rocid);

  //aFECInterface.clrcal(4,1,31,0,1);
//tbmcmd(int mfec, int fecchannel,int tbmchannel, int hubaddress, int portaddress,int offset, int databyte, int direction)

// hardware configuration at SiDet Sept 8-9, 2006  Oct 5, 2006
// has a 2x4 plaquette with rocid=tbm order 0-7; N.B. ROCID#1 is broken
//  int mfec=3;
  int mfec=8;
  int mfecchannel=1;
  int tbmchannel=14; //  0xE is TBM A ; 0xF is TBM B
  int hubaddress=12;
  int portaddress=1;
  int theROC=0;

  aFECInterface.injectrsttbm(mfec,1);


  //aFECInterface.synccontrolregister(mfec);
  aFECInterface.injectrstroc(mfec,1);



  for(int iroc=0;iroc<8;iroc++){

    if (0){
      aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 0xFD, 5);
      sleep(1);
      aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 0xFD, 4);
      sleep(1);
      aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 0xFD, 5);
      sleep(1);
      aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 0xFD, 4);
      sleep(1);
      aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 0xFD, 5);
      sleep(1);
    }
//	cout<<"woah "<<iroc<<endl;
//	getchar();
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 0xFD, 4);
   
  }

  if (0){
    aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 0, 1, 0);
    sleep(1);
    aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 0, 0, 0);
    sleep(1);
    aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 0, 1, 0);
    sleep(1);
    aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 0, 0, 0);
    sleep(1);
  }
  

  // setup tbm
  cout << "Now setting up TBM" << endl;


  aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 0, 1, 0);


  aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 1, 0xc0, 0);

  aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 2, 0x14, 0);

  aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 5, 0xff, 0);

  aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 6, 0x80, 0);

  aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 7, 0xff, 0);



	cout<<"Now ClrCal -ing all ROCs... press a key.";getchar();

	for (int roc=0;roc<8;++roc)
	  aFECInterface.clrcal(mfec, mfecchannel, hubaddress, portaddress, roc);

	 std::cout << "Mask off all pixels... press a key to start"<< std::endl;getchar();
  for(int iroc=0;iroc<8; iroc++) {
  //  {int iroc=1;
    for(int col=0;col<52;col++) {
      for(int row=0;row<80;row++){
        aFECInterface.progpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 0, 0);
        //      usleep(10);
      }
    }
  }




  // TBM settings; empirical scan of bias and gains
  if(0) {
    
    for(int g=0; g < 256; g+=8) {
      aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 6, g, 0);
      cout << "set output driver bias to " << dec << g << " continue?" << endl; 
      sleep(1);
    }
    for(int g=0; g < 256; g+=8) {
      aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 5, g, 0);
      cout << "set input driver bias to " << dec << g << " continue?" << endl; 
      sleep(1);
    }
    for(int g=0; g < 256; g+=8) {
      aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 7, g, 0);
      cout << "set output DAC gain to " << dec << g << " continue?" << endl; 
      sleep(1);
    }
  }


  for(int iroc=0;iroc<8;iroc++){

    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 254, 120);  //WBC
// was 74 to match 80 bx in TTCci between CAL+SYNC and L1A; 120 match to 126

    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 1, 6);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 2, 140);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 3, 128);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 4, 15);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 5, 0);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 6, 0);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 7, 35);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 8, 0);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 9, 35);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 10, 90);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 11, 29);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 12, 70);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 13, 30);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 14, 6);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 15, 30);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 16, 115);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 17, 100);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 18, 115);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 19, 90);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 20, 100);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 21, 160);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 22, 99);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 23, 0);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 24, 0);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 25, 80);
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, iroc, 26, 90);

  }


  std::cout << "Will enable dcol" << std::endl;
  

  for(int iroc=0;iroc<8;iroc++){
    for(int col=0;col<52;col+=2){  
      // suspect an interface bug col is column address not double column!
      aFECInterface.dcolenable(mfec, mfecchannel, hubaddress, portaddress, iroc, col/2, 1);
    }
  }

  { int row=0; int col=26;
    cout << "KME column " << col << " progpix attempt" << endl;
    //    aFECInterface.clrcal(mfec,mfecchannel,hubaddress,portaddress,1);
    aFECInterface.progpix(mfec, mfecchannel, hubaddress, portaddress, 1, col, row, 1, 5);
    aFECInterface.calpix(mfec, mfecchannel, hubaddress, portaddress, 1, col, row, 1);
    getchar();
  }

  std::cout << "Mask off all pixels" << std::endl;
  for(int iroc=0;iroc<8; iroc++) {
  //  {int iroc=1;
    for(int col=0;col<52;col++) {
      for(int row=0;row<80;row++){
	aFECInterface.progpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 0, 0);
	//	usleep(10);
      }
    }
  }
	

  std::cout << "Arm and enable pixels" << std::endl;

  
  //  for(int iroc=1;iroc<2;iroc++){
  {int iroc=theROC;
    aFECInterface.progpix(mfec, mfecchannel, hubaddress, portaddress, iroc, 20, 20, 1, 5);
    aFECInterface.calpix(mfec, mfecchannel, hubaddress, portaddress, iroc, 20, 20, 1);
  }

  //std::cout << "will sleep 10 s and turn off hits"<<std::endl;
  //sleep(10);

  //for(int iroc=0;iroc<8;iroc++){
  //  aFECInterface.clrcal(mfec,mfecchannel,hubaddress,portaddress,iroc);
  //}

  //for (int k=0;k<10;k++){
  //  aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 1, 0xc0, 0);
  //  sleep(2);
  //  aFECInterface.tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, 4, 1, 0x00, 0);
  //  sleep(2);
  //}





  //for(int iroc=0;iroc<10;iroc++){
  //  aFECInterface.progpix(4, 1, 31, 0, iroc, 20, 20, 1, 5);
  //  aFECInterface.calpix(4, 1, 31, 0, iroc, 20, 20, 1);
  //}

  aFECInterface.injectrsttbm(mfec,1);
  aFECInterface.injectrstroc(mfec,1);
  aFECInterface.clockphaseselect(mfec,0);

  aFECInterface.enablecallatency(mfec,0);

  aFECInterface.disableexttrigger(mfec,0);

  aFECInterface.injecttrigger(mfec,0);

  //for(int ii=0;ii<30;ii+=1){
  //  cout << "callatency:"<<ii<<endl;
  //  aFECInterface.callatencycount(mfec,ii);
  //  sleep(1);
  //}

  aFECInterface.callatencycount(mfec,79); 
// meaningless if using TTC to send CAL-SYNC & L1A, but must be non-zero


  //aFECInterface.clrcal(mfec,mfecchannel,hubaddress,portaddress,1);

  // vcal scan shows 0, 1, many hits: apparantly more than one pixel is masked!
  // was due to bad ROC id=1; other chips OK
  for(int vcal=0;vcal<256;vcal++) {
    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, 1, 25, vcal);
    cout<< "vcal=" << dec << vcal <<endl;
    // vary cal delay setting on ROC to see if this helps.
      //    for(int caldel=0; caldel<256; caldel++){
      // cout << "vcal=" << dec << vcal ;
      //      cout << " caldel=" << caldel << endl;
      //          aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, 1, 26, caldel);
      //    }
	  usleep(10000);

  }

  cout << "clear dcol?" << endl;
  getchar();
  for(int col=0;col<52;col+=2){  // suspect an interface bug col is column address not double column!
    aFECInterface.dcolenable(mfec, mfecchannel, hubaddress, portaddress, 1, col/2, 0);
  }
  std::cout << "Mask off all pixels" << std::endl;
  for(int iroc=0;iroc<8; iroc++) {
  //  {int iroc=theROC;
    for(int col=0;col<52;col++) {
      for(int row=0;row<80;row++){
	aFECInterface.progpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 0, 0);
	//	usleep(10);
      }
    }
  }


    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, 1, 26, 80);
    //    aFECInterface.progdac(mfec, mfecchannel, hubaddress, portaddress, 1, 25, 80);

    cout << "begin pixel scan? y=to go: " << endl;
    char r=getchar();  cout << "OK" << endl;
    if(r=='y') {
      //	  const int iroc=theROC;
      for(int iroc=0; iroc<8; iroc++) {
      for(int row=0;row<80; row++) {
    //    {int row=0;
	//	cout << "Continue scan?" << endl; getchar();
	//	for(int col=0;col<26; col++) {
	for(int col=0;col<52; col++) {
	  //	  cout << "roc,col,row=" << iroc << "," << col << "," << row << endl;
	  aFECInterface.clrcal(mfec, mfecchannel, hubaddress, portaddress, iroc);
	  aFECInterface.calpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 1);
	  aFECInterface.progpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 1, 5);
	  usleep(10);
	  aFECInterface.clrcal(mfec, mfecchannel, hubaddress, portaddress, iroc);
	  /* -----------
	  if (col==25) {
	    cout << "That worked right?  pause - resume?" << endl;
	    getchar();
	  }
         ------------- */
	}
      }
    }
    }
    
    cout << "begin multiple pixel scan? y=to go: " << endl;
    //    char r3=getchar();  cout << "OK" << endl;
    if(1) {
      const int iroc=theROC; {
	for(int off=0; off<5; off++) {
	  aFECInterface.clrcal(mfec, mfecchannel, hubaddress, portaddress, iroc);
	  for(int row=off;row<80; row+=16) {
	    for(int col=off;col<52; col+=10) {
	      aFECInterface.calpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 1);
	      aFECInterface.progpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 1, 5);
	    }
	  }
	  sleep(10);
	  aFECInterface.clrcal(mfec, mfecchannel, hubaddress, portaddress, iroc);
	}
      }
    }
	      

    cout << "begin roc scan? y" << endl;
    char r2=getchar();
    if(r2=='y'){
      for(int iroc=0;iroc<8;iroc++){
	aFECInterface.clrcal(mfec, mfecchannel, hubaddress, portaddress, 1);
      }
      for(int iroc=0;iroc<8;iroc++){
	const int col=10;
	const int row=0;
	cout << "Go for roc# " << iroc << "?" << endl;
	getchar();
	aFECInterface.calpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 1);
	aFECInterface.progpix(mfec, mfecchannel, hubaddress, portaddress, iroc, col, row, 1, 5);

	cout << "waiting for next chip..." <<endl;
	getchar();
	aFECInterface.clrcal(mfec, mfecchannel, hubaddress, portaddress, iroc);
      }

    }
    aFECInterface.clrcal(mfec, mfecchannel, hubaddress, portaddress, theROC);
    aFECInterface.calpix(mfec, mfecchannel, hubaddress, portaddress, theROC, 10, 0, 1);

  return 0;

}
