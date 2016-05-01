#include <iomanip>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <cstdlib>

#include "PixelFECInterface/include/PixelPh1FECInterface.h"
#include "PixelFECInterface/include/TBMReadException.h"

using namespace std;
using namespace pos;

namespace {
  bool PRINT = false;
  bool PRINT_old = PRINT;
  void PRINT_ON() {
    PRINT_old = PRINT;
    PRINT = true;
  }
  void PRINT_RESTORE() {
    PRINT = PRINT_old;
  }

  void hexprintword(uint32_t w) {
    cout << hex
         << setw(2) << setfill('0') << ((w&0xFF000000)>>24) << " "
         << setw(2) << setfill('0') << ((w&0x00FF0000)>>16) << " "
         << setw(2) << setfill('0') << ((w&0x0000FF00)>> 8) << " "
         << setw(2) << setfill('0') << ((w&0x000000FF)    )
         << dec;
  }
}

//--------------------------------------------------------------------------
// Here comes the constructor -- with two versions, one for HAL, one for CAEN
PixelPh1FECInterface::PixelPh1FECInterface(RegManager * const RegManagerPtr,
                                     const int vmeslot,
                                     unsigned int fecCrate,
                                     unsigned int fecSlot): pRegManager(RegManagerPtr),
fecCrate_(fecCrate),
fecSlot_(fecSlot)
{
    //Constructor stuff
    if (PRINT) cout << "PixelPh1FECInterface: "  << "PixelPh1FECInterface Constructor" << endl;
    
    maxbuffersize_=1000;
    cout << "PixelPh1FECInterface::maxbuffersize_ set to only 1000!!! change this in future\n";
    fecdebug = 0;
    
    for (int tfecchannel=1;tfecchannel<=2;tfecchannel++) {
        for (int tmfec=0;tmfec<9;tmfec++) {
            qbufnsend[tmfec][tfecchannel]=0;
            qbufnerr[tmfec][tfecchannel]=0;
            qbufn[tmfec][tfecchannel]=0;
            qbufn_old[tmfec][tfecchannel]=0;
            memset(qbuf[tmfec][tfecchannel], 0, qbufsize);
        }
    }

    //disableinternalclk();
    switchclk(0xFF7CFFD8);

    PRINT_ON();
    unsigned long data;
    getversion(&data);
    getGeneral();
    PRINT_RESTORE();
    
}
//------------------------------------------------------------------------
PixelPh1FECInterface::~PixelPh1FECInterface(void)
{
    //Destructor stuff
}
//-------------------------------------------------------------------------
/* There are two versions of getversion() for both hal and CAEN (4 total)
 The form getversion(unsigned long *data) returns the integer 0..255
 version number of the mfec firmware that is loaded into an mfec, but
 assumes an mfec installed into position 1.  The
 getversion(const int mfec, unsigned long *data) form queries a particular
 mfec for the version (all mfecs on a VME FEC board will have the same
 mfec version sice it is loaded from a single eeprom.) */

unsigned PixelPh1FECInterface::getGeneral() {
  valword value = pRegManager->ReadReg("GenReg");
  if (PRINT) cout << "PixelPh1FECInterface: Get FEC general register: 0x" << hex << value.value() << dec << endl;
  return value.value();
}

int PixelPh1FECInterface::getversion(unsigned long *data) {
    valword value;
    value = pRegManager->ReadReg("GenReg.VERSIONM01");
    *data = value.value();
    if (PRINT) cout << "PixelPh1FECInterface: " <<"Get FEC version finds firmware version: "<<value.value()<<endl;
    return value;
}
//-----------------------------------------------------------------------
// Get the STATUS word of the FEC. Includes the QPLL/TTCrx ready bits
int PixelPh1FECInterface::getStatus(void) {
    valword value;
    value = pRegManager->ReadReg("STATUS");
    if (PRINT) cout << "PixelPh1FECInterface: " <<"Get FEC status "<<value.value()<<endl;
    return value;
}
//--------------------------------------------------------------------------
int PixelPh1FECInterface::getversion(const int mfec, unsigned long *data) {

    const string names[8] = {
        "VERSIONM01",
        "VERSIONM02",
        "VERSIONM03",
        "VERSIONM04",
        "VERSIONM05",
        "VERSIONM06",
        "VERSIONM07",
        "VERSIONM08"};
    
    
    valword value;
    value = pRegManager->ReadReg(names[mfec-1]);
    
    *data = value.value();
    
    //*data = (value & 0xFF000000) >> 24;
    
    if (PRINT) cout << "PixelPh1FECInterface: " <<"Get FEC version finds firmware version: "<<*data<<endl;
    return 0;
}


// Nik added these two function for GLIB/CTA board internal clk switch 

void PixelPh1FECInterface::switchclk(unsigned int clk_word){
  if (PRINT) cout << "PixelPh1FECInterface: "  << "switchclk 0x" << std::hex << clk_word << std::dec << std::endl;
  outputwordhal("CLKReg", clk_word);
}

void PixelPh1FECInterface::enableinternalclk(){
  if (PRINT) cout << "PixelPh1FECInterface: "  << "enableinternalclk" << std::endl;
  outputwordhal("CLKSWITCH.ENABLE", 0);
  //outputwordhal("ctrl.ttc_xpoint_A_out3", 3); 
}

void PixelPh1FECInterface::disableinternalclk(){
  outputwordhal("CLKSWITCH.DISABLE", 1);
  //outputwordhal("ctrl.ttc_xpoint_A_out3", 0);
}

//----------------------------------------------------------------------------
//
int PixelPh1FECInterface::writeCSregister(int mfec, int fecchannel,
                                       int cscommand) {
    //const string names[8] = {"CSREGM1","CSREGM2","CSREGM3","CSREGM4",
    //		   "CSREGM5","CSREGM6","CSREGM7","CSREGM8"};
    // expect the lower 8 bits of the CS register (shift for other channel)
    uint32_t value;
    uint32_t value2;
    int cscommand1, cscommand2;
    
    value = mfecCSregister[mfec];
    
    if (PRINT) {
        cout << "writeCSregister first reads: "<<hex<< value <<dec<<endl;
        if (value & 0x00000100) cout << "CH1 Receive Timeout"<<endl;
        if (value & 0x00000200) cout << "CH1 Receive Complete"<<endl;
        if (value & 0x00000400) cout << "CH1 Hub Address Error Timeout"<<endl;
        if (value & 0x00000800) cout << "CH1 Send Started"<<endl;
        if (value & 0x00001000) cout << "CH1 Receive Count Error"<<endl;
        if (value & 0x00002000) cout << "CH1 Receive Failure"<<endl;
        
        if (value & 0x01000000) cout << "CH2 Receive Timeout"<<endl;
        if (value & 0x02000000) cout << "CH2 Receive Complete"<<endl;
        if (value & 0x04000000) cout << "CH2 Hub Address Error Timeout"<<endl;
        if (value & 0x08000000) cout << "CH2 Send Started"<<endl;
        if (value & 0x10000000) cout << "CH2 Receive Count Error"<<endl;
        if (value & 0x20000000) cout << "CH2 Receive Failure"<<endl;
    }
    
    if (fecchannel == 1) {
        value = value & 0x00FF0000; // only preserve fec ch2 mode bits
        cscommand2 = cscommand;
        value = value | cscommand2;
    } else if (fecchannel == 2) {
        value = value & 0x000000FF; // only preserve fec ch1 mode bits
        cscommand1 = cscommand << 16;
        value = value | cscommand1;
    } else {
        //    value = value & 0x00000000; // toss the mode bits from both channels
        cscommand1 = cscommand << 16;
        value = cscommand | cscommand1;
    }
    
    value2 = value;
    switch (mfec) {
        case 1:
            pRegManager->WriteReg("CSReg.CSREGM1", value2);
            break;
        case 2:
            pRegManager->WriteReg("CSREGM2", value2);
            break;
        case 3:
            pRegManager->WriteReg("CSREGM3", value2);
            break;
        case 4:
            pRegManager->WriteReg("CSREGM4", value2);
            break;
        case 5:
            pRegManager->WriteReg("CSREGM5", value2);
            break;
        case 6:
            pRegManager->WriteReg("CSREGM6", value2);
            break;
        case 7:
            pRegManager->WriteReg("CSREGM7", value2);
            break;
        case 8:
            pRegManager->WriteReg("CSREGM8", value2);
            break;
    }
    
    
    if (PRINT) {
        cout << "writeCSregister puts      : "<<hex<< value2 <<dec<<endl;
    }
    return value;
}
//----------------------------------------------------------------

// To implement this later!!! Check with Russell!!!
void PixelPh1FECInterface::haltest(void) {
    unsigned long int data;
    data = 0x01020304;
    pRegManager->WriteReg("SOUT_BUF1M1", data);
      
    sleep(1);
/*
    sleep(1);
    data = 0x01020304;
    sleep(1);
    pRegManager->WriteReg("TESTREG1", data);
    data = 0x05060708;
    sleep(1);
    pRegManager->WriteReg("TESTREG1", data);
    data = 0x090a0b0c;
    sleep(1);
    pRegManager->WriteReg("TESTREG1", data);
    sleep(1);
  */  
}
//-------------------------------------------------------------------------------
// mfecbusy reads the csreg and waits if the send_started is still
// in effect.  This should change state on it's own.  Timeout used in
// case it doesnt change.
void PixelPh1FECInterface::mfecbusy(int mfec, int fecchannel,
                                 unsigned int *ch1, unsigned int *ch2) {
    
    //const string names[8] = {"CSREGM1","CSREGM2","CSREGM3","CSREGM4",
    //		   "CSREGM5","CSREGM6","CSREGM7","CSREGM8"};
    
    valword value;
    int timeout;
    // CSTIMEOUT of 300 is large enough for a mfec to shift out an entirepRegManager
    // rocinit (typical wait for small column wise op is 20)
    const int CSTIMEOUT = 300;
    
    switch (mfec) {
        case 1:
            value = pRegManager->ReadReg("CSReg.CSREGM1");
            break;
        case 2:
            value = pRegManager->ReadReg("CSREGM2");
            break;
        case 3:
            value = pRegManager->ReadReg("CSREGM3");
            break;
        case 4:
            value = pRegManager->ReadReg("CSREGM4");
            break;
        case 5:
            value = pRegManager->ReadReg("CSREGM5");
            break;
        case 6:
            value = pRegManager->ReadReg("CSREGM6");
            break;
        case 7:
            value = pRegManager->ReadReg("CSREGM7");
            break;
        case 8:
            value = pRegManager->ReadReg("CSREGM8");
            break;
    }
    
    *ch1 = (value.value() >> 8) & 0x7F;
    *ch2 = (value.value() >> 24) & 0x7F;
    
    timeout = 0;
    
    if ((fecchannel == 1)&&((*ch1 & 0x8)==0x8)&&(*ch1!=0)) { //send started -
        //wait until complete or timeout
        //      if (PRINT) {
        //cout << "<WAITING FOR READY (1)"<<hex<<value<<" "<<timeout<<dec<<" > ";
        //      }
        timeout = 0;
        do {
            switch (mfec) {
                case 1:
                    value = pRegManager->ReadReg("CSReg.CSREGM1");
                    break;
                case 2:
                    value = pRegManager->ReadReg("CSREGM2");
                    break;
                case 3:
                    value = pRegManager->ReadReg("CSREGM3");
                    break;
                case 4:
                    value = pRegManager->ReadReg("CSREGM4");
                    break;
                case 5:
                    value = pRegManager->ReadReg("CSREGM5");
                    break;
                case 6:
                    value = pRegManager->ReadReg("CSREGM6");
                    break;
                case 7:
                    value = pRegManager->ReadReg("CSREGM7");
                    break;
                case 8:
                    value = pRegManager->ReadReg("CSREGM8");
                    break;
            }
            
            *ch1 = (int) (value.value() >> 8) & 0x7F;
            timeout++;
            //    if (PRINT) {
            //cout << "<WAITING FOR READY "<<hex<<value<<" "<<timeout<<dec<< " > ";
            //      }
        } while (((*ch1 & 0x8) == 0x8) && (timeout < CSTIMEOUT));
        
        if (PRINT) cout << "PixelPh1FECInterface: "  << "CH1 timeout=" << dec << timeout <<endl;
        if (timeout>=CSTIMEOUT) { cout << "ERROR mfecbusy channel 1"<<endl; }
        
    }
    if ((fecchannel == 1) && (fecdebug > 0)) {
        if (((*ch1 & 0x02) != 0x02) && (*ch1 != 0))  {
            analyzeFecCSRChan(mfec, 1, *ch1);
            if (fecdebug == 2) writeCSregister(mfec, fecchannel, 0x08);
        }
    }
    
    if ((fecchannel == 2)&&((*ch2 & 0x8)==0x8)&&(*ch2!=0)) { //send started -
        //wait until complete or timeout
        //    if (PRINT) {
        //cout << "<WAITING FOR READY "<<hex<<value<<" "<<timeout<<dec<< " > ";
        //    }
        timeout = 0;
        do {
            switch (mfec) {
                case 1:
                    value = pRegManager->ReadReg("CSReg.CSREGM1");
                    break;
                case 2:
                    value = pRegManager->ReadReg("CSREGM2");
                    break;
                case 3:
                    value = pRegManager->ReadReg("CSREGM3");
                    break;
                case 4:
                    value = pRegManager->ReadReg("CSREGM4");
                    break;
                case 5:
                    value = pRegManager->ReadReg("CSREGM5");
                    break;
                case 6:
                    value = pRegManager->ReadReg("CSREGM6");
                    break;
                case 7:
                    value = pRegManager->ReadReg("CSREGM7");
                    break;
                case 8:
                    value = pRegManager->ReadReg("CSREGM8");
                    break;
            }
            *ch2 = (int) (value.value() >> 24) & 0x7F;
            timeout++;
            //      if (PRINT) {
            //cout << "<WAITING FOR READY "<<hex<<value<<" "<<timeout<<dec<< " > ";
            //      }
        } while (((*ch2 & 0x8) == 0x8) && (timeout < CSTIMEOUT));
        
        if (PRINT) cout << "PixelPh1FECInterface: "  << "CH2 timeout=" << dec << timeout <<endl;
        if (timeout>=CSTIMEOUT) { cout << "ERROR mfecbusy channel 2"<<endl; }
        
    }
    if ((fecchannel == 2) && (fecdebug > 0)) {
        if (((*ch2 & 0x02) != 0x02) && (*ch2 != 0)) {
            analyzeFecCSRChan(mfec, 2, *ch2);
            if (fecdebug == 2) writeCSregister(mfec, fecchannel, 0x08);
        }
    }
    
    mfecCSregister[mfec] = value.value();
}
//--------------------------------------------------------------------------------
// Finds out if *previous* transmission was received okay.
// Reads the mfec combined control/status register which returns and sets
// functions of ch1 and ch2 independently
int PixelPh1FECInterface::getfecctrlstatus(const int mfec, unsigned long *data) {
    //const string names[8] = {"CSREGM1","CSREGM2","CSREGM3","CSREGM4",
    //		   "CSREGM5","CSREGM6","CSREGM7","CSREGM8"};
    
    valword value;
    switch (mfec) {
        case 1:
            value = pRegManager->ReadReg("CSReg.CSREGM1");
            break;
        case 2:
            value = pRegManager->ReadReg("CSREGM2");
            break;
        case 3:
            value = pRegManager->ReadReg("CSREGM3");
            break;
        case 4:
            value = pRegManager->ReadReg("CSREGM4");
            break;
        case 5:
            value = pRegManager->ReadReg("CSREGM5");
            break;
        case 6:
            value = pRegManager->ReadReg("CSREGM6");
            break;
        case 7:
            value = pRegManager->ReadReg("CSREGM7");
            break;
        case 8:
            value = pRegManager->ReadReg("CSREGM8");
            break;
    }
    *data = value.value();
    if (PRINT) cout << "PixelPh1FECInterface: "  << "Getting FEC cntrstatus register: 0x" << hex << *data << dec <<endl;
    return 0;
}
//----------------------------------------------------------------------------------
//// output one word in HAL single mode
void PixelPh1FECInterface::outputwordhal(const char *halname, unsigned int data) {
  pRegManager->WriteReg(halname, data);
  if (PRINT) {
    valword value =  pRegManager->ReadReg(halname);	
    cout <<"PixelPh1FECInterface: outputwordhal " << halname << " 0x" << hex << data << " readback 0x" << value;
    if (value.value() != data)
      cout << "NOT OK";
    cout << endl;
  }
}

