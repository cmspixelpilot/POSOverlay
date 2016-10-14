/* PixelFECInterface.cc - This is the implementation of the PixelFECInterface
   doroshenko@physics.rutgers.edu and stone@physics.rutgers.edu 5-11-07
*/

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <cstdlib>

/* comment out next line to turn off block transfer */
#define BLOCKTRANSFER

#include "PixelFECInterface/include/PixelFECInterface.h"
#include "PixelFECInterface/include/TBMReadException.h"

#define ret_error(ret) \
  if (ret!=0) { \
    cout << "Return value:"<<ret<<" in "<<__FILE__<<" on line "<<dec<<__LINE__<<endl; \
}

using namespace std;
using namespace pos;

namespace {
  const bool PRINT = false;
  //const bool PRINT = true;
}

//--------------------------------------------------------------------------
// Here comes the constructor -- with two versions, one for HAL, one for CAEN
#ifdef USE_HAL
PixelFECInterface::PixelFECInterface(const HAL::VMEDevice * const vmeDeviceP, 
				     const int vmeslot,
				     unsigned int fecCrate,
				     unsigned int fecSlot): vmeDevicePtr(vmeDeviceP),
							    fecCrate_(fecCrate),
							    fecSlot_(fecSlot)
{
 //Constructor stuff
  if (PRINT) cout << "PixelFECInterface Constructor called" << endl;
  pfecvmeslot = vmeslot;

  maxbuffersize_=1000;
  fecdebug = 0;

  for (int tfecchannel=1;tfecchannel<=2;tfecchannel++) {
    for (int tmfec=0;tmfec<9;tmfec++) {
      qbufnsend[tmfec][tfecchannel]=0;
      qbufnerr[tmfec][tfecchannel]=0;
      qbufn[tmfec][tfecchannel]=0;
      qbufn_old[tmfec][tfecchannel]=0;
    }
  }

  setssid(4);
}
//------------------------------------------------------------------------
PixelFECInterface::~PixelFECInterface(void) 
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

