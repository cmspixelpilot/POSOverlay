#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelFEDTBMDelayCalibration.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include <iomanip>

using namespace pos;
using namespace std;

///////////////////////////////////////////////////////////////////////////
// Decode the FIFO-2 data in  transparent mode from piggy
// ADD SIZE
int decodePTrans(unsigned * data1, unsigned * data2, const int length) {
unsigned long mydat[16]=
{0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};

if(length<16) return -2;
  // Print & analyze the data buffers
int tempcode1=0;
int tempcode2=0;
int tempcode3=0;

  for(int icx=0;icx<16;icx++) {
if( ((data1[icx]&0xf0)==0x80) && ((data2[icx]&0xf0)==0x80) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
if( ((data1[icx]&0xf0)==0x90) && ((data2[icx]&0xf0)==0x90) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}



if((data1[icx]&0xf0)!=(data2[icx]&0xf0))tempcode1=1;
if( ((data1[icx]&0xf0)!=mydat[icx])|((data2[icx]&0xf0)!=mydat[icx]))tempcode2=2;

  }





  //if((tempcode1)!=0)cout<<"Buffers 0-15 dont match each other!"<<endl;
  //if((tempcode2)!=0)cout<<"Buffers 0-15 dont match expected pattern!"<<endl;
  //if((tempcode3)!=0)cout<<"Buffers 0-15 dont match event numbers!"<<endl;

return (tempcode1+tempcode2+tempcode3);


} // end

///////////////////////////////////////////////////////////////////////////
//// Decode the FIFO-2 data in  transparent mode from piggy
//// ADD SIZE
int decodePTrans2(unsigned  * data1, unsigned * data2, const int length) {
  //unsigned long mydat[16]=
  //{0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};

if(length<16) return -2;
  // Print & analyze the data buffers
  int tempcode1=0;//trailer
  int tempcode2=0;//rocs
  int tempcode3=0;//event
 int tempcode4=0;//header
 int tempcode5=0;
 //int tempcode6=0;
//header event number check
//
int mytr1=0;
int mytr2=0;

    for(int icx=0;icx<8;icx++) {
    if( ((data1[icx]&0xf0)==0x80) && ((data2[icx]&0xf0)==0x80) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
    if( ((data1[icx]&0xf0)==0x90) && ((data2[icx]&0xf0)==0x90) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}

    //if( ((data1[icx]&0xf0)==0xa0) && ((data2[icx]&0xf0)==0xa0) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
   // if( ((data1[icx]&0xf0)==0xb0) && ((data2[icx]&0xf0)==0xb0) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}

    if((data1[icx]&0xf0)==0x80)mytr1++;
    if((data1[icx]&0xf0)==0x90)mytr1++;
    if((data1[icx]&0xf0)==0xa0)mytr1++;
    if((data1[icx]&0xf0)==0xb0)mytr1++;
    if((data2[icx]&0xf0)==0x80)mytr2++;
    if((data2[icx]&0xf0)==0x90)mytr2++;
    if((data2[icx]&0xf0)==0xa0)mytr2++;
    if((data2[icx]&0xf0)==0xb0)mytr2++;


      }
if((mytr1!=4)|(mytr2!=4))tempcode4=8;
//Trailer check
mytr1=0;
mytr2=0;
    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xf0)==0xc0)mytr1++;
    if((data1[icx]&0xf0)==0xd0)mytr1++;
    if((data1[icx]&0xf0)==0xe0)mytr1++;
    if((data1[icx]&0xf0)==0xf0)mytr1++;
    if((data2[icx]&0xf0)==0xc0)mytr2++;
    if((data2[icx]&0xf0)==0xd0)mytr2++;
    if((data2[icx]&0xf0)==0xe0)mytr2++;
    if((data2[icx]&0xf0)==0xf0)mytr2++; 

      }
if((mytr1!=4)|(mytr2!=4))tempcode1=1;

//Rocs check
mytr1=0;
mytr2=0;
    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xf0)==0x70)mytr1++;
    if((data2[icx]&0xf0)==0x70)mytr2++;

      }
