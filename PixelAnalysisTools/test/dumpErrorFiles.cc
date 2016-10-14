//
// Hex ascii reading works only with compiled code 
// call by .L dumpBinaryFile.C+
// and then   
// dumpBinaryFiles("PhysicsData1_366.dmp")
// Corrected for SLC4 7/07

#include <iostream>
#include <fstream>
using namespace std;

const bool skipResetMessage = false;
int countErrors[37][20];
int tog0word=0;

// Include the helper decoding class 
/////////////////////////////////////////////////////////////////////////////
class MyDecode {
public:
  MyDecode() {}
  ~MyDecode() {}
  static int error(int error);
  static int data(int error);
private:
};
//////////////////////////////////////////////////////////////////////
int waitForInput() {
  int dummy=0;
  cout<<" Enter 0 to continue, -1 to stop  "; 
  cin>>dummy;
  return dummy;
}
/////////////////////////////////////////////////////////////////////////////
// Decode error FIFO
// Works for both, the error FIFO and the SLink error words. d.k. 25/04/07
int MyDecode::error(int word) {
  const bool PRINT_ERRORS = true;
  const int SELECT_CHANNEL = 999;
  int status = -1;
  bool skipReport=false;
  const bool printFirstReset = true;

  const unsigned int  errorMask      = 0x3e00000;
  const unsigned int  dummyMask      = 0x03600000;
  const unsigned int  gapMask        = 0x03400000;
  const unsigned int  timeOut        = 0x3a00000;
  const unsigned int  eventNumError  = 0x3e00000;
  const unsigned int  trailError     = 0x3c00000;
  const unsigned int  fifoError      = 0x3800000;
 
//  const unsigned int  timeOutChannelMask = 0x1f;  // channel mask for timeouts
  const unsigned int  eventNumMask = 0x1fe000; // event number mask
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
  unsigned int event   =  (word & eventNumMask) >>13;
  unsigned int channel = 0;

  if( (word&errorMask) == dummyMask ) { // DUMMY WORD
    //cout<<" Dummy word";
    tog0word=0;return 0;

  } else if( (word&errorMask) == gapMask ) { // GAP WORD
    //cout<<" Gap word";
    tog0word=0;return 0;

  } else if( (word&errorMask)==timeOut ) { // TIMEOUT
//cout<<"timeout word 0x"<<hex<<word<<" tog0word 0x"<<tog0word<<dec<<endl;    
 int prints=0;
     if((((word&0xfc000000)>>24)+((word&0x1800)>>11))>0){ // for sure the timeout counter
   if(PRINT_ERRORS||channel==SELECT_CHANNEL) cout<<"Timeout Error - ";
     int index= (word & 0x1F);  // index within a group of 4/5
     unsigned int chip0 = (word& BlkNumMask)>>8;
     int offset0 = offsets[chip0];
          for(int i=0;i<5;i++) {
       if( (index & 0x1) != 0) {
         channel = offset0 + i + 1;
         if(PRINT_ERRORS||channel==SELECT_CHANNEL) cout<<"channel: "<<channel<<" ";
       }
       index = index >> 1;
     }
countErrors[channel][11]++;
  if(PRINT_ERRORS){
  cout<<" :Timeout counter:  "<<(((word&0xfc000000)>>24)+((word&0x1800)>>11));
  prints=1;}
     
if((tog0word>0)&&(( (tog0word & eventNumMask) >>13)==event)){
  if(PRINT_ERRORS)cout<<" :Pedestal Correction value: ";
     
if(tog0word&0x200){   if(PRINT_ERRORS)cout<<"-"<<dec<<(((~tog0word)&0x1ff)+1);} else {
  if(PRINT_ERRORS)cout<<dec<<(tog0word&0x1ff);}
}

tog0word=0;     
} else {  
if(tog0word!=0){  

if(PRINT_ERRORS)cout<<" Orphan Timeout Pedestal Correction value: ";
     
if(tog0word&0x200){   if(PRINT_ERRORS)cout<<"-"<<dec<<(((~tog0word)&0x1ff)+1);} else {
  if(PRINT_ERRORS)cout<<dec<<(tog0word&0x1ff);}
 if(PRINT_ERRORS) cout<<":event: "<<((tog0word & eventNumMask) >>13)<<endl;  
  tog0word=0;
} 
tog0word=word;

}
     if(PRINT_ERRORS||channel==SELECT_CHANNEL) if(prints)cout<<":event: "<<event<<endl;

   } else if( (word&errorMask) == eventNumError ) { // EVENT NUMBER ERROR
     channel =  (word & channelMask) >>26;
     unsigned int tbm_event   =  (word & tbmEventMask);
     tog0word=0; 
     if(PRINT_ERRORS||channel==SELECT_CHANNEL) 
       cout<<"Event Number Error- channel: "<<channel<<" tbm event nr. "
         <<tbm_event;
     status=-12;
     countErrors[channel][12]++;
     if(PRINT_ERRORS||channel==SELECT_CHANNEL) cout<<":event: "<<event<<endl;
          
   } else if( ((word&errorMask) == trailError)) {  // TRAILER
    channel =  (word & channelMask) >>26;
    unsigned int tbm_status   =  (word & tbmStatusMask);

    if(word & overflowMask) {
      if(PRINT_ERRORS||channel==SELECT_CHANNEL) 
	cout<<"Overflow Error- "<<"channel: "<<channel<<" ";
      status=-10;
      countErrors[channel][10]++;
    }

    if(word & RocErrMask) {
      if(PRINT_ERRORS||channel==SELECT_CHANNEL) 
	cout<<"Number of Rocs Error- "<<"channel: "<<channel<<" ";
      status=-14;
      countErrors[channel][14]++;
    }
    if(word & FsmErrMask) {
      if(PRINT_ERRORS||channel==SELECT_CHANNEL||true) 
	cout<<"Finite State Machine Error- "<<"channel: "<<channel
	    <<" Error status:0x"<<hex<< ((word & FsmErrMask)>>9)<<dec<<" ";
      status=-15;
      countErrors[channel][15]++;
    }

    // trailer errors/messages
    if( (word & RocErrMask) | (word & FsmErrMask) | (word & overflowMask) ) {

      if(PRINT_ERRORS||channel==SELECT_CHANNEL) cout<<":event: "<<event<<endl;

    } else {

      if(event>1 || printFirstReset) 
	if(tbm_status==0x60) {
	  if(!skipResetMessage)  
	      cout<<"Trailer Message- "<<"channel: "<<channel
		  <<" TBM status:0x"<<hex<<tbm_status<<dec
		  <<" TBM-Reset received ";
	} else { 
	  if(PRINT_ERRORS) cout<<"Trailer Error- "<<"channel: "<<channel
		<<" TBM status:0x"<<hex<<tbm_status<<dec<<" ";
	  status=-16;
	  countErrors[channel][16]++;
	}
      else skipReport=true;
      if(PRINT_ERRORS) cout<<":event: "<<event<<endl;
      
    }  // trailer essages

   } else if((word&errorMask)==fifoError) {  // FIFO
   tog0word=0;    
    if(word & Fif2NFMask) cout<<"A fifo 2 is Nearly full- ";
    if(word & TrigNFMask) cout<<"The trigger fifo is nearly Full - ";
    if(word & ChnFifMask) {
      cout<<"fifo-1 is nearly full for channel(s) of this FPGA//";
      if(word & ChnFifMask0) cout<<" 1 //";
      if(word & ChnFifMask1) cout<<" 2 //";
      if(word & ChnFifMask2) cout<<" 3 //";
      if(word & ChnFifMask3) cout<<" 4 //";
      if(word & ChnFifMask4) cout<<" 5 ";
    }
    status=-13;
    countErrors[0][13]++;
    cout<<":event: "<<event<<endl;

  } else {
    cout<<" Unknown error?"<<hex<<word<<dec<<" event "<<event<<endl;
  }
  
  //if(!skipReport) {
    //if(event>0) cout<<":event: "<<event;
  //}

  return status;
}
///////////////////////////////////////////////////////////////////////////
int MyDecode::data(int word) {
  const bool CHECK_PIXELS = false;
  const bool PRINT_PIXELS = false;
  const int ROCMAX = 16;  
  const unsigned int plsmsk = 0xff;   // pulse height
  const unsigned int pxlmsk = 0xff00; // pixel index
  const unsigned int dclmsk = 0x1f0000;
  const unsigned int rocmsk = 0x3e00000;
  const unsigned int chnlmsk = 0xfc000000;
  int status = -1;

  int roc = ((word&rocmsk)>>21);
  // Check for embeded special words
  if(roc>0 && roc<25) {  // valid ROCs go from 1-24
    if(PRINT_PIXELS) cout<<"data "<<hex<<word<<dec;
    unsigned int channel = ((word&chnlmsk)>>26);
    if(channel>0 && channel<37) {  // valid channels 1-36
      //cout<<hex<<word<<dec;
      int dcol=(word&dclmsk)>>16;
      int pix=(word&pxlmsk)>>8;
      int adc=(word&plsmsk);
      if(PRINT_PIXELS) cout<<" Channel- "<<channel<<" ROC- "<<roc<<" DCOL- "<<dcol<<" Pixel- "
	  <<pix<<" ADC- "<<adc<<endl;
      if(CHECK_PIXELS) {
	if(roc>ROCMAX) 
	  cout<<" wrong roc number "<<channel<<"/"<<roc<<"/"<<dcol<<"/"<<pix<<"/"<<adc<<endl;
	if(dcol<0 || dcol>25)
	  cout<<" wrong dcol number "<<channel<<"/"<<roc<<"/"<<dcol<<"/"<<pix<<"/"<<adc<<endl;
	if(pix<2 || pix>181)
	  cout<<" wrong pix number "<<channel<<"/"<<roc<<"/"<<dcol<<"/"<<pix<<"/"<<adc<<endl;
      }
      status=0;
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
  //bool error = false;
  bool halt = false;
  bool wait = false;

  for(int ichan=0;ichan<37;++ichan) 
    for(int ierr=0;ierr<20;++ierr)
      countErrors[ichan][ierr]==0;

  //char filename[80] = "../../../SCRATCH/PhysicsData1_366.dmp";  

  char * filename;
  if(argc>1) {
    filename = argv[1];
  }


  ifstream in_file;  // data file pointer
  in_file.open(filename, ios::binary|ios::in ); // 
  //cout<<in_file.bad()<<" "<<in_file.good()<<" "<<in_file<<endl;
  if (!in_file) {
    cout << " File not found " << endl;
    return(1); // signal error
  }

  // read the header
  unsigned long  runnr;
  in_file.read((char*)&runnr,4);
  if (in_file.eof()) {
  std::cout << "End of input file" <<  std::endl;
  return false;
  }
  cout<<" run number = "<<runnr;
  if(skipResetMessage) cout<<" - tbm trailer messages suppressed "; 
  cout<<endl;

  const int numMax=999999999;
  int numEvent=0; 
  int status=0;
  unsigned long word32=0;
  unsigned long count=0;
  double time=0, time0=0;
	 // Event loop 
  while (numEvent < numMax) {  // 
    in_file.read((char*)&time,8);
    if (in_file.eof()) {
      std::cout << "End of input file" <<  std::endl;
      break;
    }
    in_file.read((char*)&count,4);
    if(numEvent==0) {time0=time;}
    cout<<" time in sec. = "<<(time-time0)<<", count = "<<count<<endl;
    tog0word=0;
    // fifo loop 
    for(int num=0;num<count;++num)  {  // 

      //error=false;
      
      // Has to be a header
      //in_file.read((char*)&word64,8);
      in_file.read((char*)&word32,4);
      //cout<<num<<" "<<hex<<word32<<dec<<endl;

      numEvent++;
   
 
      if(DumpRaw) {

	cout<<hex<<word32<<dec<<endl;
	//int dummy=0;
	//cout<<" Enter 0 to continue, -1 to stop "; 
	//cin>>dummy;
	//if(dummy==-1) break;
	
      } else {
	
	if(word32!=0) status = MyDecode::data(word32);
	//else cout<<" Unknown error, word in hex "<<hex<<word32<<dec<<endl;
	//if(error) cout<<" event "<<event_id<<endl;
	//if(status==-12) wait = true;  // halt on ENE 
	
	if(halt||wait) {
	  int dummy=0;
	  cout<<" Enter 0 to continue, -1 to stop, 999 continous "; 
	  cin>>dummy;
	  if(dummy==999) halt=false;
	  if(dummy==-1) break;
	  //cout<<dummy<<" "<<halt<<endl;
	  wait=false;
	}
	
      } //  dump RAW
    } // count
  } // end event loop

  in_file.close();
  //cout << " Close input file " << endl;

  cout<<" Summary, events "<<numEvent<<endl;



  // 10 - overflow ()
  // 11 - timeout (29)
  // 12 - ENE (31)
  // 13 - fifo (28)
  // 14 - num of rocs (36)
  // 15 - fsm error ()
  // 16 - trailer  (30)
  //
  const string errorString[20] = {
  // 0   1   2   3   4   5   6   7   8   9 
    " "," "," "," "," "," "," "," "," "," ",
    "overflow   ","timeout    ","ENE        ","fifo       ","num-of-rocs",
    "fsm        ","trailer    ",
    " "," "," "};
  int countAllChanErrors[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  cout<<" channel error count "<<endl;
  for(int ichan=0;ichan<37;++ichan) 
    for(int ierr=0;ierr<20;++ierr) 
      if(countErrors[ichan][ierr]>0) {
	cout<<ichan<<" "<<errorString[ierr]<<" "
	    <<countErrors[ichan][ierr]<<endl;
	countAllChanErrors[ierr] +=countErrors[ichan][ierr];
      } 
    for(int ierr=0;ierr<20;++ierr) 
      if(countAllChanErrors[ierr]>0) 
	cout<<"Tot. "<<errorString[ierr]<<" - "<<countAllChanErrors[ierr]<<endl;
      
  return(0);
}




