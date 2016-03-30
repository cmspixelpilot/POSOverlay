#include "TBM.h"
#include "PixelFECInterface/include/PixelFECInterface.h"
#include <iostream>
#include <iomanip>

#include "ControlNetwork.h"

TBM::TBM(const int aHubId, ControlNetwork *aCN){
  //  cout << "ho" <<endl;
  hubaddress=aHubId;
  cn=aCN;
}


void TBM::Execute(SysCommand* command){
  int *value1, *value2;  // note: pointers into syscommand arguments need no allocation
  int data=0;
  if(command->type==SysCommand::kTBM){
    if(command->Keyword("dual")      ){ 
      setTBM(tbmB,4,0x00, 0x03);
    } else if(command->Keyword("single")    ){
      setTBM(tbmB,4,0x03, 0x03);
    }
    else if(command->Keyword("disablepkam") ){ setTBMs(0, 0x01, 0x01);}  
    else if(command->Keyword("disableclock") ){ setTBMs(0, 0x02, 0x02);}
    else if(command->Keyword("enablepkam") ){ setTBMs(0, 0x00, 0x01);}
    else if(command->Keyword("disableauto") ){setTBMs(0, 0x80, 0x80); }
    else if(command->Keyword("enableauto") ){ setTBMs(0, 0x00, 0x80);}
    else if(command->Keyword("disableautonpkam") ){setTBMs(0, 0x81, 0x81); }
    else if(command->Keyword("enableautonpkam") ){ setTBMs(0, 0x00, 0x81);}
    
    else if(command->Keyword("setA",&value1,&value2)){ setTBM(tbmA,*value1,*value2);}
    else if(command->Keyword("setB",&value1,&value2)){ setTBM(tbmB,*value1,*value2);}
    else if(command->Keyword("inputbias",&value1)    ){ setTBMDAC(0,*value1); }
    else if(command->Keyword("outputbias",&value1)   ){ setTBMDAC(1,*value1); }
    
    else if(command->Keyword("tbmadelay",&value1)    ){ setDelayTBMa(0,*value1); } // value1=128 Token, 64 Header/Trailer, rest ROC data on port 0 and 1
    else if(command->Keyword("tbmbdelay",&value1)    ){ setDelayTBMb(0,*value1); } // value1=128 Token, 64 Header/Trailer, rest ROC data on port 0 and 1
    
    else if(command->Keyword("tbmpkamcount",&value1)    ){ setPKAMCountTBM(0,*value1); } // 
    else if(command->Keyword("tbmplldelay",&value1)    ){ setPLLDelayTBM(0,*value1); } // 
    else if(command->Keyword("tbmautoreset",&value1)    ){ setAutoResetTBM(0,*value1); }
    
    else if(command->Keyword("settemperaturebits",&value1)   ){ setTBMDAC(1,*value1); } // value1=[0-32] - 6 bits
    
    else if(command->Keyword("dacgain",&value1)   ){ setTBMDAC(2,*value1); }
    else if(command->Keyword("mode","cal")   ){setTBMs(1,0xC0,0xC0);}
    else if(command->Keyword("mode","clear") ){setTBMs(1,0x80,0xC0);}
    else if(command->Keyword("mode","sync")  ){setTBMs(1,0x00,0xC0);}
    else if(command->Keyword("ignore", "triggers") ){setTBMs( 0,0x10, 0x10);}
    else if(command->Keyword("accept", "triggers") ){setTBMs( 0,0x00, 0x10);}
    else if(command->Keyword("disable","triggers") ){setTBMs( 0,0x40, 0x40);}
    else if(command->Keyword("enable", "triggers") ){setTBMs( 0,0x00, 0x40);}
    else if(command->Keyword("inject", "trigger") ){setTBMs( 2,0x01, 0x01);}
    else if(command->Keyword("inject", "sync") )   {setTBMs( 2,0x02, 0x02);}
    else if(command->Keyword("reset",  "roc") )    {setTBMs( 2,0x04, 0x04);}
    else if(command->Keyword("inject", "cal") )    {setTBMs( 2,0x08, 0x08);}
    else if(command->Keyword("reset",  "tbm") )    {setTBMs( 2,0x10, 0x10);}
    else if(command->Keyword("clear",  "stack")  ) {setTBMs( 2,0x20, 0x20);}
    else if(command->Keyword("clear",  "token") )  {setTBMs( 2,0x40, 0x40);}
    else if(command->Keyword("clear",  "counter")) {setTBMs( 2,0x80, 0x80);}
    else if(command->Keyword("dump","A")){       dump(tbmA); }
    else if(command->Keyword("dump","B")){       dump(tbmB); }
    else if(command->Keyword("readA",&value1)){ readTBM(tbmA,*value1,data);}
    else if(command->Keyword("readB",&value1)){ readTBM(tbmB,*value1,data);}
    else if(command->Keyword("readA")){ 
      for (int reg=0; reg<8; reg++){ 
	readTBM(tbmA,reg,data); 
      }
    }
    else if(command->Keyword("readB")){ 
      for (int reg=0; reg<8; reg++){ 
	readTBM(tbmB,reg,data); 
      }
    }
    else if(command->Keyword("readstackA")){ 
      readStack(tbmA); 
    }
    else if(command->Keyword("readstackB")){ 
      readStack(tbmB); 
    }
    else{
      cout << "unknown tbm command " << command->toString() << endl;
    }
  }else{
    cout <<" TBM::Execute> should never be here" << endl;
  }
}

