#include <fstream>
#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>

using namespace std;

#include "VMEDevice.hh"
//#include "VMEDummyBusAdapter.hh"
#include "VMEAddressTable.hh"
#include "VMEAddressTableASCIIReader.hh"
#include "CAENLinuxBusAdapter.hh"
#include "PixelFECInterface/include/PixelFECInterface.h"

#include "CAENVMElib.h"

void waithere();
void analyzeError(CVErrorCodes ret);  // Wills error analyser
void analyzeFecCsr(int status);


int main(int argc, void *argv[])
{

  string str1;
  string dacfilename;
  string mystr;
  int i,j;
  int ic, ir;
  unsigned long itemp;
  int itemp2; 
  int kk;

  int ret;

#define LINK (0)

#ifdef USE_HAL
  unsigned long vmeslotaddr = 0x30000000;
#else
  long aBHandle;
  short Link=LINK;
#endif

  // load test vector:  std::vector <unsigned char> testroc ((80*52), 0x82);
  std::vector <unsigned char> testroc ((80*52), 0x00);
  std::vector <unsigned char> testroc2 ((80*52), 0x00);

  //  std::vector <unsigned char> dacvec (100, 0);
  std::vector <unsigned char> dacvec (100, 0);

  dacvec[0] = 5;
  dacvec[1] = 152;
  dacvec[2] = 255;
  dacvec[3] = 14;
  dacvec[4] = 0;
  dacvec[5] = 0;
  dacvec[6] = 35;
  dacvec[7] = 0;
  dacvec[8] = 35;
  dacvec[9] = 119;
  dacvec[10] = 29;
  dacvec[11] = 90;
  dacvec[12] = 38;
  dacvec[13] = 6;
  dacvec[14] = 75;
  dacvec[15] = 110;
  dacvec[16] = 75;
  dacvec[17] = 114;
  dacvec[18] = 102;
  dacvec[19] = 60;
  dacvec[20] = 200;
  dacvec[21] = 100;
  dacvec[22] = 100;
  dacvec[23] = 100;
  dacvec[24] = 40;
  dacvec[25] = 120;
  dacvec[26] = 0;


  unsigned long int mydataval;
  unsigned long int *mydata;
  mydata = &mydataval;
  
  // Set one pixel on one of the test vectors
      testroc2[0] = 0x80;
      testroc2[78] = 0x80;
      //      testroc2[4] = 0x80;

      cout<<"\nThis is a simple manual Unit Test for the PixelFECInterface.\n";
      waithere();
      cout<<"This test application was compiled to use: ";
      #ifdef USE_HAL
      cout<<"HAL\n";
      #else
      cout<<"CAEN\n";
      #endif

      cout<<"First, try attaching to the PCI<-->VME conduit.\n";

#ifdef USE_HAL
  // HAL::VMEDummyBusAdapter busAdapter;
    HAL::CAENLinuxBusAdapter busAdapter( HAL::CAENLinuxBusAdapter::V2718 );

    cout << "FROM HAL : CAEN Bus Adapter object created\n";


    HAL::VMEAddressTableASCIIReader addressTableReader( "PFECAddressMap.dat" );
    cout << "Read in the PFECAddressMap.dat.";
    
    HAL::VMEAddressTable addressTable("Test fecaddrtable",addressTableReader);
  cout << " (addressTable) ";
    HAL::VMEDevice PixFECCard(addressTable, busAdapter, vmeslotaddr);
  cout << " (PixFECCard) " << endl;



  /*
  for (i=0;i<-1;i++) {
    PixFECCard.read("TESTREG2", &value);
    cout << "READ:" <<hex<<value<<endl;
    PixFECCard.write("TESTREG2", 0x00003300);
    usleep(100000);
    PixFECCard.write("TESTREG2", 0x00003400);
    usleep(100000);
    PixFECCard.read("TESTREG2", &value);
    cout << "READ:" <<hex<<value<<endl;

  }
  */

#else

#define VMEDEVICE (0xec800000)
#define VMEBOARD (cvV2718)
    CVBoardTypes VMEBoard=VMEBOARD;
    short Device=VMEDEVICE;
    
    if( CAENVME_Init(VMEBoard, Device, Link, &aBHandle) != cvSuccess ) 
      {
	printf("\n\n Error opening the device\n");
	exit(1);
      }
    
    
#endif

    cout << "\nSuccessfully initialized the communication conduit.\n";
    cout << "Now instatiating a FEC object.\n";

#ifdef USE_HAL
    PixelFECInterface fec1(&PixFECCard, 2);
#else
    PixelFECInterface fec1(0x30000000, aBHandle);
 #endif


    cout << "FEC constructor has been called.\n";
    cout << "Now try setting the SSID of the FEC VME board.\n";

    cout<<"Sending 100 (Pixels) to SSID register of FEC carrier.\n";
    fec1.setssid(4); // set to 100 -> PIXELS

    cout<<"Query for mfec eeprom version number.\n";

    fec1.getversion(&itemp);
    cout << "GETVERSION from mfec returned: "<<hex<<itemp<<endl;
  /*
    mydataval=0x01020304;
    i = 4;
    fec1.outputblock(1,1, (unsigned int *) mydata, i);
    sleep(1);
    mydataval=0x05060708;
    fec1.outputblock(1,1,  (unsigned int *) mydata, i);
    sleep(1);

    mydataval=0x090a0b0c;
    fec1.outputblock(1,1,  (unsigned int *) mydata, i);

    exit(1);

  */

    // add text to enter params from cosmo here 
 

    cout << "\nYou should have COSMO running and able to select pixels to read out.";


    fec1.fecDebug(0);

    //              goto finaltest;
    //    goto shorttest;

    //        goto bobtest;

    //    goto danek;

    //    goto danektest;

    waithere();

    cout << "Setting CAL on a single pixel using progpix(1) to enable and calpix to set cal\n";
    ic = 0; ir = 0;
    fec1.progpix(6,1,16,0,15,ic,ir,1,0);
    fec1.calpix(6,1,16,0,15,ic,ir,1);

    cout << "Pixel should be on.  Next to disable pixel with progpix().";
    waithere();
    fec1.progpix(6,1,16,0,15,ic,ir,0,0);

    cout << "\nPixel should be off.  Now again to enable the pixel with progpix()"; 
    waithere();
    fec1.progpix(6,1,16,0,15,ic,ir,1,0);

    cout << "\nPixel should be back on.  Now to turn off using clrcal";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 15);

    cout << "\nPixel should be off again. Next to set disable for the pixel using progpix().\nPixel should stay off.";
    waithere();
    fec1.progpix(6,1,16,0,15,ic,ir,0,0);
    
    
    cout << "Next we'll enable a calibrating pixel, but keep the dcol off.";
    waithere();
    fec1.progpix(6,1,16,0,15,ic,ir,1,0);
    fec1.calpix(6,1,16,0,15,ic,ir,1);
    fec1.dcolenable(6,1,16,0,15,0,0);


    cout << "Now to enable the column.  Pixel should read out.";
    waithere();
    fec1.dcolenable(6,1,16,0,15,0,1);

    cout << "Pixel should be reading out now.  Let's turn it off next.";
    waithere();
    fec1.dcolenable(6,1,16,0,15,0,0);

    cout << "Pixel should be off now.  Let's turn column on again.";
    waithere();
    fec1.dcolenable(6,1,16,0,15,0,1);

    cout << "No pixel should be reading out now.\n";
    cout << "Now let's test the DAC setting routine progdac().\n";
    cout << "Please enter the current VCAL DAC setting from the COSMO window: ";
    cin >> itemp;

    cout << "\nIs VCAL range set to high(1) or low(0) ?\n";
    cout << "Enter 1 or 0 :";
    cin >> itemp2;

    if (itemp2 == 1) {
      fec1.progdac(6,1,16,0,15,253,0x04);
    } else {
      fec1.progdac(6,1,16,0,15,253,0x00);
    }      
    
    waithere();

    cout << "Now we are sweeping VCAL up and down.\n";
    cout << "You should see last-dac height on scope move up and down several times.\n";

    fec1.dcolenable(6,1,16,0,15,0,1);
    for (j=0;j<1;j++) {
      for (i=0;i<256;i++) {
	fec1.progdac(6, 1, 16, 0, 15, 25, i);
	usleep(50);
      }
      for (i=255;i>0;i--) {
	fec1.progdac(6, 1, 16, 0, 15, 25, i);
	usleep(50);
      }
    }
    cout << "\nDone with sweep.  Setting VCAL back to start value of: " <<dec<<itemp<<endl;
    
    waithere();
    fec1.progdac(6, 1, 16, 0, 15, 25, itemp);

    cout << "Now we will change just ROC speed from 40 to 20 MHz a few times.\n";
    waithere();
    /* int rocsetchipcontrolregister(int mfec, int mfecchannel,
       int tbmchannel, int hubaddress, int portaddress,	int rocid,
       int calhighrange, int chipdisable, int halfspeed); */
    fec1.rocsetchipcontrolregister(6,1,16,0,15,itemp2,0,0); sleep(1);
    fec1.rocsetchipcontrolregister(6,1,16,0,15,itemp2,0,1); sleep(1);
    fec1.rocsetchipcontrolregister(6,1,16,0,15,itemp2,0,0); sleep(1);
    fec1.rocsetchipcontrolregister(6,1,16,0,15,itemp2,0,1); sleep(1);
    fec1.rocsetchipcontrolregister(6,1,16,0,15,itemp2,0,0); sleep(1);

    cout << "Now we will set a pixel reading out.\n";
    waithere();
    fec1.dcolenable(6,1,16,0,15,0,1);
    fec1.progpix(6,1,16,0,15,ic,ir,1,0);
    fec1.calpix(6,1,16,0,15,ic,ir,1);

    cout << "Pixel should be reading out.  Next turn off ROC daq.\n";
    waithere();
    fec1.rocsetchipcontrolregister(6,1,16,0,15,itemp2,1,0);

    cout << "Pixel should have stopped reading out.  Let's turn DAQ back on.\n";
    waithere();
    fec1.rocsetchipcontrolregister(6,1,16,0,15,1,0,0);

    cout << "Pixel should be back on readout.\n";
    cout << "Setting VCAL range to low range.\n";
    cout << "   Typically, if pixel was reading out on high range,\n this will cause pixel to stop reading out.\n";
    waithere();
    fec1.rocsetchipcontrolregister(6,1,16,0,15,0,0,0);

    cout << "Pixel may have stopped reading out.  Next set VCAL range back to high.\n";
    waithere();
    fec1.rocsetchipcontrolregister(6,1,16,0,15,1,0,0);

    cout << "Pixel should be reading out now.\n";
    cout << "\nNext we will program all the dacs for the ROC using progalldacs()\n";
    waithere();
    fec1.progalldacs(6,1,16,0,15,134,0x04, dacvec);

    cout << "Everything should be reading out.  Let's change the vcal in the vector argument and watch the pixel stop reading out.\n";
    waithere();
    dacvec[24] = 0;
    fec1.progalldacs(6,1,16,0,15,134,0x04, dacvec);

    cout << "Let's change it back now";
    waithere();
    dacvec[24] = itemp;
    fec1.progalldacs(6,1,16,0,15,134,0x04, dacvec);

    cout << "Pixel should be reading out now\n";
    cout << "Now we'll try a rocinit setting all pixels to mask disable.\n";
    waithere();
    fec1.rocinit(6, 1, 16, 0, 15, 0, 5);

    cout << "Pixel should have turned off due to rocinit mask disabling all pixels\n";
    cout << "Now use roctrimload() to enable first pixel\n";
    waithere();
    fec1.roctrimload(6,1,16,0,15,testroc2);

    cout << "Pixel is reading out because CAL is still set on it.  Let's call CLRCAL\n";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 15);
    cout << "Pixel should be off but enabled.\n";
    cout << "Use calpix() to set cal on 2 pixels and cause them to readout.\n";
    waithere();
    fec1.calpix(6,1,16,0,15,0,0,1);
    fec1.calpix(6,1,16,0,15,0,78,1);
    cout << "\nNow to use roctrimload() to mask disable 1st pixel\n";
    waithere();
    fec1.roctrimload(6,1,16,0,15,testroc);

    cout << "\nPixel should be off now.\n";
    cout << "\nNow let's use coltrimload() to turn the pixel on again\n";
    waithere();
    fec1.coltrimload(6,1,16,0,15,0,1, testroc2); // one col
    cout << "\nPixel should be on due to unmasking with coltrimload()\n";
    cout << "\nNext to disable the pixel with coltrimload()\n";
    waithere();
    fec1.coltrimload(6,1,16,0,15,0,1, testroc);
    cout << "\nPixel should be disable now.\n";
    waithere();

 finaltest:
    /*    cout << "\nclrcal";
    waithere();
        fec1.clrcal(6, 1, 16, 0, 14, 1);
        fec1.clrcal(6, 1, 16, 0, 14, 1);
	fec1.clrcal(6, 1, 16, 0, 15, 1); */
    cout << "\nclrcal";
    fec1.calpix(6,1,16,0,15,0,0,1);
    fec1.roctrimload(6,1,16,0,15,testroc2);
    waithere();
    fec1.clrcal(6, 1, 16, 0, 14, 1);
    cout << "\nclrcal";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 14, 1);
    cout << "\nclrcal";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 14, 1);
    cout << "\nclrcal";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 14, 1);
    cout << "\nclrcal on real roc chipid";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 15, 1);  // clrcal the ROC
    cout << "\nclrcal";
    waithere(); 
    fec1.dcolenable(6,1,16,0,15,0,1, 1); // send dcol enable
    fec1.progpix(6,1,16,0,15,0,0,1,0, 1); // send progpix
    fec1.calpix(6,1,16,0,15,0,0,1, 1); // cal the pixel, should be readingout
    fec1.clrcal(6, 1, 16, 0, 14, 1);
    cout << "\nclrcal";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 14, 1);
    cout << "\nclrcal";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 14, 1);
    
    cout << "\noutbuf";
    waithere();
    fec1.qbufsend(6, 1);


    cout << "now to nonexist hub\n";
    waithere();
    fec1.clrcal(6, 1, 15, 0, 14);
    waithere();
    exit(1);

 shorttest:
    waithere();
    cout<<"NEXT\n";
    waithere();
	fec1.clrcal(6, 1, 15, 0, 15);
    waithere();
    for (int m=0;m<200;m++) {
	fec1.clrcal(6, 1, 15, 0, 15, 1);
    }
    fec1.clrcal(6, 1, 16, 0, 15, 1);
    for (int m=0;m<20;m++) {
	fec1.clrcal(6, 1, 15, 0, 15, 1);
    }

    // over the buffer limit
    for (int m=0;m<111;m++) {
	fec1.clrcal(6, 1, 15, 0, 15, 1);
    }

    cout << "\nclrcal";
    waithere();
    fec1.qbufsend(6, 1);
    
    exit(1);

 bobtest:
    cout << "BOBTEST\n";
    cout << "rocinit (disable entire ROC) followed by calpix(on 0,0).\n";
     waithere();
    fec1.rocinit(6, 1, 16, 0, 15, 0, 5);
    fec1.calpix(6,1,16,0,15,10,13,1);

    cout << "\nLast test in this group completed.\n";
    exit(1);

 danektest:
    cout << "Pixel should be reading out now\n";
    cout << "Now we'll set clrcal and turn any reading pixels off.\n";
    waithere();
    fec1.clrcal(6, 1, 16, 0, 15);
    cout << "Pixel should be off\n";
    cout << "Now we'll try a rocinit setting all pixels to mask enable.\n";
     waithere();
    fec1.rocinit(6, 1, 16, 0, 15, 1, 5);
    cout << "Pixels should still be off.  Next manually PLACE wrong setting into READBACK PHASE.\n";
    cout << "Now we'll try a rocinit setting all pixels to mask DISable.\n";
    //    waithere();
    fec1.rocinit(6, 1, 16, 0, 15, 0, 5);
    //   cout << "All pixels should be disabled.  SET CAL manually via COSMO.\n";

    //    cout << "Next let's calibrate a couple pixels.\n";
    //   waithere();
        fec1.calpix(6,1,16,0,15,0,0,1);

    for (kk=60;kk<79;kk++) {
      //      fec1.calpix(6,1,16,0,15,48,kk,1);
    }

    cout << "Now we'll try a rocinit setting all pixels to mask enable.\n";
        waithere();
    fec1.rocinit(6, 1, 16, 0, 15, 1, 5);


    cout << "\nLast test in this group completed.\n";
    exit(1);

 danek:

  // Set the FEC to Pixel mode = 4
  ret = fec1.setssid(4);  
  if(ret != 0) {  // Error
    cout<<"Error in setssid "<<hex<<ret<<dec<<endl;
    analyzeError( CVErrorCodes(ret));
    return -1;
  }
  cout<<"Set FEC to pixel mode "<<endl;


  unsigned long data = 0;
  ret = fec1.getversion(&data); //  
  if(ret != 0) {  // Error
    cout<<"Error in getversion "<<hex<<ret<<dec<<endl;
    analyzeError( CVErrorCodes(ret));
    return -1;
  }
  cout<<" mFEC Firmware Version "<<data<<endl;;

  // Init all mFECs
  for(int i = 6;i<9;i++) {
    ret = fec1.getfecctrlstatus(i,&data);  
    if(ret != 0) {  // Error
      cout<<"Error in getstatus "<<hex<<ret<<dec<<endl;
      analyzeError( CVErrorCodes(ret));
      return -1;
    }
    cout<<" mfec= "<<i<<" status "<<hex<<data<<" "<<ret<<dec<<endl;   

    // What does this realy do?
    // Internal triggers work for both settings
    // Externa: works for both? 
    //fec1.disableexttrigger(i,0);// 0 - Enable external(TTC) triggers
    // 1- Disable external(TTC) triggers

    //cout<<" after enable/disable ext trig "<<endl;
    //fec1.getfecctrlstatus(i,&data);  
    //cout<<"FEC Status = "<<hex<<data<<dec<<endl;
    //cout<<" FEC cached registers "<<hex<<fec1.getControlReg(mfec)<<" "
    //  <<  fec1.getCsReg(mfec)<<dec<<endl;
    
    // Settings for internal triggers
    //fec1.enablecallatency(i, 0);  // 1=generate cal+trig, 0=cal only
    // Strange but for CAL only something has to be written into the COUNT register
    //fec1.callatencycount(i, 1); // cal-trig delay in bx
    ///fec1.callatencycount(i, 150); // cal-trig delay in bx
    

  }


  // Select mFEC(1-8) & channel (1,2)
  int mfec = 6; // mfec number, 8 on the frontpanel
  int channel = 1; // select 1st mFEC channel
  cout<<" Use MFEC = "<<mfec<<" CHANNEL = "<<channel<<endl;

  ret = fec1.getfecctrlstatus(mfec,&data);  
  if(ret != 0) {  // Error
    cout<<"Error in getstatus "<<hex<<ret<<dec<<endl;
    analyzeError( CVErrorCodes(ret));
    return -1;
  }
  cout<<"FEC Status = "<<hex<<data<<dec<<endl;
  analyzeFecCsr(data);

  // Reset 
  fec1.injectrsttbm( mfec, 1);  // send reset-TBM
  //cout<<" after resettbm "<<endl;
  fec1.injectrstroc( mfec, 1);  // send reset-ROC
  cout<<" after reset tbm/roc "<<endl;

  // Select HUBS
  //  int hubaddress = 31;  // If one hub selected
  int hubaddress = 16;  // If one hub selected
  //int value2=0;
  ///////////  cout<<" enter hub :";
  ///////////  cin>>hubaddress;
  int numhubs = 1;
  
  // for trims
  vector<unsigned char> maskTrims(4160);
  int m =0;
  for(int icol=0;icol<52;icol++) {
    for(int irow=0;irow<80;irow++) {
      //maskTrims[m]= char(0x80); // Enable + trim=0
      //maskTrims[m]= char(0x8F); // Enable + trim=F
      maskTrims[m]= char(0x00); // Disable + trim=0
      //if(icol==col && irow==row) maskTrims[m]= char(0x80); // select one pixel
      m++;
    }
  }


  // INIT TBM
  // Test the TBM commands 
  const int tbmA = 0xE; //  TBM-A 
  const int tbmB = 0xF; //  TBM-B 
  int portaddress = 0;

   //                   mfec  channel  tbm#   hubaddress port offset data dir(write=0)
   fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   2,  0xF0,   0); // reset
   fec1.tbmcmd(mfec, channel, tbmB,   hubaddress, 4,   2,  0xF0,   0); // reset
   fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   0,     1,   0); // set seed, 1=40MHz
   fec1.tbmcmd(mfec, channel, tbmB,   hubaddress, 4,   0,     1,   0); // set seed, 1=40MHz
   fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   1,  0xC0,   0); // set mode = CAL
   fec1.tbmcmd(mfec, channel, tbmB,   hubaddress, 4,   1,  0xC0,   0); // set mode = CAL
   fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   4,  0x00,   0); // enable A
   //fec1.tbmcmd(mfec, channel, tbmB,   hubaddress, 4,   4,  0x03,   0); //disable B(single)
   fec1.tbmcmd(mfec, channel, tbmB,   hubaddress, 4,   4,  0x00,   0); //enable B(dual mode)


   if(hubaddress==22) { // special setting for module 22

     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   5,   150,   0); // set DAC0(inputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   6,   100,   0); // set DAC1
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   7,   200,   0); // set DAC2(dacgain)

   } else if( hubaddress==28) {

     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   5,   150,   0); // set DAC0(inputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   6,   175,   0); // set DAC1(outputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   7,   250,   0); // set DAC2(dacgain)

   } else if( hubaddress==15) {

     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   5,   150,   0); // set DAC0(inputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   6,   175,   0); // set DAC1(outputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   7,   250,   0); // set DAC2(dacgain)

   } else if( hubaddress==23) {

     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   5,   150,   0); // set DAC0(inputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   6,   175,   0); // set DAC1(outputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   7,   250,   0); // set DAC2(dacgain)

   } else {

     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   5,   150,   0); // set DAC0(inputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   6,   200,   0); // set DAC1(outputbias)
     fec1.tbmcmd(mfec, channel, tbmA,   hubaddress, 4,   7,   200,   0); // set DAC2(dacgain)

   }

   cout<<"Init TBM, hubaddress = "<<hubaddress<<endl;
   
   // Setup all ROCs
   cout<<"Init All ROCs for Hub: "<<hubaddress<<endl;
   for(int iroc=15;iroc<16;iroc++){  // Loop over rocs
     //     portaddress = iroc/4;  // 
     portaddress = 0;  // 
     cout<<iroc<<"/"<<portaddress<<" ";
 goto skipdacs;
     // Setup DACs
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x1, 6);   //Vdigi
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x2, 120); // 120 Vana
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x3, 128); //Vsf
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x4, 8);   //Vcom
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x5, 0);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x6, 0);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x7, 35);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x8, 0);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0x9, 35);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0xA, 88);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0xB, 7);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0xC, 40); //VtrhComp(60)
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0xD, 30);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0xE, 10);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc, 0xF, 40);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x10, 50);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x11, 100);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x12, 115);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x13, 200);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x14, 120);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x15, 200);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x16, 99);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x17, 0);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x18, 0);
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x19, 100); //Vcal
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x1A, 110); // 120 CalDel
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0x1B, 0);   // RangeTemp 
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0xFD, 4);   //CtrlReg - 0-low/4-high vcal range
     fec1.progdac(mfec, channel, hubaddress, portaddress, iroc,0xFE, 144); //WBC (144)
     
   skipdacs:
     //cout<<" after dac init "<<endl;
     //fec1.getfecctrlstatus(mfec,&data);  
     //analyzeFecCsr(data);

     // Clear cal
     fec1.clrcal(mfec, channel, hubaddress, portaddress, iroc);
     
     //cout<<" after clr "<<endl;
     //fec1.getfecctrlstatus(mfec,&data);  
     //analyzeFecCsr(data);

     // Mask all
     int trim = 0; // set all trims to 0
     int mask = 0; // mask (disable) all pixels
     for(int idcol=0;idcol<26;idcol++) {  // Disable all dcols
       fec1.dcolenable(mfec, channel, hubaddress, portaddress, iroc, idcol,mask);//disable
     }
     
     //cout<<" after mask "<<endl;
     //fec1.getfecctrlstatus(mfec,&data);  
     //analyzeFecCsr(data);

     // Disbale all pixels
     fec1.rocinit(mfec, channel, hubaddress, portaddress, iroc, 0, trim);  // disable 
     //fec1.rocinit(mfec, channel, hubaddress, portaddress, iroc, 1, trim); // enable

     //cout<<" after roc init "<<endl;
     //fec1.getfecctrlstatus(mfec,&data);  
     //analyzeFecCsr(data);

     // Disable/Enable all pixel with ROCTRIMLOAD
     //fec1.roctrimload(mfec, channel, hubaddress, portaddress, iroc, maskTrims);
      
