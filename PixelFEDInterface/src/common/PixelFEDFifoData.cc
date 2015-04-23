// 
//#include <iostream>
//using namespace std;

#include "PixelFEDInterface/include/PixelFEDFifoData.h"

namespace {
  //const bool PRINT = false;
  const bool PRINT = true;
}

///////////////////////////////////////////////////////////////////////////
// Decode the FIFO-1 data in  transparent mode 
// ADD SIZE
void PixelFEDFifoData::decodeTransparentData(uint32_t * data, const int length) {
  const bool DECODE = false;
  if(DECODE) {
    cout<<" It takes 4 clock cycles to decode a channel. "<<endl
	<<" This means there's a lag between the Raw and Decoded Words. "<<endl;
  }
  // Print & analyze the data buffer
  for(int icx=0;icx<length;icx++) {    
     uint32_t tmp = (data[icx] & 0xffc00000) >> 22;
    cout<<"Data in FIFO 1, slot #"<<icx<<" is "<<hex<<data[icx]<<dec<<". The raw pulse height is "<<tmp<<".     ";
    if(DECODE &&  (data[icx] & 0x3fffff)>0 ) {
      cout<<"ROC Toggle = "<<dec<<((data[icx]&0x200000)>>21)<<endl;
      cout<<"DCOL = "<<dec<<((data[icx]&0x1f0000)>>16)<<endl;
      cout<<"PxL = "<<dec<<((data[icx]&0xff00)>>8)<<endl;
      cout<<"Charge deposit = "<<dec<<((data[icx]&0xff))<<endl;}
    else {
      cout << endl;}

  }
  
  cout<<" There's never Transparent Header Decoded Info "<<endl;
  cout<<" There's never Transparent Trailer Decoded Info "<<endl;	  
} // end
///////////////////////////////////////////////////////////////////////////
// Decode the FIFO-1 data in normal mode 
// ADD SIZE
void PixelFEDFifoData::decodeNormalData(uint32_t * data, const int length) {
  //const bool DECODE = false;
  cout<<" Decode FIFO1 normal data "<<length<<endl;
  uint32_t olddata=0;
  uint32_t newdata=0;
  int count=0;
  bool trailer=true;
  int channel = 0;
  static uint32_t lastValue[37] = {0};

  // Print & analyze the data buffer
  for(int icx=0;icx<length;icx++) {   // add size as an argument    
    newdata=data[icx];
    if(icx>0 && newdata==olddata) {
      count++;
      cout<<icx<<" raw "<<hex<<newdata<<dec<<" same data "<<count<<endl;
      //if(count>10) break;  // why?, I am not sure. There seem to be 5 same lines beween channels.
      lastValue[channel]=newdata;

    } else {  // new, valid data

      count=0;
      olddata = newdata;
      channel = (newdata & 0xfc000000) >> 26;
      cout<<icx<<" raw "<<hex<<newdata<<dec;

      if(trailer && newdata == lastValue[channel]) {
	cout<<" leftover from previous event "<<endl;
	continue;
      }

      if(channel>0 && channel<37) { // valid channel
	  int roc  = (newdata & 0x3e00000)>>21;
      
	if(roc==30) { // this is a tbm trailer, or error trailer 
	  uint32_t status = newdata & 0xff;
	  uint32_t rocerr = newdata & 0x800;
	  uint32_t fsmerr = (newdata & 0x600)>>8;
	  uint32_t ovferr = newdata & 0x100;
	  if(ovferr>0) cout<<" Too many words-  ";
	  if(rocerr>0) cout<<" Wrong number of ROCS-  ";
	  if(fsmerr>0) cout<<" State Machine error # "<<fsmerr<<"- ";
	    cout<<" TBM Trailer (event status) = "<<hex<<status<<" Channel "<<dec<<channel<<endl;
	    trailer=true;
	  } else if(roc==31) {
	   uint32_t status = newdata & 0xff;
	    cout<<" TBM Header (event number) = "<<dec<<status<<" Channel "<<dec<<channel<<endl;
	    trailer=false;
	} else if( roc>0 && ((newdata&0x1fff00) == 0x0) ) { // this is a last dac
	  int roc = (newdata & 0x3e00000)>>21;
	  int dac = (newdata & 0xff);
	  cout<<" Last dac for roc "<<roc<<" = "<<dac<<endl;
	} else {
	  int dcol = (newdata & 0x1f0000)>>16;
	  int pix  = (newdata & 0xff00)>>8;
	  int ana  = (newdata & 0xff);
	  cout<<" Channel "<<dec<<channel<<" roc/dcol/pix "<<roc<<" "<<dcol<<" "<<pix
	      <<" ana "<<ana<<hex<<"/"<<ana<<dec<<endl;
	}
      } else {
	cout <<" error, invalid channel number "<<channel<<endl; 
      } // if valid channel 
    } // if new data 
  } // for loop

  
} // end
////////////////////////////////////////////////////////////////////////////
// Decode FIFO 3 Data
void PixelFEDFifoData::decodeSpyFifo3(uint32_t * data, const int length) {
  //  const bool DECODE = false;
  cout<<" Decode FIFO3 n "<<length<<endl;
  //  uint32_t olddata=0;
  //  int count=0;
 
  // Find the Slink headers
  if( (length%2) != 0 ) cout<<" FIFO3 data length not even? "<<length<<endl;
  int halfMarker = length/2;
  cout<<" Slink header "<<endl;
  // The header looks OK  
  if( (data[0]&0xf0000000) != 0x50000000 ) 
    cout<<" error on data header "<<hex<<data[0]<<dec<<endl;
  else
    cout<<hex<<data[0]<<" "<<data[halfMarker]<<dec<<", event num = "<<(data[0]&0xffffff)
	<<", source id = "<<((data[halfMarker]&0xfff00)>>8)<<", BX "<<((data[halfMarker]&0xfff00000)>>20)
      <<endl;
  cout<<" Slink trailer "<<endl;
  if( (data[halfMarker-1]&0xf0000000) != 0xa0000000 ) 
    cout<<" error on data trailer "<<hex<<data[halfMarker-1]<<dec<<endl;
  else
    cout<<hex<<data[halfMarker-1]<<" "<<data[length-1]<<dec<<", length "<<((data[halfMarker-1]&0xffffff)>>0)
	<<", TTS = "<< ((data[length-1]&0xf0)>>4)<<dec<<endl;
  
  
  // The trailer is full of "1"s.

  // Print & analyze the data buffer
  for(int i=0;i<length;i++) {   // add size as an argument    
    if(i != 0 && i != (halfMarker-1) && i !=halfMarker && i != (length-1) ) {
      //cout<<i<<" "<<hex<<data[i]<<dec<<" ";
      decodeSpyDataFifo(data[i]);
    }
  }
}
//
// Decode FIFO 2 spy data 
// The headers are still not treated correctly.
// 
void PixelFEDFifoData::decodeSpyDataFifoSlink64(uint32_t word, ostream &out) {  
  const bool ignoreInvalidData=false;
  if(word&0xfffffff){ 
    const uint32_t plsmsk = 0xff;
    const uint32_t pxlmsk = 0xff00;
    const uint32_t dclmsk = 0x1f0000;
    const uint32_t rocmsk = 0x3e00000;
    const uint32_t chnlmsk = 0xfc000000;
    uint32_t chan= ((word&chnlmsk)>>26);
    uint32_t roc= ((word&rocmsk)>>21);

// Check for embeded special words
if(roc>25){

if((word&0xffffffff)==0xffffffff) {out<<" fifo-2 End of Event word"<<endl;
} else if (roc==26) {out<<"Gap word, Private Data = 0x"<<hex<<(word&0xff)<<dec<<endl;
} else if (roc==27) {out<<"Dummy Data Word, Private Data = 0x"<<hex<<(word&0xff)<<dec<<endl;
} else {decodeErrorFifoSlink64(word);} 

} else if(chan>0 && chan<37) {
      //cout<<hex<<word<<dec;
      out<<" Chnl- "<<chan;
      out<<" ROC- "<<((word&rocmsk)>>21);
      out<<" DCOL- "<<((word&dclmsk)>>16);
      out<<" Pixel- "<<((word&pxlmsk)>>8);
      out<<" ADC- "<<(word&plsmsk)<<endl;
    } else {
     if(!ignoreInvalidData) cout<<" Invalid channel, possible Fifo-2 event count "<<chan<<" "<<hex<<(word&0xffffffff)<<dec<<endl;
    }
  } else {
    if(!ignoreInvalidData)cout<<" Possible Fifo-2 Begin of Event, data = "<<hex<<word<<dec<<endl;       
  }
} // end
/////////////////////////////////////////////////////////////////////////////
//
// Decode FIFO 2 spy data 
// The headers are still not treated correctly.
// 
void PixelFEDFifoData::decodeSpyDataFifo(uint32_t word, ostream &out) {  
  const bool ignoreInvalidData=false;
  if(word&0xfffffff){ 
    const uint32_t plsmsk = 0xff;
    const uint32_t pxlmsk = 0xff00;
    const uint32_t dclmsk = 0x1f0000;
    const uint32_t rocmsk = 0x3e00000;
    const uint32_t chnlmsk = 0xfc000000;
    uint32_t chan= ((word&chnlmsk)>>26);
    uint32_t roc= ((word&rocmsk)>>21);

// Check for embeded special words
if(roc>25){

if((word&0xffffffff)==0xffffffff) {out<<" fifo-2 End of Event word"<<endl;
} else if (roc==26) {out<<"Gap word, Private Data = 0x"<<hex<<(word&0xff)<<dec<<endl;
} else if (roc==27) {out<<"Dummy Data Word, Private Data = 0x"<<hex<<(word&0xff)<<dec<<endl;
} else {decodeErrorFifo(word);} 

} else if(chan>0 && chan<37) {
      //cout<<hex<<word<<dec;
      out<<" Chnl- "<<chan;
      out<<" ROC- "<<((word&rocmsk)>>21);
      out<<" DCOL- "<<((word&dclmsk)>>16);
      out<<" Pixel- "<<((word&pxlmsk)>>8);
      out<<" ADC- "<<(word&plsmsk)<<endl;
    } else {
     if(!ignoreInvalidData) cout<<" Invalid channel, possible Fifo-2 event count "<<chan<<" "<<hex<<(word&0xffffffff)<<dec<<endl;
    }
  } else {
    if(!ignoreInvalidData)cout<<" Possible Fifo-2 Begin of Event, data = "<<hex<<word<<dec<<endl;       
  }
} // end
/////////////////////////////////////////////////////////////////////////////
// Decode error FIFO 
void PixelFEDFifoData::decodeErrorFifo(uint32_t word) {   
// Works for both, the error FIFO and the SLink error words. d.k. 25/04/07

  const uint32_t  errorMask      = 0x3e00000;
  const uint32_t  timeOut        = 0x3a00000;
  const uint32_t  eventNumError  = 0x3e00000;
  const uint32_t  trailError     = 0x3c00000;
  const uint32_t  fifoError      = 0x3800000;

//  const uint32_t  timeOutChannelMask = 0x1f;  // channel mask for timeouts
  const uint32_t  eventNumMask = 0x1fe000; // event number mask
  const uint32_t  channelMask = 0xfc000000; // channel num mask
  const uint32_t  tbmEventMask = 0xff;    // tbm event num mask
  const uint32_t  overflowMask = 0x100;   // data overflow
  const uint32_t  tbmStatusMask = 0xff;   //TBM trailer info
  const uint32_t  BlkNumMask = 0x700;   //pointer to error fifo #
  const uint32_t  FsmErrMask = 0x600;   //pointer to FSM errors
  const uint32_t  RocErrMask = 0x800;   //pointer to #Roc errors 
  const uint32_t  ChnFifMask = 0x1f;   //channel mask for fifo error 
  const uint32_t  ChnFifMask0 = 0x1;   //channel mask for fifo error
  const uint32_t  ChnFifMask1 = 0x2;   //channel mask for fifo error
  const uint32_t  ChnFifMask2 = 0x4;   //channel mask for fifo error
  const uint32_t  ChnFifMask3 = 0x8;   //channel mask for fifo error
  const uint32_t  ChnFifMask4 = 0x10;   //channel mask for fifo error
  const uint32_t  Fif2NFMask = 0x40;   //mask for fifo2 NF 
  const uint32_t  TrigNFMask = 0x80;   //mask for trigger fifo NF 

 const int offsets[8] = {0,4,9,13,18,22,27,31};


  if(word&0xffffffff){

    cout<<"error word "<<hex<<word<<dec<<endl;
  
if( (word&errorMask)==timeOut ) { // TIMEOUT
// More than 1 channel within a group can have a timeout error
    uint32_t index = (word & 0x1F);  // index within a group of 4/5
    uint32_t chip = (word& BlkNumMask)>>8;
      int offset = offsets[chip];
      cout<<"Timeout Error- channels: ";
      for(int i=0;i<5;i++) {
	if( (index & 0x1) != 0) {
	  int channel = offset + i + 1;
	  cout<<channel<<" ";
	}
	index = index >> 1;
      }
//end of timeout  chip and channel decoding
      
    } else if( (word&errorMask) == eventNumError ) { // EVENT NUMBER ERROR
      uint32_t channel =  (word & channelMask) >>26;
      uint32_t tbm_event   =  (word & tbmEventMask);

      cout<<"Event Number Error- channel: "<<channel<<" tbm event nr. "
	  <<tbm_event;

    } else if( ((word&errorMask) == trailError)) {
      uint32_t channel =  (word & channelMask) >>26;
      uint32_t tbm_status   =  (word & tbmStatusMask);
if(word & RocErrMask)cout<<"Number of Rocs Error- "<<"channel: "<<channel<<" ";
if(word & FsmErrMask)cout<<"Finite State Machine Error- "<<"channel: "<<channel<<
" Error status:0x"<<hex<< ((word & FsmErrMask)>>9)<<dec<<" ";;
if(word & overflowMask)cout<<"Overflow Error- "<<"channel: "<<channel<<" ";
if(!((word & RocErrMask)|(word & FsmErrMask)|(word & overflowMask))) cout<<"Trailer Error- ";
cout<<"channel: "<<channel<<" TBM status:0x"<<hex<<tbm_status<<dec<<" ";
      
    } else if((word&errorMask)==fifoError) {
if(word & Fif2NFMask)cout<<"A fifo 2 is Nearly full- ";
if(word & TrigNFMask)cout<<"The trigger fifo is nearly Full - ";
if(word & ChnFifMask)cout<<"fifo-1 is nearly full for channel(s) of this FPGA//";
if(word & ChnFifMask0)cout<<" 1 //";
if(word & ChnFifMask1)cout<<" 2 //";
if(word & ChnFifMask2)cout<<" 3 //";
if(word & ChnFifMask3)cout<<" 4 //";
if(word & ChnFifMask4)cout<<" 5 ";
    }
      uint32_t event   =  (word & eventNumMask) >>13;
      //uint32_t tbm_status   =  (word & tbmStatusMask);

       if(event>0)cout<<":event: "<<event;
       cout<<endl;
    
    
  }
  
}
/////////////////////////////////////////////////////////////////////////////
// Decode error FIFO 
void PixelFEDFifoData::decodeErrorFifoSlink64(uint32_t word) {   
// Works for both, the error FIFO and the SLink error words. d.k. 25/04/07

  const uint32_t  errorMask      = 0x3e00000;
  const uint32_t  timeOut        = 0x3a00000;
  const uint32_t  eventNumError  = 0x3e00000;
  const uint32_t  trailError     = 0x3c00000;
  const uint32_t  fifoError      = 0x3800000;

//  const uint32_t  timeOutChannelMask = 0x1f;  // channel mask for timeouts
  const uint32_t  eventNumMask = 0x1fe000; // event number mask
  const uint32_t  channelMask = 0xfc000000; // channel num mask
  const uint32_t  tbmEventMask = 0xff;    // tbm event num mask
  const uint32_t  overflowMask = 0x100;   // data overflow
  const uint32_t  tbmStatusMask = 0xff;   //TBM trailer info
  const uint32_t  BlkNumMask = 0x700;   //pointer to error fifo #
  const uint32_t  FsmErrMask = 0x600;   //pointer to FSM errors
  const uint32_t  FsmErrRepMask = 0x200;   //pointer to 1st FSM error bit 
  const uint32_t  RocErrMask = 0x800;   //pointer to #Roc errors 
  const uint32_t  ChnFifMask = 0x1f;   //channel mask for fifo error 
  const uint32_t  ChnFifMask0 = 0x1;   //channel mask for fifo error
  const uint32_t  ChnFifMask1 = 0x2;   //channel mask for fifo error
  const uint32_t  ChnFifMask2 = 0x4;   //channel mask for fifo error
  const uint32_t  ChnFifMask3 = 0x8;   //channel mask for fifo error
  const uint32_t  ChnFifMask4 = 0x10;   //channel mask for fifo error
  const uint32_t  Fif2NFMask = 0x40;   //mask for fifo2 NF 
  const uint32_t  TrigNFMask = 0x80;   //mask for trigger fifo NF 
  const uint32_t  RocNNMask = 0xF8000;   //mask for Roc NeitherNor error

 const int offsets[8] = {0,4,9,13,18,22,27,31};


  if(word&0xffffffff){

    cout<<"error word "<<hex<<word<<dec<<endl;
  
if( (word&errorMask)==timeOut ) { // TIMEOUT
// More than 1 channel within a group can have a timeout error
    uint32_t index = (word & 0x1F);  // index within a group of 4/5
    uint32_t chip = (word& BlkNumMask)>>8;
      int offset = offsets[chip];
      cout<<"Timeout Error- channels: ";
      for(int i=0;i<5;i++) {
	if( (index & 0x1) != 0) {
	  int channel = offset + i + 1;
	  cout<<channel<<" ";
	}
	index = index >> 1;
      }
//end of timeout  chip and channel decoding
      
    } else if( (word&errorMask) == eventNumError ) { // EVENT NUMBER ERROR
      uint32_t channel =  (word & channelMask) >>26;
      uint32_t tbm_event   =  (word & tbmEventMask);

      cout<<"Event Number Error- channel: "<<channel<<" tbm event nr. "
	  <<tbm_event;

    } else if( ((word&errorMask) == trailError)) {
      uint32_t channel =  (word & channelMask) >>26;
      uint32_t tbm_status   =  (word & tbmStatusMask);
if(word & RocErrMask)cout<<"Number of Rocs Error- "<<"channel: "<<channel<<" ";
if(word & FsmErrMask)cout<<"Finite State Machine Error- "<<"channel: "<<channel<<
" Error status:0x"<<hex<< ((word & FsmErrMask)>>9)<<dec<<" ";;
if(word & overflowMask)cout<<"Overflow Error- "<<"channel: "<<channel<<" ";
if(!((word & RocErrMask)|(word & FsmErrMask)|(word & overflowMask))) cout<<"Trailer Error- ";
cout<<"channel: "<<channel<<" TBM status:0x"<<hex<<tbm_status<<dec<<" ";
      
    } else if((word&errorMask)==fifoError) {
if(word & Fif2NFMask)cout<<"A fifo 2 is Nearly full- ";
if(word & TrigNFMask)cout<<"The trigger fifo is nearly Full - ";
if(word & ChnFifMask)cout<<"fifo-1 is nearly full for channel(s) of this FPGA//";
if(word & ChnFifMask0)cout<<" 1 //";
if(word & ChnFifMask1)cout<<" 2 //";
if(word & ChnFifMask2)cout<<" 3 //";
if(word & ChnFifMask3)cout<<" 4 //";
if(word & ChnFifMask4)cout<<" 5 ";

    }
      uint32_t event   =  (word & eventNumMask) >>13;
      //uint32_t tbm_status   =  (word & tbmStatusMask);

       if((event>0)&&(((word & FsmErrRepMask)>>9)!=1)){cout<<":event: "<<event<<" "<<((word & FsmErrMask)>>9);}
       else if(((word & FsmErrRepMask)>>9)==1){cout<<":ROC w/black err: "<<((word&RocNNMask)>>15);}
       cout<<endl;
    
    
  }
  
}
//////////////////////////////////////////////////////////////////////////////
// Decode temperatur fifo
void PixelFEDFifoData::decodeTemperatureFifo(uint32_t word) {  
  const uint32_t dacmsk = 0xff;
  const uint32_t rocmsk = 0x3e00000;
  const uint32_t chnlmsk = 0xfc000000;
  uint32_t channel = ((word&chnlmsk)>>26);
  if(word&0xfffffff) cout<<" Channel- "<<channel   
			 <<" ROC- " <<((word&rocmsk)>>21)
			 <<" Last DAC value - "<<(word&dacmsk)<<endl;
//   if(word&0xfffffff && channel==1) cout<<" Channel- "<<channel   
// 			 <<" ROC- " <<((word&rocmsk)>>21)
// 			 <<" Last DAC value - "<<(word&dacmsk)<<endl;
} // end
//////////////////////////////////////////////////////////////////////////////
// Decode TTS fifo
void PixelFEDFifoData::decodeTTSFifo(uint32_t word) {  

int itts=word&0xf;
switch (itts)
{
	  case 0://0000
	cout<<"**TTS**Disconnected    "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 1://0001
	cout<<"**TTS**Overflow Warning"<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 2://0010
	cout<<"**TTS**Out of Sync     "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 3://0011
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 4://0100
	cout<<"**TTS**Busy            "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 5://0101
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 6://0110
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 7://0111
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 8://1000
	cout<<"**TTS**Ready           "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 9://1001
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 10://1010
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 11://1011
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 12://1100
	cout<<"**TTS**Error           "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 13://1101
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 14://1110
	cout<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
	  case 15://1111
	cout<<"**TTS**Disconnected    "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;
	break;
}	  



} // end
////////////////////////////////////////////////////////////////////////////
// Decode Slink Data
void PixelFEDFifoData::decodeSlink64(uint64_t * ldata, const int length, std::ostream &out) {
//cout<<hex<<ldata[0]<<endl;
//cout<<hex<<ldata[1]<<endl;

  out<<" Decode "<<length<<" 64 bit Slink words, position 0 - "<<length-1<<endl;
 
  //Find the Slink headers
  out<<" Slink header "<<endl;
  // The header looks OK  
  if( (ldata[0]&0xf000000000000000LL) != 0x5000000000000000LL ) 
    out<<" error on data header "<<hex<<ldata[0]<<dec<<endl;
  else
    out<<hex<<ldata[0]<<" "<<dec<<", event num = "<<((ldata[0]&0xffffff00000000LL)>>32)
	<<", source id = "<<((ldata[0]&0x00000000fff00LL)>>8)<<", BX "<<((ldata[0]&0x00000000fff00000LL)>>20)
      <<endl;
  out<<" Slink trailer "<<endl;
  if( (ldata[length-1]&0xf000000000000000LL) != 0xa000000000000000LL ) 
    out<<" error on data trailer "<<hex<<ldata[length-1]<<dec<<endl;
  else
    out<<hex<<ldata[length-1]<<" "<<dec<<", length "<<((ldata[length-1]&0xffffff00000000LL)>>32)
	<<", TTS = "<< ((ldata[length-1]&0x00000000f0LL)>>4)<<endl;
  
  // Print & analyze the data buffer
  for(int i=1;i<length-1;i++) {

  uint64_t hidat = (ldata[i]&0xffffffff00000000LL)>>32;
  uint64_t lodat = (ldata[i]&0x00000000ffffffffLL);
//we may need to change this part if the lo/hi word is compiler dependent in future
  uint32_t data=(uint32_t) hidat;
  PixelFEDFifoData::decodeSpyDataFifoSlink64(data, out);
//if(i==1)cout<<"1st word ="<<hex<<data<<endl;
  uint32_t data2=(uint32_t) lodat;
  PixelFEDFifoData::decodeSpyDataFifoSlink64(data2, out);
//if(i==1)cout<<"2nd word ="<<hex<<data<<endl;



  }
    
  
}
////////////////////////////////////////////////////////////////////////////////////
void PixelFEDFifoData::decodeSpyDataFifo(const uint32_t *const data, const int length) {
  for(int i=0;i<length;i++) {	
    cout<<i<<" raw "<<hex<<data[i]<<dec;
    decodeSpyDataFifo(data[i]);
  } // for
}
/////////////////////////////////////////////////////////////////////////////////////
void PixelFEDFifoData::decodeErrorFifo(const uint32_t *const data, const int length) {
  for(int i=0;i<length;i++) {	  
    decodeErrorFifo(data[i]);  // decode it
  } // for 
}