void TBM::Init(){
  setTBMs(0, 0x01);
  setTBMs(1, 0xC0);
  setTBMs(2, 0xF0); //reset
  setTBMs(3, 0x00);
  setTBMs(4, 0x00);
  setTBMDAC(0, 150);
  setTBMDAC(1, 150);
  setTBMDAC(2, 150);
}

// Combine the updated bits with the old value
// e.g. change=0xF to flip all allowed bits to 1.
int TBM::replace(const int oldValue, const int change, const int mask)
  const
{
  int newValue = (oldValue & ~mask) | (change & mask);
  return newValue;
}


int TBM::setTBM(const int tbmChannel, const int tbmRegister, const int value, const int mask){
  int * r;  // no need to allocate, will point to existing memory
  if (tbmChannel==tbmA && tbmRegister>=0 && tbmRegister<=7){
    //if (mask==0)//fake read
      r= & tbmAReg[tbmRegister];
    //  else {//real read
      //readTBM(tbmA,tbmRegister,*r); 
    // r= & tbmAReg[tbmRegister];
    // }
  }else if (tbmChannel==tbmB && tbmRegister>=0 && tbmRegister<=7){
    r= & tbmBReg[tbmRegister];
  }else{
    cout << "illegal argument for setTBM " << tbmChannel << " " << tbmRegister << endl;
    return -1;
  }
  
  if (mask==0){
    (*r) = value & 0xFF;
  }else{
      (*r) = replace( *r, value & 0xFF, mask);
  }
  cn->interface->tbmcmd(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, tbmRegister, *r,   0);
  return 0;
}

void TBM::setTBMs(const int tbmRegister, const int value, const int mask){
  // set both TBMs
  setTBM(tbmA, tbmRegister, value, mask);
  //if (tbmRegister<5){setTBM(tbmB, tbmRegister, value);}
  setTBM(tbmB, tbmRegister, value, mask);
}

void TBM::setTBMDAC(const int DACAddress, const int value){
  //DACAddress=0,1,2
  setTBM(tbmA, DACAddress+5, value);
}

void TBM::setDelayTBMa(const int DACAddress, const int value){
  //DACAddress=0,1,2
  setTBM(tbmA, DACAddress+5, value);
}

void TBM::setDelayTBMb(const int DACAddress, const int value){
  //DACAddress=0,1,2
  setTBM(tbmB, DACAddress+5, value);
}

void TBM::setPKAMCountTBM(const int DACAddress, const int value){
  //DACAddress=0,1,2
  setTBM(tbmA, DACAddress+4, value);
  setTBM(tbmB, DACAddress+4, value);
}

void TBM::setAutoResetTBM(const int DACAddress, const int value){
  //DACAddress=0,1,2
  setTBM(tbmA, DACAddress+6, value);
  setTBM(tbmB, DACAddress+6, value);
}

void TBM::setPLLDelayTBM(const int DACAddress, const int value){
  //DACAddress=0,1,2
  setTBM(tbmA, DACAddress+7, value);
}


// int TBM::readTBM(const int tbmChannel, const int tbmRegister, int& value){
//   if (tbmChannel==tbmA && tbmRegister>=0 && tbmRegister<=7){
//     value=tbmAReg[tbmRegister]; // as long as we can't really read
//   }else if (tbmChannel==tbmB && tbmRegister>=0 && tbmRegister<=4){
//     value=tbmBReg[tbmRegister];
//   }else{
//     value=255;
//     cout << "illegal argument for readTBM " << tbmChannel << " " << tbmRegister << endl;
//     return -1;
//   }
  
  
//   cout << "reading from TBM not really implemented yet, cached value is " << value << endl;
//   cn->interface->tbmcmd(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, tbmRegister, 255,   1);
//   return 0;
// }

//###################################################
int TBM::readTBM(const int tbmChannel, const int tbmRegister, int& value){
  if (tbmChannel==tbmA && tbmRegister>=0 && tbmRegister<=7){
    for (int i=0; i< 1; i++){
      value=cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, tbmRegister);
      //value=cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 1, tbmRegister);
      //cout << "channel " << cn->channel <<  endl;
      cout << "hub " << hubaddress << ": TBM A register " << tbmRegister << ": " << value << endl; 
    }
  }else if (tbmChannel==tbmB && tbmRegister>=0 && tbmRegister<=7){  
    value=cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, tbmRegister);
    cout << "hub " << hubaddress << ": TBM B register " << tbmRegister << ": " << value << endl;
  }else{
    value=255;
    cout << "illegal argument for readTBM " << tbmChannel << " " << tbmRegister << endl;
    return -1;
  }
   
  return 0;
}