void PixelPh1FECInterface::outputblock(const int mfec, const int fecchannel, std::vector<uint32_t> wordcont) {
 
    const string names[2][1] = {
        {"BOUT_BUF1M1"},
        {"BOUT_BUF2M1"} };
    
    // implement throw exception for the cases when mfec >/< bla and channel >/< bla
    //std::cout << "name " << names[fecchannel-1][mfec-1] << " size of the word vector " << wordcont.size()<< std::endl;
    pRegManager->WriteBlockReg( names[fecchannel-1][mfec-1], wordcont);
}


int PixelPh1FECInterface::resetdoh(const int mfec, const int fecchannel) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "resetdoh(" << mfec << ", " << fecchannel << ")" << endl;
    // fecchannel determines if value will be shifted on CS register word
    writeCSregister(mfec, fecchannel, 0x8000);
    return 0;
}

int PixelPh1FECInterface::injectrstroc(const int mfec, const int bitstate) {
  disableexttrigger(mfec, 1); // JMTBAD to be taken out if FW changes
  
  if (PRINT) cout << "PixelPh1FECInterface: "  << "injectrstroc(" << mfec << ", " << bitstate << ")" << endl;
    //const string names[8] = {"INRSTROCM1","INRSTROCM2","INRSTROCM3","INRSTROCM4",
    //		             "INRSTROCM5","INRSTROCM6","INRSTROCM7","INRSTROCM8"};
    switch (mfec)
    {
        case 1:   outputwordhal("GenReg.INRSTROCM1", bitstate); break;
        case 2:   outputwordhal("INRSTROCM2", bitstate); break;
        case 3:   outputwordhal("INRSTROCM3", bitstate); break;
        case 4:   outputwordhal("INRSTROCM4", bitstate); break;
        case 5:   outputwordhal("INRSTROCM5", bitstate); break;
        case 6:   outputwordhal("INRSTROCM6", bitstate); break;
        case 7:   outputwordhal("INRSTROCM7", bitstate); break;
        case 8:   outputwordhal("INRSTROCM8", bitstate); break;
    }

    disableexttrigger(mfec, 0);
    return 0;
}
int PixelPh1FECInterface::injecttrigger(const int mfec, const int bitstate) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "injecttrigger(" << mfec << ", " << bitstate << ")" << endl;
    switch (mfec)
    {
        case 1:   outputwordhal("GenReg.INTRIGM1", bitstate); break;
        case 2:   outputwordhal("INTRIGM2", bitstate); break;
        case 3:   outputwordhal("INTRIGM3", bitstate); break;
        case 4:   outputwordhal("INTRIGM4", bitstate); break;
        case 5:   outputwordhal("INTRIGM5", bitstate); break;
        case 6:   outputwordhal("INTRIGM6", bitstate); break;
        case 7:   outputwordhal("INTRIGM7", bitstate); break;
        case 8:   outputwordhal("INTRIGM8", bitstate); break;
    }
    return 0;
}
int PixelPh1FECInterface::injectrsttbm(const int mfec, const int bitstate) {
  disableexttrigger(mfec, 1); // JMTBAD to be taken out if FW chang 
  if (PRINT) cout << "PixelPh1FECInterface: "  << "injectrsttbm(" << mfec << ", " << bitstate << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.INRSTTBMM1", bitstate); break;
        case 2:  outputwordhal("INRSTTBMM2", bitstate); break;
        case 3:  outputwordhal("INRSTTBMM3", bitstate); break;
        case 4:  outputwordhal("INRSTTBMM4", bitstate); break;
        case 5:  outputwordhal("INRSTTBMM5", bitstate); break;
        case 6:  outputwordhal("INRSTTBMM6", bitstate); break;
        case 7:  outputwordhal("INRSTTBMM7", bitstate); break;
        case 8:  outputwordhal("INRSTTBMM8", bitstate); break;
    }
    disableexttrigger(mfec, 0);
    return 0;
}
int PixelPh1FECInterface::injectrstcsr(const int mfec, const int bitstate) {
  disableexttrigger(mfec, 1); // JMTBAD to be taken out if FW chang 
  if (PRINT) cout << "PixelPh1FECInterface: "  << "injectrstcsr(" << mfec << ", " << bitstate << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.INRSTCSRM1", bitstate); break;
        case 2:  outputwordhal("INRSTCSRM2", bitstate); break;
        case 3:  outputwordhal("INRSTCSRM3", bitstate); break;
        case 4:  outputwordhal("INRSTCSRM4", bitstate); break;
        case 5:  outputwordhal("INRSTCSRM5", bitstate); break;
        case 6:  outputwordhal("INRSTCSRM6", bitstate); break;
        case 7:  outputwordhal("INRSTCSRM7", bitstate); break;
        case 8:  outputwordhal("INRSTCSRM8", bitstate); break;
    }
    disableexttrigger(mfec, 0); // JMTBAD to be taken out if FW chang 
    return 0;
}
int PixelPh1FECInterface::enablecallatency(const int mfec, const int bitstate) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "enablecallatency(" << mfec << ", " << bitstate << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.ENCALLATENCYM1", bitstate); break;
        case 2:  outputwordhal("ENCALLATENCYM2", bitstate); break;
        case 3:  outputwordhal("ENCALLATENCYM3", bitstate); break;
        case 4:  outputwordhal("ENCALLATENCYM4", bitstate); break;
        case 5:  outputwordhal("ENCALLATENCYM5", bitstate); break;
        case 6:  outputwordhal("ENCALLATENCYM6", bitstate); break;
        case 7:  outputwordhal("ENCALLATENCYM7", bitstate); break;
        case 8:  outputwordhal("ENCALLATENCYM8", bitstate); break;
    }
    return 0;
}
int PixelPh1FECInterface::disableexttrigger(const int mfec, const int bitstate) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "disableexttrigger(" << mfec << ", " << bitstate << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.DISEXTTRIGM1", bitstate); break;
        case 2:  outputwordhal("DISEXTTRIGM2", bitstate); break;
        case 3:  outputwordhal("DISEXTTRIGM3", bitstate); break;
        case 4:  outputwordhal("DISEXTTRIGM4", bitstate); break;
        case 5:  outputwordhal("DISEXTTRIGM5", bitstate); break;
        case 6:  outputwordhal("DISEXTTRIGM6", bitstate); break;
        case 7:  outputwordhal("DISEXTTRIGM7", bitstate); break;
        case 8:  outputwordhal("DISEXTTRIGM8", bitstate); break;
    }
    return 0;
}
int PixelPh1FECInterface::loopnormtrigger(const int mfec, const int bitstate) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "loopnormtrigger(" << mfec << ", " << bitstate << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.LOOPNORMTRIGM1", bitstate); break;
        case 2:  outputwordhal("LOOPNORMTRIGM2", bitstate); break;
        case 3:  outputwordhal("LOOPNORMTRIGM3", bitstate); break;
        case 4:  outputwordhal("LOOPNORMTRIGM4", bitstate); break;
        case 5:  outputwordhal("LOOPNORMTRIGM5", bitstate); break;
        case 6:  outputwordhal("LOOPNORMTRIGM6", bitstate); break;
        case 7:  outputwordhal("LOOPNORMTRIGM7", bitstate); break;
        case 8:  outputwordhal("LOOPNORMTRIGM8", bitstate); break;
    }
    return 0;
}
int PixelPh1FECInterface::loopcaltrigger(const int mfec, const int bitstate) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "loopcaltrigger(" << mfec << ", " << bitstate << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.LOOPCALTRIGM1", bitstate); break;
        case 2:  outputwordhal("LOOPCALTRIGM2", bitstate); break;
        case 3:  outputwordhal("LOOPCALTRIGM3", bitstate); break;
        case 4:  outputwordhal("LOOPCALTRIGM4", bitstate); break;
        case 5:  outputwordhal("LOOPCALTRIGM5", bitstate); break;
        case 6:  outputwordhal("LOOPCALTRIGM6", bitstate); break;
        case 7:  outputwordhal("LOOPCALTRIGM7", bitstate); break;
        case 8:  outputwordhal("LOOPCALTRIGM8", bitstate); break;
    }
    return 0;
}
int PixelPh1FECInterface::callatencycount(const int mfec, const int latency) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "callatencycount(" << mfec << ", " << latency << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.CALLATCNTM1", latency); break;
        case 2:  outputwordhal("CALLATCNTM2", latency); break;
        case 3:  outputwordhal("CALLATCNTM3", latency); break;
        case 4:  outputwordhal("CALLATCNTM4", latency); break;
        case 5:  outputwordhal("CALLATCNTM5", latency); break;
        case 6:  outputwordhal("CALLATCNTM6", latency); break;
        case 7:  outputwordhal("CALLATCNTM7", latency); break;
        case 8:  outputwordhal("CALLATCNTM8", latency); break;
    }
    return 0;
}
int PixelPh1FECInterface::FullBufRDaDisable(const int mfec, const int disable) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "FullBufRDaDisable(" << mfec << ", " << disable << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.DISRDACHECKM1", disable); break;
        case 2:  outputwordhal("DISRDACHECKM2", disable); break;
        case 3:  outputwordhal("DISRDACHECKM3", disable); break;
        case 4:  outputwordhal("DISRDACHECKM4", disable); break;
        case 5:  outputwordhal("DISRDACHECKM5", disable); break;
        case 6:  outputwordhal("DISRDACHECKM6", disable); break;
        case 7:  outputwordhal("DISRDACHECKM7", disable); break;
        case 8:  outputwordhal("DISRDACHECKM8", disable); break;
    }
    return 0;
}
int PixelPh1FECInterface::AllRDaDisable(const int mfec, const int disable) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "AllRDaDisable(" << mfec << ", " << disable << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.DISRDAM1", disable); break;
        case 2:  outputwordhal("DISRDAM2", disable); break;
        case 3:  outputwordhal("DISRDAM3", disable); break;
        case 4:  outputwordhal("DISRDAM4", disable); break;
        case 5:  outputwordhal("DISRDAM5", disable); break;
        case 6:  outputwordhal("DISRDAM6", disable); break;
        case 7:  outputwordhal("DISRDAM7", disable); break;
        case 8:  outputwordhal("DISRDAM8", disable); break;
    }
    return 0;
}

int PixelPh1FECInterface::testFiberEnable(const int mfec, const int enable) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "testFiberEnable(" << mfec << ", " << enable << ")" << endl;
    switch (mfec)
    {
        case 1:  outputwordhal("GenReg.TESTFIBERM1", enable); break;
        case 2:  outputwordhal("TESTFIBERM2", enable); break;
        case 3:  outputwordhal("TESTFIBERM3", enable); break;
        case 4:  outputwordhal("TESTFIBERM4", enable); break;
        case 5:  outputwordhal("TESTFIBERM5", enable); break;
        case 6:  outputwordhal("TESTFIBERM6", enable); break;
        case 7:  outputwordhal("TESTFIBERM7", enable); break;
        case 8:  outputwordhal("TESTFIBERM8", enable); break;
    }
    return 0;
}