int PixelFECInterface::getversion(unsigned long *data) {
  uint32_t value;
  vmeDevicePtr->read("VERSIONM08", &value);
  *data = value;
  if (PRINT) cout<<"Get FEC version finds firmware version: "<<value<<endl;
  return value;
}
//-----------------------------------------------------------------------
// Get the STATUS word of the FEC. Includes the QPLL/TTCrx ready bits
int PixelFECInterface::getStatus(void) {
  uint32_t value;
  vmeDevicePtr->read("STATUS", &value);
  if (PRINT) cout<<"Get FEC status "<<value<<endl;
  return value;
}
//---------------------------------------------------------------
// This sets up the Trigger FPGA configuration register (SSID) to
// PIXELS (100)
int PixelFECInterface::setssid(const int ssid) {
  uint32_t value;
  vmeDevicePtr->read("CONFIG00", &value);
  if (PRINT) cout << "Set SSID in Trigger FPGA to:"<<hex<<ssid<<dec<<endl;
  vmeDevicePtr->write("SSID", ssid);
  vmeDevicePtr->read("SSID", &value);

  if (value != (unsigned long) ssid) {
    cout << "\nERROR PixelFECInterface SSID unable to change: " <<
      value << ", " << endl;
    return -1;
  }

  return value;
}
//--------------------------------------------------------------------------
int PixelFECInterface::getversion(const int mfec, unsigned long *data) {
  uint32_t value;
  const string names[8] = {"VERSIONM01","VERSIONM02","VERSIONM03","VERSIONM04",
			   "VERSIONM05","VERSIONM06","VERSIONM07","VERSIONM08"};

  vmeDevicePtr->read(names[mfec-1], &value);
  cout<<"Get FEC version finds firmware version: "<<value<<endl;

//   /* This getversion reads the firmware level from the loaded mfec.
//      One needs to be specified since one may not be installed. */
//   switch (mfec) 
//     {
//     case 1: 
//       vmeDevicePtr->read("VERSIONM01", &value);
//       break;
//     case 2:
//       vmeDevicePtr->read("VERSIONM02", &value);
//       break;
//     case 3:
//       vmeDevicePtr->read("VERSIONM03", &value);
//       break;
//     case 4:
//       vmeDevicePtr->read("VERSIONM04", &value);
//       break;
//     case 5:
//       vmeDevicePtr->read("VERSIONM05", &value);
//       break;
//     case 6:
//       vmeDevicePtr->read("VERSIONM06", &value);
//       break;
//     case 7:
//       vmeDevicePtr->read("VERSIONM07", &value);
//       break;
//     case 8:
//       vmeDevicePtr->read("VERSIONM08", &value);
//       break;
//     }      

  *data = value;

  //*data = (value & 0xFF000000) >> 24;
  
  if (PRINT) cout<<"Get FEC version finds firmware version: "<<*data<<endl;
  return 0;
}
//----------------------------------------------------------------------------
//
int PixelFECInterface::writeCSregister(int mfec, int fecchannel, 
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
        vmeDevicePtr->write("CSREGM1", value2);
	//    vmeDevicePtr->write("CSREGM1", value2);
    break;
  case 2: 
    vmeDevicePtr->write("CSREGM2", value2);
    break;
  case 3: 
    vmeDevicePtr->write("CSREGM3", value2);
    break;
  case 4: 
    vmeDevicePtr->write("CSREGM4", value2);
    break;
  case 5: 
    vmeDevicePtr->write("CSREGM5", value2);
    break;
  case 6: 
    vmeDevicePtr->write("CSREGM6", value2);
    break;
  case 7: 
    vmeDevicePtr->write("CSREGM7", value2);
    break;
  case 8: 
    vmeDevicePtr->write("CSREGM8", value2);
    break;
  }


  if (PRINT) {
    cout << "writeCSregister puts      : "<<hex<< value2 <<dec<<endl;
  }
  return value;
}
//----------------------------------------------------------------
void PixelFECInterface::haltest(void) {
  unsigned long int data;

  sleep(1);
  data = 0x01020304;
  sleep(1);
  vmeDevicePtr->write("TESTREG1", data);
  data = 0x05060708;
  sleep(1);
  vmeDevicePtr->write("TESTREG1", data);
  data = 0x090a0b0c;
  sleep(1);
  vmeDevicePtr->write("TESTREG1", data);
  sleep(1);

}
//-------------------------------------------------------------------------------
// mfecbusy reads the csreg and waits if the send_started is still
// in effect.  This should change state on it's own.  Timeout used in
// case it doesnt change.
void PixelFECInterface::mfecbusy(int mfec, int fecchannel,
				 unsigned int *ch1, unsigned int *ch2) {

  //const string names[8] = {"CSREGM1","CSREGM2","CSREGM3","CSREGM4",
  //		   "CSREGM5","CSREGM6","CSREGM7","CSREGM8"};

  uint32_t value;
  int timeout;
  // CSTIMEOUT of 300 is large enough for a mfec to shift out an entire
  // rocinit (typical wait for small column wise op is 20)
  const int CSTIMEOUT = 300;

  switch (mfec) {
  case 1:
    vmeDevicePtr->read("CSREGM1", &value);
    break;
  case 2: 
    vmeDevicePtr->read("CSREGM2", &value);
    break;
  case 3: 
    vmeDevicePtr->read("CSREGM3", &value);
    break;
  case 4: 
    vmeDevicePtr->read("CSREGM4", &value);
    break;
  case 5: 
    vmeDevicePtr->read("CSREGM5", &value);
    break;
  case 6: 
    vmeDevicePtr->read("CSREGM6", &value);
    break;
  case 7: 
    vmeDevicePtr->read("CSREGM7", &value);
    break;
  case 8: 
    vmeDevicePtr->read("CSREGM8", &value);
    break;
  }

  *ch1 = (value >> 8) & 0x7F;
  *ch2 = (value >> 24) & 0x7F;

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
	vmeDevicePtr->read("CSREGM1", &value);
	break;
      case 2: 
	vmeDevicePtr->read("CSREGM2", &value);
	break;
      case 3: 
	vmeDevicePtr->read("CSREGM3", &value);
	break;
      case 4: 
	vmeDevicePtr->read("CSREGM4", &value);
	break;
      case 5: 
	vmeDevicePtr->read("CSREGM5", &value);
	break;
      case 6: 
	vmeDevicePtr->read("CSREGM6", &value);
	break;
      case 7: 
	vmeDevicePtr->read("CSREGM7", &value);
	break;
      case 8: 
	vmeDevicePtr->read("CSREGM8", &value);
	break;
      }

      *ch1 = (int) (value >> 8) & 0x7F;
      timeout++;
      //    if (PRINT) {
      //cout << "<WAITING FOR READY "<<hex<<value<<" "<<timeout<<dec<< " > ";
      //      }
    } while (((*ch1 & 0x8) == 0x8) && (timeout < CSTIMEOUT));

    if (PRINT) cout << "CH1 timeout=" << dec << timeout <<endl;
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
	vmeDevicePtr->read("CSREGM1", &value);
	break;
      case 2: 
	vmeDevicePtr->read("CSREGM2", &value);
	break;
      case 3: 
	vmeDevicePtr->read("CSREGM3", &value);
	break;
      case 4: 
	vmeDevicePtr->read("CSREGM4", &value);
	break;
      case 5: 
	vmeDevicePtr->read("CSREGM5", &value);
	break;
      case 6: 
	vmeDevicePtr->read("CSREGM6", &value);
	break;
      case 7: 
	vmeDevicePtr->read("CSREGM7", &value);
	break;
      case 8: 
	vmeDevicePtr->read("CSREGM8", &value);
	break;
      }
      *ch2 = (int) (value >> 24) & 0x7F;
      timeout++;
      //      if (PRINT) {
      //cout << "<WAITING FOR READY "<<hex<<value<<" "<<timeout<<dec<< " > ";
      //      }
    } while (((*ch2 & 0x8) == 0x8) && (timeout < CSTIMEOUT));

    if (PRINT) cout << "CH2 timeout=" << dec << timeout <<endl;
    if (timeout>=CSTIMEOUT) { cout << "ERROR mfecbusy channel 2"<<endl; }

  }
  if ((fecchannel == 2) && (fecdebug > 0)) {
    if (((*ch2 & 0x02) != 0x02) && (*ch2 != 0)) {
      analyzeFecCSRChan(mfec, 2, *ch2);
      if (fecdebug == 2) writeCSregister(mfec, fecchannel, 0x08);
    }
  }

  mfecCSregister[mfec] = value;

}
//--------------------------------------------------------------------------------
// Finds out if *previous* transmission was received okay.
// Reads the mfec combined control/status register which returns and sets
// functions of ch1 and ch2 independently
int PixelFECInterface::getfecctrlstatus(const int mfec, unsigned long *data) {
  //const string names[8] = {"CSREGM1","CSREGM2","CSREGM3","CSREGM4",
  //		   "CSREGM5","CSREGM6","CSREGM7","CSREGM8"};

  uint32_t value;
  if (PRINT) cout << "Getting FEC cntrstatus register" <<endl;
  switch (mfec) {
  case 1:
    vmeDevicePtr->read("CSREGM1", &value);
    break;
  case 2: 
    vmeDevicePtr->read("CSREGM2", &value);
    break;
  case 3: 
    vmeDevicePtr->read("CSREGM3", &value);
    break;
  case 4: 
    vmeDevicePtr->read("CSREGM4", &value);
    break;
  case 5: 
    vmeDevicePtr->read("CSREGM5", &value);
    break;
  case 6: 
    vmeDevicePtr->read("CSREGM6", &value);
    break;
  case 7: 
    vmeDevicePtr->read("CSREGM7", &value);
    break;
  case 8: 
    vmeDevicePtr->read("CSREGM8", &value);
    break;
  }
  *data = value;
  return 0;
}
//----------------------------------------------------------------------------------------------
//
int PixelFECInterface::outputbuffer(const int mfec, const int fecchannel,
				    unsigned long data) {
//   const string names[2][8] = { {"BOUT_BUF1M1","BOUT_BUF1M2","BOUT_BUF1M3","BOUT_BUF1M4",
// 			        "BOUT_BUF1M5","BOUT_BUF1M6","BOUT_BUF1M7","BOUT_BUF1M8"},
// 			       {"BOUT_BUF2M1","BOUT_BUF2M2","BOUT_BUF2M3","BOUT_BUF2M4",
// 			        "BOUT_BUF2M5","BOUT_BUF2M6","BOUT_BUF2M7","BOUT_BUF2M8"} };

  //  uint32_t tempdata;
  switch (mfec) {
  case 1:
    if (fecchannel == 1) {
      outputwordhalblock("BOUT_BUF1M1", data);
    } else if (fecchannel == 2) {
      outputwordhalblock("BOUT_BUF2M1", data);
    }
    break;
  case 2: 
    if (fecchannel == 1) {
      outputwordhalblock("BOUT_BUF1M2", data);
    } else if (fecchannel == 2) {
      outputwordhalblock("BOUT_BUF2M2", data);
    }
    break;
  case 3: 
    if (fecchannel == 1) {
      outputwordhalblock("BOUT_BUF1M3", data);
    } else if (fecchannel == 2) {
      outputwordhalblock("BOUT_BUF2M3", data);
    }
    break;
  case 4: 
    if (fecchannel == 1) {
      outputwordhalblock("BOUT_BUF1M4", data);
    } else if (fecchannel == 2) {
      outputwordhalblock("BOUT_BUF2M4", data);
    }
    break;
  case 5: 
    if (fecchannel == 1) {
      outputwordhalblock("BOUT_BUF1M5", data);
    } else if (fecchannel == 2) {
      outputwordhalblock("BOUT_BUF2M5", data);
    }
    break;
  case 6: 
    if (fecchannel == 1) {
      outputwordhalblock("BOUT_BUF1M6", data);
    } else if (fecchannel == 2) {
      outputwordhalblock("BOUT_BUF2M6", data);
    }
    break;
  case 7: 
    if (fecchannel == 1) {
      outputwordhalblock("BOUT_BUF1M7", data);
    } else if (fecchannel == 2) {
      outputwordhalblock("BOUT_BUF2M7", data);
    }
    break;
  case 8: 
    if (fecchannel == 1) {
      outputwordhalblock("BOUT_BUF1M8", data);
    } else if (fecchannel == 2) {
      outputwordhalblock("BOUT_BUF2M8", data);
    }
    break;
  }
  return 0;
}
//-----------------------------------------------------------------------------------
//// output 1 word in HAL block mode
void PixelFECInterface::outputwordhalblock(const char *halname, unsigned int data) {
  vmeDevicePtr->writeBlock(halname, 4, (char *) &data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
}
//----------------------------------------------------------------------------------
//// output one word in HAL single mode
void PixelFECInterface::outputwordhal(const char *halname, unsigned int data) {
  vmeDevicePtr->write(halname, data);
}
//-----------------------------------------------------------------------------------
//// output block
int PixelFECInterface::outputblock(const int mfec, const int fecchannel,
				    unsigned int *data, int ndata) {
//   const string names[2][8] = { {"BOUT_BUF1M1","BOUT_BUF1M2","BOUT_BUF1M3","BOUT_BUF1M4",
// 			        "BOUT_BUF1M5","BOUT_BUF1M6","BOUT_BUF1M7","BOUT_BUF1M8"},
// 			       {"BOUT_BUF2M1","BOUT_BUF2M2","BOUT_BUF2M3","BOUT_BUF2M4",
// 			        "BOUT_BUF2M5","BOUT_BUF2M6","BOUT_BUF2M7","BOUT_BUF2M8"} };


  //cout << "OUTPUTBLOCK asked to transfer ndata:"<<dec<<ndata<<endl;
  if (ndata > 0) ndata = ((ndata-1)/4 + 1)*4;

  if (mfec == 1) {
    if (fecchannel == 1) {
            vmeDevicePtr->writeBlock("BOUT_BUF1M1", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    } else if (fecchannel == 2) {
      vmeDevicePtr->writeBlock("BOUT_BUF2M1", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    }
  } else if (mfec == 2) {
    if (fecchannel == 1) {
      vmeDevicePtr->writeBlock("BOUT_BUF1M2", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    } else if (fecchannel == 2) {
      vmeDevicePtr->writeBlock("BOUT_BUF2M2", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    }
  } else if (mfec == 3) {
    if (fecchannel == 1) {
      vmeDevicePtr->writeBlock("BOUT_BUF1M3", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    } else if (fecchannel == 2) {
      vmeDevicePtr->writeBlock("BOUT_BUF2M3", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    }
  } else if (mfec == 4) {
    if (fecchannel == 1) {
      vmeDevicePtr->writeBlock("BOUT_BUF1M4", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    } else if (fecchannel == 2) {
      vmeDevicePtr->writeBlock("BOUT_BUF2M4", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    }
  } else if (mfec == 5) {
    if (fecchannel == 1) {
      vmeDevicePtr->writeBlock("BOUT_BUF1M5", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    } else if (fecchannel == 2) {
      vmeDevicePtr->writeBlock("BOUT_BUF2M5", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    }
  } else if (mfec == 6) {
    if (fecchannel == 1) {
      vmeDevicePtr->writeBlock("BOUT_BUF1M6", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    } else if (fecchannel == 2) {
      vmeDevicePtr->writeBlock("BOUT_BUF2M6", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    }
  } else if (mfec == 7) {
    if (fecchannel == 1) {
      vmeDevicePtr->writeBlock("BOUT_BUF1M7", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    } else if (fecchannel == 2) {
      vmeDevicePtr->writeBlock("BOUT_BUF2M7", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    }
  } else if (mfec == 8) {
    if (fecchannel == 1) {
      vmeDevicePtr->writeBlock("BOUT_BUF1M8", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    } else if (fecchannel == 2) {
      vmeDevicePtr->writeBlock("BOUT_BUF2M8", ndata, (char *) data,
			       HAL::HAL_NO_VERIFY,
			       HAL::HAL_NO_INCREMENT);
    }
  }
    
  return 0;
}

int PixelFECInterface::resetdoh(const int mfec, const int fecchannel) {
  // fecchannel determines if value will be shifted on CS register word
  writeCSregister(mfec, fecchannel, 0x8000);
  return 0;
}

int PixelFECInterface::injectrstroc(const int mfec, const int bitstate) {
  //const string names[8] = {"INRSTROCM1","INRSTROCM2","INRSTROCM3","INRSTROCM4",
  //		             "INRSTROCM5","INRSTROCM6","INRSTROCM7","INRSTROCM8"};
  switch (mfec) 
    {
    case 1:   outputwordhal("INRSTROCM1", bitstate); break;
    case 2:   outputwordhal("INRSTROCM2", bitstate); break;
    case 3:   outputwordhal("INRSTROCM3", bitstate); break;
    case 4:   outputwordhal("INRSTROCM4", bitstate); break;
    case 5:   outputwordhal("INRSTROCM5", bitstate); break;
    case 6:   outputwordhal("INRSTROCM6", bitstate); break;
    case 7:   outputwordhal("INRSTROCM7", bitstate); break;
    case 8:   outputwordhal("INRSTROCM8", bitstate); break;
    }
  return 0;
}
int PixelFECInterface::injecttrigger(const int mfec, const int bitstate) {
  switch (mfec)
    {
    case 1:   outputwordhal("INTRIGM1", bitstate); break;
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
int PixelFECInterface::injectrsttbm(const int mfec, const int bitstate) {
  switch (mfec) 
    {
    case 1:  outputwordhal("INRSTTBMM1", bitstate); break;
    case 2:  outputwordhal("INRSTTBMM2", bitstate); break;
    case 3:  outputwordhal("INRSTTBMM3", bitstate); break;
    case 4:  outputwordhal("INRSTTBMM4", bitstate); break;
    case 5:  outputwordhal("INRSTTBMM5", bitstate); break;
    case 6:  outputwordhal("INRSTTBMM6", bitstate); break;
    case 7:  outputwordhal("INRSTTBMM7", bitstate); break;
    case 8:  outputwordhal("INRSTTBMM8", bitstate); break;
    }
  return 0;
}
int PixelFECInterface::injectrstcsr(const int mfec, const int bitstate) {
  switch (mfec) 
    {
    case 1:  outputwordhal("INRSTCSRM1", bitstate); break;
    case 2:  outputwordhal("INRSTCSRM2", bitstate); break;
    case 3:  outputwordhal("INRSTCSRM3", bitstate); break;
    case 4:  outputwordhal("INRSTCSRM4", bitstate); break;
    case 5:  outputwordhal("INRSTCSRM5", bitstate); break;
    case 6:  outputwordhal("INRSTCSRM6", bitstate); break;
    case 7:  outputwordhal("INRSTCSRM7", bitstate); break;
    case 8:  outputwordhal("INRSTCSRM8", bitstate); break;
    }
  return 0;
}
int PixelFECInterface::enablecallatency(const int mfec, const int bitstate) {
  switch (mfec)
    {
    case 1:  outputwordhal("ENCALLATENCYM1", bitstate); break;
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
int PixelFECInterface::disableexttrigger(const int mfec, const int bitstate) {
  switch (mfec)
    { 
    case 1:  outputwordhal("DISEXTTRIGM1", bitstate); break;
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
int PixelFECInterface::loopnormtrigger(const int mfec, const int bitstate) {
  switch (mfec) 
    {
    case 1:  outputwordhal("LOOPNORMTRIGM1", bitstate); break;
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
int PixelFECInterface::loopcaltrigger(const int mfec, const int bitstate) {
  switch (mfec)
    {
    case 1:  outputwordhal("LOOPCALTRIGM1", bitstate); break;
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
int PixelFECInterface::callatencycount(const int mfec, const int latency) {
  switch (mfec) 
    {
    case 1:  outputwordhal("CALLATCNTM1", latency); break;
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
int PixelFECInterface::FullBufRDaDisable(const int mfec, const int disable) {
  switch (mfec)
    {
    case 1:  outputwordhal("DISRDACHECKM1", disable); break;
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
int PixelFECInterface::AllRDaDisable(const int mfec, const int disable) {
  switch (mfec)
    {
    case 1:  outputwordhal("DISRDAM1", disable); break;
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

int PixelFECInterface::testFiberEnable(const int mfec, const int enable) {
  switch (mfec)
    {
    case 1:  outputwordhal("TESTFIBERM1", enable); break;
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
int PixelFECInterface::readback(const int mfec, const int channel) {
  const string names[2][8] = { {"INP_BUF1M1","INP_BUF1M2","INP_BUF1M3","INP_BUF1M4",
			        "INP_BUF1M5","INP_BUF1M6","INP_BUF1M7","INP_BUF1M8"},
			       {"INP_BUF2M1","INP_BUF2M2","INP_BUF2M3","INP_BUF2M4",
			        "INP_BUF2M5","INP_BUF2M6","INP_BUF2M7","INP_BUF2M8"} };

  uint32_t value = 0;
  if (PRINT) cout << "Getting FIFO readback register" <<endl;

  if(mfec<1 || mfec>8) {
    cout<<" PixelFECInterface: Wrong mfec number "<<mfec<<endl;
    return 3;
  }
  if(channel<1 || channel>2) {
    cout<<" PixelFECInterface: Wrong mfec channel number "<<channel<<endl;
    return 2;
  }

  vmeDevicePtr->read(names[channel-1][mfec-1], &value);

//   switch (mfec) {
//   case 1:
//     if(channel==1)      vmeDevicePtr->read("INP_BUF1M1", &value);
//     else if(channel==2) vmeDevicePtr->read("INP_BUF2M1", &value);    
//     break;
//   case 2: 
//     if(channel==1)      vmeDevicePtr->read("INP_BUF1M2", &value);
//     else if(channel==2) vmeDevicePtr->read("INP_BUF2M2", &value);    
//     break;
//   case 3: 
//     if(channel==1)      vmeDevicePtr->read("INP_BUF1M3", &value);
//     else if(channel==2) vmeDevicePtr->read("INP_BUF2M3", &value);    
//     break;
//   case 4: 
//     if(channel==1)      vmeDevicePtr->read("INP_BUF1M4", &value);
//     else if(channel==2) vmeDevicePtr->read("INP_BUF2M4", &value);    
//     break;
//   case 5: 
//     if(channel==1)      vmeDevicePtr->read("INP_BUF1M5", &value);
//     else if(channel==2) vmeDevicePtr->read("INP_BUF2M5", &value);    
//     break;
//   case 6: 
//     if(channel==1)      vmeDevicePtr->read("INP_BUF1M6", &value);
//     else if(channel==2) vmeDevicePtr->read("INP_BUF2M6", &value);    
//     break;
//   case 7: 
//     if(channel==1)      vmeDevicePtr->read("INP_BUF1M7", &value);
//     else if(channel==2) vmeDevicePtr->read("INP_BUF2M7", &value);    
//     break;
//   case 8: 
//     //if(channel==1) vmeDevicePtr->readBlock("INP_BUF1M8", 4, (char *) value,
//     //				   HAL::HAL_NO_INCREMENT);
//     if(channel==1)      vmeDevicePtr->read("INP_BUF1M8", &value);
//     else if(channel==2) vmeDevicePtr->read("INP_BUF2M8", &value);    
//     break;
//   }

  //cout<<"from read back "<<hex<<value<<dec<<" "<<mfec<<" "<<channel<<endl;
  return (int) value;
}
//---------------------------------------------------------------------------------
// To read the mfec word with the HUB and byte COUNT
// Read data is returned in *data. Return same data..
// byte defines the offset to access the correct information 
// byte = 0 - transmitted HUB address
// byte = 1 - received HUB address
// byte = 2 - transmitted byte COUNT
// byte = 3 - received byte COUNT
// byte = 4 - return the full word  
int PixelFECInterface::getByteHubCount(const int mfec, const int channel,
				       const int byte, int * data) {
  const string names[2][8] = { {"STAT1_M1","STAT1_M2","STAT1_M3","STAT1_M4",
			        "STAT1_M5","STAT1_M6","STAT1_M7","STAT1_M8"},
			       {"STAT2_M1","STAT2_M2","STAT2_M3","STAT2_M4",
			        "STAT2_M5","STAT2_M6","STAT2_M7","STAT2_M8"} };

  uint32_t value = 0;
  int ret = 0;
  if (PRINT) cout << "Getting the HUB & BYTE COUNT register" <<endl;

  if(mfec<1 || mfec>8) {
    cout<<" PixelFECInterface: Wrong mfec number "<<mfec<<endl;
    return 3;
  }
  if(channel<1 || channel>2) {
    cout<<" PixelFECInterface: Wrong mfec channel number "<<channel<<endl;
    return 2;
  }

  vmeDevicePtr->read(names[channel-1][mfec-1], &value);

//   switch (mfec) {
//   case 1:
//     if(channel==1)      vmeDevicePtr->read("STAT1_M1", &value);
//     else if(channel==2) vmeDevicePtr->read("STAT2_M1", &value);    
//     else ret=2;
//     break;
//   case 2: 
//     if(channel==1)      vmeDevicePtr->read("STAT1_M2", &value);
//     else if(channel==2) vmeDevicePtr->read("STAT2_M2", &value);    
//     else ret=2;
//     break;
//   case 3: 
//     if(channel==1)      vmeDevicePtr->read("STAT1_M3", &value);
//     else if(channel==2) vmeDevicePtr->read("STAT2_M3", &value);    
//     else ret=2;
//     break;
//   case 4: 
//     if(channel==1)      vmeDevicePtr->read("STAT1_M4", &value);
//     else if(channel==2) vmeDevicePtr->read("STAT2_M4", &value);    
//     else ret=2;
//     break;
//   case 5: 
//     if(channel==1)      vmeDevicePtr->read("STAT1_M5", &value);
//     else if(channel==2) vmeDevicePtr->read("STAT2_M5", &value);    
//     else ret=2;
//     break;
//   case 6: 
//     if(channel==1)      vmeDevicePtr->read("STAT1_M6", &value);
//     else if(channel==2) vmeDevicePtr->read("STAT2_M6", &value);    
//     else ret=2;
//     break;
//   case 7: 
//     if(channel==1)      vmeDevicePtr->read("STAT1_M7", &value);
//     else if(channel==2) vmeDevicePtr->read("STAT2_M7", &value);    
//     else ret=2;
//     break;
//   case 8: 
//     if(channel==1)      vmeDevicePtr->read("STAT1_M8", &value);
//     else if(channel==2) vmeDevicePtr->read("STAT2_M8", &value);    
//     else ret=2;
//     break;
//   default:
//     ret=3;
//     break;
//   }

  if(byte<0||byte>4) {*data=0; ret=1;}   // signal out of range, return 0
  else if(byte==4) {*data = value;}      // for 4 return the whole register
  else {*data = (int) (value  >> (8*byte)) & 0xFF;} // for 0-3 return a byte

  return ret;
}


// ELSE CAEN ----------------------------------------------------------
// specific to direct CAEN library access

#else
PixelFECInterface::PixelFECInterface(const unsigned long fecBase, 
				     long aBHandle,
				     unsigned int fecCrate,
				     unsigned int fecSlot):
  fecCrate_(fecCrate),
  fecSlot_(fecSlot)
{
 // Constructor stuff
  if (PRINT) cout << "PixelFECInterface Constructor2 called" << endl;
  BHandle = aBHandle;
  dw = cvD32;
  am = cvA32_U_DATA;
  pfecvmeslot = fecBase;

  maxbuffersize_=500;

  mfecaddress[8] = 0x00;  mfecaddress[7] = 0x40;  mfecaddress[6] = 0x80;
  mfecaddress[5] = 0xC0;  mfecaddress[4] = 0x100;  mfecaddress[3] = 0x140;
  mfecaddress[2] = 0x180;  mfecaddress[1] = 0x1C0;

  if (PRINT) cout << "CAEN FEC constructor called with slot base of "<<hex<<pfecvmeslot<<dec<<endl;
 fecdebug = 0;

  for (unsigned int tfecchannel=1;tfecchannel<=2;tfecchannel++) {
    for (unsigned int tmfec=0;tmfec<9;tmfec++) {
      qbufnsend[tmfec][tfecchannel]=0;
      qbufnerr[tmfec][tfecchannel]=0;
      qbufn[tmfec][tfecchannel]=0;
      qbufn_old[tmfec][tfecchannel]=0;
    }
  }


}
PixelFECInterface::~PixelFECInterface(void) 
{
 // Destructor stuff
}

int PixelFECInterface::writeCSregister(int mfec, int fecchannel, 
				       int cscommand) {
  // expect the lower 8 bits of the CS register (shift for other channel)
  uint32_t value;
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

  //  value = value | cscommand;  //
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CSREG,
			   &value, am, dw);
  ret_error(ret);

  if (PRINT) {
    cout << "writeCSregister puts      : "<<hex<< value <<dec<<endl;
  }
    
  return 0;
}

int PixelFECInterface::resetdoh(const int mfec, const int fecchannel) {
  uint32_t value;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec]
			    + CSREG, &value, am, dw);
  ret_error(ret);
  if (PRINT) cout<<"RESET DOH read "<<hex<<value<<dec<<endl;
  ret = CAENVME_WriteCycle(BHandle,
			      pfecvmeslot + mfecaddress[mfec] + CSREG,
			      &value, am, dw);
  if (fecchannel == 1) {
  value = value | 0x00008000;
    ret = CAENVME_WriteCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + CSREG,
			          &value, am, dw);
    ret_error(ret);
    } else if (fecchannel == 2) {
  value = value | 0x80000000;
  ret = CAENVME_WriteCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + CSREG,
			      &value, am, dw);
  ret_error(ret);
    }
  if (PRINT) cout<<"RESET DOH puts  "<<hex<<value<<dec<<endl;
  return 0;
}

int PixelFECInterface::injectrstroc(const int mfec, const int bitstate) {
  uint32_t value, prevvalue;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec]
			  + CONTROLREG, &value, am, dw);
  ret_error(ret)
  prevvalue = value;
  if (PRINT) cout<<"Injectrstroc"<<endl;
  value = bitstate & 0x00000001;
  value = value | (prevvalue & 0xFFFFFFFE);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::injecttrigger(const int mfec, const int bitstate) {
  uint32_t value, prevvalue;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec]
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  if (PRINT) cout<<"Inject norm trigger"<<endl;
  value = bitstate & 0x00000001;
  value = value << 1;
  value = value | (prevvalue & 0xFFFFFFFD);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::injectrsttbm(const int mfec, const int bitstate) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Inject rst tbm"<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] 
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  value = bitstate & 0x00000001;
  value = value << 2;
  value = value | (prevvalue & 0xFFFFFFFB);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::injectrstcsr(const int mfec, const int bitstate) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Inject rst csr"<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec]
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  value = bitstate & 0x00000001;
  value = value << 3;
  value = value | (prevvalue & 0xFFFFFFF7);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::enablecallatency(const int mfec, const int bitstate) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"EnableCalLatency"<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] 
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  value = bitstate & 0x00000001;
  value = value << 4;
  value = value | (prevvalue & 0xFFFFFFEF);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::disableexttrigger(const int mfec, const int bitstate) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Disable Ext Trigger"<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] 
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  value = bitstate & 0x00000001;
  value = value << 5;
  value = value | (prevvalue & 0xFFFFFFDF);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::loopnormtrigger(const int mfec, const int bitstate) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Loop Norm Trigger"<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] 
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  value = bitstate & 0x00000001;
  value = value << 6;
  value = value | (prevvalue & 0xFFFFFFBF);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::loopcaltrigger(const int mfec, const int bitstate) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Loop cal trigger"<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] 
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  value = bitstate & 0x00000001;
  value = value << 7;
  value = value | (prevvalue & 0xFFFFF7F);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::callatencycount(const int mfec, const int latency) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Cal latency set count: " <<hex<<latency<<dec<<endl;;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] 
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  value = latency & 0x000000FF;
  value = value << 8;
  value = value | (prevvalue & 0xFFFF00FF);
  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::FullBufRDaDisable(const int mfec, const int disable) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Disable Full Buf RDa Check: " <<hex<<disable<<dec<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] 
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  if (disable != 0) value = prevvalue | 0x00100000;
  else value = prevvalue & 0xFFEFFFFF;

  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::testFiberEnable(const int mfec, const int enable) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Enable Fiber Test: " <<hex<<enable<<dec<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec]
                          + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  if (enable != 0) value = prevvalue | 0x00400000;
  else value = prevvalue & 0xFFBFFFFF;

  ret = CAENVME_WriteCycle(BHandle,
                           pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
                           &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::AllRDaDisable(const int mfec, const int disable) {
  uint32_t value, prevvalue;
  if (PRINT) cout<<"Disable All RDa: " <<hex<<disable<<dec<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] 
			  + CONTROLREG, &value, am, dw);
  ret_error(ret);
  prevvalue = value;
  if (disable != 0) value = prevvalue | 0x00200000;
  else value = prevvalue & 0xFFDFFFFF;

  ret = CAENVME_WriteCycle(BHandle,
			   pfecvmeslot + mfecaddress[mfec] + CONTROLREG,
			   &value, am, dw);
  ret_error(ret);
  return 0;
}
int PixelFECInterface::getversion(unsigned long *data) {
  uint32_t value;
  if (PRINT) cout << "Reading address:"<<hex<<(pfecvmeslot + 28)<<dec<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + 28, &value, am, dw);
  ret_error(ret);
  if (PRINT) cout << "CAEN READ:"<<hex<<value<<dec<<endl;
  *data = (value & 0xFF000000) >> 24;
  
  if (PRINT) cout<<"Get FEC version finds firmware version: "<<*data<<endl;
  return ret;
}

// Get FEC STATUS word. Includes QPLL and TTCrx ready bits 
iint PixelFECInterface::getStatus(void) {
  uint32_t value;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + 0x30C, &value, am, dw);
  ret_error(ret);
  if (PRINT) cout<<"Get FEC status "<<value<<endl;
  return value;
}

nt PixelFECInterface::getversion(const int mfec, unsigned long *data) {

  uint32_t value;
  cout << "\nWARNING: GETVERSION OLD VERSION CALLED.  DOES NOT NEED MFEC PARAM"<<endl;
  if (PRINT) cout << "Reading address:"<<hex<<(pfecvmeslot + 28)<<dec<<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + 28, &value, am, dw);
  ret_error(ret);
  if (PRINT) cout << "CAEN READ:"<<hex<<value<<dec<<endl;
  *data = (value & 0xFF000000) >> 24;
  
  if (PRINT) cout<<"Get FEC version finds firmware version: "<<*data<<endl;
  return 0;
}

int PixelFECInterface::outputbuffer(const int mfec, const int fecchannel,
				    unsigned long data) {
  if (fecchannel == 1) {
    ret = CAENVME_WriteCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + CH1OUT,
			     &data, am, dw);
    ret_error(ret);
  } else if (fecchannel == 2) {
  ret = CAENVME_WriteCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + CH2OUT,
			   &data, am, dw);
  ret_error(ret);
  }
  return 0;
}

int PixelFECInterface::outputblock(const int mfec, const int fecchannel,
				    unsigned int *data, int ndata) {
  int nb; //number of bytes written
  ret = CAENVME_SetFIFOMode(BHandle, 1);
  ret_error(ret);

  // need to send mult of 4 val for ndata
  if (ndata > 0) ndata = ((ndata-1)/4 + 1)*4;


  if (fecchannel == 1) {
    ret = CAENVME_BLTWriteCycle(BHandle,
				pfecvmeslot + mfecaddress[mfec] + CH1OUT,
				(char *) data,
				ndata,
				cvA32_U_BLT,
				cvD32,
				&nb);
    switch (ret)
      {
      case cvSuccess   :  if (PRINT) fprintf(stdout," BLK XFER Cycle completed normally\n");
	if (PRINT) fprintf(stdout," Written %u bytes  \n",nb); 
	break ;
      case cvBusError     : fprintf(stdout," BLK XFER Bus Error !!!\n");
	fprintf(stdout," Written %u bytes",nb);
	break ;
      case cvCommError : fprintf(stdout," BLK XFER Communication Error !!!");
	break ;
      default          : fprintf(stdout," BLK XFER Unknown Error !!!");
	break ;
      }
  } else if (fecchannel == 2) {
    ret = CAENVME_BLTWriteCycle(BHandle,
				pfecvmeslot + mfecaddress[mfec] + CH2OUT,
				(char *) data,
				ndata,
				cvA32_U_BLT,
				cvD32,
				&nb);
    switch (ret)
      {
      case cvSuccess   :  if (PRINT) fprintf(stdout," BLK XFER Cycle completed normally\n");
	if (PRINT) fprintf(stdout," Written %u bytes",nb); 
	break ;
      case cvBusError     : fprintf(stdout," BLK XFER Bus Error !!!\n");
	fprintf(stdout," Written %u bytes",nb);
	break ;
      case cvCommError : fprintf(stdout," BLK XFER Communication Error !!!");
	break ;
      default          : fprintf(stdout," BLK XFER Unknown Error !!!");
	break ;
      }
  }

  //  Leave BLK FIFO mode available through the BLTWriteCycle() function
  //    ret = CAENVME_SetFIFOMode(BHandle, 0);
  return 0;
}


void PixelFECInterface::mfecbusy(int mfec, int fecchannel,
				 unsigned int *ch1, unsigned int *ch2) {
  // mfecbusy reads the csreg and waits if the send_started is still
  // in effect.  This should change state on it's own.  Timeout used in
  // case it doesnt change.

  unsigned int value;
  int timeout;
  // CSTIMEOUT of 300 is large enough for a mfec to shift out an entire
  // rocinit (typical wait for small column wise op is 20)
#define CSTIMEOUT (10000)
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + CSREG,
			  &value, am, dw);
  ret_error(ret);

  *ch1 = (value >> 8) & 0x7F;
  *ch2 = (value >> 24) & 0x7F;

  timeout = 0;

  //  cout << "In mfecbusy, read back status:"<<hex<<*ch1<<dec<<endl;

  
  if ((fecchannel == 1)&&((*ch1 & 0x8)==0x8)&&(*ch1!=0)) { //send started -
    //wait until complete or timeout
    //      if (PRINT) {
    //cout << "<WAITING FOR READY (I) " << hex << value << " > ";
	//      }
    timeout = 0;
    do {
      ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + CSREG,
			      &value, am, dw);
      ret_error(ret);

	*ch1 = (int) (value >> 8) & 0x7F;
      timeout++;
      //      if (PRINT) {
      //cout << "<WAITING FOR READY (II)" << hex << value << " > ";
      //      }
    } while ((((*ch1 & 0x8)==0x8)&&(*ch1!=0)) && (timeout < CSTIMEOUT));

    if (PRINT)     cout << "CH1 timeout=" << dec << timeout <<endl;

    if (timeout>=CSTIMEOUT) { cout << "ERROR mfecbusy TO 1"<<endl; }
  }
  if ((fecchannel == 1) && (fecdebug > 0)) {
    if (((*ch1 & 0x02) != 0x02) && (*ch1 != 0))  { 
      analyzeFecCSRChan(mfec,1, *ch1);
      if (fecdebug == 2) writeCSregister(mfec, fecchannel, 0x08);
    }
  }
  if ((fecchannel == 2)&&((*ch2 & 0x8)==0x8)&&(*ch2!=0)) { //send started - 
    //wait until complete or timeout
    //if (PRINT) {
    //cout << "<WAITING FOR READY (III)" << hex << value << " > ";
    //}
    timeout = 0;
    do {
      ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + CSREG,
			      &value, am, dw);
      ret_error(ret);

	*ch2 = (int) (value >> 24) & 0x7F;
      timeout++;
      //if (PRINT) {
      //cout << "<WAITING FOR READY (IV)" << hex << value << " > ";
      //}
    } while (((*ch2 & 0x8) == 0x8) && (timeout < CSTIMEOUT));
    //if (PRINT) 
    //cout << "CH2 timeout=" << dec << timeout <<endl;
    
    if (timeout>=CSTIMEOUT) { cout << "ERROR mfecbusy TO 2"<<endl; }

  }
  if ((fecchannel == 2) && (fecdebug > 0)) {
    if (((*ch2 & 0x02) != 0x02) && (*ch2 != 0)) {
      analyzeFecCSRChan(mfec, 2, *ch2);
      if (fecdebug == 2) writeCSregister(mfec, fecchannel, 0x08);
    }
  }
  
  mfecCSregister[mfec] = value;

}


// This sets up the Trigger FPGA configuration register (SSID) to
// PIXELS (100)
int PixelFECInterface::setssid(const int ssid) {
  uint32_t value;
  if (PRINT) cout << "Set SSID in Trigger FPGA to:"<<hex<<ssid<<dec<<endl;
  unsigned long data;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + SSID, &value, am, dw);
  ret_error(ret);
  value = value & 0xFFFF0FFF; // mask for doing xor
  data = ssid << 12;
  data = data | value;
  ret = CAENVME_WriteCycle(BHandle, pfecvmeslot + SSID, &data, am, dw);
  ret_error(ret);
  if (PRINT) cout << "CAEN writing SSID register: "<<hex<<data<<dec<<endl;
  // Now reread the ssid register to see if it's been set to param
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + SSID, &value, am, dw);
  ret_error(ret);
  value = value & 0x0000F000;
  value = value >> 12;
  if (value != (unsigned long) ssid) {
    cout << "\nERROR PixelFECInterface SSID unable to change: " <<
      value << ", " << ret << endl;
  }
  return ret;
}

int PixelFECInterface::getfecctrlstatus(const int mfec, unsigned long *data) {
  // read the mfec combined control/status register which returns and sets
  // functions of ch1 and ch2 independently
  uint32_t value;
  if (PRINT) cout << "Getting FEC cntrstatus register" <<endl;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + CSREG,
			  &value, am, dw);
  ret_error(ret);
  *data = value;
  return ret;
}

// To read back the information from the mFEC input FIFOs
// Return the word from the FIFO
int PixelFECInterface::readback(const int mfec, const int channel) {
  int INPREG = 4;
  if(channel==2) INPREG=20;
  uint32_t value;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + INPREG,
                          &value, am, dw);
  //cout<<" from, readback "<<hex<<value<<dec<<endl;
  ret_error(ret);
  return value;
}
// To read the mfec word with the HUB and byte COUNT
// Read data is returned in *data. Return status.
// byte defines the offset to access the correct information 
// byte = 0 - transmitted HUB address
// byte = 1 - received HUB address
// byte = 2 - transmitted byte COUNT
// byte = 3 - received byte COUNT  
int PixelFECInterface::getByteHubCount(const int mfec, const int channel,
				       const int byte, int * data) {
  int offset=0;
  if (channel==1){ offset=8; }else if (channel==2){ offset=24;}
  uint32_t value;
  ret = CAENVME_ReadCycle(BHandle, pfecvmeslot + mfecaddress[mfec] + offset,
                          &value, am, dw);
  ret_error(ret);

  if(byte<0||byte>4) {*data=0; ret=1;}   // signal out of range, return 0
  else if(byte==4) {*data = value;}      // for 4 return the whole register
  else {*data = (int) (value  >> (8*byte)) & 0xFF;} // for 0-3 return a byte

  return ret;
}

#endif // ENDIF CAEN

///////////////////////////////////////////////////////////////////
// Here are some routines in common to both HAL and CAEN
// Set the debug flag
void PixelFECInterface::fecDebug(int newstate) {
  if ((fecdebug == 0) && (newstate > 0))
    cout << "\nSetting FECDEBUG to ON (" << newstate << ")" << endl;
  if ((fecdebug > 0) && (newstate == 0)) 
    cout << "\nSetting FECDEBUG to OFF (" << newstate << ")" << endl;
  fecdebug = newstate;
}
//------------------------------------------------------------------------------
//
unsigned char PixelFECInterface::cinttogray(unsigned int igray) {
  // cintogray
  if (PRINT) cout<<"CINTTOGRAY "<<igray<<" -> "<<(igray^(igray>>1))<<endl;;
  return (igray^(igray>>1));
}
//------------------------------------------------------------------------------
// Analyze the CSR register, print errors.
void PixelFECInterface::analyzeFecCSRChan(int mFEC, int ichan, unsigned int ival) {
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
int PixelFECInterface::qbufsend(void) {
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
int PixelFECInterface::qbufsend(int mfec, int fecchannel) {
  unsigned int ch1stat, ch2stat;

  // append a final FF to data
  qbuf[mfec][fecchannel][qbufn[mfec][fecchannel]++] = 0xff;

  //  cout << "\nqbufsend writing "<<qbufn[mfec][fecchannel]<<" bytes\n";

  qbufnsend[mfec][fecchannel]++;

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
  writeCSregister(mfec, fecchannel, 0x8e);

#ifdef BLOCKTRANSFER
  if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << qbufn[mfec][fecchannel] <<endl;
      outputblock(mfec, fecchannel,
		  (unsigned int*) qbuf[mfec][fecchannel],
		  qbufn[mfec][fecchannel]);
#else
  unsigned long *iword;
  int i;
      // Now load the data
      for (i=0;i<qbufn[mfec][fecchannel];i+=4) {
	iword = (long unsigned int*) &qbuf[mfec][fecchannel][i];
	outputbuffer(mfec, fecchannel,  *iword);
	if (PRINT) {
	  cout<<"Final FEC data "<<  "("<<i<<hex<<"): "<< *iword ;
	  cout<< "   ndata:" << dec << qbufn[mfec][fecchannel] <<endl;
	}
      }

#endif
      // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
  writeCSregister(mfec, fecchannel, 0x8f);

  qbufn[mfec][fecchannel] = 0;

  return 0;
}
//-----------------------------------------------------------------------------------------
// Set the WBC + reset
int PixelFECInterface::rocsetwbc(int mfec, int mfecchannel, int tbmchannel,
			      int hubaddress, int portaddress, int rocid,
			       int wbcvalue) {

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
int PixelFECInterface::rocsetchipcontrolregister(int mfec, int mfecchannel,
						 int hubaddress,
						 int portaddress,
						 int rocid,
						 int calhighrange,
						 int chipdisable,
						 int halfspeed, 
						 const bool buffermode) {
  if (halfspeed)
    printf("JMT PixelFECInterface::rocsetchipcontrolregister halfspeed is %i and it's not used any more\n", halfspeed);
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
int PixelFECInterface::tbmreset(int mfec, int fecchannel, int tbmchannel,
				int hubaddress) {
  return tbmcmd(mfec, fecchannel, tbmchannel, hubaddress, 4, 2, 16, 0);
}
// Reset ROC
int PixelFECInterface::rocreset(int mfec, int fecchannel, int tbmchannel,
				int hubaddress) {
 return tbmcmd(mfec, fecchannel, tbmchannel, hubaddress, 4, 2, 4, 0);
}
//
//--------------------------------------------------------------------------------------
// Clear calibrate, works for the whole ROC
// Single of buffered command. 
int PixelFECInterface::clrcal(int mfec, int fecchannel,
			      int hubaddress, int portaddress, int rocid,
			      bool buffermode) {
#ifndef BLOCKTRANSFER  
    unsigned long *iword;
#endif
    unsigned char txdata[1024];
    unsigned int ch1stat, ch2stat;
    int current, i, ndata;
    
 
    if (buffermode) {

      if (PRINT) cout << "Buffer mode clrcal"<<endl;

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

      if (PRINT) cout << "Direct mode clrcal"<<endl;

      if (qbufn[mfec][fecchannel] > 0)  {
	qbufsend(mfec,fecchannel);
	cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode"<<endl;
      }
      if (PRINT) cout << "CLRCAL ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"
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
      
#ifdef BLOCKTRANSFER
      if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata <<endl;
      outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
      // Now load the data
      for (i=0;i<ndata;i+=4) {
	iword = (long unsigned int*) &txdata[i];
	outputbuffer(mfec, fecchannel,  *iword);
	if (PRINT) cout<<"Final FEC data (ndata:"<<dec<<ndata<<dec<<")  ("<<i<<"): "<< *iword <<endl;
      }
#endif
      // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
      writeCSregister(mfec, fecchannel, 0x07);
      
      if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
      
      return 0;
    }
}
//-----------------------------------------------------------------------------------------
// Program 1 pixel, separate trim&mask
int PixelFECInterface::progpix1(int mfec, int fecchannel,
			      int hubaddress, int portaddress, int rocid,
			       int coladdr, int rowaddress,
			       int mask, int trim,
			       bool buffermode) {
  // set trims on a pixel
  //const bool PRINT=true;
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif
  unsigned char coltemp0, coltemp1, coltemp2, tmask, ttrim, databyte;
  unsigned char txdata[1024];
  unsigned int ch1stat, ch2stat;
  int current, i, ndata;

  if (buffermode) {

    if (PRINT) cout<<"progpix1 buffered mask and trim"<<endl;
    if (PRINT) cout << "PROGPIX1 ROC CMD: mfec:"<<dec<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" trim:"<<trim<<endl;

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
    if (PRINT) cout<<"progpix1 direct mask and trim"<<endl;
    if (PRINT) cout << "PROGPIX1 ROC CMD: mfec:"<<dec<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" trim:"<<trim<<" mask:"<<mask<<endl;
    
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
      if (PRINT) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
    }
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    if (PRINT) cout <<endl;
#ifdef BLOCKTRANSFER
    if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata <<endl;
    outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
    // Now load the data
    for (i=0;i<ndata;i+=4) {
      iword = (long unsigned int*) &txdata[i];
      outputbuffer(mfec, fecchannel,  *iword);
      if (PRINT) cout<<"Final FEC data (ndata:"<<dec<<ndata<<dec<<")  ("<<i<<"): "<< *iword <<endl;
    }
#endif
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
    
    return 0;
  }
}
//----------------------------------------------------------------------------------------------------
// Program one pixel, mask&trim packed together in a char.
int PixelFECInterface::progpix(int mfec, int fecchannel,
			       int hubaddress, int portaddress, int rocid,
			       int coladdr, int rowaddress,
			       unsigned char databyte,
			       bool buffermode) {
  //const bool PRINT = true;
  // set trims on a pixel
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif

  unsigned char txdata[1024];
  unsigned int ch1stat, ch2stat;
  int current, i, ndata;
  unsigned char coltemp0, coltemp1, coltemp2;
 
  //unsigned int nnn = (unsigned int) databyte;
  //if(hubaddress==4 && rocid==0 && coladdr==0 && rowaddress==0) 
  //cout << "PROGPIX, hubaddress:"<<hubaddress<<" portaddress:"
  // <<portaddress<<" rocid:"<<rocid<<" "<<coladdr<<" "<<rowaddress<<" "<<hex<<nnn<<dec<<" "<<buffermode<<endl;

  if (buffermode) {
    if (PRINT) cout<<"progpix buffered databyte"<<endl;
    if (PRINT) cout << "PROGPIX Q ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"
		    <<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" databyte:"<<std::hex<<(unsigned int)(databyte)<<std::dec<< endl;
    
    assert(0<=coladdr);assert(coladdr<=51);assert(0<=rowaddress);assert(rowaddress<=79);
    
    if (qbufn[mfec][fecchannel] >= maxbuffersize_)  {
      //cout << "PixelFECInterface::progpix:ERROR mfec " << mfec <<":"<<fecchannel<<" OVER BUFFER LIMIT("<<qbufn[mfec][fecchannel]<<")"<<endl;
      if (PRINT) cout << "qbufn = " << qbufn[mfec][fecchannel] << " > maxbufsize = " << maxbuffersize_ << "; qbufsending..." << endl;
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

    if (PRINT) cout<<"progpix direct databyte"<<endl;
    if (PRINT) cout << "PROGPIX ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"
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
      if (PRINT) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
    }
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0; 
    
    if (PRINT) cout <<endl;
#ifdef BLOCKTRANSFER
    if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata <<endl;
    outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
    // Now load the data
    for (i=0;i<ndata;i+=4) {
      iword = (long unsigned int*) &txdata[i];
      outputbuffer(mfec, fecchannel,  *iword);
      if (PRINT) cout<<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
#endif
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
    
    return 0;
  }
}
//--------------------------------------------------------------------------------------------
int PixelFECInterface::calpix(int mfec, int fecchannel,
			      int hubaddress, int portaddress, int rocid,
			      int coladdr, int rowaddress,
			      int caldata,
			      bool buffermode) {
  // set trims on a pixel
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif
  unsigned char txdata[1024];
  int current, i, ndata;
  unsigned int ch1stat, ch2stat;
  unsigned char coltemp0, coltemp1, coltemp2;

  if (buffermode) {
    if (PRINT) cout << "CALPIX ROC CMD: mfec:"<<dec<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" caldata:"<<caldata<<endl;
    
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

    if (PRINT) cout << "CALPIX ROC CMD: mfec:"<<dec<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" caldata:"<<caldata<<endl;
    
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
      if (PRINT) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
    }
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    if (PRINT) cout <<endl;
#ifdef BLOCKTRANSFER
    if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata <<endl;
    outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
    // Now load the data
    for (i=0;i<ndata;i+=4) {
      iword = (long unsigned int*) &txdata[i];
      outputbuffer(mfec, fecchannel,  *iword);
      if (PRINT) cout<<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
#endif
    
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
    
    return 0;
  }
}
//----------------------------------------------------------------------------
// Enable or disable a dcol. Does NOT program pixels.
int PixelFECInterface::dcolenable(int mfec, int fecchannel,
				  int hubaddress, int portaddress, int rocid,
				  int dcol, int dcolstate,
				  bool buffermode) {
  // Enable or disable a double column
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif
  unsigned char txdata[1024];
  int current, i, ndata;
  unsigned int ch1stat, ch2stat;
  unsigned char coltemp0;

  if (buffermode) {
    if (PRINT) cout << "dcol CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" dcol:"<<dcol<<" dcolstate:"<<dcolstate<<endl;
    
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

    if (PRINT) cout << "dcol CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" dcol:"<<dcol<<" dcolstate:"<<dcolstate<<endl;
    
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
      if (PRINT) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
    }
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    if (PRINT) cout <<endl;
#ifdef BLOCKTRANSFER
    if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata <<endl;
    outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
    // Now load the data
    for (i=0;i<ndata;i+=4) {
      iword = (long unsigned int*) &txdata[i];
      outputbuffer(mfec, fecchannel,  *iword);
      if (PRINT) cout<<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
#endif
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
    
    return 0;
  }
}
//---------------------------------------------------------------------------------------------------
// Program one dac
int PixelFECInterface::progdac(int mfec, int fecchannel,
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

#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif
  unsigned char txdata[1024];
  int current, ndata;
  unsigned int ch1stat, ch2stat;



  if (buffermode) {

    if (PRINT) cout << "PROGDAC ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" dacaddress:"<<dacaddress<<" dacvalue:"<<dacvalue<<endl;

    if (qbufn[mfec][fecchannel] >= maxbuffersize_)  {
      //cout << "PixelFECInterface::progdac: ERROR mfec " << mfec <<":"<<fecchannel<<" OVER BUFFER LIMIT ("<<qbufn[mfec][fecchannel]<<")"<<endl;;
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
    
    if (PRINT) cout << "PROGDAC ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" dacaddress:"<<dacaddress<<" dacvalue:"<<dacvalue<<endl;

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
    txdata[current++] = 0;
    
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
    
#ifdef BLOCKTRANSFER
    // if (PRINT)     
    //if(fecchannel==1 && hubaddress==31 && rocid==0 && dacaddress>27) 
    //cout << "Using BLOCK TRANSFER 02 ndata:" << dec << ndata <<endl;
      outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
    // Now load the data
    for (int i=0;i<ndata;i+=4) {
      iword = (long unsigned int*) &txdata[i];
      outputbuffer(mfec, fecchannel,  *iword);
      if (PRINT) cout<<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
    }
#endif
    
    // reset and send GO
    writeCSregister(mfec, fecchannel, 0x07);
    
    if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
    
    return 0;
  }
}
//----------------------------------------------------------------------------------------
// Program all DACS, use multi single commands
// 26 DACs + WBC + control, add the temperature register
int PixelFECInterface::progalldacs(int mfec, int fecchannel,
                                   int hubaddress, int portaddress, int rocid,
                                   int wbc, int chipcontrol,
                                   const std::vector<unsigned char>& dacs) {
  assert(0);
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif
  unsigned char txdata[1024];
  int current, i, ndata;
  unsigned int ch1stat, ch2stat;

  if (PRINT) cout << "PROGALLDACS ROC CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<endl;

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


#ifdef BLOCKTRANSFER
  if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata <<endl;
      outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
      // Now load the data
      for (i=0;i<ndata;i+=4) {
    iword = (long unsigned int*) &txdata[i];
    outputbuffer(mfec, fecchannel,  *iword);
    if (PRINT) cout<<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
      }
#endif
      // reset and send GO
    writeCSregister(mfec, fecchannel, 0x07);

  if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 

  return 0;
}


// ROC DACs 
//---------------------------------------------------------------------------------
// Use single or buffered commands
// Extend to  buffered commands
void PixelFECInterface::setAllDAC(const PixelHdwAddress& theROC,
                                  const std::vector<unsigned int>& dacs,
				  const bool buffermode) {

  assert(dacs.size()==30);
  static bool jmt_warned = false;
  if (!jmt_warned) {
    printf("****************************************************************************\n");
    printf("JMT needs to fix the hardcoded magic numbers in PixelFECInterface::setAllDAC\n");
    printf("                           (this is the only warning)\n");
    printf("****************************************************************************\n");
    jmt_warned = true;
  }

  //std::cout << "In PixelFECInterface::setAllDAC "<<theROC.mfec()<<" "<<theROC.mfecchannel()<<" "
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
void PixelFECInterface::setMaskAndTrimAll(const PixelHdwAddress& theROC,
                                          const std::vector<unsigned char>& allPixels,
					  const bool buffermode) {
  std::cout << "In PixelFECInterface::setMaskAndTrimAll" << std::endl;

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
void PixelFECInterface::setDcolEnableAll(const PixelHdwAddress& theROC,
					 unsigned char mask,
					 const bool buffermode) {
  std::cout << "In PixelFECInterface::setDcolEnableAll" << std::endl;

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
int PixelFECInterface::rocinit(int NCOLS, int mfec, int fecchannel,
			      int hubaddress, int portaddress, int rocid,
			       int mask, int trim) {

  // set trims/masks on a roc all to same one value

#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif

  unsigned char txdata[1024];
  int current, i, ndata, ic;
  unsigned int ch1stat, ch2stat;
  unsigned char coltemp0, coltemp1, coltemp2, tmask, ttrim, databyte;
  if (PRINT) cout << "ROCINIT CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" trim:"<<trim<<endl;

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

  // Now send the number of columns of compressed data to follow
  //  txdata[current++] = 52; // full chip of cols is 52
  //  txdata[current++] = 10; // full chip of cols is 52
  txdata[current++] = NCOLS; // full chip of cols is 52

 
  // Now set the mask and trim
  tmask = (mask & 0x01) << 7;
  ttrim = trim & 0x0F;
  databyte = tmask | ttrim;
  
  //for (ic=0;ic<52;ic=ic++) {
  for (ic=0;ic<NCOLS;++ic) {
    if (PRINT) cout << "CURRENT COLUMN:" << ic << endl;
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
    //if (PRINT) cout << "databyte: " << hex << unsigned(databyte) << dec << endl;
  }
  
  if (PRINT) cout << "ROCINIT BUFFER USAGE: "<< dec  <<current<< " bytes."<<endl;
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

  if (PRINT) cout <<endl;
  // Now load the data
#ifdef BLOCKTRANSFER
      if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata << endl;
      outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
  for (i=0;i<ndata;i+=4) {
    iword = (long unsigned int*) &txdata[i];
    //    usleep(10); /* temp 18.5 */
    outputbuffer(mfec, fecchannel, *iword);
    if (PRINT) cout<<"Final FEC data (ndata:"<<ndata<<")  ("<<i<<"): "<< *iword <<endl;
  }
#endif
    // reset and send it
  writeCSregister(mfec, fecchannel, 0x37);

  if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 

  return 0;
}
//------------------------------------------------------------------------------------
// Load trims&masks to all pixels in a ROC. 
// Use the column mode, 8 columns at a time.
int PixelFECInterface::roctrimload(int mfec, int fecchannel,
				   int hubaddress, int portaddress,
				   int rocid,
				   const std::vector<unsigned char>& allPixels){
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif

  unsigned char txdata[1024];
  int current, i, j, ndata, ic;
  unsigned char coltemp0, coltemp1, coltemp2, databyte;
  int pixptr;
  unsigned int ch1stat, ch2stat;
  unsigned int idatabyte;
  int incol;
  int incolMax = 8;

  //  unsigned short int cdata;

  // Check that there is nothing for the buffer mode?
  if (qbufn[mfec][fecchannel] > 0)  {
    qbufsend(mfec,fecchannel);
    cout << "mfec " << mfec <<":"<<fecchannel<<" leftover from buffer mode "
	 <<qbufn[mfec][fecchannel]<<endl;
  }
  mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 


  if (PRINT) cout << "ROCTRIMLOAD CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" "<<endl;

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
    if (PRINT) cout << "CURRENT COLUMN:" << ic << endl;
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
      if (PRINT) cout << "column (grayed/hex): " << hex << (int) coltemp2 << dec<<endl;
      for (j=0;j<80;j++) {
	// Now send the mask and trim
	databyte = allPixels[pixptr++];
	//if (PRINT) cout << "databyte=" << hex << (unsigned)(databyte) << dec << endl;
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
    if (PRINT) cout << "TOTAL byte count: " << dec << current <<endl;
    for (i=0;i<current;i++) {
      if (PRINT) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
    }
    
    
    // add 3 zeros to pad remainder of 4 byte word
    ndata = current;
    txdata[current++] = 0;
    txdata[current++] = 0;
    txdata[current++] = 0;
    
    if (PRINT) cout <<endl;
    // Now load the data
    
#ifdef BLOCKTRANSFER
    if (PRINT) cout << "Using BLOCK TRANSFER"<<endl;
    outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
    for (i=0;i<ndata;i+=4) {
      iword = (long unsigned int*) &txdata[i];
      //    usleep(10); /* temp 18.5 */
      outputbuffer(mfec, fecchannel, *iword);
      if (PRINT) cout<<"Final FEC data (ndata:"<<ndata<<")  ("<<i<<"): "<< *iword <<endl;
    }
#endif
    writeCSregister(mfec, fecchannel, 0x17);
    
  }
  
  if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
  
  return 0;
  
}
// Load trims&masks to a group of columns, columns<=8.
// Use the fast column mode.
//-----------------------------------------------------------------------------------------------------
int PixelFECInterface::coltrimload(int mfec, int fecchannel,
				   int hubaddress, int portaddress,
				   int rocid,
				   int colstart, int numcols,
				   const std::vector<unsigned char>& allPixels){
  // coltrimload sets between 1 and 12 cols (max TX buffer size) with mask/trim
  // data
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif

  unsigned char txdata[1024];
  int current, i, j, ndata;
  unsigned char coltemp0, coltemp1, coltemp2, databyte;
  int pixptr;
  unsigned int ch1stat, ch2stat;
  unsigned int idatabyte;
  int incol;

  //  unsigned short int cdata;

  if (PRINT) cout << "COLTRIMLOAD CMD: mfec:"<<mfec<<" fecchannel:"<<fecchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" rocid:"<<rocid<<" "<<endl;

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
    if (PRINT) cout<<"column(grayed/hex):"<<hex<<(int) coltemp2<<dec<<endl;
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
  
#ifdef BLOCKTRANSFER
  if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata << endl;
  outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
  for (i=0;i<ndata;i+=4) {
    iword = (long unsigned int*) &txdata[i];
    //    usleep(10); /* temp 18.5 */
    outputbuffer(mfec, fecchannel, *iword);
    if (PRINT) cout<<"Final FEC data (ndata:"<<ndata<<")  ("<<i<<"): "<< *iword <<endl;
  }
#endif
  
  writeCSregister(mfec, fecchannel, 0x17);
  
  if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 
  return 0;
  
}
//-------------------------------------------------------------------------------------------------
// Send same mask&trim to all pixels in one column.
// Uses multiple commands.
int PixelFECInterface::sendcoltoroc(const int mfec, int fecchannel,
				    int hubaddress, int portaddress, int rocid,
				    int coladdr, int mask, int trim) {
  unsigned char coltemp0, coltemp1, coltemp2, tmask, ttrim;
  int current, krow, rowaddress, i, ndata;
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif
  unsigned char databyte;
  unsigned int ch1stat, ch2stat;
  unsigned char txdata[1024];

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

  // Now load the data
#ifdef BLOCKTRANSFER
      if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata <<endl;
      outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
      for (i=0;i<ndata;i+=4) {
	iword = (long unsigned int*) &txdata[i];
	//    usleep(10); /* temp 18.5 */
	outputbuffer(mfec, fecchannel, *iword);
	if (PRINT) cout<<"Final FEC data (ndata:"<<ndata<<")  ("<<i<<"): "<< *iword <<endl;
      }
#endif

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
int PixelFECInterface::tbmread(int mfec, int fecchannel,
           int tbmchannel, int hubaddress, int portaddress,int offset) {
  const int direction = 1;
  const int databyte = 0xffffffff;
  unsigned int t;
#ifndef BLOCKTRANSFER
  unsigned long *iword;
#endif
  unsigned int ch1stat, ch2stat;
  unsigned char txdata[1024];
  int current, i, ndata;
  if (PRINT) cout << "TBM Command: mfec:"<<mfec<<" tbmchannel:"<<tbmchannel<<" hubaddress:"<<hubaddress
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
    if (PRINT) cout<<" ("<<hex<< (int) txdata[i]<<") ";
  }
  // add 3 zeros to pad remainder of 4 byte word
  ndata = current;
  txdata[current++] = 0;
  txdata[current++] = 0;
  txdata[current++] = 0;

  if (PRINT) cout <<endl;

  // Reset ALL buffer memory pointer
  writeCSregister(mfec, fecchannel, 0x08);

#ifdef BLOCKTRANSFER
  if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata << "\n";
      outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
      // Now load the data
      for (i=0;i<ndata;i+=4) {
    iword = (long unsigned int*) &txdata[i];
    outputbuffer(mfec, fecchannel,  *iword);
    if (PRINT) cout<<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<endl;
      }
#endif
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
int PixelFECInterface::tbmcmd(int mfec, int fecchannel,
	   int tbmchannel, int hubaddress, int portaddress,
	   int offset, int databyte, int direction) {
  unsigned int t;
#ifndef BLOCKTRANSFER  
  unsigned long *iword;
#endif
  unsigned int ch1stat, ch2stat;
  unsigned char txdata[1024];
  int current, i, ndata;
  if (PRINT) cout << "TBM Command: mfec:"<<mfec<<" tbmchannel:"<<tbmchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<portaddress<<" offset:"<<offset<<" databyte:"<<databyte<<" direction:"<<direction<<endl;

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
     if (PRINT) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
  }
  // add 3 zeros to pad remainder of 4 byte word
  ndata = current;
  txdata[current++] = 0;
  txdata[current++] = 0;
  txdata[current++] = 0;

  if (PRINT) cout <<endl;

  // Reset ALL buffer memory pointer
  writeCSregister(mfec, fecchannel, 0x08);

#ifdef BLOCKTRANSFER
  if (PRINT) cout << "Using BLOCK TRANSFER ndata:" << dec << ndata << endl;
      outputblock(mfec, fecchannel, (unsigned int*) txdata, ndata);
#else
      // Now load the data
      for (i=0;i<ndata;i+=4) {
    iword = (long unsigned int*) &txdata[i];
    outputbuffer(mfec, fecchannel,  *iword);
    if (PRINT) cout<<"Final FEC data (ndata:"<<hex<<ndata<<")  ("<<i<<"): "<< *iword <<dec<<endl;
      }
#endif
      // Now data is in txdata.  Ready to initiate tx.  Reset and send go.
  writeCSregister(mfec, fecchannel, 0x07);

  if (fecdebug == 2) mfecbusy(mfec, fecchannel, &ch1stat, &ch2stat); 

 return 0;
}
//--------------------------------------------------------------------------------
// Set TBM speed
int PixelFECInterface::tbmspeed(int mfec, int fecchannel, int tbmchannel,
				int hubaddress, 
				int speed) {
  return tbmcmd(mfec, fecchannel, tbmchannel, hubaddress, 4, 0, speed, 0);
}
//
int PixelFECInterface::tbmspeed2(int mfec, int fecchannel, 
				 int tbmchannel, int hubaddress, int portaddress){
  unsigned int t;
  unsigned long *iword;
  unsigned int ch1stat, ch2stat;
  unsigned char txdata[1024];
  int current, i, ndata;
  if (PRINT) cout << "TBM Command: mfec:"<<mfec<<" tbmchannel:"<<tbmchannel<<" hubaddress:"<<hubaddress<<" portaddress:"<<endl;

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
    if (PRINT) cout<<" ("<<hex<< (int) txdata[i]<<") "<<dec;
  }
  // add 3 zeros to pad remainder of 4 byte word
  ndata = current;
  txdata[current++] = 0;
  txdata[current++] = 0;
  txdata[current++] = 0;

  if (PRINT) cout <<endl;

  // Reset ALL buffer memory pointer
  writeCSregister(mfec, fecchannel, 0x08);

  // Now load the data
  for (i=0;i<ndata;i+=4) {
    iword = (long unsigned int*) &txdata[i];
    outputbuffer(mfec, fecchannel, *iword);
    if (PRINT) cout<<"Final FEC data (ndata:"<<ndata<<")  ("<<i<<"): "<< *iword <<endl;
  }

  // Now data is in txdata.  Ready to initiate tx.  Reset and send go.

  writeCSregister(mfec, fecchannel, 0x07);
  if (PRINT) cout << "SENDDATA mfec:"<<hex<<mfec<<dec;
 
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
int PixelFECInterface::testFiber(const int mfec, const int channel,
                                 int* rda, int * rck) {
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
int PixelFECInterface::delay25Test(int mymfec, 
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

  //cout<<mymask<<" "<<mytrim<<" "<<nTry<<" "<<commands<<endl;

  success0 = success1 = success2 = success3 = success4 = 0;
  int cntgood, cntbad, j;
  unsigned int ch1, ch2;
  unsigned long data=0;
  unsigned char databyte;

  unsigned int         dataReceivedMask=0x00000200;
  if (myfecchannel==2) dataReceivedMask=0x02000000;

  std::vector <unsigned char> testroc2 ((80*52), 0x00);

  databyte =  ((mymask << 7)|mytrim);

  assert(databyte==0);

  //  cout << "DATABYTE:" << hex <<(unsigned int) databyte <<dec<<endl;

  for (j=0;j<(80*52);j++) testroc2[j]= databyte;
   
  cntgood=0; cntbad = 0;
  for (j=0;j<nTry;j++) {
    calpix(mymfec, myfecchannel, myhubaddress, myportaddress,

		 myrocid, 0, 0, 1);
    mfecbusy(mymfec, myfecchannel, &ch1, &ch2);
    getfecctrlstatus(mymfec,&data);  

    if ((data & dataReceivedMask) == dataReceivedMask) {  // receive complete
      cntgood++;
    } else {
      cntbad++;
    }

    //cout<<"-1- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<" "<<ch1<<" "<<ch2<<dec<<endl;
    if(cntbad == 4) { //this point is clearly nonoptimal, so give up
      //break;
      return 0;
    }

  }

  success0 = cntgood;



  cntgood=0; cntbad = 0;

  for (j=0;j<nTry;j++) {

    tbmspeed(mymfec,myfecchannel,mytbmchannel,myhubaddress, 1);

    mfecbusy(mymfec, myfecchannel, &ch1, &ch2);

    getfecctrlstatus(mymfec,&data);  

    if ((data & dataReceivedMask) == dataReceivedMask) {
      
      // receive complete

      cntgood++;

    } else {

      cntbad++;
      
    }
    //cout<<"-2- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<dec<<endl;
    if(cntbad == 4) {
      //break;
      return 0;
    }
    
  }
  
  success1 = cntgood;



  int masksetting, trimsetting;

  cntgood=0; cntbad = 0;

  if ((databyte & 0x80)==0x80) masksetting = 1;

  else masksetting = 0;

  trimsetting = (0xf)&databyte;

				 

  for (j=0;j<nTry;j++) {

    rocinit(52, mymfec,myfecchannel,myhubaddress,myportaddress,myrocid,
            masksetting,trimsetting);

    mfecbusy(mymfec, myfecchannel, &ch1, &ch2);

    getfecctrlstatus(mymfec,&data);  

    if ((data & dataReceivedMask) == dataReceivedMask) {

      // receive complete

      cntgood++;

    } else {

      cntbad++;

    }
    //cout<<"-3- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<dec<<endl;
    if(cntbad == 4) {
      //break;
      return 0;
    }

  }

  success2 = cntgood;


  if(commands == 0 || commands == 1) {

    cntgood=0; cntbad = 0;
    
    for (j=0;j<nTry;j++) {

      std::string filename;

      if (getenv("BUILD_HOME")==0){
	filename=std::string(getenv("XDAQ_ROOT"))+"/dat/PixelFECInterface/dat/infeccmd.dat";
      }
      else {
	filename=std::string(getenv("BUILD_HOME"))+"/pixel/PixelFECInterface/dat/infeccmd.dat";
      }
      
      std::ifstream infec(filename.c_str());
      
      assert(infec.is_open());
      assert(infec.good());
      assert(!infec.eof());
      
      string strbuf;
      
      for(int i=0;i<j+1;i++) {
	getline(infec,strbuf);
	//cout <<"strbuf="<<strbuf<<endl;
	assert(!infec.eof());
      }
      
      //cout <<"strbuf="<<strbuf<<endl;
      
      /*
	if (in.eof()) {
	cout << "Found eof will close and open again"<<endl;
	in.close();	
	in.open("infeccmd.dat",ios::in);
	assert(!in.eof());
	getline(in,strbuf);
	
	}
	
	assert(!in.eof());
	//assert(in.good());
	
      */
      
      //string strbuf="7 33 4 114 7 104 1 0";
      
      //string strbuf="307 33 4 114 7 104 1 0 32 3 8 25 200 0 32 4 4 7 104 128 0 32 3 8 253 4 0 32 1 1 0 32 4 2 7 104 1 0 32 3 24 25 200 0 32 4 20 7 104 128 0 32 3 24 253 4 0 32 1 17 0 32 4 18 7 104 1 0 32 3 40 25 200 0 32 4 36 7 104 128 0 32 3 40 253 4 0 32 1 33 0 32 4 34 7 104 1 0 32 3 56 25 200 0 32 4 52 7 104 128 0 32 3 56 253 4 0 32 1 49 0 32 4 50 7 104 1 0 32 3 72 25 200 0 32 4 68 7 104 128 0 32 3 72 253 4 0 32 1 65 0 32 4 66 7 104 1 0 32 3 88 25 200 0 32 4 84 7 104 128 0 32 3 88 253 4 0 32 1 81 0 32 4 82 7 104 1 0 32 3 104 25 200 0 32 4 100 7 104 128 0 32 3 104 253 4 0 32 1 97 0 32 4 98 7 104 1 0 32 3 120 25 200 0 32 4 116 7 104 128 0 32 3 120 253 4 0 32 1 113 0 32 4 114 7 104 1 0 32 3 136 25 200 0 32 4 132 7 104 128 0 32 3 136 253 4 0 32 1 129 0 32 4 130 7 104 1 0 32 3 152 25 200 0 32 4 148 7 104 128 0 32 3 152 253 4 0 32 1 145 0 32 4 146 7 104 1 0 ";
      
      //string strbuf="984 97 4 4 24 101 128 0 97 3 8 253 4 0 97 1 1 0 97 4 2 24 101 1 0 97 3 24 25 250 0 97 4 20 24 101 128 0 97 3 24 253 4 0 97 1 17 0 97 4 18 24 101 1 0 97 3 40 25 250 0 97 4 36 24 101 128 0 97 3 40 253 4 0 97 1 33 0 97 4 34 24 101 1 0 97 3 56 25 250 0 97 4 52 24 101 128 0 97 3 56 253 4 0 97 1 49 0 97 4 50 24 101 1 0 96 3 72 25 250 0 96 4 68 24 101 128 0 96 3 72 253 4 0 96 1 65 0 96 4 66 24 101 1 0 96 3 56 25 250 0 96 4 52 24 101 128 0 96 3 56 253 4 0 96 1 49 0 96 4 50 24 101 1 0 96 3 40 25 250 0 96 4 36 24 101 128 0 96 3 40 253 4 0 96 1 33 0 96 4 34 24 101 1 0 96 3 24 25 250 0 96 4 20 24 101 128 0 96 3 24 253 4 0 96 1 17 0 96 4 18 24 101 1 0 96 3 8 25 250 0 96 4 4 24 101 128 0 96 3 8 253 4 0 96 1 1 0 96 4 2 24 101 1 0 33 3 8 25 250 0 33 4 4 24 101 128 0 33 3 8 253 4 0 33 1 1 0 33 4 2 24 101 1 0 33 3 24 25 250 0 33 4 20 24 101 128 0 33 3 24 253 4 0 33 1 17 0 33 4 18 24 101 1 0 33 3 40 25 250 0 33 4 36 24 101 128 0 33 3 40 253 4 0 33 1 33 0 33 4 34 24 101 1 0 33 3 56 25 250 0 33 4 52 24 101 128 0 33 3 56 253 4 0 33 1 49 0 33 4 50 24 101 1 0 33 3 72 25 250 0 33 4 68 24 101 128 0 33 3 72 253 4 0 33 1 65 0 33 4 66 24 101 1 0 33 3 88 25 250 0 33 4 84 24 101 128 0 33 3 88 253 4 0 33 1 81 0 33 4 82 24 101 1 0 34 3 8 25 250 0 34 4 4 24 101 128 0 34 3 8 253 4 0 34 1 1 0 34 4 2 24 101 1 0 34 3 24 25 250 0 34 4 20 24 101 128 0 34 3 24 253 4 0 34 1 17 0 34 4 18 24 101 1 0 34 3 40 25 250 0 34 4 36 24 101 128 0 34 3 40 253 4 0 34 1 33 0 34 4 34 24 101 1 0 34 3 56 25 250 0 34 4 52 24 101 128 0 34 3 56 253 4 0 34 1 49 0 34 4 50 24 101 1 0 34 3 72 25 250 0 34 4 68 24 101 128 0 34 3 72 253 4 0 34 1 65 0 34 4 66 24 101 1 0 34 3 88 25 250 0 34 4 84 24 101 128 0 34 3 88 253 4 0 34 1 81 0 34 4 82 24 101 1 0 34 3 104 25 250 0 34 4 100 24 101 128 0 34 3 104 253 4 0 34 1 97 0 34 4 98 24 101 1 0 34 3 120 25 250 0 34 4 116 24 101 128 0 34 3 120 253 4 0 34 1 113 0 34 4 114 24 101 1 0 35 3 8 25 250 0 35 4 4 24 101 128 0 35 3 8 253 4 0 35 1 1 0 35 4 2 24 101 1 0 35 3 24 25 250 0 35 4 20 24 101 128 0 35 3 24 253 4 0 35 1 17 0 35 4 18 24 101 1 0 35 3 40 25 250 0 35 4 36 24 101 128 0 35 3 40 253 4 0 35 1 33 0 35 4 34 24 101 1 0 35 3 56 25 250 0 35 4 52 24 101 128 0 35 3 56 253 4 0 35 1 49 0 35 4 50 24 101 1 0 35 3 72 25 250 0 35 4 68 24 101 128 0 35 3 72 253 4 0 35 1 65 0 35 4 66 24 101 1 0 35 3 88 25 250 0 35 4 84 24 101 128 0 35 3 88 253 4 0 35 1 81 0 35 4 82 24 101 1 0 35 3 104 25 250 0 35 4 100 24 101 128 0 35 3 104 253 4 0 35 1 97 0 35 4 98 24 101 1 0 35 3 120 25 250 0 35 4 116 24 101 128 0 35 3 120 253 4 0 35 1 113 0 35 4 114 24 101 1 0 35 3 136 25 250 0 35 4 132 24 101 128 0 35 3 136 253 4 0 35 1 129 0 35 4 130 24 101 1 0 35 3 152 25 250 0 35 4 148 24 101 128 0 35 3 152 253 4 0 35 1 145 0 35 4 146 24 101 1 0";
      
      
      istringstream instring(strbuf);
      
      int nwords;
      
      instring >> nwords;
      
      assert(nwords>1);
      assert(nwords<1025);
      
      //cout << "Words to read:"<<nwords<<endl;
      
      qbufn[mymfec][myfecchannel]=nwords;
      
      for (int i=0;i<nwords;i++){
	int tmp;
	instring >> tmp;
	qbuf[mymfec][myfecchannel][i]=tmp;
	if (j==0){
	  //cout << "qbuf["<<mymfec<<"]["<<myfecchannel<<"]["<<i<<"]="
	  //     << (int)qbuf[mymfec][myfecchannel][i]<<endl;
	}
      }    
      
      int hubandport = (myhubaddress << 3) | myportaddress;
      
      int n = 0;
      int countnext = 0;
      while(n < nwords) {
	qbuf[mymfec][myfecchannel][n] = hubandport;
	countnext = qbuf[mymfec][myfecchannel][n+1];
	n += countnext + 3;
      }
      
      qbufsend(mymfec, myfecchannel);
      
      mfecbusy(mymfec, myfecchannel, &ch1, &ch2);
      
      getfecctrlstatus(mymfec,&data);  
      
      if ((data & dataReceivedMask) == dataReceivedMask) {
	
	// receive complete
	
	cntgood++;
	
      } else {
	
	cntbad++;
	
      }
      //cout<<"-4- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<dec<<endl;
      if(cntbad == 4) {
	//break;
        return 0;
      }
    
    }
    
    success3 = cntgood;

  }


  cntgood=0; cntbad = 0;

  if(commands == 0 || commands == 2) {
    
    for (j=0;j<nTry;j++) {
      
      //    myfec.roctrimload(mymfec,myfecchannel,myhubaddress,myportaddress,myrocid,testroc2);
      
      coltrimload(mymfec,myfecchannel,myhubaddress,myportaddress,
		  myrocid,
		  0,8,
		  testroc2);
      
      coltrimload(mymfec,myfecchannel,myhubaddress,myportaddress,
		  myrocid,
		  8,8,
		  testroc2);
      
      coltrimload(mymfec,myfecchannel,myhubaddress,myportaddress,
		  myrocid,
		  16,8,
		  testroc2);
      
      coltrimload(mymfec,myfecchannel,myhubaddress,myportaddress,
		  myrocid,
		  24,8,
		  testroc2);
      
      coltrimload(mymfec,myfecchannel,myhubaddress,myportaddress,
		  myrocid,
		  32,8,
		  testroc2);
      
      coltrimload(mymfec,myfecchannel,myhubaddress,myportaddress,
		  myrocid,
		  40,8,
		  testroc2);
      
      mfecbusy(mymfec, myfecchannel, &ch1, &ch2);
      
      getfecctrlstatus(mymfec,&data);
      
      if ((data & dataReceivedMask) == dataReceivedMask) {
	
	// receive complete
	
	cntgood++;
	
      } else {
	
	cntbad++;
	
      }
      //cout<<"-5- "<<j<<" "<<cntgood<<" "<<cntbad<<" "<<hex<<data<<dec<<endl;

      if(cntbad == 4) {
	//break;
        return 0;
      }
      
    }
    
    success4 = cntgood;
  }

  return 0;

}