int TBM::dump(const int tbmChannel){
  // print a human readable memory dump, cf http://www.physics.rutgers.edu/~bartz/cms/tbm2/mem.html
  int b0,b1,b4,inputbias,outputbias,dacgain;
  b0=cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, 0);//readTBM(tbmChannel,0, b0);
  if (b0&0x01){cout << "40 MHz"<<endl; }else{cout << "20 MHz" << endl;}
  if (b0&0x02){cout << "TBM clock disabled"<<endl; }else{cout << "TBM clock enabled" << endl;}
  if (b0&0x08){cout << "pause readout"<<endl; }else{cout << "readout not paused" << endl;}
  if (b0&0x10){cout << "ignore incoming triggers"<<endl; }else{cout << "accept incomig triggers" << endl;}
  if (b0&0x20){cout << "stack readback mode"<<endl; }
  if (b0&0x40){cout << "trigger out disabled"<<endl; }else{cout << "trigger out enabled" << endl;}
  b1=cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, 1);//readTBM(tbmChannel,1,b1);
  cout << "stack count=" << (b1&0x3F) << endl;
  int mode=(b1>>6)&3;
  if((mode==0) or (mode==1)){cout << "mode=sync" << endl;
  }else if(mode==2){ cout << "mode=ClearTriggerCounter" << endl;
  }else if(mode==3){ cout << "mode=Calibration" << endl;}
  //2,3 = stack
  b4=cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, 4);//readTBM(tbmChannel,1,b4);
  if( b4&0x01 ){cout << "analog output driver disabled" << endl;}else{cout << "analog output driver enabled" << endl;}
  if( b4&0x02 ){cout << "token out driver disabled" << endl;}else{cout << "token out driver enabled" << endl;}
  if( b4&0x04 ){cout << "readout clock forced on" << endl;}

  // dacs only exit in tbm channel A
  inputbias=cn->interface->tbmread(cn->mfec, cn->channel, tbmA, hubaddress, 4, 5);
  cout << "inputbias=" << inputbias << endl;
  outputbias=cn->interface->tbmread(cn->mfec, cn->channel, tbmA, hubaddress, 4, 6);
  cout << "outputbias=" << outputbias << endl;
  dacgain=cn->interface->tbmread(cn->mfec, cn->channel, tbmA, hubaddress, 4, 7);
  cout << "dacgain=" << dacgain << endl;

  cout << "stack " << endl;
  readStack(tbmChannel,true);
  return 0;
}



int TBM::readStack(const int tbmChannel,bool explain){
  int * r;  // no need to allocate, will point to existing memory
  int statusbits=0;
  int eventnumber=0;
  if (tbmChannel==tbmA || tbmChannel==tbmB ){
    if (tbmChannel==tbmA) {
      r= & tbmAReg[0];
    }
    else if (tbmChannel==tbmB) {
      r= & tbmBReg[0];
    }
    (*r) = 0x38; // pause readout, ignore triggers, stack readback mode
    cn->interface->tbmcmd(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, 0, *r,   0);
    // read stack count
    int stackcount = cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, 1) & 0x3F; 
    cout << "stack count " << stackcount << endl; 
    for (int i=0; i< 32; i++){
      // read stack status bits
      statusbits = cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, 3) & 0x3F; 
      if (not explain){ cout << i << ": statusbits " << statusbits << endl;}
      // read eventnumber
      eventnumber = cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, 2); 
      if (not explain){cout << i << ": eventnumber " << eventnumber << endl;}

      if(explain){
	cout << setw(4) << eventnumber << " [";
	if (statusbits&0x01) cout << "full ";
	if (statusbits&0x02) cout << "cal ";
	if (statusbits&0x04) cout << "reset ";
	if (statusbits&0x08) cout << "sync-trigger ";
	if (statusbits&0x10) cout << "sync-error ";
	if (statusbits&0x20) cout << "roc-reset ";
	if (statusbits&0x40) cout << "tbm-reset ";
	if (statusbits&0x80) cout << "no token pass ";
	cout << "]" << endl;
      }
    }
    (*r) = 0x1; // reset pause readout, ignore triggers, stack readback mode
    cn->interface->tbmcmd(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, 0, *r,   0);

    return 0;
  }
    
  //cn->interface->tbmread(cn->mfec, cn->channel, tbmChannel, hubaddress, 4, tbmRegister); 
  
  else {
    cout << "illegal argument for readStack. no tbmchannel " << tbmChannel << endl; 
    return -1;
  }
  
}
//#################################################