if((mytr1!=8)||(mytr2!=8))tempcode2=2;
//hits check
mytr1=0;
mytr2=0;

    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xff)==0x10)mytr1++;
    if((data2[icx]&0xff)==0x10)mytr2++;
    if((data1[icx]&0xff)==0x2a)mytr1++;
    if((data2[icx]&0xff)==0x2a)mytr2++;
    if((data1[icx]&0xff)==0x31)mytr1++;
    if((data2[icx]&0xff)==0x31)mytr2++;
    if((data1[icx]&0xfe)==0x44)mytr1++;
    if((data2[icx]&0xfe)==0x44)mytr2++;

      }

if((mytr1<7*4)||(mytr2<6*4))tempcode5=16;



      if((tempcode1)!=0)cout<<"missed trailer"<<endl;
      if((tempcode2)!=0)cout<<"missed roc"<<endl;
      if((tempcode3)!=0)cout<<"event number mismatch"<<endl;
      if((tempcode4)!=0)cout<<"missed header"<<endl;
      if((tempcode5)!=0)cout<<"missed hits"<<endl;


      return (tempcode1+tempcode2+tempcode3+tempcode4+tempcode5);


      } // end
//


///////////////////////////////////////////////////////////////////////////
// Decode the FIFO-2 data in  transparent mode from piggy
// ADD SIZE
// Checks for 1 hit per ROC from both chs
void decode(unsigned * data1, unsigned * data2, const int length) {
  unsigned long mydat[16]=
    {0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};
  
int a[4]={0,0,0,0};
  if(length<64) return;
  // Print & analyze the data buffers
  int tbmheader=0; int rocheader=0; int correctdata=0; int tbmtrailer=0;
  bool tbmh=false; bool tbmt=false; bool roch=false; bool data=false;
  if ( (data1[0]&0xf0)==0x80 && (data1[1]&0xf0)==0x90 && (data1[2]&0xf0)==0xa0 && (data1[3]&0xf0)==0xb0 && (data2[0]&0xf0)==0x80 && (data2[1]&0xf0)==0x90 && (data2[2]&0xf0)==0xa0 && (data2[3]&0xf0)==0xb0 ) {
    tbmheader++;
    a[0]++;
  }
  
  if ( (data1[4]&0xf0)==0x70 && (data1[11]&0xf0)==0x70 && (data1[18]&0xf0)==0x70 && (data1[25]&0xf0)==0x70 && (data1[32]&0xf0)==0x70 && (data1[39]&0xf0)==0x70 && (data1[46]&0xf0)==0x70 && (data1[53]&0xf0)==0x70 && (data2[4]&0xf0)==0x70 && (data2[11]&0xf0)==0x70 && (data2[18]&0xf0)==0x70 && (data2[25]&0xf0)==0x70 && (data2[32]&0xf0)==0x70 && (data2[39]&0xf0)==0x70 && (data2[46]&0xf0)==0x70 && (data2[53]&0xf0)==0x70 ){
    rocheader++;
    a[1]++;
  }

  for ( int icx=0;icx<8;icx++  ) {
    if ( (data1[5+(7*icx)]&0xf0)==0x10 &&  (data1[6+(7*icx)]&0xf0)==0x20 && (data1[7+(7*icx)]&0xf0)==0x30 &&  (data1[8+(7*icx)]&0xf0)==0x40 && (data1[9+(7*icx)]&0xf0)==0x50 &&  (data1[10+(7*icx)]&0xf0)==0x60 && (data2[5+(7*icx)]&0xf0)==0x10 &&  (data2[6+(7*icx)]&0xf0)==0x20 && (data2[7+(7*icx)]&0xf0)==0x30 &&  (data2[8+(7*icx)]&0xf0)==0x40 && (data2[9+(7*icx)]&0xf0)==0x50 &&  (data2[10+(7*icx)]&0xf0)==0x60  ){
	  correctdata++;
	  a[2]++;
    }
  }
  
  if ( (data1[60]&0xf0)==0xc0 && (data1[61]&0xf0)==0xd0 && (data1[62]&0xf0)==0xe0 && (data1[63]&0xf0)==0xf0 && (data2[60]&0xf0)==0xc0 && (data2[61]&0xf0)==0xd0 && (data2[62]&0xf0)==0xe0 && (data2[63]&0xf0)==0xf0 ) {
    tbmtrailer++;
    a[3]++;
  }
  
  
  cout << "tbmheader = " << tbmheader << endl;
  cout << "rocheader = " << rocheader << endl;
  cout << "correctdata = " << correctdata << endl;
  cout << "tbmtrailer = " << tbmtrailer << endl;
  cout << "a[] = " << a[0] << " "  << a[1] << " "  << a[2] << " "  << a[3] << endl;
  return;
      
      
} // end