//       // With single pixel access
//       for(int icol=0;icol<52;icol++) {
// 	for(int irow=0;irow<80;irow++) {
// 	  fec1.progpix(mfec, channel, hubaddress, portaddress, iroc, icol, irow, 0, trim); //disable
// 	  //fec1.progpix(mfec, channel, hubaddress, portaddress, iroc, icol, irow, 1, trim); //enable
// 	}
//       }

     // Now enable something

      // Enable all dcolumns
      cout<<" enable all dcols for ROC  "<<iroc<<endl;
      for(int idcol=0;idcol<26;idcol++) {  // enable all dcols
	fec1.dcolenable(mfec, channel, hubaddress, portaddress, iroc, idcol, 1);//enable 
	//fec1.dcolenable(mfec, channel, hubaddress, portaddress, iroc, idcol,0 );//disable 
      }
      
      // Enable 1 dcol
      //fec1.dcolenable(mfec, channel, hubaddress, portaddress, iroc, dcol, 1); //enable dc
      
      // Enable all pixels
      // With ROCINIT   
      //fec1.rocinit(mfec, channel, hubaddress, portaddress, iroc, mask, trim);
      
      //     // With TRIMLOAD
      //     // Enable all pixel with ROCTRIMLOAD
      //     fec1.roctrimload(mfec, channel, hubaddress, portaddress, iroc, maskTrims);
      
      //     cout<<" after roctrimload "<<endl;
      //     fec1.getfecctrlstatus(mfec,&data);  
      //     analyzeFecCsr(data);
      
      // With single pixel access
      //for(int icol=0;icol<52;icol++) {
      // for(int irow=0;irow<80;irow++) {
      //fec1.progpix(mfec, channel, hubaddress, portaddress, iroc, icol, irow, 1, trim); //enable
      //}
      //}
      
      int col = 10;
      int row = 13; 
      // Enable only 1 pixel  
      fec1.progpix(mfec, channel, hubaddress, portaddress, iroc, col, row, 1, trim);
      
      // Cal enable for one pixel
      int cal = 1; // select CAP cal=1
      fec1.calpix( mfec, channel, hubaddress, portaddress, iroc, col, row, cal);

      cout<<"Enable pixel col/pix = "<<col<<" "<<row<<" for roc "<<iroc<<endl;;

   }  // end loop over ROCs
      