//-------------------------------------------------------------------------------------
// To read back the information from the mFEC input FIFOs
// Return the word from the FIFO
int PixelPh1FECInterface::readback(const int mfec, const int channel) {

    const string names[2][8] = {
        {"InFIFOCh1.INP_BUF1M1","INP_BUF1M2","INP_BUF1M3","INP_BUF1M4",
         "INP_BUF1M5","INP_BUF1M6","INP_BUF1M7","INP_BUF1M8"},
        {"InFIFOCh2.INP_BUF2M1","INP_BUF2M2","INP_BUF2M3","INP_BUF2M4",
         "INP_BUF2M5","INP_BUF2M6","INP_BUF2M7","INP_BUF2M8"} };
    
    
    if(mfec<1 || mfec>8) {
        cout<<" PixelPh1FECInterface: Wrong mfec number "<<mfec<<endl;
        return 3;
    }
    if(channel<1 || channel>2) {
        cout<<" PixelPh1FECInterface: Wrong mfec channel number "<<channel<<endl;
        return 2;
    }
    
    valword value = pRegManager->ReadReg(names[channel-1][mfec-1]);
    if (PRINT) cout << "PixelPh1FECInterface: "  << "Getting FIFO readback register: 0x" << std::hex << value << std::dec <<endl;

    return (int) value.value();
}
//---------------------------------------------------------------------------------
// To read the mfec word with the HUB and byte COUNT
// Read data is returned in *data. Return same data..
// byte defines the offset to access the corrvecect information
// byte = 0 - transmitted HUB address
// byte = 1 - received HUB address
// byte = 2 - transmitted byte COUNT
// byte = 3 - received byte COUNT
// byte = 4 - return the full word
int PixelPh1FECInterface::getByteHubCount(const int mfec, const int channel,
                                       const int byte, int * data) {
    const string names[2][8] = {
        {"StatCh1.STAT1_M1","STAT1_M2","STAT1_M3","STAT1_M4",
         "STAT1_M5","STAT1_M6","STAT1_M7","STAT1_M8"},
        {"StatCh2.STAT2_M1","STAT2_M2","STAT2_M3","STAT2_M4",
         "STAT2_M5","STAT2_M6","STAT2_M7","STAT2_M8"} };
    
    valword value;
    int ret = 0;
    if (PRINT) cout << "PixelPh1FECInterface: byte + hub count:\n";
    
    if(mfec<1 || mfec>8) {
        cout<<" PixelPh1FECInterface: Wrong mfec number "<<mfec<<endl;
        return 3;
    }
    if(channel<1 || channel>2) {
        cout<<" PixelPh1FECInterface: Wrong mfec channel number "<<channel<<endl;
        return 2;
    }

    value = pRegManager->ReadReg(names[channel-1][mfec-1]);
    if (PRINT) cout << hex << "0x" << value << dec << endl;
    if(byte<0||byte>4) {*data=0; ret=1;}   // signal out of range, return 0
    else if(byte==4) {*data = value.value();}      // for 4 return the whole register
    else {*data = (int) (value.value()  >> (8*byte)) & 0xFF;} // for 0-3 return a byte
    
    return ret;
}

// Set the debug flag

void PixelPh1FECInterface::fecDebug(int newstate) {
    if ((fecdebug == 0) && (newstate > 0))
        cout << "\nSetting FECDEBUG to ON (" << newstate << ")" << endl;
    if ((fecdebug > 0) && (newstate == 0))
        cout << "\nSetting FECDEBUG to OFF (" << newstate << ")" << endl;
    fecdebug = newstate;
}
//------------------------------------------------------------------------------
//
unsigned char PixelPh1FECInterface::cinttogray(unsigned int igray) {
    // cintogray
    if (PRINT) cout << "PixelPh1FECInterface: " <<"CINTTOGRAY "<<igray<<" -> "<<(igray^(igray>>1))<<endl;;
    return (igray^(igray>>1));
}
//------------------------------------------------------------------------------
// Analyze the CSR register, print errors.
void PixelPh1FECInterface::analyzeFecCSRChan(int mFEC, int ichan, unsigned int ival) {
    if (fecdebug == 1) cout << "Previous fec command FAILED ";
    else if (fecdebug == 2) cout << "Current fec command FAILED ";
    cout << "CSR Register:0x" << hex <<ival<<dec<<endl;
    if ((ival & 0x01) == 0x01) cout <<"slot="<<fecSlot_<<" mFEC="<<mFEC<<" CH="<<ichan<<" Receive timeout"<<endl;
    if ((ival & 0x02) == 0x02) cout <<"slot="<<fecSlot_<<" mFEC="<<mFEC<<" CH="<<ichan<<" Receive complete"<<endl;
    if ((ival & 0x04) == 0x04) cout <<"slot="<<fecSlot_<<" mFEC="<<mFEC<<" CH="<<ichan<<" HubAddress error"<<endl;
    if ((ival & 0x08) == 0x08) cout <<"slot="<<fecSlot_<<" mFEC="<<mFEC<<" CH="<<ichan<<" Send started but not finished"<<endl;
    if ((ival & 0x10) == 0x10) cout <<"slot="<<fecSlot_<<" mFEC="<<mFEC<<" CH="<<ichan<<" Receive count error"<<endl;
    if ((ival & 0x20) == 0x20) cout <<"slot="<<fecSlot_<<" mFEC="<<mFEC<<" CH="<<ichan<<" Receive error"<<endl;
    if ((ival & 0x40) == 0x40) cout <<"slot="<<fecSlot_<<" mFEC="<<mFEC<<" CH="<<ichan<<" Stop Failure"<<endl;
}
//-----------------------------------------------------------------------------
// Send buffered data for all channels
int PixelPh1FECInterface::qbufsend(void) {
    int tmfec, tfecchannel;
    for (tfecchannel=1;tfecchannel<=2;tfecchannel++) {
        for (tmfec=0;tmfec<9;tmfec++) {
            if (qbufn[tmfec][tfecchannel] > 0) qbufsend(tmfec, tfecchannel);
        }
    }
    return 0;
}