////////////////////////////////////////////////////////////////////////////////////
uint32_t PixelFEDFifoData::decodeErrorFifo(const uint32_t *const data, const int length, const uint64_t channelMask) {
  // only report errors from channels in the mask; if nth bit of mask is on, then decode errors from channel n
  // All const here should come from header file; reused in several places (kme)
  const uint32_t  decodeMask = 0xfc000000; // channel info mask
  const uint32_t offset = 26; // position of channel info
  const uint32_t errorMask  = 0x3e00000;  
  const uint32_t trailError = 0x3c00000;
  const uint32_t  overflowMask = 0x100;   // data overflow
  const uint32_t  FsmErrMask = 0x600;   //pointer to FSM errors
  const uint32_t  RocErrMask = 0x800;   //pointer to #Roc errors 
  const uint32_t tbmStatusMask = 0xff; 
  uint32_t count=0;
  
  for(int i=0; i<length; i++) {
    uint32_t chan = (data[i] & decodeMask) >> offset;
    uint64_t thischannel = ( (long long) 0x1 << (chan-1));
    if( (channelMask & thischannel) && //require channel to enabled
	( !((errorMask & data[i])==trailError) | 
	  (  ((errorMask & data[i])==trailError) &&  
	     ((data[i] & RocErrMask)|
	      (data[i] & FsmErrMask)|
	      (data[i] & overflowMask) |
	      (data[i] & (tbmStatusMask != 255)))  //ignore trailer "errors" from cal-sync only (temporary kludge)
	     )
	  )
	) { 
      decodeErrorFifo(data[i]);
      count++;
    }
    else if(PRINT) std::cout << "Supressing error fifo content from disabled channel=" << chan << std::endl;
  }
  return count;
}

////////////////////////////////////////////////////////////////////////////////////
void PixelFEDFifoData::decodeTemperatureFifo(const uint32_t *const data, 
					     const int length) {
  for(int i=0;i<length;i++) {	  
    decodeTemperatureFifo(data[i]);
  } // for
}
////////////////////////////////////////////////////////////////////////////////////
void PixelFEDFifoData::decodeTTSFifo(const uint32_t *const data, 
					     const int length) {
  for(int i=0;i<length;i++) {	  
    decodeTTSFifo(data[i]);
  } // for
}