//   // Reset 
//   fec1.injectrsttbm( mfec, 1);  // send reset-TBM
//   //cout<<" after resettbm "<<endl;
//   fec1.injectrstroc( mfec, 1);  // send reset-ROC
//   cout<<" after reset tbm/roc "<<endl;

//   fec1.getfecctrlstatus(mfec,&data);  
//   cout<<"FEC Status = "<<hex<<data<<dec<<endl;
//   analyzeFecCsr(data);

//  CAENVME_End(aBHandle);  // close VME access
  return 0;


}
///////////////////////////////////////////////////
void analyzeFecCsr(int status) {
  cout<<hex<<status<<dec<<endl;
  // Channel 1
  if( (status & 0x0100) == 0x0100 )  cout<<" ch1 receive timeout "<<endl;
  if( (status & 0x0200) == 0x0200 )  cout<<" ch1 receive complete "<<endl;
  if( (status & 0x0400) == 0x0400 )  cout<<" ch1 hub address erros "<<endl;
  if( (status & 0x0800) == 0x0800 )  cout<<" ch1 send started (not finished!) "<<endl;
  if( (status & 0x1000) == 0x1000 )  cout<<" ch1 receive count error "<<endl;
  if( (status & 0x2000) == 0x2000 )  cout<<" ch1 receive error "<<endl;
  // Channel 2
  if( (status & 0x01000000) == 0x01000000 )  cout<<" ch2 receive timeout "<<endl;
  if( (status & 0x02000000) == 0x02000000 )  cout<<" ch2 receive complete "<<endl;
  if( (status & 0x04000000) == 0x04000000 )  cout<<" ch2 hub address erros "<<endl;
  if( (status & 0x08000000) == 0x08000000 )  cout<<" ch2 send started (not finished!) "<<endl;
  if( (status & 0x10000000) == 0x10000000 )  cout<<" ch2 receive count error "<<endl;
  if( (status & 0x20000000) == 0x20000000 )  cout<<" ch2 receive error "<<endl;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
// General error analysis
void analyzeError(CVErrorCodes ret)
 {
  switch (ret) {
  case cvGenericError   : cout<<" Generic error !!!"<<endl;
    break ;
  case cvBusError : cout<<" Bus Error !!!"<<endl;
    break ;
  case cvCommError : cout<<" Communication Error !!!"<<endl;
    break ;
  case cvInvalidParam : cout<<" Invalid Param Error !!!"<<endl;
    break ;
  default          : cout<<" Unknown Error !!!"<<endl;
    break ;
  }
}


void waithere(void)
{
  string str1;
  cout << "\nContinue? ";
  getline(cin, str1);
  if (str1[0] == 'n') exit(1);
}