//--------------------------------------------------------------------------------
// Send buffered data for one channel
int PixelPh1FECInterface::qbufsend(int mfec, int fecchannel) {
    unsigned int ch1stat, ch2stat;
    
    // append a final FF to data
    qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0xff;
    
    //  cout << "\nqbufsend writing "<<qbufn[mfec][fecchannel]<<" bytes\n";
    
    qbufnsend[mfec][fecchannel]++;

    if (PRINT) cout << "PixelPh1FECInterface: "  << "qbufsend(" << mfec << ", " << fecchannel << "), nsend = " << qbufnsend[mfec][fecchannel] << endl;
    
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); //must make sure channel is not busy, wait if necessary!!!!!!
    // FIXME could be more efficient - e.g. go to another channel
    
    if (fecdebug==1) {
        
        if (((fecchannel == 1)&&((ch1stat & 0x02) != 0x02) && (ch1stat != 0))||
            ((fecchannel == 2)&&((ch2stat & 0x02) != 0x02) && (ch2stat != 0))){
            
            qbufnerr[mfec][fecchannel]++;
            
            static unsigned int count=0;
            
            if (count < 50000) {
                
                static ofstream dmp("feccmd.dmp");
                
                count++;
                
                cout << "%Found channel with error: slot="<<fecSlot_<<" "<<mfec<<" "
                << fecchannel<<" sent "<<qbufnsend[mfec][fecchannel]
                << " commands and had "<<qbufnerr[mfec][fecchannel]
                << " errors."<<endl;
                
                dmp << (int)qbufn_old[mfec][fecchannel]-1;
                for(int i=0;i<qbufn_old[mfec][fecchannel]-1;i++){
                    dmp << " " << (int) qbuf_old[mfec][fecchannel][i];
                }
                dmp << endl;
                
                cout << mfec << " " << fecchannel << " " << (int)qbufn_old[mfec][fecchannel];
                for(int i=0;i<qbufn_old[mfec][fecchannel];i++){
                    cout << " " << (int) qbuf_old[mfec][fecchannel][i];
                }
                
                cout << endl;
            }
        }
    }
    
    qbufn_old[mfec][fecchannel]=qbufn[mfec][fecchannel];
    
    for(int i=0;i<qbufn[mfec][fecchannel];i++){
        qbuf_old[mfec][fecchannel][i]=qbuf[mfec][fecchannel][i];
    }
    
    
    // Now data is ready.  Ready to initiate tx.  Reset and send go.
    // Reset the appropriate mfec channel
    //  writeCSregister(mfec, fecchannel, 0x08);
    // enable arm auto send reset mFEC
    writeCSregister(mfec, fecchannel, 0x88);
    //cout << "after 88: " << hex << pRegManager->ReadReg("CSReg.CSREGM1") << dec << endl;

    std::vector<uint32_t> wordvec;
    unsigned int *iword;
    int i;
    // Now load the data to the word container
    if (PRINT) cout<<"PixelPh1FECInterface: Final FEC data (ndata: " << qbufn[mfec][fecchannel] << ")\n";
    for (i=0;i<qbufn[mfec][fecchannel];i+=4) {
        iword = (unsigned int*) &qbuf[mfec][fecchannel][i];
        wordvec.push_back( *iword );
        if (PRINT) {
          cout<<"PixelPh1FECInterface:    ("<<setw(2)<<i<<"): ";
          hexprintword(*iword);
          cout << endl;
        }
    }
    
    outputblock(mfec, fecchannel, wordvec);
    
    // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
    writeCSregister(mfec, fecchannel, 0x87);
    //cout << "after 87: 0x" << hex << pRegManager->ReadReg("CSReg.CSREGM1") << dec << endl;
    //mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); //must make sure channel is not busy, wait if necessary!!!!!!
    //unsigned long csreg;
    //getfecctrlstatus(mfec,&csreg);
    //cout << "csreg direct read 0x" << hex << csreg << " from mfecbusy 0x" << mfecCSregister[mfec] << dec << endl;


    qbufn[mfec][fecchannel] = 0;
    memset(qbuf[mfec][fecchannel], 0, qbufsize); // JMTBAD for good measure to be sure new FEC doesn't pick up stuff after the 0xFF...
    
    return 0;
}
//-----------------------------------------------------------------------------------------
// Set the WBC + reset
int PixelPh1FECInterface::rocsetwbc(int mfec, int mfecchannel, int tbmchannel,
                                 int hubaddress, int portaddress, int rocid,
                                 int wbcvalue) {
    
  if (PRINT) cout << "PixelPh1FECInterface: "  << std::dec << "rocsetwbc("
		  << "mfec=" << mfec << ", "
		  << "mfecchannel=" << mfecchannel << ", "
		  << "tbmchannel=" << tbmchannel << ", "
		  << "hubaddress=" << hubaddress << ", "
		  << "portaddress=" << portaddress << ", "
		  << "rocid=" << rocid << ", "
		  << "wbcvalue=" << wbcvalue << std::endl;

    // Clear the buffer if not in buffer mode)
    if (qbufn[mfec][mfecchannel] > 0)  {
        qbufsend(mfec,mfecchannel);
        cout << "mfec " << mfec <<":"<<mfecchannel<<" leftover from buffer mode "
        <<qbufn[mfec][mfecchannel]<<endl;
    }
    
    progdac(mfec, mfecchannel, hubaddress, portaddress, rocid, 254, wbcvalue);
    rocreset(mfec, mfecchannel, tbmchannel, hubaddress);  // need rocreset here
    return 0;
}
//-----------------------------------------------------------------------------
// Set the control register
int PixelPh1FECInterface::rocsetchipcontrolregister(int mfec, int mfecchannel,
                                                 int hubaddress,
                                                 int portaddress,
                                                 int rocid,
                                                 int calhighrange,
                                                 int chipdisable,
                                                 int halfspeed,
                                                 const bool buffermode) {

  if (PRINT) cout << "PixelPh1FECInterface: "  << std::dec << "rocsetchipcontrolregister("
		  << "mfec=" << mfec << ", "
		  << "mfecchannel=" << mfecchannel << ", "
		  << "hubaddress=" << hubaddress << ", "
		  << "portaddress=" << portaddress << ", "
		  << "rocid=" << rocid << ", "
		  << "calhighrange=" << calhighrange << ", "
		  << "chipdisable=" << chipdisable << ", "
		  << "halfspeed=" << halfspeed << ", "
		  << "buffermode=" << buffermode
		  << ")" << std::endl;

    if (halfspeed)
        printf("JMT PixelPh1FECInterface::rocsetchipcontrolregister halfspeed is %i and it's not used any more\n", halfspeed);

    unsigned char mydata;
    mydata = 0;
    if (halfspeed > 0) mydata |= 0x01;
    if (chipdisable > 0) mydata |= 0x02;
    if (calhighrange > 0) mydata |= 0x04;
    
    // Clear the buffer if not in buffer mode)
    if(!buffermode) {
        if (qbufn[mfec][mfecchannel] > 0)  {
            qbufsend(mfec,mfecchannel);
            cout << "mfec " << mfec <<":"<<mfecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][mfecchannel]<<endl;
        }
    }
    
    progdac(mfec, mfecchannel, hubaddress, portaddress, rocid, 253, mydata, buffermode);
    
    return 0;
}
//-----------------------------------------------------------------------------------
// Reset TBM
int PixelPh1FECInterface::tbmreset(int mfec, int fecchannel, int tbmchannel,
                                int hubaddress) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "tbmreset: ";
  return tbmcmd(mfec, fecchannel, tbmchannel, hubaddress, 4, 2, 16, 0);
}
// Reset ROC
int PixelPh1FECInterface::rocreset(int mfec, int fecchannel, int tbmchannel,
                                int hubaddress) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "rocreset: ";
    return tbmcmd(mfec, fecchannel, tbmchannel, hubaddress, 4, 2, 4, 0);
}
//
//--------------------------------------------------------------------------------------
// Clear calibrate, works for the whole ROC
// Single of buffered command.
int PixelPh1FECInterface::clrcal(int mfec, int fecchannel,
                              int hubaddress, int portaddress, int rocid,
                              bool buffermode) {

    unsigned int *iword;
    unsigned char txdata[1024];
    unsigned int ch1stat, ch2stat;
    int current, i, ndata;
    std::vector<uint32_t>  wordvec;
    
    
    if (buffermode) {
        
        if (PRINT) cout << "PixelPh1FECInterface: "  << "Buffer mode clrcal"<<endl;
        
        // Check that there is nothing for the buffer mode?
        if (qbufn[mfec][fecchannel] >= maxbuffersize_)  {
            qbufsend(mfec,fecchannel);
        }
        assert(qbufn[mfec][fecchannel] < maxbuffersize_);
        
        // If the hub and port are different, resend the new hub and port
        //  if (!((qbuflasthub[mfec][fecchannel] == hubaddress) &&
        //       (qbuflastport[mfec][fecchannel] == portaddress)) ) {
        // First are the hub (5 bits) and port (3 bits)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] =
        (hubaddress << 3) | portaddress ;
        //      qbuflasthub[mfec][fecchannel] = hubaddress;
        //      qbuflastport[mfec][fecchannel] = portaddress;
        //  }
        
        // Now send the command length to follow, NUM
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 1;  // 1 for clrcal
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = (rocid << 4) | (0x01);
        
        // terminate with 0
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0;
        //  txdata[current++] = 0xFF; /* added ff to  match cosmo - remove */
        
        //  cout << "QBUF length is " << qbufn[mfec][fecchannel] << " for mfec:" << dec << mfec << " and fecchannel:" << fecchannel <<endl;
        return 0;
        
    } else { // direct mode
        
        if (PRINT) cout << "PixelPh1FECInterface: "  << "Direct mode clrcal"<<endl;
        
        if (qbufn[mfec][fecchannel] > 0)  {
            qbufsend(mfec,fecchannel);
            cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode"<<endl;
        }
        if (PRINT) cout << "PixelPh1FECInterface: "  << "CLRCAL ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"
            <<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<endl;
        
        mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        current = 0;
        // First are the hub (5 bits) and port (3 bits)
        txdata[current++] = (hubaddress << 3) | portaddress ;
        
        // Now send the command length to follow, NUM
        txdata[current++] = 1;  // 1 for clrcal
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        txdata[current++] = (rocid << 4) | (0x01);
        
        // terminate with 0
        txdata[current++] = 0;
        //  txdata[current++] = 0xFF; /* added ff to  match cosmo - remove */
        
        if (PRINT) {
            for (i=0;i<current;i++) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
            cout <<endl;
        }
        
        // add 3 zeros to pad remainder of 4 byte word
        ndata = current;
        txdata[current++] = 0;
        txdata[current++] = 0;
        txdata[current++] = 0;
        
        
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        // Reset the appropriate mfec channel
        writeCSregister(mfec, fecchannel, 0x08);
        
        // Now load the data to the word container
        for (i=0;i<ndata;i+=4) {
            iword = (unsigned int*) &txdata[i];
            wordvec.push_back( *iword );
            if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword  <<dec<<endl;
        }
        
        outputblock(mfec, fecchannel, wordvec);
        
        
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        writeCSregister(mfec, fecchannel, 0x07);
        
        if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        return 0;
    }
}
//-----------------------------------------------------------------------------------------
// Program 1 pixel, separate trim&mask
int PixelPh1FECInterface::progpix1(int mfec, int fecchannel,
                                int hubaddress, int portaddress, int rocid,
                                int coladdr, int rowaddress,
                                int mask, int trim,
                                bool buffermode) {
    // set trims on a pixel
    //const bool PRINT=true;

    unsigned int *iword;
    unsigned char coltemp0, coltemp1, coltemp2, tmask, ttrim, databyte;
    unsigned char txdata[1024];
    unsigned int ch1stat, ch2stat;
    int current, i, ndata;
    std::vector<uint32_t>  wordvec;
    
    if (buffermode) {
        
        if (PRINT) cout << "PixelPh1FECInterface: " <<"progpix1 buffered mask and trim"<<endl;
        if (PRINT) cout << "PixelPh1FECInterface: "  << "PROGPIX1 ROC CMD: mfec:"<<dec<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" trim:"<<trim<<endl;
        
        assert(0<=coladdr);assert(coladdr<=51);assert(0<=rowaddress);assert(rowaddress<=79);
        
        if (qbufn[mfec][fecchannel] >= maxbuffersize_)  {
            //cout << "ERROR mfec " << mfec <<":"<<fecchannel<<" OVER BUFFER LIMIT"<<endl;
            qbufsend(mfec,fecchannel);
        }
        assert(qbufn[mfec][fecchannel] < maxbuffersize_);
        
        // If the hub and port are different, resend the new hub and port
        //  if (!((qbuflasthub[mfec][fecchannel] == hubaddress) &&
        //       (qbuflastport[mfec][fecchannel] == portaddress)) ) {
        // First are the hub (5 bits) and port (3 bits)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] =
        (hubaddress << 3) | portaddress ;
        //    qbuflasthub[mfec][fecchannel] = hubaddress;
        //    qbuflastport[mfec][fecchannel] = portaddress;
        //  }
        
        // Now send the command length to follow, NUM
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 4;  // 4 for progpix
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = (rocid << 4) | (0x04);
        
        // First calculate the ColAdr
        coltemp0 = coladdr;
        coltemp1 = coladdr / 2;
        coltemp1 = 2*(cinttogray(coltemp1));
        coltemp2 = coltemp1 | (coltemp0 & 0x01);
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = coltemp2;
        
        // Now calculate the row addr
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = cinttogray(rowaddress);
        
        // Now send the mask and trim
        tmask = (mask & 0x01) << 7;
        ttrim = trim & 0x0F;
        databyte = tmask | ttrim;
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = databyte;
        
        // terminate with 0
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0;
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        return 0;
    } else { // direct mode
        if (PRINT) cout << "PixelPh1FECInterface: " <<"progpix1 direct mask and trim"<<endl;
        if (PRINT) cout << "PixelPh1FECInterface: "  << "PROGPIX1 ROC CMD: mfec:"<<dec<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" trim:"<<trim<<" mask:"<<mask<<endl;
        
        // Check that there is nothing for the buffer mode?
        if (qbufn[mfec][fecchannel] > 0)  {
            qbufsend(mfec,fecchannel);
            cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][fecchannel]<<endl;
        }
        
        
        assert(0<=coladdr);
        assert(coladdr<=51);
        
        assert(0<=rowaddress);
        assert(rowaddress<=79);
        
        mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        current = 0;
        // First are the hub (5 bits) and port (3 bits)
        txdata[current++] = (hubaddress << 3) | portaddress ;
        
        // Now send the command length to follow, NUM
        txdata[current++] = 4;  // 4 for Prog_Pix
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        txdata[current++] = (rocid << 4) | (0x04);
        
        // First calculate the ColAdr
        coltemp0 = coladdr;
        coltemp1 = coladdr / 2;
        coltemp1 = 2*(cinttogray(coltemp1));
        coltemp2 = coltemp1 | (coltemp0 & 0x01);
        txdata[current++] = coltemp2;
        
        // Now calculate the row addr
        txdata[current++] = cinttogray(rowaddress);
        
        // Now send the mask and trim
        tmask = (mask & 0x01) << 7;
        ttrim = trim & 0x0F;
        databyte = tmask | ttrim;
        txdata[current++] = databyte;
        
        // terminate with 0
        txdata[current++] = 0;
        txdata[current++] = 0xFF;
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        // Reset the appropriate mfec channel
        writeCSregister(mfec, fecchannel, 0x08);
        
        for (i=0;i<current;i++) {
            if (PRINT) cout << "PixelPh1FECInterface: " <<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        }
        // add 3 zeros to pad remainder of 4 byte word
        ndata = current;
        txdata[current++] = 0;
        txdata[current++] = 0;
        txdata[current++] = 0;
        
        if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;
        
        // Now load the data to the word container
        for (i=0;i<ndata;i+=4) {
            iword = (unsigned int*) &txdata[i];
            wordvec.push_back( *iword );
            if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
        }
        
        outputblock(mfec, fecchannel, wordvec);
        
        
        writeCSregister(mfec, fecchannel, 0x07);
        
        if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        return 0;
    }
}
//----------------------------------------------------------------------------------------------------
// Program one pixel, mask&trim packed together in a char.
int PixelPh1FECInterface::progpix(int mfec, int fecchannel,
                               int hubaddress, int portaddress, int rocid,
                               int coladdr, int rowaddress,
                               unsigned char databyte,
                               bool buffermode) {
    //const bool PRINT = true;
    // set trims on a pixel

    unsigned int *iword;
    unsigned char txdata[1024];
    unsigned int ch1stat, ch2stat;
    int current, i, ndata;
    unsigned char coltemp0, coltemp1, coltemp2;
    std::vector<uint32_t>  wordvec;
    
    //unsigned int nnn = (unsigned int) databyte;
    //if(hubaddress==4 && rocid==0 && coladdr==0 && rowaddress==0)
    //cout << "PROGPIX, hubaddress:"<<hubaddress<<" portaddress:"
    // <<portaddress<<" rocid:"<<rocid<<" "<<coladdr<<" "<<rowaddress<<" "<<hex<<nnn<<dec<<" "<<buffermode<<endl;
    
    if (buffermode) {
        if (PRINT) cout << "PixelPh1FECInterface: " <<"progpix buffered databyte"<<endl;
        if (PRINT) cout << "PixelPh1FECInterface: "  << "PROGPIX Q ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"
		    <<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" databyte:"<<std::hex<<(unsigned int)(databyte)<<std::dec<< endl;
        
        assert(0<=coladdr);assert(coladdr<=51);assert(0<=rowaddress);assert(rowaddress<=79);
        
        if (qbufn[mfec][fecchannel] >= maxbuffersize_)  {
            //cout << "PixelPh1FECInterface::progpix:ERROR mfec " << mfec <<":"<<fecchannel<<" OVER BUFFER LIMIT("<<qbufn[mfec][fecchannel]<<")"<<endl;
            if (PRINT) cout << "PixelPh1FECInterface: "  << "qbufn = " << qbufn[mfec][fecchannel] << " > maxbufsize = " << maxbuffersize_ << "; qbufsending..." << endl;
            qbufsend(mfec,fecchannel);
        }
        assert(qbufn[mfec][fecchannel] < maxbuffersize_);
        
        // If the hub and port are different, resend the new hub and port
        //  if (!((qbuflasthub[mfec][fecchannel] == hubaddress) &&
        //       (qbuflastport[mfec][fecchannel] == portaddress)) ) {
        // First are the hub (5 bits) and port (3 bits)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] =
        (hubaddress << 3) | portaddress ;
        //    qbuflasthub[mfec][fecchannel] = hubaddress;
        //    qbuflastport[mfec][fecchannel] = portaddress;
        //  }
        
        // Now send the command length to follow, NUM
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 4;  // 4 for ProgPix
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = (rocid << 4) | (0x04);
        
        // First calculate the ColAdr
        coltemp0 = coladdr;
        coltemp1 = coladdr / 2;
        coltemp1 = 2*(cinttogray(coltemp1));
        coltemp2 = coltemp1 | (coltemp0 & 0x01);
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = coltemp2;
        
        // Now calculate the row addr
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = cinttogray(rowaddress);
        
        // Now send the mask and trim
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = databyte;
        
        // terminate with 0
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0;
        
        //cout<<"QBUF length is "<<qbufn[mfec][fecchannel]<<" for mfec:"<<dec<<mfec<<" and fecchannel:"<<fecchannel<<endl;
        
        return 0;
        
    } else { // direct mode
        
        if (PRINT) cout << "PixelPh1FECInterface: " <<"progpix direct databyte"<<endl;
        if (PRINT) cout << "PixelPh1FECInterface: "  << "PROGPIX ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"
		    <<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" databyte:"
		    <<hex<<int(databyte)<<dec<<endl;
        
        // Check that there is nothing for the buffer mode?
        if (qbufn[mfec][fecchannel] > 0)  {
            qbufsend(mfec,fecchannel);
            cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][fecchannel]<<endl;
        }
        
        assert(0<=coladdr);
        assert(coladdr<=51);
        
        assert(0<=rowaddress);
        assert(rowaddress<=79);
        
        
        mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        current = 0;
        // First are the hub (5 bits) and port (3 bits)
        txdata[current++] = (hubaddress << 3) | portaddress ;
        
        // Now send the command length to follow, NUM
        txdata[current++] = 4;  // 4 for Prog_Pix
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        txdata[current++] = (rocid << 4) | (0x04);
        
        // First calculate the ColAdr
        coltemp0 = coladdr;
        coltemp1 = coladdr / 2;
        coltemp1 = 2*(cinttogray(coltemp1));
        coltemp2 = coltemp1 | (coltemp0 & 0x01);
        txdata[current++] = coltemp2;
        
        // Now calculate the row addr
        txdata[current++] = cinttogray(rowaddress);
        
        // Now send the mask and trim
        txdata[current++] = databyte;
        
        // terminate with 0
        txdata[current++] = 0;
        txdata[current++] = 0xFF;
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        // Reset the appropriate mfec channel
        writeCSregister(mfec, fecchannel, 0x08);
        
        for (i=0;i<current;i++) {
            if (PRINT) cout << "PixelPh1FECInterface: " <<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        }
        // add 3 zeros to pad remainder of 4 byte word
        ndata = current;
        txdata[current++] = 0;
        txdata[current++] = 0;
        txdata[current++] = 0;
        
        if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;
        
        // Now load the data to the word container
        for (i=0;i<ndata;i+=4) {
            iword = (unsigned int*) &txdata[i];
            wordvec.push_back( *iword );
            if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
        }
        
        outputblock(mfec, fecchannel, wordvec);
        
        
        writeCSregister(mfec, fecchannel, 0x07);
        
        if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        return 0;
    }
}
//--------------------------------------------------------------------------------------------
int PixelPh1FECInterface::calpix(int mfec, int fecchannel,
                              int hubaddress, int portaddress, int rocid,
                              int coladdr, int rowaddress,
                              int caldata,
                              bool buffermode) {
    // set trims on a pixel
    unsigned int *iword;
    unsigned char txdata[1024];
    int current, i, ndata;
    unsigned int ch1stat, ch2stat;
    unsigned char coltemp0, coltemp1, coltemp2;
    std::vector<uint32_t>  wordvec;
    
    if (buffermode) {
        if (PRINT) cout << "PixelPh1FECInterface: "  << "CALPIX ROC CMD: mfec:"<<dec<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" caldata:"<<caldata<<endl;
        
        assert(0<=coladdr);assert(coladdr<=51);assert(0<=rowaddress);assert(rowaddress<=79);
        
        if (qbufn[mfec][fecchannel] >= maxbuffersize_)  {
            //cout << "ERROR mfec " << mfec <<":"<<fecchannel<<" OVER BUFFER LIMIT"<<endl;
            qbufsend(mfec,fecchannel);
        }
        assert(qbufn[mfec][fecchannel] < maxbuffersize_);
        
        // If the hub and port are different, resend the new hub and port
        //  if (!((qbuflasthub[mfec][fecchannel] == hubaddress) &&
        //       (qbuflastport[mfec][fecchannel] == portaddress)) ) {
        // First are the hub (5 bits) and port (3 bits)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] =
        (hubaddress << 3) | portaddress ;
        //    qbuflasthub[mfec][fecchannel] = hubaddress;
        //    qbuflastport[mfec][fecchannel] = portaddress;
        //  }
        
        // Now send the command length to follow, NUM
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 4;  // 4 for calpix
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = (rocid << 4) | (0x02);
        
        // First calculate the ColAdr
        coltemp0 = coladdr;
        coltemp1 = coladdr / 2;
        coltemp1 = 2*(cinttogray(coltemp1));
        coltemp2 = coltemp1 | (coltemp0 & 0x01);
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = coltemp2;
        
        // Now calculate the row addr
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = cinttogray(rowaddress);
        
        // Now send caldata (1 for cal with cap, 2 for cal with bump)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = caldata;
        
        // terminate with 0
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0;
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        
        return 0;
        
    } else { // direct mode
        
        if (PRINT) cout << "PixelPh1FECInterface: "  << "CALPIX ROC CMD: mfec:"<<dec<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" caldata:"<<caldata<<endl;
        
        // Check that there is nothing for the buffer mode?
        if (qbufn[mfec][fecchannel] > 0)  {
            qbufsend(mfec,fecchannel);
            cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][fecchannel]<<endl;
        }
        
        assert(0<=coladdr);
        assert(coladdr<=51);
        
        assert(0<=rowaddress);
        assert(rowaddress<=79);
        
        mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        current = 0;
        // First are the hub (5 bits) and port (3 bits)
        txdata[current++] = (hubaddress << 3) | portaddress ;
        
        // Now send the command length to follow, NUM
        txdata[current++] = 4;  // 4 for Prog_Pix
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        txdata[current++] = (rocid << 4) | (0x02);
        
        // First calculate the ColAdr
        coltemp0 = coladdr;
        coltemp1 = coladdr / 2;
        coltemp1 = 2*(cinttogray(coltemp1));
        coltemp2 = coltemp1 | (coltemp0 & 0x01);
        txdata[current++] = coltemp2;
        
        // Now calculate the row addr
        txdata[current++] = cinttogray(rowaddress);
        
        // Now send caldata (1 for cal with cap, 2 for cal with bump)
        txdata[current++] = caldata;
        
        // terminate with 0
        txdata[current++] = 0;
        txdata[current++] = 0xFF;
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        // Reset the appropriate mfec channel
        writeCSregister(mfec, fecchannel, 0x08);
        
        for (i=0;i<current;i++) {
            if (PRINT) cout << "PixelPh1FECInterface: " <<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        }
        // add 3 zeros to pad remainder of 4 byte word
        ndata = current;
        txdata[current++] = 0;
        txdata[current++] = 0;
        txdata[current++] = 0;
        
        if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;
        
        // Now load the data to the word container
        for (i=0;i<ndata;i+=4) {
            iword = (unsigned int*) &txdata[i];
            wordvec.push_back( *iword );
            if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
        }
        
        outputblock(mfec, fecchannel, wordvec);
        
        
        writeCSregister(mfec, fecchannel, 0x07);
        
        if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        return 0;
    }
}
//----------------------------------------------------------------------------
// Enable or disable a dcol. Does NOT program pixels.
int PixelPh1FECInterface::dcolenable(int mfec, int fecchannel,
                                  int hubaddress, int portaddress, int rocid,
                                  int dcol, int dcolstate,
                                  bool buffermode) {
    // Enable or disable a double column

    unsigned int *iword;
    unsigned char txdata[1024];
    int current, i, ndata;
    unsigned int ch1stat, ch2stat;
    unsigned char coltemp0;
    std::vector<uint32_t>  wordvec;
    
    if (buffermode) {
        if (PRINT) cout << "PixelPh1FECInterface: "  << "dcol CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" dcol:"<<dcol<<" dcolstate:"<<dcolstate<<endl;
        
        if (qbufn[mfec][fecchannel] >= maxbuffersize_)  {
            //cout << "ERROR mfec " << mfec <<":"<<fecchannel<<" OVER BUFFER LIMIT"<<endl;
            qbufsend(mfec,fecchannel);
        }
        assert(qbufn[mfec][fecchannel] < maxbuffersize_);
        
        // If the hub and port are different, resend the new hub and port
        //  if (!((qbuflasthub[mfec][fecchannel] == hubaddress) &&
        //       (qbuflastport[mfec][fecchannel] == portaddress)) ) {
        // First are the hub (5 bits) and port (3 bits)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] =
        (hubaddress << 3) | portaddress ;
        //    qbuflasthub[mfec][fecchannel] = hubaddress;
        //    qbuflastport[mfec][fecchannel] = portaddress;
        //  }
        
        // Now send the command length to follow, NUM
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 4;  // 4 for progpix
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = (rocid << 4) | (0x4);
        
        // First calculate the ColAdr
        coltemp0 = cinttogray(dcol*2);
        coltemp0 = coltemp0 & 0xFE;
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = coltemp0;
        
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0x40; // dec64 col addr
        
        if (dcolstate == 1)
            qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0x80;
        else
            qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0x0;
        
        // terminate with 0
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0;
        
        return 0;
        
    } else { // direct mode
        
        if (PRINT) cout << "PixelPh1FECInterface: "  << "dcol CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" dcol:"<<dcol<<" dcolstate:"<<dcolstate<<endl;
        
        // Check that there is nothing for the buffer mode?
        if (qbufn[mfec][fecchannel] > 0)  {
            qbufsend(mfec,fecchannel);
            cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][fecchannel]<<endl;
        }
        
        // Check that mfec is not still sending previous command
        mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        current = 0;
        // First are the hub (5 bits) and port (3 bits)
        txdata[current++] = (hubaddress << 3) | portaddress ;
        
        // Now send the command length to follow, NUM
        txdata[current++] = 4;  // 4 for Prog_Pix
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        txdata[current++] = (rocid << 4) | (0x4);
        
        // First calculate the ColAdr
        coltemp0 = cinttogray(dcol*2);
        coltemp0 = coltemp0 & 0xFE;
        txdata[current++] = coltemp0;
        
        txdata[current++] = 0x40; // dec 64 col address
        
        if (dcolstate == 1) txdata[current++] = 0x80;
        else txdata[current++] = 0x0;
        
        // terminate with 0
        txdata[current++] = 0;
        txdata[current++] = 0xFF;
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        // Reset the appropriate mfec channel
        writeCSregister(mfec, fecchannel, 0x08);
        
        for (i=0;i<current;i++) {
            if (PRINT) cout << "PixelPh1FECInterface: " <<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        }
        // add 3 zeros to pad remainder of 4 byte word
        ndata = current;
        txdata[current++] = 0;
        txdata[current++] = 0;
        txdata[current++] = 0;
        
        if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;

        // Now load the data to the word container
        for (i=0;i<ndata;i+=4) {
            iword = (unsigned int*) &txdata[i];
            wordvec.push_back( *iword );
            if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
        }
        
        outputblock(mfec, fecchannel, wordvec);
        
        writeCSregister(mfec, fecchannel, 0x07);
        
        if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        return 0;
    }
}
//---------------------------------------------------------------------------------------------------
// Program one dac
int PixelPh1FECInterface::progdac(int mfec, int fecchannel,
                               int hubaddress, int portaddress, int rocid,
                               int dacaddress, int dacvalue,
                               bool buffermode) {
    static bool jmt_warned[28] = {0};
    if (
        dacaddress == 5 ||
        dacaddress == 6 ||
        dacaddress == 8 ||
        dacaddress == 14 ||
        dacaddress == 15 ||
        dacaddress == 16 ||
        dacaddress == 18 ||
        dacaddress == 21 ||
        dacaddress == 23 ||
        dacaddress == 24 ||
        dacaddress == 27
        )
    {
        if (!jmt_warned[dacaddress])
            cout << "JMT needs to prevent the code from trying to program dac address " << dacaddress << "; no more warnings for this address." << endl;
        jmt_warned[dacaddress] = 1;
        return 0;
    }
    
    unsigned int *iword;
    unsigned char txdata[1024];
    int current, ndata;
    unsigned int ch1stat, ch2stat;
    std::vector<uint32_t>  wordvec;
    
    
    
    if (buffermode) {
        
        if (PRINT) cout << "PixelPh1FECInterface: "  << "Buffer mode PROGDAC ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" dacaddress:"<<dacaddress<<" dacvalue:"<<dacvalue<<endl;
        
        if (qbufn[mfec][fecchannel] >= maxbuffersize_)  {
            //cout << "PixelPh1FECInterface::progdac: ERROR mfec " << mfec <<":"<<fecchannel<<" OVER BUFFER LIMIT ("<<qbufn[mfec][fecchannel]<<")"<<endl;;
            qbufsend(mfec,fecchannel);
        }
        
        assert(qbufn[mfec][fecchannel] < maxbuffersize_);
        
        // If the hub and port are different, resend the new hub and port
        //  if (!((qbuflasthub[mfec][fecchannel] == hubaddress) &&
        //       (qbuflastport[mfec][fecchannel] == portaddress)) ) {
        // First are the hub (5 bits) and port (3 bits)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] =
        (hubaddress << 3) | portaddress ;
        
        //    qbuflasthub[mfec][fecchannel] = hubaddress;
        //    qbuflastport[mfec][fecchannel] = portaddress;
        //  }
        
        
        // Now send the command length to follow, NUM
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 3;  // 3 for prog dac
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = (rocid << 4) | (0x08);
        
        // Now send the dacaddress
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = dacaddress;
        
        // Now send the dac value
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = dacvalue;
        
        // terminate with 0
        qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0;
        
        return 0;
        
    } else { // direct mode
        
        if (PRINT) cout << "PixelPh1FECInterface: "  << "Regular mode PROGDAC ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" dacaddress:"<<dacaddress<<" dacvalue:"<<dacvalue<<endl;
        
        // Check that there is nothing for the buffer mode?
        if (qbufn[mfec][fecchannel] > 0)  {
            qbufsend(mfec,fecchannel);
            cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][fecchannel]<<endl;
        }
        
        //if(fecchannel==1 && hubaddress==31 && rocid==0 && dacaddress>27)
        //cout << "PROGDAC ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress
        // <<" rocid:"<<rocid<<" dacaddress:"<<dacaddress<<" dacvalue:"<<dacvalue<<" bufmode "<<buffermode<<" "<<fecdebug<<endl;
        
        mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        current = 0;
        // First are the hub (5 bits) and port (3 bits)
        txdata[current++] = (hubaddress << 3) | portaddress ;
        
        // Now send the command length to follow, NUM
        txdata[current++] = 3;  // 3 for prog dac
        
        // Now send the pixel chip address (high nibble) and the command (low nibble)
        txdata[current++] = (rocid << 4) | (0x08);
        
        // Now send the dacaddress
        txdata[current++] = dacaddress;
        
        // Now send the dac value
        txdata[current++] = dacvalue;
        
        // terminate with 0
        txdata[current++] = 0; // Nik commented out this
        
    //  txdata[current++] = 0xFF;
        
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        // Reset the appropriate mfec channel
        // Rst All FEC
        writeCSregister(mfec, fecchannel, 0x08);
        
        //if(PRINT) {
        //if(fecchannel==1 && hubaddress==31 && rocid==0 && dacaddress>27) {
        //for (i=0;i<current;i++) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        //cout <<endl;
        //}
        
        // add 3 zeros to pad remainder of 4 byte word
        ndata = current;
        txdata[current++] = 0;
        txdata[current++] = 0;
        txdata[current++] = 0;
        
        
        // Now load the data to the word container
        for (int i=0;i<ndata;i+=4) {
            iword = (unsigned int*) &txdata[i];
//            outputwordhal("SOUT_BUF1M1", *iword);
            wordvec.push_back( *iword );
            if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword << " flipped " << flipByte(*iword)  <<dec<<endl;
        }

        outputblock(mfec, fecchannel, wordvec);
        
        // reset and send GO
        writeCSregister(mfec, fecchannel, 0x07);
        
        if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        
        return 0;
    }
}
//----------------------------------------------------------------------------------------
// Program all DACS, use multi single commands
// 26 DACs + WBC + control, add the temperature register
int PixelPh1FECInterface::progalldacs(int mfec, int fecchannel,
                                   int hubaddress, int portaddress, int rocid,
                                   int wbc, int chipcontrol,
                                   const std::vector<unsigned char>& dacs) {
    assert(0);
    
    unsigned int *iword;
    unsigned char txdata[1024];
    int current, i, ndata;
    unsigned int ch1stat, ch2stat;
    std::vector<uint32_t>  wordvec;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  << "PROGALLDACS ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<endl;
    
    // There are 26 dacs to program with addresses 1..26.  Program them all
    // passing data to mfec in one tx buffer
    
    // Check that there is nothing for the buffer mode?
    if (qbufn[mfec][fecchannel] > 0)  {
        qbufsend(mfec,fecchannel);
        cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
        <<qbufn[mfec][fecchannel]<<endl;
    }
    
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    current = 0;
    // First are the hub (5 bits) and port (3 bits)
    txdata[current++] = (hubaddress << 3) | portaddress ;
    
    // Now send all sets (id-command, address, value) of dac setups
    for (i=0;i<27;i++) {  // Extend to include the temperature register
        // send num=3
        txdata[current++] = 3;  // 3 for prog single dac
        // Now send the pix chip addr (in high nibble) and command (in low nibble)
        txdata[current++] = (rocid << 4) | (0x08);
        // Now send the dacaddress
        txdata[current++] = i + 1;
        // Now send the dac value
        txdata[current++] = dacs[i];
    }
    
    // Now program the WBC and the Control register
    // send the wbc          !!!!!!!!!!  The ROC needs a reset after
    txdata[current++] = 3;  // 3 for prog single dac
    txdata[current++] = (rocid << 4) | (0x08);
    // send the dacaddress for wbc register
    txdata[current++] = 254;
    // send the wbc value
    txdata[current++] = wbc;
    
    // send the chipcontrol register values
    txdata[current++] = 3;  // 3 for prog single dac
    txdata[current++] = (rocid << 4) | (0x08);
    // send the dacaddress for the chip control register
    txdata[current++] = 253;
    // send the chip control register value contents
    txdata[current++] =  chipcontrol;
    
    // terminate with 0
    txdata[current++] = 0;
    txdata[current++] = 0xFF;
    
    if(current>maxbuffersize_) cout<<" FEC buffer overflow, reduce the data length "
        <<current<<endl;
    
    // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
    // Reset the appropriate mfec channel
    // Rst All FEC
    writeCSregister(mfec, fecchannel, 0x08);
    
    if (PRINT) {
        for (i=0;i<current;i++) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        cout <<endl;
    }
    
    // align to word
    if ((current % 4) != 0) txdata[current++] = 0;
    if ((current % 4) != 0) txdata[current++] = 0;
    if ((current % 4) != 0) txdata[current++] = 0;
    
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    
    
    // Now load the data to the word container
    for (i=0;i<ndata;i+=4) {
        iword = (unsigned int*) &txdata[i];
        wordvec.push_back( *iword );
        if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
    
    outputblock(mfec, fecchannel, wordvec);
    
    
    // reset and send GO
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    return 0;
}


// ROC DACs
//---------------------------------------------------------------------------------
// Use single or buffered commands
// Extend to  buffered commands
void PixelPh1FECInterface::setAllDAC(const PixelHdwAddress& theROC,
                                  const std::vector<unsigned int>& dacs,
                                  const bool buffermode) {

  if (PRINT) cout << "PixelPh1FECInterface: "  << "setAllDAC " << theROC << " buffermode=" << buffermode << endl;

    assert(dacs.size()==30);
    static bool jmt_warned = false;
    if (!jmt_warned) {
        printf("****************************************************************************\n");
        printf("JMT needs to fix the hardcoded magic numbers in PixelPh1FECInterface::setAllDAC\n");
        printf("                           (this is the only warning)\n");
        printf("****************************************************************************\n");
        jmt_warned = true;
    }
    
    //std::cout << "In PixelPh1FECInterface::setAllDAC "<<theROC.mfec()<<" "<<theROC.mfecchannel()<<" "
    //        <<theROC.hubaddress()<<" "<<theROC.portaddress()<<" "<<theROC.rocid()<<" "<<dacs.size()
    //    << std::endl;
    int mfec = theROC.mfec();
    int mfecchannel = theROC.mfecchannel();
    
    // Clear the buffer if not in buffer mode)
    if(!buffermode) {
        if (qbufn[mfec][mfecchannel] > 0)  {
            qbufsend(mfec,mfecchannel);
            cout << "mfec " << mfec <<":"<<mfecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][mfecchannel]<<endl;
        }
    }
    
    // Program the control register
    progdac(mfec,
            mfecchannel,
            theROC.hubaddress(),
            theROC.portaddress(),
            theROC.rocid(),
            253,
            dacs[28],buffermode);
    
    // Program the 27 DACs // JMTBAD readback
    for (unsigned int dacaddress=0;dacaddress<27;dacaddress++){
        //int ret=
        //std::cout<<(dacaddress+1)<<" "<<dacs[dacaddress]<<" ";
        progdac(mfec,
                mfecchannel,
                theROC.hubaddress(),
                theROC.portaddress(),
                theROC.rocid(),
                dacaddress+1,
                dacs[dacaddress],buffermode);
    }
    
    // Program the WBC
    //std::cout<<std::endl<<" Program WBC "<<dacs[27]<<std::endl;
    progdac(mfec,
            mfecchannel,
            theROC.hubaddress(),
            theROC.portaddress(),
            theROC.rocid(),
            254,
            dacs[27],buffermode);
    
    
    
}
//----------------------------------------------------------------------------------------------
// Program all pixels in a ROC using single commands.
void PixelPh1FECInterface::setMaskAndTrimAll(const PixelHdwAddress& theROC,
                                          const std::vector<unsigned char>& allPixels,
                                          const bool buffermode) {
  if (PRINT) std::cout << "PixelPh1FECInterface: "  << "setMaskAndTrimAll " << theROC << " buffermode=" << buffermode << std::endl;
    
    // Check that there is nothing for the buffer mode?
    int mfec = theROC.mfec();
    int mfecchannel = theROC.mfecchannel();
    
    // Clear the buffer if not in buffer mode)
    if(!buffermode) {
        if (qbufn[mfec][mfecchannel] > 0)  {
            qbufsend(mfec,mfecchannel);
            cout << "mfec " << mfec <<":"<<mfecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][mfecchannel]<<endl;
        }
    }
    
    for (unsigned int col=0;col<52;col++){
        for (unsigned int row=0;row<80;row++){
            progpix(mfec,
                    mfecchannel,
                    theROC.hubaddress(),
                    theROC.portaddress(),
                    theROC.rocid(),
                    col,
                    row,
                    allPixels[row+80*col],
                    buffermode);
        }
    }
    
}
//-------------------------------------------------------------------------------
// W
void PixelPh1FECInterface::setDcolEnableAll(const PixelHdwAddress& theROC,
                                         unsigned char mask,
                                         const bool buffermode) {
  std::cout << "setDcolEnableAll " << theROC << " buffermode=" << buffermode << " mask=0x" << std::hex << unsigned(mask) << std::dec << std::endl;
    
    // Check that there is nothing for the buffer mode?
    int mfec = theROC.mfec();
    int mfecchannel = theROC.mfecchannel();
    if(!buffermode) {
        if (qbufn[mfec][mfecchannel] > 0)  {
            qbufsend(mfec,mfecchannel);
            cout << "mfec " << mfec <<":"<<mfecchannel<<" leftover from buffer mode "
            <<qbufn[mfec][mfecchannel]<<endl;
        }
    }
    
    for (unsigned int ddcol=0;ddcol<26;ddcol++){
        dcolenable(mfec,                // why dcolenable
                   mfecchannel,
                   theROC.hubaddress(),
                   theROC.portaddress(),
                   theROC.rocid(),
                   ddcol,mask,buffermode);
    }
    
}
//-----------------------------------------------------------------------------------------------
// Program ALL pixels in a ROC to the same mask and trim
// Use the compressed column mode.
int PixelPh1FECInterface::rocinit(int NCOLS, int mfec, int fecchannel,
                               int hubaddress, int portaddress, int rocid,
                               int mask, int trim) {
  //PRINT_ON();

    // set trims/masks on a roc all to same one value
    
    unsigned int *iword;
    unsigned char txdata[1024];
    int current, i, ndata, ic;
    unsigned int ch1stat, ch2stat;
    unsigned char coltemp0, coltemp1, coltemp2, tmask, ttrim, databyte;
    std::vector<uint32_t>  wordvec;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  << "ROCINIT NCOLS = " << NCOLS << " CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" trim:"<<trim<<endl;
    
    // Check that there is nothing for the buffer mode?
    if (qbufn[mfec][fecchannel] > 0)  {
        qbufsend(mfec,fecchannel);
        cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
        <<qbufn[mfec][fecchannel]<<endl;
    }
    
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    //writeCSregister(mfec, fecchannel, 0); // testing this here(sent with 0 only for printing)
    
    
    current = 0;
    // First are the hub (5 bits) and port (3 bits)
    txdata[current++] = (hubaddress << 3) | portaddress ;
    
    //    const int NCOL = 52;

    // Now send the number of columns of compressed data to follow
    //  txdata[current++] = 52; // full chip of cols is 52
    //  txdata[current++] = 10; // full chip of cols is 52
    txdata[current++] = NCOLS; // full chip of cols is 52
    
    
    // Now set the mask and trim
    tmask = (mask & 0x01) << 7;
    ttrim = trim & 0x0F;
    databyte = tmask | ttrim;
    
    for (ic=0;ic<NCOLS;++ic) {
        if (PRINT) cout << "PixelPh1FECInterface: "  << "CURRENT COLUMN:" << ic << endl;
        // Now send  pix chip address (high nibble) and the command (low nibble)
        txdata[current++] = (rocid << 4) | (0x04);
        // Next set the col number
        coltemp0 = ic;
        coltemp1 = coltemp0 / 2;
        coltemp1 = 2*(cinttogray(coltemp1));
        coltemp2 = coltemp1 | (coltemp0 & 0x01);
        txdata[current++] = coltemp2;
        // Now send the mask and trim
        txdata[current++] = databyte;
        //if (PRINT) cout << "PixelPh1FECInterface: "  << "databyte: " << hex << unsigned(databyte) << dec << endl;
    }
    
    if (PRINT) cout << "PixelPh1FECInterface: "  << "ROCINIT BUFFER USAGE: "<< dec  <<current<< " bytes."<<endl;
    //158 byte for a rocinit
    
    // terminate with 0
    txdata[current++] = 0;
    txdata[current++] = 0xFF;
    
    if(current>maxbuffersize_) cout<<" FEC buffer overflow, reduce the data length "
        <<current<<endl;
    
    // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
    // Reset the appropriate mfec channel
    // enable compression full column and reset mFEC
    writeCSregister(mfec, fecchannel, 0x38);
    
    if (PRINT) {
        for (i=0;i<current;i++) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
    }
    
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;
    
    // Now load the data to the word container
    for (i=0;i<ndata;i+=4) {
        iword = (unsigned int*) &txdata[i];
        wordvec.push_back( *iword );
        if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
    
    outputblock(mfec, fecchannel, wordvec);
    
    
    // reset and send it
    writeCSregister(mfec, fecchannel, 0x37);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);

    //PRINT_RESTORE();
    return 0;
}
//------------------------------------------------------------------------------------
// Load trims&masks to all pixels in a ROC.
// Use the column mode, 8 columns at a time.
int PixelPh1FECInterface::roctrimload(int mfec, int fecchannel,
                                   int hubaddress, int portaddress,
                                   int rocid,
                                   const std::vector<unsigned char>& allPixels){

    unsigned int *iword;
    unsigned char txdata[1024];
    int current, i, j, ndata, ic;
    unsigned char coltemp0, coltemp1, coltemp2, databyte;
    int pixptr;
    unsigned int ch1stat, ch2stat;
    unsigned int idatabyte;
    int incol;
    int incolMax = 8;
    std::vector<uint32_t>  wordvec;
    
    //  unsigned short int cdata;
    
    // Check that there is nothing for the buffer mode?
    if (qbufn[mfec][fecchannel] > 0)  {
        qbufsend(mfec,fecchannel);
        cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
        <<qbufn[mfec][fecchannel]<<endl;
    }
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    
    if (PRINT) cout << "PixelPh1FECInterface: "  << "ROCTRIMLOAD CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" "<<endl;
    
    current = 0;
    
    if (PRINT) {
        cout << "Some of data sent to init roc:"<<hex<<endl ;
        for (i=0;i<20;i++) {
            idatabyte = (unsigned int) allPixels[i];
            cout << idatabyte << ",";
        }
        cout << dec<<endl;
    }
    
    databyte = 0;
    pixptr = 0;
    
    for (ic=0;ic<52;ic+=8) {
        if (PRINT) cout << "PixelPh1FECInterface: "  << "CURRENT COLUMN:" << ic << endl;
        mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
        current = 0;
        // First are the hub (5 bits) and port (3 bits)
        txdata[current++] = (hubaddress << 3) | portaddress ;
        // Now send the command length to follow, NUM
        if (ic == 48) incolMax = 4;
        txdata[current++] = incolMax;
        
        for (incol=0; incol<incolMax; incol++) {
            // Now send pixchip address (high nibble) and the command (low nibble)
            txdata[current++] = (rocid << 4) | (0x04);
            // Next set the col number
            coltemp0 = ic+incol;
            coltemp1 = coltemp0 / 2;
            coltemp1 = 2*(cinttogray(coltemp1));
            coltemp2 = coltemp1 | (coltemp0 & 0x01);
            txdata[current++] = coltemp2;
            if (PRINT) cout << "PixelPh1FECInterface: "  << "column (grayed/hex): " << hex << (int) coltemp2 << dec<<endl;
            for (j=0;j<80;j++) {
                // Now send the mask and trim
                databyte = allPixels[pixptr++];
                //if (PRINT) cout << "PixelPh1FECInterface: "  << "databyte=" << hex << (unsigned)(databyte) << dec << endl;
                //databyte = (char) cdata++;
                //databyte = (char) 0x60;
                txdata[current++] = databyte;
            }
        }
        
        
        // terminate with 0
        txdata[current++] = 0;
        txdata[current++] = 0xFF;  // take this ff out for final
        
        if(current>maxbuffersize_) cout<<" FEC buffer overflow, reduce the data length "
            <<current<<" "<<maxbuffersize_<<endl;
        
        // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
        // Reset the appropriate mfec channel
        // dis enable compression full column
        writeCSregister(mfec, fecchannel, 0x18);
        if (PRINT) cout << "PixelPh1FECInterface: "  << "TOTAL byte count: " << dec << current <<endl;
        for (i=0;i<current;i++) {
            if (PRINT) cout << "PixelPh1FECInterface: " <<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        }
        
        
        // add 3 zeros to pad remainder of 4 byte word
        ndata = current;
        txdata[current++] = 0;
        txdata[current++] = 0;
        txdata[current++] = 0;
        
        if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;
        
        // Now load the data to the word container
        for (i=0;i<ndata;i+=4) {
            iword = (unsigned int*) &txdata[i];
            wordvec.push_back( *iword );
            if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
        }
        
        outputblock(mfec, fecchannel, wordvec);
        
        writeCSregister(mfec, fecchannel, 0x17);
        
    }
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    return 0;
    
}
// Load trims&masks to a group of columns, columns<=8.
// Use the fast column mode.
//-----------------------------------------------------------------------------------------------------
int PixelPh1FECInterface::coltrimload(int mfec, int fecchannel,
                                   int hubaddress, int portaddress,
                                   int rocid,
                                   int colstart, int numcols,
                                   const std::vector<unsigned char>& allPixels){
    // coltrimload sets between 1 and 12 cols (max TX buffer size) with mask/trim
    // data

    unsigned int *iword;
    unsigned char txdata[1024];
    int current, i, j, ndata;
    unsigned char coltemp0, coltemp1, coltemp2, databyte;
    int pixptr;
    unsigned int ch1stat, ch2stat;
    unsigned int idatabyte;
    int incol;
    std::vector<uint32_t>  wordvec;
    
    //  unsigned short int cdata;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  << "COLTRIMLOAD CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" "<<endl;
    
    // Check that there is nothing for the buffer mode?
    if (qbufn[mfec][fecchannel] > 0)  {
        qbufsend(mfec,fecchannel);
        cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
        <<qbufn[mfec][fecchannel]<<endl;
    }
    
    current = 0;
    
    if (PRINT) {
        cout << "Some of data sent to init roc:"<<endl;
        for (i=0;i<20;i++) {
            idatabyte = (unsigned int) allPixels[i];
            cout << hex << idatabyte << ", ";
        }
        cout << dec<<endl;
    }
    
    databyte = 0;
    pixptr = 0;
    
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    current = 0;
    // First are the hub (5 bits) and port (3 bits)
    txdata[current++] = (hubaddress << 3) | portaddress ;
    // Now send the command length to follow, NUM
    txdata[current++] = (unsigned char) numcols;
    
    // Check the side limit
    if(numcols>12) cout<<"coltrimload: Reduce the number of columns to 8 "<<numcols<<endl;
    
    for (incol=0; incol<numcols; incol++) {
        // Now send pixchip address (high nibble) and the command (low nibble)
        txdata[current++] = (rocid << 4) | (0x04);
        // Next set the col number
        
        coltemp0 = incol + colstart;
        coltemp1 = coltemp0 / 2;
        coltemp1 = 2*(cinttogray(coltemp1));
        coltemp2 = coltemp1 | (coltemp0 & 0x01);
        txdata[current++] = coltemp2;
        if (PRINT) cout << "PixelPh1FECInterface: " <<"column(grayed/hex):"<<hex<<(int) coltemp2<<dec<<endl;
        for (j=0;j<80;j++) {
            // Now send the mask and trim
            databyte = allPixels[pixptr++];
            //databyte = (char) cdata++;
            //databyte = (char) 0x60;
            txdata[current++] = databyte;
        }
    }
    
    
    // terminate with 0
    txdata[current++] = 0;
    txdata[current++] = 0xFF;  // take this ff out for final
    if(current>maxbuffersize_) cout<<" FEC buffer overflow, reduce the data length "
        <<current<<endl;
    
    // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
    // Reset the appropriate mfec channel
    // dis enable compression full column
    writeCSregister(mfec, fecchannel, 0x18);
    
    if (PRINT) {
        cout << "TOTAL byte count: " << dec << current <<endl;
        for (i=0;i<current;i++) {
            cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        }
        cout <<endl;
    }
    
    // align to word
    if ((current % 4) != 0) txdata[current++] = 0;
    if ((current % 4) != 0) txdata[current++] = 0;
    if ((current % 4) != 0) txdata[current++] = 0;
    
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    // Now load the data
    
    // Now load the data to the word container
    for (i=0;i<ndata;i+=4) {
        iword = (unsigned int*) &txdata[i];
        wordvec.push_back( *iword );
        if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
    
    outputblock(mfec, fecchannel, wordvec);
    
    writeCSregister(mfec, fecchannel, 0x17);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    return 0;
    
}
//-------------------------------------------------------------------------------------------------
// Send same mask&trim to all pixels in one column.
// Uses multiple commands.
int PixelPh1FECInterface::sendcoltoroc(const int mfec, int fecchannel,
                                    int hubaddress, int portaddress, int rocid,
                                    int coladdr, int mask, int trim) {

  if (PRINT) cout << "PixelPh1FECInterface: "  << std::dec << "sendcoltoroc("
		  << "mfec=" << mfec << ", "
		  << "mfecchannel=" << fecchannel << ", "
		  << "hubaddress=" << hubaddress << ", "
		  << "portaddress=" << portaddress << ", "
		  << "rocid=" << rocid << ", "
		  << "coladdr=" << coladdr << ", "
		  << "mask=" << mask << ", "
		  << "trim=" << trim << ", "
		  << std::endl;

    unsigned char coltemp0, coltemp1, coltemp2, tmask, ttrim;
    int current, krow, rowaddress, i, ndata;

    
    unsigned int *iword;
    unsigned char databyte;
    unsigned int ch1stat, ch2stat;
    unsigned char txdata[1024];
    std::vector<uint32_t>  wordvec;
    
    // Prog_Pix MULTIPLE
    // Check that there is nothing for the buffer mode?
    if (qbufn[mfec][fecchannel] > 0)  {
        qbufsend(mfec,fecchannel);
        cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
        <<qbufn[mfec][fecchannel]<<endl;
    }
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    current = 0;
    // First are the hub (5 bits) and port (3 bits)
    txdata[current++] = (hubaddress << 3) | portaddress ;
    // Now send the command length to follow, NUM
    txdata[current++] = 162;  // 162 for prog multiple
    // Now send the pixel chip address (high nibble) and the command (low nibble)
    txdata[current++] = (rocid << 4) | (0x04);
    
    // First calculate the ColAddr
    coltemp0 = coladdr;
    coltemp1 = coladdr / 2;
    coltemp1 = 2*(cinttogray(coltemp1));
    coltemp2 = coltemp1 | (coltemp0 & 0x01);
    txdata[current++] = coltemp2;
    
    // Now calculate the row addr
    txdata[current++] = 0; // This is a gray value for 0=0
    
    // Now send the mask and trim
    tmask = (mask & 0x01) << 7;
    ttrim = trim & 0x0F;
    databyte = tmask | ttrim;
    txdata[current++] = databyte;
    
    // Now send off the data for the rest of this column
    // 80 rows for psi46 (52 cols)
    for (krow = 1; krow < 80; krow++) {
        // Calculate the row address
        rowaddress = cinttogray(krow);
        txdata[current++] = rowaddress;
        // now send off the data
        txdata[current++] = databyte;
    }
    
    // Place 00 at the end
    txdata[current++] = 0;
    
    // Place FF at the end
    txdata[current++] = 0xFF;
    if(current>maxbuffersize_) cout<<" FEC buffer overflow, reduce the data length "
        <<current<<endl;
    if (PRINT) {
        for (i=0;i<current;i++) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
        cout <<endl;
    }
    
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    // Reset the appropriate mfec channel
    writeCSregister(mfec, fecchannel, 0x08);
    
    // Now load the data to the word container
    for (i=0;i<ndata;i+=4) {
        iword = (unsigned int*) &txdata[i];
        wordvec.push_back( *iword );
        if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
    
    outputblock(mfec, fecchannel, wordvec);
    
    // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------
// TBM Commands
// Read the TBM registers.
// Added by d.k. 11/07
int PixelPh1FECInterface::tbmread(int mfec, int fecchannel,
                               int tbmchannel, int hubaddress, int portaddress,int offset) {
    const int direction = 1;
    const int databyte = 0xffffffff;
    unsigned int t;
    unsigned int *iword;
    unsigned int ch1stat, ch2stat;
    unsigned char txdata[1024];
    int current, i, ndata;
    std::vector<uint32_t>  wordvec;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  << "TBMREAD: mfec:"<<mfec<<" tbmchannel:"<<tbmchannel<<" hubaddress:"<<hubaddress
        <<" portaddress:"<<portaddress<<" offset:"<<offset<<" databyte:"<<databyte<<" direction:"
        <<direction<<endl;
    
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    current = 0;
    // First are the hub (5 bits) and port (3 bits)
    
    int tmp1 = (hubaddress << 3) | portaddress ;
    txdata[current++] = (hubaddress << 3) | portaddress ;
    
    // Next in buffer is number of bytes to shift out (for TBM is always = 2)
    txdata[current++] = 2;
    
    // Select TBM A or B and offset
    t = tbmchannel << 4;
    t = t | ((offset << 1) & 0x0E);
    t = t | (direction & 0x01);
    txdata[current++] = t;
    int tmp2 = t;
    
    // Set the data byte
    txdata[current++] = databyte;
    
    // Place 00 at the end
    txdata[current++] = 0;
    
    // Place FF at the end
    txdata[current++] = 0xFF;
    
    for (i=0;i<current;i++) {
        if (PRINT) cout << "PixelPh1FECInterface: " <<" ("<<hex<< (int) txdata[i]<<") ";
    }
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;
    
    // Reset ALL buffer memory pointer
    writeCSregister(mfec, fecchannel, 0x08);
    
    // Now load the data to the word container
    for (i=0;i<ndata;i+=4) {
        iword = (unsigned int*) &txdata[i];
        wordvec.push_back( *iword );
        if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
    
    outputblock(mfec, fecchannel, wordvec);
    
    // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 1) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    
    int out = readback(mfec,fecchannel);
    int regadd = out & 0xff;
    int outdata = (out&0xff00)>>8;
    int hubport = (out&0xff0000)>>16;
    //cout<<hex<<out<<" "<<regadd<<" "<<outdata<<" "<<hubport<<dec<<endl;
    if(hubport != tmp1) cout<<" wrong returned hub/port "<<hex<<tmp1<<" "
        <<hubport<<dec<<endl;
    if(regadd != tmp2) cout<<" wrong returned register address "<<hex<<tmp2<<" "
        <<regadd<<dec<<endl;
    if((hubport != tmp1) || (regadd != tmp2) ) {
        TBMReadException e;
        throw e;
    }
    
    return outdata;
}
///////////////////////////////////////////////////////////////////////////////////////////
// Write to TBM registers.
//
int PixelPh1FECInterface::tbmcmd(int mfec, int fecchannel,
                              int tbmchannel, int hubaddress, int portaddress,
                              int offset, int databyte, int direction) {
  //PRINT_ON();

    unsigned int t;
    unsigned int *iword;
    unsigned int ch1stat, ch2stat;
    unsigned char txdata[1024];
    int current, i, ndata;
    std::vector<uint32_t> wordvec;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  << "TBM Command: mfec:"<<mfec<<" tbmchannel:"<<tbmchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" offset:"<<offset<<" databyte:"<<databyte<<" direction:"<<direction<<endl;
    
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat);
    
    current = 0;
    // First are the hub (5 bits) and port (3 bits)
    txdata[current++] = (hubaddress << 3) | portaddress ;
    
    // Next in buffer is number of bytes to shift out (for TBM is always = 2)
    txdata[current++] = 2;
    
    // Select TBM A or B and offset
    t = tbmchannel << 4;
    t = t | ((offset << 1) & 0x0E);
    t = t | (direction & 0x01);
    txdata[current++] = t;
    
    // Set the data byte
    
    txdata[current++] = databyte;
    
    // Place 00 at the end
    txdata[current++] = 0;
    
    // Place FF at the end 
    txdata[current++] = 0xFF;
    
    for (i=0;i<current;i++) {
        if (PRINT) cout << "PixelPh1FECInterface: " <<" ("<<hex<< (int) txdata[i]<<") "<<dec;
    }
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;
    
    // Reset ALL buffer memory pointer
    writeCSregister(mfec, fecchannel, 0x08);
    

    // Now load the data to the word container
    for (i=0;i<ndata;i+=4) {
        iword = (unsigned int*) &txdata[i];
//        outputwordhal("SOUT_BUF1M1",*iword);
        wordvec.push_back( *iword );
        if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword << " flipped  " << flipByte(*iword) <<dec<<endl;
    }
    
    
    outputblock(mfec, fecchannel, wordvec);

    // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
    
    //PRINT_RESTORE();
    return 0;
}
//--------------------------------------------------------------------------------
// Set TBM speed
int PixelPh1FECInterface::tbmspeed(int mfec, int fecchannel, int tbmchannel,
                                int hubaddress, 
                                int speed) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << "tbmspeed: ";
  return tbmcmd(mfec, fecchannel, tbmchannel, hubaddress, 4, 0, speed, 0);
}
//
int PixelPh1FECInterface::tbmspeed2(int mfec, int fecchannel, 
                                 int tbmchannel, int hubaddress, int portaddress){
    unsigned int t;
    unsigned int *iword;
    unsigned int ch1stat, ch2stat;
    unsigned char txdata[1024];
    int current, i, ndata;
    std::vector<uint32_t> wordvec;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  << "TBMSPEED2: mfec:"<<mfec<<" tbmchannel:"<<tbmchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<endl;
    
    mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
    
    current = 0;
    // First are the hub (5 bits) and port (3 bits)
    txdata[current++] = (hubaddress << 3) | portaddress ;
    
    // Next in buffer is number of bytes to shift out (for TBM is always = 2)
    txdata[current++] = 2;
    
    // Select TBM A or B and offset
    t = tbmchannel << 4;
    txdata[current++] = t;
    
    // Set the data byte
    
    txdata[current++] = 1;
    
    // Place 00 at the end
    txdata[current++] = 0;
    
    // Place FF at the end 
    txdata[current++] = 0xFF;
    
    for (i=0;i<current;i++) {
        if (PRINT) cout << "PixelPh1FECInterface: " <<" ("<<hex<< (int) txdata[i]<<") "<<dec;
    }
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    if (PRINT) cout << "PixelPh1FECInterface: "  <<endl;
    
    // Reset ALL buffer memory pointer
    writeCSregister(mfec, fecchannel, 0x08);
    
    
    // Now load the data to the word container
    for (i=0;i<ndata;i+=4) {
        iword = (unsigned int*) &txdata[i];
        wordvec.push_back( *iword );
        if (PRINT) cout << "PixelPh1FECInterface: " <<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword  <<dec<<endl;
    }
    
    outputblock(mfec, fecchannel, wordvec);
    
    
    // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
    
    writeCSregister(mfec, fecchannel, 0x07);
    if (PRINT) cout << "PixelPh1FECInterface: "  << "SENDDATA mfec:"<<hex<<mfec<<dec;
    
    unsigned int ch1,ch2;
    
    mfecbusy(mfec, fecchannel,&ch1,&ch2); 
    
    if (fecchannel==2) ch1=ch2;
    
    if (ch1!=2) return 1;
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
    
    return 0;
    
}
//-------------------------------------------------------------------------------------------
// From Wolfram 
// To test the fiber connections
int PixelPh1FECInterface::testFiber(const int mfec, const int channel,
                                 int* rda, int * rck) {
  if (PRINT) cout << "PixelPh1FECInterface: "  << std::dec << "testFiber("
		  << "mfec=" << mfec << ", "
		  << "mfecchannel=" << channel << ", "
		  << std::endl;

    //cout<<" Test the fiber connections "<<endl;
    testFiberEnable(mfec,1); // Enable the fiber test
    usleep(int(0xffff*0.025)); // wait for 1.6ms
    int value=0;
    getByteHubCount(mfec,channel,4,&value); // read the whole register
    *rda = ( value      & 0xffff);
    *rck = ((value>>16) & 0xffff);
    testFiberEnable(mfec,0); // Disable the fiber test
    return 0;
}
//----------------------------------------------------------------------------------------
// Scan the Delay25
int PixelPh1FECInterface::delay25Test(int mymfec, 
                                   int myfecchannel, 
                                   int myhubaddress, 
                                   int mytbmchannel,
                                   int myportaddress,
                                   int myrocid, 
                                   int mymask, 
                                   int mytrim,
                                   int nTry,
                                   int commands,
                                   int& success0, 
                                   int& success1,
                                   int& success2, 
                                   int& success3,
                                   int& success4) {


    //myportaddress = 4;  // redefine the port to 4
    if(myportaddress==7 && myhubaddress==31)  {
        cout<<" For mfec/chan/hub/tbm/port/roc " <<mymfec<<" "<<myfecchannel<<" "<<myhubaddress<<" "
        <<mytbmchannel<<" "<<myportaddress<<" "
        <<myrocid<<" redefine hub address to 30"<<endl; 
        myhubaddress = 30; // avoid the 31 and 7 combination
    }

    //cout<<" For mfec/chan/hub/tbm/port/roc " <<mymfec<<" "<<myfecchannel<<" "<<myhubaddress<<" "
    //    <<mytbmchannel<<" "<<myportaddress<<"\n";

    //cout<<mymask<<" "<<mytrim<<" "<<nTry<<" "<<commands<<endl;
    
    success0 = success1 = success2 = success3 = success4 = 0;
    int cntgood, cntbad, j;
    unsigned int ch1, ch2;
    unsigned long data=0;
    unsigned char databyte;
    
    unsigned int         dataReceivedMask=0x00000200;
    if (myfecchannel==2) dataReceivedMask=0x02000000;

    bool giveUpEarly = false;

    std::vector <unsigned char> testroc2 ((80*52), 0x00);
    
    databyte =  ((mymask << 7)|mytrim);
    
    assert(databyte==0);
    
    //  cout << "DATABYTE:" << hex <<(unsigned int) databyte <<dec<<endl;
    
    for (j=0;j<(80*52);j++) testroc2[j]= databyte;

    int masksetting, trimsetting;
    
    cntgood=0; cntbad = 0;
    
    if ((databyte & 0x80)==0x80) masksetting = 1;
    
    else masksetting = 0;
    
    trimsetting = (0xf)&databyte;

    cntgood=0; cntbad = 0;
    for (j=0;j<nTry;j++) {
      tbmcmd(mymfec,myfecchannel,mytbmchannel,myhubaddress,myportaddress, 7, nTry*4, 0);
      //rocinit(10, mymfec,myfecchannel,myhubaddress,myportaddress,myrocid, masksetting,trimsetting);
        mfecbusy(mymfec, myfecchannel, &ch1, &ch2);
        getfecctrlstatus(mymfec,&data);  
        
        //if ((data & dataReceivedMask) == dataReceivedMask) {  // receive complete
	if (myfecchannel==2) data >>= 16;
	if ((data & 0x7F00) == 0x200) {
	  //printf("GOOD!\n");
            cntgood++;
        } else {
            cntbad++;
        }
        
	uint32_t xxx = 0xdeadbeef;
	getByteHubCount(1,1,4,(int*)&xxx);
        cout<<"-1- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<  "  xxx= " << xxx << dec<<endl;
        //cout<<"-1- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<" "<<ch1<<" "<<ch2<<dec<<endl;
        if(giveUpEarly && cntbad == 4) { //this point is clearly nonoptimal, so give up
            //break;
            return 0;
        }
        
    }
    
    success0 = cntgood;

    cntgood=0; cntbad = 0;
    
    for (j=0;j<nTry;j++) {
      //tbmcmd(1, 1, 14, 15, 4, 7, 200+nTry*4, 0);
      rocinit(5, mymfec,myfecchannel,myhubaddress,myportaddress,myrocid,
                masksetting,trimsetting);
      //      calpix(mymfec, myfecchannel, myhubaddress, myportaddress, myrocid, 0, 0, 1, true);  qbufsend();
        
        mfecbusy(mymfec, myfecchannel, &ch1, &ch2);
        
        getfecctrlstatus(mymfec,&data);  
        
	if (myfecchannel==2) data >>= 16;
	if ((data & 0x7F00) == 0x200) {
	  //if ((data & dataReceivedMask) == dataReceivedMask) {
            
            // receive complete
	  //printf("GOOD!\n");
            
            cntgood++;
            
        } else {
            
            cntbad++;
            
        }
	//cout<<"-2- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<dec<<endl;
        if(giveUpEarly && cntbad == 4) {
            //break;
            return 0;
        }
        
    }
    
    success1 = cntgood;

    cntgood=0; cntbad = 0;
    
    for (j=0;j<nTry;j++) {
      //tbmcmd(1, 1, 14, 15, 4, 7, (240+nTry*4)%256, 0);
      rocinit(31, mymfec,myfecchannel,myhubaddress,myportaddress,myrocid, masksetting,trimsetting);
        
        mfecbusy(mymfec, myfecchannel, &ch1, &ch2);
        
        getfecctrlstatus(mymfec,&data);  
        
	uint32_t xxx = 0xdeadbeef;
	getByteHubCount(1,1,4,(int*)&xxx);
	//uint32_t xxx2 = xxx;
	xxx >>= 16;
	
	if (myfecchannel==2) data >>= 16;
	if ((data & 0x7F00) == 0x200) {
	//if ((data & 0x200) == 0x200 && (xxx>>8) == (xxx&0xff)) {
	  //if ((data & dataReceivedMask) == dataReceivedMask) {
            
            // receive complete
            
	  //printf("GOOD!\n");
            cntgood++;
            
        } else {
            
            cntbad++;
            
        }
	//cout<<"-3- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<  "  xxx2= " << xxx2 << dec<<endl;
        if(giveUpEarly && cntbad == 4) {
            //break;
            return 0;
        }
        
    }
    
    success2 = cntgood;
    
    cntgood=0; cntbad = 0;

    for (j=0;j<nTry;j++) {
      //tbmcmd(1, 1, 14, 15, 4, 7, 120+nTry*4, 0);
      rocinit(40, mymfec,myfecchannel,myhubaddress,myportaddress,myrocid, masksetting,trimsetting);
        
        mfecbusy(mymfec, myfecchannel, &ch1, &ch2);
        
        getfecctrlstatus(mymfec,&data);  
        
	uint32_t xxx = 0xdeadbeef;
	getByteHubCount(1,1,4,(int*)&xxx);
	//uint32_t xxx2 = xxx;
	xxx >>= 16;
	
	if (myfecchannel==2) data >>= 16;
	if ((data & 0x7F00) == 0x200) {
	//if ((data & 0x200) == 0x200 && (xxx>>8) == (xxx&0xff)) {
	  //if ((data & dataReceivedMask) == dataReceivedMask) {
            
            // receive complete
            
	  //printf("GOOD!\n");
            cntgood++;
            
        } else {
            
            cntbad++;
            
        }
        //cout<<"-4- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<  "  xxx2= " << xxx2 << dec<<endl;
        if(giveUpEarly && cntbad == 4) {
            //break;
            return 0;
        }
        
    }
    
    success3 = cntgood;
    
    cntgood=0; cntbad = 0;

    usleep(1000);

    for (j=0;j<nTry;j++) {
      //tbmcmd(1, 1, 14, 15, 4, 7, 160+nTry*4, 0);
      rocinit(52, mymfec,myfecchannel,myhubaddress,myportaddress,myrocid, masksetting,trimsetting);

        mfecbusy(mymfec, myfecchannel, &ch1, &ch2);
	usleep(100);
        getfecctrlstatus(mymfec,&data);  
	usleep(100);
        
	uint32_t xxx = 0xdeadbeef;
	getByteHubCount(1,1,4,(int*)&xxx);
	//uint32_t xxx2 = xxx;
	xxx >>= 16;
	
	if (myfecchannel==2) data >>= 16;
	if ((data & 0x7F00) == 0x200) {
	//if ((data & 0x200) == 0x200 && (xxx>>8) == (xxx&0xff)) {
	  //if ((data & dataReceivedMask) == dataReceivedMask) {
            
            // receive complete
            
	  //printf("GOOD!\n");
            cntgood++;
            
        } else {
            
            cntbad++;
            
        }
        //cout<<"-5- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<  "  xxx2= " << xxx2 << dec<<endl;
        if(giveUpEarly && cntbad == 4) {
            //break;
            return 0;
        }
        
    }

    success4 = cntgood;
    
    return 0;
    
}


unsigned int PixelPh1FECInterface::flipByte(unsigned int input){
    uint32_t result = 0;
    result |= (input & 0x000000FF) << 24;
    result |= (input & 0x0000FF00) << 8;
    result |= (input & 0x00FF0000) >> 8;
    result |= (input & 0xFF000000) >> 24;
    return result;
}
