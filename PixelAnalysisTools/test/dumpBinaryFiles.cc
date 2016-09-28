//
// Hex ascii reading works only with compiled code 
// call by .L dumpBinaryFile.C+
// and then   
// dumpBinaryFiles("PhysicsData1_366.dmp")
// Corrected for SLC4 7/07

#include <iostream>
#include <fstream>
using namespace std;

const bool ErrorsOnly = false;
const bool printHeaders = false;
const bool CHECK_PIXELS = true;
bool PRINT_PIXELS = true;
bool HaltOnError = true;
const bool PRINT_DUMMY = false;
int dump_count = 0;
bool PRINT_PIXELS_1st100 = true;

//////////////////////////////////////////////////////////////////////
int waitForInput() {
  int dummy=0;
  cout<<" Enter 0 to continue, -1 to stop  "; 
  cin>>dummy;
  return dummy;
}

// Include the helper decoding class 
/////////////////////////////////////////////////////////////////////////////
class MyDecode {
public:
  MyDecode() {}
  ~MyDecode() {}
  static int error(int error);
  static int data(int error, int &channel, int &roc, int &dcol, int &pix);
  static int header(unsigned long long word64);
  static int trailer(unsigned long long word64);
private:
};
/////////////////////////////////////////////////////////////////////////////
int MyDecode::header(unsigned long long word64) {
  int fed_id=(word64>>8)&0xfff;
  int event_id=(word64>>32)&0xffffff;
  unsigned int bx_id=(word64>>20)&0xfff;
  cout<<" Header "<<hex<<word64<<dec<<" for FED "
      <<fed_id<<" event "<<event_id<<" bx "<<bx_id<<endl;
  return 0;
}
//
int MyDecode::trailer(unsigned long long word64) {
  int slinkLength = int( (word64>>32) & 0xffffff );
  int crc         = int( (word64&0xffff0000)>>16 );
  int tts         = int( (word64&0xf0)>>4);
  int slinkError  = int( (word64&0xf00)>>8);
  cout<<" Trailer "<<hex<<word64<<dec<<" len "<<slinkLength
      <<" tts "<<tts<<" error "<<slinkError<<" crc "<<hex<<crc<<dec<<endl;
  return 0;
}
//
// Decode error FIFO
// Works for both, the error FIFO and the SLink error words. d.k. 25/04/07
int MyDecode::error(int word) {
  int status = -1;
  const unsigned int  errorMask      = 0x3e00000;
  const unsigned int  dummyMask      = 0x03600000;
  const unsigned int  gapMask        = 0x03400000;
  const unsigned int  timeOut        = 0x3a00000;
  const unsigned int  eventNumError  = 0x3e00000;
  const unsigned int  trailError     = 0x3c00000;
  const unsigned int  fifoError      = 0x3800000;
 
//  const unsigned int  timeOutChannelMask = 0x1f;  // channel mask for timeouts
  //const unsigned int  eventNumMask = 0x1fe000; // event number mask
  const unsigned int  channelMask = 0xfc000000; // channel num mask
  const unsigned int  tbmEventMask = 0xff;    // tbm event num mask
  const unsigned int  overflowMask = 0x100;   // data overflow
  const unsigned int  tbmStatusMask = 0xff;   //TBM trailer info
  const unsigned int  BlkNumMask = 0x700;   //pointer to error fifo #
  const unsigned int  FsmErrMask = 0x600;   //pointer to FSM errors
  const unsigned int  RocErrMask = 0x800;   //pointer to #Roc errors
  const unsigned int  ChnFifMask = 0x1f;   //channel mask for fifo error
  const unsigned long  ChnFifMask0 = 0x1;   //channel mask for fifo error
  const unsigned long  ChnFifMask1 = 0x2;   //channel mask for fifo error
  const unsigned long  ChnFifMask2 = 0x4;   //channel mask for fifo error
  const unsigned long  ChnFifMask3 = 0x8;   //channel mask for fifo error
  const unsigned long  ChnFifMask4 = 0x10;   //channel mask for fifo error
  const unsigned int  Fif2NFMask = 0x40;   //mask for fifo2 NF
  const unsigned int  TrigNFMask = 0x80;   //mask for trigger fifo NF
 
 const int offsets[8] = {0,4,9,13,18,22,27,31};
 
 //cout<<"error word "<<hex<<word<<dec<<endl;                                                                                  
  if( (word&errorMask) == dummyMask ) { // DUMMY WORD
    if(PRINT_DUMMY) cout<<" Dummy word"<<endl;
    return 0;
  } else if( (word&errorMask) == gapMask ) { // GAP WORD
    if(PRINT_DUMMY) cout<<" Gap word"<<endl;
    return 0;
  } else if( (word&errorMask)==timeOut ) { // TIMEOUT
     // More than 1 channel within a group can have a timeout error
     unsigned int index = (word & 0x1F);  // index within a group of 4/5
     unsigned int chip = (word& BlkNumMask)>>8;
     int offset = offsets[chip];
     cout<<"Timeout Error- channels: ";
     for(int i=0;i<5;i++) {
       if( (index & 0x1) != 0) {
         int chan = offset + i + 1;
         cout<<chan<<" ";
       }
       index = index >> 1;
     }
     cout<<endl;
     //end of timeout  chip and channel decoding
      
   } else if( (word&errorMask) == eventNumError ) { // EVENT NUMBER ERROR
     unsigned int channel =  (word & channelMask) >>26;
     unsigned int tbm_event   =  (word & tbmEventMask);
      
     cout<<"Event Number Error- channel: "<<channel<<" tbm event nr. "
         <<tbm_event<<endl;
                                                                                            
   } else if( ((word&errorMask) == trailError)) {
    unsigned int channel =  (word & channelMask) >>26;
    unsigned int tbm_status   =  (word & tbmStatusMask);
    if(word & RocErrMask)
      cout<<"Number of Rocs Error- "<<"channel: "<<channel<<" "<<endl;
    if(word & FsmErrMask)
      cout<<"Finite State Machine Error- "<<"channel: "<<channel
	  <<" Error status:0x"<<hex<< ((word & FsmErrMask)>>9)<<dec<<" "<<endl;
    if(word & overflowMask)
      cout<<"Overflow Error- "<<"channel: "<<channel<<" "<<endl;
    //if(!((word & RocErrMask)|(word & FsmErrMask)|(word & overflowMask)))
    if(tbm_status!=0)
      cout<<"Trailer Error- "<<"channel: "<<channel<<" TBM status:0x"
	  <<hex<<tbm_status<<dec<<" "<<endl;
    
  } else if((word&errorMask)==fifoError) {
    if(word & Fif2NFMask) cout<<"A fifo 2 is Nearly full- ";
    if(word & TrigNFMask) cout<<"The trigger fifo is nearly Full - ";
    if(word & ChnFifMask){
     cout<<"fifo-1 is nearly full for channel(s) of this FPGA//";
     if(word & ChnFifMask0)cout<<" 1 //";
     if(word & ChnFifMask1)cout<<" 2 //";
     if(word & ChnFifMask2)cout<<" 3 //";
     if(word & ChnFifMask3)cout<<" 4 //";
     if(word & ChnFifMask4)cout<<" 5 ";
                         }
    
    cout<<endl;
    
  } else {
    cout<<" Unknown error?";
  }
  
  //unsigned int event   =  (word & eventNumMask) >>13;
  //unsigned int tbm_status   =  (word & tbmStatusMask);
    
  //if(event>0) cout<<":event: "<<event;
  //cout<<endl;

  return status;
}
///////////////////////////////////////////////////////////////////////////
int MyDecode::data(int word, int &c, int &r, int &d, int &p) {
  if(ErrorsOnly) PRINT_PIXELS=false;

  const int ROCMAX = 24;  
  const unsigned int plsmsk = 0xff;   // pulse height
  const unsigned int pxlmsk = 0xff00; // pixel index
  const unsigned int dclmsk = 0x1f0000;
  const unsigned int rocmsk = 0x3e00000;
  const unsigned int chnlmsk = 0xfc000000;
  int status = 0;
if(dump_count==100){cout<<"Finished dumping the 1st 100 hits, modify the code to dump more!!!"<<endl; dump_count++;}
  int roc = ((word&rocmsk)>>21);
  // Check for embeded special words
  if(roc>0 && roc<25) {  // valid ROCs go from 1-24
    if(PRINT_PIXELS&&(dump_count<100)) {cout<<"data "<<hex<<word<<dec;if(PRINT_PIXELS_1st100)dump_count++;}
    unsigned int channel = ((word&chnlmsk)>>26);
    if(channel>0 && channel<37) {  // valid channels 1-36
      //cout<<hex<<word<<dec;
      int dcol=(word&dclmsk)>>16;
      int pix=(word&pxlmsk)>>8;
      int adc=(word&plsmsk);
      // print the roc number according to the online 0-15 scheme
      if(PRINT_PIXELS&&(dump_count<100)) {cout<<" Channel- "<<channel<<" ROC- "<<(roc-1)<<" DCOL- "<<dcol<<" Pixel- "
	  <<pix<<" ADC- "<<adc<<endl;if(PRINT_PIXELS_1st100)dump_count++;}
      if(CHECK_PIXELS) {
	if(roc>ROCMAX) 
	  cout<<" wrong roc number "<<channel<<"/"<<roc<<"/"<<dcol<<"/"<<pix<<"/"<<adc<<endl;
	if(dcol<0 || dcol>25)
	  cout<<" wrong dcol number "<<channel<<"/"<<roc<<"/"<<dcol<<"/"<<pix<<"/"<<adc<<endl;
	if(pix<2 || pix>181)
	  cout<<" wrong pix number chan/roc/dcol/pix/adc = "<<channel<<"/"<<roc<<"/"<<dcol<<"/"<<pix<<"/"<<adc<<endl;
      }
      c = channel;
      r = roc-1; // start roc counting from 0
      d = dcol;
      p = pix;
      status++;
    } else {
      cout<<"Wrong channel "<<channel<<endl;
      return -2;
    }
  } else {  // error word
    //cout<<"error word "<<hex<<word<<dec;
    status=error(word);
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////
main(int argc, char **argv) {
  const bool DumpRaw=false;
  bool error = false;
  bool halt = false;
  if(ErrorsOnly) halt=false;
  if(ErrorsOnly) HaltOnError=false;
  const bool HaltForEachEvent = false;
  const bool findHotPixels = false;

  char * filename;
  if(argc>1) {
    filename = argv[1];
  }
 
  int numEvent=0; 
  int eventsWithHits=0;
  float eventSize=0;
  int hitsTotal=0;
  int channel=-1, roc=-1, dcol=-1, pix=-1;

  //char filename[80] = "../../../SCRATCH/PhysicsData1_366.dmp";  
  ifstream in_file;  // data file pointer
  in_file.open(filename, ios::binary|ios::in ); // 
  //cout<<in_file.bad()<<" "<<in_file.good()<<" "<<in_file<<endl;
  if (!in_file) {
    cout << " File not found " << endl;
    return(1); // signal error
  }

  // read the header
  unsigned long long  runnr;
  in_file.read((char*)&runnr,8);
  if (in_file.eof()) {
    std::cout << "End of input file" <<  std::endl;
    cout<<" Event "<<numEvent<<" with hits "<<eventsWithHits<<endl;

    return false;
  }
  cout<<" run number = "<<runnr<<endl;

  const int numMax=10000000;
  int status=0;
  unsigned long long word64=0;
  //bool noTrailer=false;
  //bool noHeader=true;
  bool eof = false;
  int event_id_old=0;
  int fed_id0 = 0;

  // Event loop 
  while (numEvent < numMax) {  // 

    error=false;

    // Has to be a header
    in_file.read((char*)&word64,8);
    //cout<<hex<<word64<<dec<<endl;
    if (in_file.eof()) {
      break;
    }
    
    if(DumpRaw) {
      cout<<hex<<word64<<dec<<endl;
      int dummy=0;
      cout<<" Enter 0 to continue, -1 to stop "; 
      cin>>dummy;
      if(dummy==-1) break;

    } else {
 
      // Is it a header
      // If not loop until we find something that looks like a header
      if ((word64 >> 56) != 0x50){
	std::cout << "DATA CORRUPTION! ";
	std::cout << "Expected to find header, but read: 0x"
		  <<std::hex<<word64<<std::dec <<  std::endl;
	//return false;
	error=true;
	halt=true;

	if(word64!=0x0 && halt) status = waitForInput();
	// keep looing for header
	if(status==-1) break;
	else continue;
      }
       
      if(printHeaders) status = MyDecode::header(word64);
     
      int fed_id=(word64>>8)&0xfff;
      int event_id=(word64>>32)&0xffffff;
      //unsigned int bx_id=(word64>>20)&0xfff;
      if(numEvent==0) {
	cout<<" EVENT 1 - Trailer errors might appear becouse of the initial reset"<<endl;
	fed_id0 = fed_id;
      }
      if(fed_id != fed_id0 ) {
	cout<<" Wrong fedid "<<fed_id<<" "<<fed_id0<<" "<<event_id<<" "
	    <<numEvent<<endl;
	if(halt) status = waitForInput();
      }
      if(event_id < event_id_old+1 ) {
	if(event_id_old<16000000) // it wraps around on 2^24
	  cout<<" out of sequence event number "<<event_id<<" should be "
	    <<(event_id_old+1)<<" event "
	    <<numEvent<<", was there a DAQ resync? "<<endl;
	//if(halt) status = waitForInput();
      }
      event_id_old=event_id;
      numEvent++;

      // read events until a trailer
      int i=0;
      int hitsInEvent=0;
      do {
	in_file.read((char*)&word64,8);
	if ( in_file.eof() ) {
	  cout<<" eof om the middle of event "<<endl;
	  eof=true;
	  break;             
	}

	// Loop for data, exit on trailer
	if( (word64 >> 56) != 0xa0 ) {
	  status=0;
	  int data1 = int(  word64     &0x00000000ffffffffL ); //lower bits
	  int data2 = int( (word64>>32)&0x00000000ffffffffL ); //higher bits
	  if(PRINT_DUMMY) cout<<i<<" "<<hex<<word64<<" "<<data1<<" "<<data2<<dec;
	  status = MyDecode::data(data1, channel, roc, dcol, pix);
	  if(status>0) {
	    hitsInEvent++;
	    //cout<<channel<<" "<<roc<<" "<<dcol<<" "<<pix<<endl;
	  } else if(status<0) {
	    error=true;
	    if(HaltOnError) {
	      //cout<<" wrong status "<<status<<" "<<event_id<<endl;
	      halt=true;
	    }
	  }
	  status = MyDecode::data(data2, channel, roc, dcol, pix);	
	  if(status>0) {
	    hitsInEvent++;
	    //cout<<channel<<" "<<roc<<" "<<dcol<<" "<<pix<<endl;
	  } else if(status<0) {
	    error=true;
	    if(HaltOnError) {
	      //cout<<" wrong status "<<status<<" "<<event_id<<endl;
	      halt=true;
	    }
	  }

	  i++;
	}
      } while((word64 >> 56) != 0xa0);  // found trailer, exit
      
      if(eof) break;

      if(printHeaders) status = MyDecode::trailer(word64);

      int slinkLength = int( (word64>>32) & 0xffffff );
      //int crc         = int( (word64&0xffff0000)>>16 );
      //int tts         = int( (word64&0xf0)>>4);
      //int slinkError  = int( (word64&0xf00)>>8);
      if(slinkLength != (i+2) ) cout <<"Error in the event length: slink = "<<slinkLength
				     <<" count = "<<(i+2)<<endl; 
      //
      if(hitsInEvent>0) eventsWithHits++;
      hitsTotal +=hitsInEvent;
      eventSize += float(slinkLength);
      if(error) cout<<" for event "<<event_id<<endl;
      if(halt && HaltForEachEvent) {
	int dummy=0;
	cout<<" Enter 0 to continue, -1 to stop, 999 continous "; 
	cin>>dummy;
	if(dummy==999) halt=false;
	if(dummy==-1) break;
	//cout<<dummy<<" "<<halt<<endl;
      }

    } //  dump RAW
  } // end event loop


  // eventSize is in units of 64bits (8 bytes) and include the Slink 
  // header and trailer
  float tmp=0;
  if(numEvent>0) tmp=eventSize/float(numEvent);
  cout<<" Events "<<numEvent<<", events with hits "<<eventsWithHits<<", hits "
      <<hitsTotal<<", data size in 64bit words "<<eventSize
      <<", data per event "<<tmp<<" 64bit words"<<endl;

  in_file.close();
  //cout << " Close input file " << endl;
  return(0);
}