PixelFEDTBMDelayCalibration::PixelFEDTBMDelayCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  cout << "Greetings from the PixelFEDTBMDelayCalibration copy constructor." << endl;
}

void PixelFEDTBMDelayCalibration::initializeFED() {
  setFEDModeAndControlRegister(0x8,0x30010);
}

xoap::MessageReference PixelFEDTBMDelayCalibration::beginCalibration(xoap::MessageReference msg) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  std::string retr_fn(outputDir() + "/RETR.txt");
  cout << "writing RETR lines to file " << retr_fn << endl;
  retrf.open(retr_fn);

  std::string root_fn(outputDir() + "/TBMDelay.root");
  cout << "writing histograms to file " << root_fn << endl;
  rootf = new TFile(root_fn.c_str(), "create");
  assert(rootf->IsOpen());

  tempCalibObject->writeASCII(outputDir());

  DumpFIFOs = tempCalibObject->parameterValue("DumpFIFOs") == "yes";

  //  const std::vector<PixelROCName>& rocs = tempCalibObject->rocList();
  //PixelRootDirectoryMaker rootDirs(rocs, rootf);

  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    std::vector<unsigned int> dacvals = tempCalibObject->scanValues(dacname);
    if (dacvals.size() > 1)
      dacstoscan.push_back(dacname);
  }

  if (dacstoscan.empty() && tempCalibObject->parameterValue("NoScanOK") != "yes") {
    cout << "no dacs in scan?" << endl;
    assert(0);
  }

  h_nfiforeaderrors = new TH1F("nfiforeaderrors", "", 1, 0, 1);
  h_nerrors = new TH1F("nerrors", "", 50, 0, 50);
  h_nhits = new TH1F("nhits", "", 50, 0, 50);
  h_nskip = new TH1F("nskip", "", 50, 0, 50);

  const TString sdecode[nDecode] = { "fifoErr", "wrongRoc", "wrongPix", "rightPix" };

  for (int idecode = 0; idecode < nDecode; ++idecode) {
    for (size_t i = 0; i < dacstoscan.size(); ++i) {
      const std::string& iname = dacstoscan[i];
      const TString itname(iname.c_str());
      const std::vector<unsigned>& ivals = tempCalibObject->scanValues(iname);
      const size_t ni = ivals.size();
      std::vector<double> ibins(ni+1);
      for (size_t k = 0; k < ni; ++k)
	ibins[k] = double(ivals[k]);
      ibins[ni] = ibins[ni-1] + (ibins[ni-1] - ibins[ni-2]);

      TH1F* h = new TH1F(itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";ntrig", ni, &ibins[0]);
      scans1d[idecode].push_back(h);

      for (size_t j = i+1; j < dacstoscan.size(); ++j) {
	const std::string jname = dacstoscan[j];
	const TString jtname(jname.c_str());
	const std::vector<unsigned>& jvals = tempCalibObject->scanValues(jname);
	const size_t nj = jvals.size();
	std::vector<double> jbins(nj+1);
	for (size_t k = 0; k < nj; ++k)
	  jbins[k] = double(jvals[k]);
	jbins[nj] = jbins[nj-1] + (jbins[nj-1] - jbins[nj-2]);
      
	TH2F* h2 = new TH2F(jtname + "_v_" + itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";" + jtname, ni, &ibins[0], nj, &jbins[0]);
	scans2d[idecode].push_back(h2);
      }
    }
  }
      
  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::endCalibration(xoap::MessageReference msg) {
  cout << "close RETR output file" << endl;
  retrf.close();
  
  cout << "In PixelFEDTBMDelayCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::execute(xoap::MessageReference msg) {
  Attribute_Vector parameters(2);
  parameters[0].name_="WhatToDo";
  parameters[1].name_="StateNum";
  Receive(msg, parameters);

  const unsigned state = atoi(parameters[1].value_.c_str());

  if (parameters[0].value_=="RetrieveData")
    RetrieveData(state);
  else if (parameters[0].value_=="Analyze")
    Analyze();
  else {
    cout << "ERROR: PixelFEDTBMDelayCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
    assert(0);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
  return reply;
}

void PixelFEDTBMDelayCalibration::RetrieveData(unsigned state) {
  assert(retrf.is_open());

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  const std::vector<PixelROCName>& rocs = tempCalibObject->rocList();
  typedef std::set< std::pair<unsigned int, unsigned int> > colrow_t;
  const colrow_t colrows = tempCalibObject->pixelsWithHits(state);

  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  retrf << "RETR event " << event_ << " state " << state << " ";
  if (DumpFIFOs) std::cout << "RETR event " << event_ << " state " << state << " ";
  std::map<std::string, unsigned int> currentDACValues;
  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    const unsigned dacvalue = tempCalibObject->scanValue(tempCalibObject->scanName(dacnum), state);
    currentDACValues[dacname] = dacvalue;
    retrf << dacname << " " << dacvalue << " ";
    if (DumpFIFOs) std::cout << dacname << " " << dacvalue << " ";
  }
  if (DumpFIFOs) std::cout << std::endl;

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDInterface* iFED = FEDInterface_[vmeBaseAddress];

    uint64_t buffer3[pos::slinkDepth];
    uint32_t bufferErr[36*1024];
    const int status3 = iFED->spySlink64(buffer3);
    const int statusErr = iFED->drainErrorFifo(bufferErr);

    if (status3 <= 0) {
      retrf << "ERROR reading a fifo on FED # " << fednumber << " in crate # " << crate_ << ": status3 = " << status3 << endl;
      std::cout << "ERROR reading a fifo on FED # " << fednumber << " in crate # " << crate_ << ": status3 = " << status3 << endl;
      h_nfiforeaderrors->Fill(0);
      FillEm(state, fifoErr);
      usleep(1000);
      std::cout << "readDigFEDStatus(): ";
      iFED->readDigFEDStatus(false);
      continue;
    }

    FIFO3Decoder decode3(buffer3);
    ErrorFIFODecoder decodeErr(bufferErr, statusErr);

    retrf << "nerr " << statusErr << " ";
    h_nerrors->Fill(statusErr);

    const unsigned nhits = decode3.nhits();
    retrf << "nhits " << nhits << " {hits ";
    h_nhits->Fill(nhits);

    if (DumpFIFOs) {
      uint32_t buffer2[9][pos::fifo2Depth];
      int status2[9] = {0};
      for (int chip = 1; chip <= 7; chip += 2) {
	//if (chip > 4) {
	//  std::cout << "not dumping fifo2 except for chip1,2,3\n";
	//  continue;
	//}
	status2[chip] = iFED->drainDataFifo2(chip, buffer2[chip]);
	std::cout << "----------------------------------" << std::endl;
	if (status2[chip] < 0)
	  std::cout << "Spy FIFO 2 for chip = " << chip << " status = " << status2[chip] << std::endl;
	else {
	  std::cout << "Contents of Spy FIFO 2 for chip = " << chip << "(status2 = " << status2[chip] << ")" <<std::endl;
	  std::cout << "----------------------------------" << std::endl;
	  for (int i = 0; i <= status2[chip]; ++i)
	    std::cout << "Clock " << std::setw(2) << i << " = 0x" << std::hex << buffer2[chip][i] << std::dec << std::endl;
	  std::cout << "----------------------------------" << std::endl;
	}
      }
      if (status2[1] > 0 && status2[3] > 0) {
	cout<<"decodePTrans return: " << decodePTrans(buffer2[1],buffer2[3],16)<<endl;
	cout<<"decodePTrans2 return: " << decodePTrans2(buffer2[1],buffer2[3],16)<<endl;
	cout << "decode: " << endl;
	decode(buffer2[1],buffer2[3],64);
      }

      std::cout << "----------------------" << std::endl;
      std::cout << "Contents of Spy FIFO 3" << std::endl;
      std::cout << "----------------------" << std::endl;
      for (int i = 0; i <= status3; ++i)
	std::cout << "Clock " << std::setw(2) << i << " = 0x " << std::hex << std::setw(8) << (buffer3[i]>>32) << " " << std::setw(8) << (buffer3[i] & 0xFFFFFFFF) << std::dec << std::endl;
      std::cout << "FIFO3Decoder thinks:\n"
		<< "nhits: " << decode3.nhits() << "\n";
      for (unsigned i = 0; i < decode3.nhits(); ++i)
	std::cout << "#" << i << ": ch: " << decode3.channel(i)
		  << " rocid: " << decode3.rocid(i) << " dcol: " << decode3.dcol(i)
		  << " pxl: " << decode3.pxl(i) << " pulseheight: " << decode3.pulseheight(i)
		  << " col: " << decode3.column(i) << " row: " << decode3.row(i) << std::endl;

      std::cout << "Contents of Error FIFO" << std::endl;
      std::cout << "----------------------" << std::endl;
      for (int i = 0; i <= statusErr; ++i)
	std::cout << "Clock " << i << " = 0x" << std::hex << bufferErr[i] << std::dec << std::endl;
      std::cout << "ErrorFIFODecoder thinks:\n";
      decodeErr.printToStream(std::cout);
    }

    unsigned nskip = 0;

    for (unsigned ihit = 0; ihit < nhits; ++ihit) {
      const unsigned channel = decode3.channel(ihit);
      const unsigned rocid = decode3.rocid(ihit);
      assert(rocid > 0);

      const PixelROCName& roc = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);

      // Skip if this ROC is not on the list of ROCs to calibrate.
      // Also skip if we're in singleROC mode, and this ROC is not being calibrated right now.
      vector<PixelROCName>::const_iterator foundROC = find(rocs.begin(), rocs.end(), roc);
      if (foundROC == rocs.end()) { // || !tempCalibObject->scanningROCForState(roc, state)) {
	FillEm(state, wrongRoc);
	++nskip;
      }
      else {
	const unsigned col = decode3.column(ihit);
	const unsigned row = decode3.row(ihit);
      
	retrf << "hit #" << ihit << " roc " << roc << " (" << rocid << ") ch " << channel << " col " << col << " row " << row << " ";

	if (colrows.find(std::make_pair(col, row)) == colrows.end())
	  FillEm(state, wrongPix);
	else
	  FillEm(state, rightPix);
      }
    }

    h_nskip->Fill(nskip);
    retrf << "hits} nskip " << nskip << endl; 

    if (DumpFIFOs) {
      usleep(1000);
      std::cout << "readDigFEDStatus(): ";
      iFED->readDigFEDStatus(false);
    }
  }
}

void PixelFEDTBMDelayCalibration::Analyze() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  rootf->Write();
  rootf->Close();
}

void PixelFEDTBMDelayCalibration::FillEm(unsigned state, int which) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  int k = 0;
  for (size_t i = 0; i < dacstoscan.size(); ++i) {
    const std::string& iname = dacstoscan[i];
    const double ival(tempCalibObject->scanValue(iname, state));
    scans1d[which][i]->Fill(ival);
    
    for (size_t j = i+1; j < dacstoscan.size(); ++j, ++k) {
      const std::string jname = dacstoscan[j];
      const double jval(tempCalibObject->scanValue(jname, state));
      scans2d[which][k]->Fill(ival, jval);
    }
  }
}
