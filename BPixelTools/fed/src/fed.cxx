#include <iostream>
#include <iomanip>
#include <time.h>
#include <unistd.h>  // for usleep
#include <math.h>
#include <stdio.h>
#include <string.h>

using namespace std;
#include "VMELock.h"

// HAL includes
#include "VMEDevice.hh"
#include "VMEAddressTable.hh"
#include "VMEAddressTableASCIIReader.hh"
#include "CAENLinuxBusAdapter.hh"

// Pixel includes
#include "PixelFEDInterface/include/PixelFEDInterface.h" // PixFED class definitionl
#include "PixelFEDInterface/include/PixelFEDFifoData.h" // PixFED data decode
#include "CalibFormats/SiPixelObjects/interface/PixelFEDCard.h" //The FED settings structure


using namespace pos;

// ----------------------------------------------------------------------

#include "SimpleCommand.h"
#include "MultiplexingServer.h"

    string aoh[37]={"",
		"2B-2","2B-1","2B-0","2A-2","2A-1","2A-0",  // 1-6
		"1B-2","1B-1","1B-0","1A-2","1A-1","1A-0",  // 7-12
		"4B-2","4B-1","4B-0","4A-2","4A-1","4A-0",  // 13-18
		"3B-2","3B-1","3B-0","3A-2","3A-1","3A-0",  // 19-24
		"6B-2","6B-1","6B-0","6A-2","6A-1","6A-0",  // 25-30
		"5B-2","5B-1","5B-0","5A-2","5A-1","5A-0"
    };

// ----------------------------------------------------------------------
/////////////////// obsolete ///////////////////////
int initialize(PixelFEDInterface &fed1, HAL::VMEDevice &PixFEDCard) {

  // Reset FED
  int status = fed1.reset();
  if (status != 0) exit(0); 
  
  // Try to read the board id
  //long unsigned data = 0;
  uint32_t data = 0;
  PixFEDCard.read("READ_GA",&data);
  cout<<" Board ID = "<<data<<endl;

  // Setup from file
  string fileName("params_fed.dat"); // define the file name
  PixelFEDCard pixelFEDCard(fileName); // instantiate the FED settings(should be private)
  
  status = fed1.setupFromDB(pixelFEDCard);
  if (status == -1) {
    cout<<" No configuration file "<<endl;
    return(0);
  }

  fed1.BaselineCorr_off();
  fed1.resetSlink();
  fed1.setModeRegister(0x8);  // ignore slink LFF
  status = fed1.enableSpyMemory(0); // bit 1 = 1 in mode reg
  int mode = fed1.getModeRegister(); //check the mode
  cout << " ----------------> Mode register = "<<hex<<mode<<endl;

  int cntrl = fed1.getControlRegister();  // read it
  int cntrl2 = fed1.getCcntrl();  // last written
  cout << " Control register " << hex << cntrl << " " << cntrl2 << dec << endl; 
  
  int value=0;
  value = 0x18; // transparent gate from L1A, TTC event#, for V4 fifoI =0x18
  value = value | 0x1; // set transparent bit
  // Set the control register
  cout << " Load control register with " << hex << value << dec << " dec= " << value << endl;
  status = fed1.setControlRegister(value);  
  return(0); 
}





// ======================================================================
int readChannel(PixelFEDInterface &fed1, int channel, int* data, int nValue=4096){

  // read the fifo for one channel and fill it into the buffer at *data
  if((channel<1) || (channel>36)){
    cout << "illegal channel " << channel << endl;
  }
  // are new events coming in?
  int nloop=0;
  int eventNumber=fed1.readEventCounter();
  usleep(1000); 
  while(eventNumber==fed1.readEventCounter()){
    usleep(1000); 
    nloop++;
    if (nloop>=100){
      cout << "timeout, make sure we have triggers" << endl;
      return -1;
    }
  }

  // drain the fifo, always drain the full fifo, i.e. 1024*4
  // unsigned long buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  uint32_t buffer[(36*1024)];
  //int status = fed1.drainFifo1(channel, buffer, 4096); 
  int myread = fed1.drainFifo1(channel, buffer, 4096);
  //fed1.drain_transBuffer(channel,buffer);
  //cout << "here" << endl;

  //int nread=nValue;
  int nread = myread;
  for(int i=0; i<nValue; i++){
    int marker=buffer[i]&0xff;                           // the lowest bits may contain extra data
    data[i] = ((buffer[i] & 0xffc00000)>>22) & 0xffffff; // analyze word
    if (marker==0xff){
      nread=i;
    }
  }
  return nread;
}


// ======================================================================
int readChannel2(PixelFEDInterface &fed1, int channel, int* data, int nValue=4096){
  static int eventNumber=0;
  //static unsigned long buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  uint32_t buffer[(36*1024)];

  assert((channel>0) && (channel<37));

  // are new events coming in?
  int nloop=0;
  int newEventNumber=fed1.readEventCounter();
  while(eventNumber==newEventNumber){
    usleep(1000); 
    nloop++;
    if (nloop>=100){
      cout << "timeout, make sure we have triggers" << endl;
      return -1;
    }
    newEventNumber=fed1.readEventCounter();
  }

  // drain the fifo, always drain the full fifo, i.e. 1024*4
  //int status = fed1.drainFifo1(channel, buffer, 4096); 
  fed1.drainFifo1(channel, buffer, 4096); 
  int nread=nValue;
  for(int i=0; i<nValue; i++){
    int marker=buffer[i]&0xff;                           // the lowest bits may contain extra data
    data[i] = ((buffer[i] & 0xffc00000)>>22) & 0xffffff; // analyze word
    if (marker==0xff){
      nread=i;
    }
  }
  return nread;
}


// ======================================================================
int readBaseline(PixelFEDInterface &fed1, int channel, unsigned long &value){
  // read the baseline of one channel
  // are new events coming in?
  int nloop=0;
  int eventNumber=fed1.readEventCounter();
  usleep(1000); 
  while(eventNumber==fed1.readEventCounter()){
    //cout<<" no new event"<<endl;
    usleep(1000); 
    nloop++;
    if (nloop>=100){
      cout << "timeout, make sure we have triggers" << endl;
      return -1;
    }
  }
    
  //unsigned long buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  uint32_t buffer[(36*1024)];
  int status = fed1.drainFifo1(channel, buffer, 4096); 
  value=0;
  const int nValue=15;
  for(int i=0; i<nValue; i++){
    unsigned long data = (buffer[i] & 0xffc00000)>>22; // analyze word
    value+=data;
  }
  value=value/nValue;
  return status;
}





// ======================================================================
int readBaselines(PixelFEDInterface &fed1, unsigned long *values){
  // get the baselines of all channels
  // are new events coming in?
  int nloop=0;
  int eventNumber=fed1.readEventCounter();
  usleep(1000); 
  while(eventNumber==fed1.readEventCounter()){
    //cout<<" no new event"<<endl;
    usleep(1000); 
    nloop++;
    if (nloop>=100){
      cout << "timeout, make sure we have triggers" << endl;
      return 0;
    }
  }
 
  //unsigned long buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  uint32_t buffer[(36*1024)];
  for(int channel=0; channel<36; channel++){
    fed1.drainFifo1(channel+1, buffer, 4096); 
    unsigned long value=0;
    const int nValue=20;
    for(int i=0; i<nValue; i++){
      unsigned long data = (buffer[i] & 0xffc00000)>>22; // analyze word
      value+=data;
    }
    values[channel]=value/nValue;
  }
  return 1;
}


// ======================================================================
int readRMS2(PixelFEDInterface &fed1, double *mean, double *rms, const int nChannelArg=0, int *channelListArg=0){
  int buffer[4097];
  const int offset=150;    // start rms (avoid possible readouts)
  const int N=400;        // use n points (if available)
  int nChannel=36;
  int channelList[36]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36};
  if (nChannelArg>0){
    nChannel=nChannelArg;
    for(int i=0; i<nChannel; i++){ channelList[i]=channelListArg[i]; }
  }

  for(int ch=0; ch<nChannel; ch++){
    int nbuf=readChannel(fed1, channelList[ch], buffer,N+offset);
    double sum=0,sum2=0;
    for(int i=offset; i<nbuf; i++){
      sum+=double(buffer[i]);
      sum2+=double(buffer[i])*double(buffer[i]);
    }
    mean[ch]=sum/double(nbuf-offset);
    rms[ch]=sqrt(sum2/double(nbuf-offset)-mean[ch]*mean[ch]);
  }
  return 1;
}

// ======================================================================
int readRMS(PixelFEDInterface &fed1, double *mean, double *rms){
  int buffer[4097];
  const int offset=150;    // start rms (avoid possible readouts)
  const int N=400;        // use n points (if available)
  for(int ch=1; ch<37; ch++){
    int nbuf=readChannel(fed1, ch, buffer,N+offset);
    double sum=0,sum2=0;
    for(int i=offset; i<nbuf; i++){
      sum+=double(buffer[i]);
      sum2+=double(buffer[i])*double(buffer[i]);
    }
    mean[ch-1]=sum/double(nbuf-offset);
    rms[ch-1]=sqrt(sum2/double(nbuf-offset)-mean[ch-1]*mean[ch-1]);
  }
  return 1;
}


// ======================================================================
int readoutLength(int ndata, int* d, const int header0, const int nroc){
  // determine the readout length, assuming that the header position is known
//   cout << "header= " << header << endl;
//   cout << "tstart= " <<header+8+nroc*3 << endl;
//   cout << "d-1 " << d[header-1] << endl;
//   cout << "d0 " << d[header] << endl;
  int header=header0;
  if (header0==0){
    // guess header position
    double dub=-1024.;
    for(int i=2; i<400; i++){
      double fom=(d[i-1]+d[i-2]+d[i+3]-(d[i]+d[i+1]+d[i+2]))/3.;
      if (fom>dub) {
	double dmax=fom*0.1;
	if(  (fabs(d[i+3]-d[i-2])<dmax)
	     &&(fabs(d[i+3]-d[i-1])<dmax)
	     &&(fabs(d[i-1]-d[i-2])<dmax)
	     &&(fabs(d[i]  -d[i+1])<dmax)
	     &&(fabs(d[i]  -d[i+2])<dmax)
	     &&(fabs(d[i+1]-d[i+2])<dmax)
	     ){
	  dub=fom; header=i;
	}
      }
    }
  }

  double blk=(d[header-1]+d[header-2]+d[header-3]+d[header+3])/4.;
  double ub=(d[header]+d[header+1]+d[header+2])/3.;
  if (blk-ub<10){
    cout << "blk ="<< blk << "  ub=" << ub << "????????" << endl;
  }
  double ubmax=0.8*ub+0.2*blk;
  double bmin=0.8*blk+0.2*ub;
  double bmax=1.2*blk-0.2*ub;
//   cout << "blk ="<< blk << "  ub=" << ub << endl;
//   cout << "ubmax ="<< ubmax << "  bmin/max=" << bmin <<"/"<< bmax << endl;

  for(int i=header+8+nroc*3; i< ndata-8; i+=3){
    if (
	(d[i]<ubmax)&&(d[i+1]<ubmax)&&
	(d[i+2]>bmin)&&(d[i+2]<bmax)&&
	(d[i+3]>bmin)&&(d[i+3]<bmax)&&
	(d[i+8]>bmin)&&(d[i+8]<bmax)&&
	(d[i+9]>bmin)&&(d[i+9]<bmax)&&
	(d[i+10]>bmin)&&(d[i+10]<bmax)){
      return i-header;
    }
  }
  return -1;
}


// ======================================================================
int analyzeBuffer(int ndata, int* d,int verbose=0){
  const int dmax=1023;
  // find black level and rms:
  

  //1) make a frequency histogram of the data
  int histo[dmax+1]={0};
  for(int i=0; i<ndata; i++){
    if ((d[i]<0) || (d[i]>dmax)){
      std::cout << "data out of range " << d[i] << endl;
      return -1;
    }
    histo[d[i]]++;
  }

  // 2) get the median
  int s=0,median=0;
  while( (s<ndata/2) && (median<dmax) ){ s+=histo[median]; median++;}
  if((median<2)||(median>dmax)){
    std::cout << "junk data, median= " <<  median << endl;
    return -1;
  }

  //3) estimate the truncated (50%) mean and the noise
  int w=0, coverage=histo[median]; 
  double mean=histo[median]*double(median);
  while( ((w<1)|| ((coverage<(ndata/2)))) && (median-w>1)&&(median+w<dmax) ){
    w++;
    coverage+=histo[median-w]+histo[median+w]; 
    mean+=(median-w)*histo[median-w]+(median+w)*histo[median+w];
  }
  mean=mean/coverage;
  
  double rms=0;
  //  for(int j=median-w; j<=median+w; j++){
  //  rms+=histo[j]*(double(j)-mean)*(double(j)-mean)/coverage;
  //}
  int nrms=0;
  for(int i=0; i<ndata; i++){
    if (fabs(d[i]-mean)<10){
      rms+=(d[i]-mean)*(d[i]-mean);
      nrms++;
    }
  }
  rms=rms/nrms;
  rms=sqrt(rms);
  //cout << "median = " << median << " width " << w <<"("<<double(coverage)/double(ndata) << ")    ";
  //cout <<" mean="<<mean << "   rms=" << rms << endl;
  if((rms>10.)||(w>10)){
    cout << "funky data" <<  "  mean=" << mean << "  rms="<< rms <<  endl;
  }
  if(fabs(double(d[0])-mean)>(4*rms)){
    cout << "buffer doesn't start with black??  d[0]="<< d[0] << "  mean=" << mean << "  rms="<< rms <<  endl;
  }
  
  // now try to identify data packets
  int hpos=0;   // header position=start of ub
  int hbmax=5;   // max number of blacks preceding the first ub
  int hblen=0;   // number of blacks preceding the first ub
  int tpos=0;
  double hub=0;
  double tub=0;

  for(int i=1; i<ndata-3; i++){
    
    // header
    if(i>=hbmax){ hblen=hbmax;}else{ hblen=i;}
    double hb=d[i+3];   for(int j=1; j<=hblen; j++){ hb+=d[i-j]; }
    hb/=(hblen+1);  // preceding series of blacks + one following the ub
    double hu=(d[i]+d[i+1]+d[i+2])/3.;
    if((hb-hu)>8*rms && (fabs(hb-mean)<8*rms)){
      // might be a header, double check
      double chsqu=pow(d[i]-hu,2)+pow(d[i+1]-hu,2)+pow(d[i+2]-hu,2);
      double chsqb=pow(d[i+3]-hb,2);  for(int j=1; j<=hblen; j++){ chsqb+=pow(d[i-j]-hb,2);}
      //      cout << i <<  "u=" << hu << "  chsqu=" <<chsqu << " (" <<5*rms*rms <<")  b="<<hb << "  chsqb=" <<  chsqb <<  " (" << 2*(hblen+5)*rms*rms << ")  hblen=" << hblen  <<endl;
      if ((chsqu<10.*3.*rms*rms) && (chsqb<(10.*(hblen+2)*rms*rms))){
	if(hpos==0){
	  // first ub, take it
	  hpos=i; hub=hb-hu;
	}else{
	  // already had an ub, accept the new one if its amplitude is larger
	  if (hb-hu>hub){
	    if((verbose>0)&&(hub>100)){
	      cout << "header at  " << i << " ub=" << hu << "  b=" << hb << " b-u=" << hb-hu << endl;
	      cout << "--> supersedes previously found ub at " << hpos << endl;
	    }
	    hpos=i; hub=hb-hu; 
	  }
	}
      }else{
	// chi**2 check failed for header candidate
	if(verbose>0){
	  cout << "header candidate at " << i <<  " ub=" << hu << "  b=" << hb << " b-u=" << hb-hu << " failed chsq test " << endl;
	  cout << "chsqb=" << chsqb << "  chsqu= " << chsqu << "   rms= "<< rms << endl;
	}//verbose
      }
    
    }


    // trailer
    int tbmax=5;
    int tblen=0;
    if((i+8)<(ndata-tbmax)){ tblen=tbmax;}
    else if ((i+8)<ndata){ tblen=ndata-i;}
    else{tblen=0;}
    double tb=d[i+2]+d[i+3]; for(int j=0; j<tblen; j++){ tb+=d[i+j+8]; }
    tb/=(tblen+2);
    double tu=(d[i]+d[i+1])/2.;
    if((tb-tu)>20 && (fabs(tb-mean)<8.*rms) && ((tb-tu)>(8.*rms))){
      double chsqtu=pow(d[i]-tu,2)+pow(d[i+1]-tu,2);
      double chsqtb=pow(d[i+2]-tb,2)+pow(d[i+3]-tb,2);
      for(int j=0; j<tblen; j++){ 
	chsqtb+=pow(d[i+8+j]-tb,2); 
	//cout <<i+j+8 <<  " " << d[i+j+8] << " "  <<pow(d[i+8+j]-tb,2) << endl; 
      }
      //cout << i <<  "u=" << tu << "  chsqu=" <<chsqtu << " (" <<5*rms*rms <<")  b="<<tb << "  chsqb=" <<  chsqtb <<  " (" << (hblen+5)*rms*rms << ")  hblen=" << hblen  <<endl;

      if ((chsqtu<10*2*rms*rms) && (chsqtb<(10*(tblen+1)*rms*rms))){
	// trailer candidate
	// cout << "trailer at " << i << " ub=" << tu << "  b=" << tb << " b-u=" << tb-tu << endl;
	if(tpos==0){
	  tpos=i; tub=tb-tu;
	}else{
	  // already had an ub
	  if (tb-tu>tub){
	    //cout << "--> supersedes previously found ub at " << tpos << endl;
	    tpos=i; tub=tb-tu; 
	  }
	}
      }
    }
  } // i

  // decode header/trailer
  if(hpos==0){
    cout << "header not found" << endl;
    return 1;
  }else{
    cout << "header at " << hpos << " ub=" << hub  << endl;
  }
  if(tpos==0){
    cout << "trailer not found" << endl;
    return 1;
  }else{
    cout << "trailer at " << tpos << " ub=" << tub << endl;
  }
  if((tpos>0)&&(hpos>0)){
    cout << "readout length " << tpos-hpos << endl;
  }
  if(fabs(tub-hub)>4.*rms){
    cout << "header/trailer ub mismatch " << endl;
  }
  
  double ub=(tub+hub)/2.;
//   cout << "h0 " << 4.*(d[hpos+4]-mean)/ub << " " <<  rms/ub << endl;
//   cout << "h1 " << 4.*(d[hpos+5]-mean)/ub <<  rms/ub << endl;
//   cout << "h2 " << 4.*(d[hpos+6]-mean)/ub <<  rms/ub << endl;
//   cout << "h3 " << 4.*(d[hpos+7]-mean)/ub <<  rms/ub << endl;
//   cout << "t0 " << 4.*(d[tpos+4]-mean)/ub <<  rms/ub << endl;
//   cout << "t1 " << 4.*(d[tpos+5]-mean)/ub <<  rms/ub << endl;
//   cout << "t2 " << 4.*(d[tpos+6]-mean)/ub <<  rms/ub << endl;
//   cout << "t3 " << 4.*(d[tpos+7]-mean)/ub <<  rms/ub << endl;

  int counter= int(1.5+4*(d[hpos+4]-mean)/ub)*64
              +int(1.5+4*(d[hpos+5]-mean)/ub)*16
              +int(1.5+4*(d[hpos+6]-mean)/ub)*4
              +int(1.5+4*(d[hpos+7]-mean)/ub);
  int tword=   int(1.5+4*(d[tpos+4]-mean)/ub)*64
              +int(1.5+4*(d[tpos+5]-mean)/ub)*16
              +int(1.5+4*(d[tpos+6]-mean)/ub)*4
              +int(1.5+4*(d[tpos+7]-mean)/ub);
  cout << "event counter " << counter << endl;
  if (tword&(1<<7)) cout << "not token pass" << endl;
  if (tword&(1<<6)) cout << "TBM reset" << endl;
  if (tword&(1<<5)) cout << "ROC reset" << endl;
  if (tword&(1<<4)) cout << "Sync Trigger Error" << endl;
  if (tword&(1<<3)) cout << "Sync Trigger" << endl;
  if (tword&(1<<2)) cout << "Event Counter Cleared" << endl;
  if (tword&(1<<1)) cout << "Cal-Inject Issued" << endl;
  if (tword&1 ) cout << "Stack full" << endl;
  return 0;
}// analyzeBuffer





//--------------------------------------------------
PixelFEDInterface* fed[33]={0};  
HAL::VMEDevice * PixFEDCard[33]={0};
HAL::CAENLinuxBusAdapter *busAdapter;
int fedSlot;
std::map<int, int> slotFromSector;
HAL::VMEAddressTableASCIIReader addressTableReader("data/FEDAddressMap.dat");
HAL::VMEAddressTable addressTable("Test address table",addressTableReader);
//--------------------------------------------------


PixelFEDInterface* initPixelFEDInterface(HAL::CAENLinuxBusAdapter *busAdapter, int base, int slot){
  unsigned long fedBase=base;
  cout << "initializing fed in slot " << std::dec << slot << " with base " << std::hex << base << std::dec << endl;
  
  // store the fed card
  PixFEDCard[slot] = new HAL::VMEDevice(addressTable, *busAdapter, fedBase);
  //fed[slot] = new PixelFEDInterface(PixFEDCard[slot]); // Instantiate the FED class OK
  PixelFEDInterface* f = new PixelFEDInterface(PixFEDCard[slot]); // Instantiate the FED class, OK

    
  return f;
}


int getIndex(const sector_t s){
  int idx=abs(s.phi)-1;
  if (s.phi<0)    idx+=16;
  if (s.z=='N')   idx+=8;
  return idx;
}

bool validChannel(const int channel){
  if ((channel<1)||(channel>36)){
    cout << "invalid channel number "<< channel << endl;
    return false;
  }
  return true;
}




// ======================================================================

void exec(SimpleCommand* c){

  static int offset=0;  // start of readout
  int channel=0,slot=0, par=0;
  int ch1,ch2,ch3,ch4,ch5,ch6;
  int header,nroc;
  sector_t sector;
  unsigned long value;
  int base;
  unsigned long values[36]={0};
  double mean[37]={0};
  double rms[37]={0};
  int buffer[4097];

  if(c->Keyword("initfed", slot, base)){
    fed[slot]=initPixelFEDInterface(busAdapter,base,slot);

  }else if(c->Keyword("initfed", slot, base, sector)){
    fed[slot]=initPixelFEDInterface(busAdapter,base,slot);
    char buffer[10]; 
    sprintf(buffer,"%+2d%1s",sector.phi,((sector.z=='N') ? "N" : "P"));
    cout << "map sector " << buffer << " to slot " << slot << endl;

    slotFromSector[getIndex(sector)]=slot;

  }else if(c->Keyword("initfed", slot, base, "*")){
    cout << "initializing fed in slot " << std::dec << slot << " with base " << std::hex << base << std::dec << endl;
    fed[slot]=initPixelFEDInterface(busAdapter,base,slot);
    cout << " map all sectors to fed in slot " << slot << endl;
    for(int idx=0; idx<32; idx++){
      slotFromSector[idx]=slot;
    }
    fedSlot=slot;

  }else if(c->Keyword("fed", slot)){
    fedSlot=slot;

  }else if(c->Keyword("fed", sector)){
    slot=slotFromSector[getIndex(sector)];
    if((slot>=0)&&(slot<33)){
      fedSlot=slot;
    }else{
      cout <<"unknown sector" << endl;
    }

  }else if(c->Keyword("offset", par )){
    offset=par;

  }else if(c->Keyword("get",channel, par)){
    if((channel<1)||(channel>36)){
      cout << "illegal channel " << channel << endl;
    }else{
      int n=readChannel(*fed[fedSlot], channel, buffer,par);
      for(int i=0; i<n; i++){ cout << buffer[i] << " ";}
      cout << endl;
    }

  }else if(c->Keyword("getUB",channel)){
    if((channel<1)||(channel>36)){
      cout << "illegal channel " << channel << endl;
    }else{
      readChannel(*fed[fedSlot], channel, buffer);
      cout << buffer[offset] << endl;
    }


  }else if(c->Keyword("findUB",channel)){
    if((channel<1)||(channel>36)){
      cout << "illegal channel " << channel << endl;
    }else{
    const int ubmin=50;  // require at least that, difference between baseline and ub level in ADC counts
    int n=readChannel(*fed[fedSlot], channel, buffer,1024);
    int ub=ubmin;
    int tpos=0;
    int hpos=0;
    //double blk=buffer[0];  
    //double rms=0;
    //int nblk=1;
    for(int i=2; i<n-3; i++){
      //header
      int dh=(buffer[i-2]+buffer[i-1]-buffer[i]-buffer[i+1]-buffer[i+2]+buffer[i+3])/3;
      if (dh>ub){
	ub=dh; hpos=i;
      }
      //trailer
      int dt=(-buffer[i]-buffer[i+1]+buffer[i+2]+buffer[i+3])/2;
      if ( (ub>ubmin) && (offset>0) && ((i-offset)>(6+8*3)) && (abs(dt-ub)<10)){
	tpos=i;
      }
      
    }
    cout << "ub "<< ub << " at " << hpos << endl;
    }

  }else if(c->Keyword("ho",channel)){
    if((channel<1)||(channel>36)){
      cout << "illegal channel " << channel << endl;
    }else{
    int n=readChannel(*fed[fedSlot], channel, buffer,1024);
    analyzeBuffer(n,buffer);
    }

  }else if(c->Keyword("rl",channel,header,nroc)){
    if(validChannel(channel)){
      int n=readChannel(*fed[fedSlot], channel, buffer,1024);
      cout << readoutLength(n,buffer,header,nroc) << endl;
    }

  }else if(c->Keyword("read",channel)){
    if((channel<1)||(channel>36)){
      cout << "illegal channel " << channel << endl;
    }else{
      readBaseline(*fed[fedSlot], channel, value);
      cout << "baseline = " << value << endl;
    }

  }else if(c->Keyword("read")){
    cout << "reading from fed in slot " << fedSlot << endl;
    if (readBaselines(*fed[fedSlot], values)) {
      cout << "baselines = " ;
      for(channel=0; channel<36; channel++){
	cout << setw(5)<< values[channel];
      }
      cout << endl;
    }
   }else if(c->Keyword("test")){
     int ev[20];
     int i=0;
     ev[i++]=fed[fedSlot]->readEventCounter();
     ev[i++]=fed[fedSlot]->readEventCounter();
     ev[i++]=fed[fedSlot]->readEventCounter();
     ev[i++]=fed[fedSlot]->readEventCounter();
     PixFEDCard[fedSlot]->write("LRES",0x80000000); 
     for(; i<20; ){
     ev[i++]=fed[fedSlot]->readEventCounter();
     }
     for(i=0; i<20; i++){    cout << ev[i] << " "; }
     cout << endl;

  }else if(c->Keyword("read","rms")){
    PixFEDCard[fedSlot]->write("LRES",0x80000000); 
    if (readRMS(*fed[fedSlot], mean,rms)) {
      for(channel=0; channel<36; channel++){
	cout << setw(6) << setprecision(2) << fixed << mean[channel] << " "
	     << setw(6) << setprecision(2) << fixed << rms[channel]  << " ";
      }
      cout << endl;
    }


  }else if(c->Keyword("read","rms2")){
    PixFEDCard[fedSlot]->write("LRES",0x80000000); 
    if (readRMS2(*fed[fedSlot], mean,rms)) {
      for(channel=0; channel<36; channel++){
	cout << setw(6) << setprecision(2) << fixed << mean[channel] << " "
	     << setw(6) << setprecision(2) << fixed << rms[channel]  << " ";
      }
      cout << endl;
    }


  }else if(c->Keyword("rms",ch1,ch2,ch3,ch4,ch5,ch6)){
    PixFEDCard[fedSlot]->write("LRES",0x80000000); // reset fifo1
    int channels[6];
    channels[0]=ch1; channels[1]=ch2; channels[2]=ch3; channels[3]=ch4; channels[4]=ch5; channels[5]=ch6;
    if (readRMS2(*fed[fedSlot], mean,rms,6,channels)) {
      for(channel=0; channel<6; channel++){
	cout << setw(6) << setprecision(2) << fixed << mean[channel] << " "
	     << setw(6) << setprecision(2) << fixed << rms[channel]  << " ";
      }
      cout << endl;
    }

  }else if(c->Keyword("range")){
    int amp[37]={0};
    for(int ch=1; ch<37; ch++){
      readChannel(*fed[fedSlot], ch, buffer,100);
      int minvalue=1024;
      int maxvalue=0;
      for(int i=1; i<100; i++){
	if (buffer[i]<minvalue) minvalue=buffer[i];
	if (buffer[i]>maxvalue) maxvalue=buffer[i];
      }
      amp[ch]=maxvalue-minvalue;
      cout << setw(2) << ch << ":   " << setw(4) << minvalue << " . ." << setw(4)
	   << maxvalue <<  "      " << setw(4) << maxvalue-minvalue << endl;
    }
    for(int ch=1; ch<37; ch++){ cout << ","<<setw(4)<<amp[ch];} cout << endl;


  }else if(c->Keyword("rms")){
    const int N=140;
    for(int ch=1; ch<37; ch++){
      readChannel(*fed[fedSlot], ch, buffer);
 
      double sum=0,sum2=0;
      for(int i=100; i<100+N; i++){
	sum+=double(buffer[i]);
	sum2+=double(buffer[i])*double(buffer[i]);
      }
      mean[ch]=sum/double(N);
      rms[ch]=sqrt(sum2/double(N)-mean[ch]*mean[ch]);
      cout << setw(2) << ch << ":   "
	   << setw(6) << setprecision(1) << fixed << mean[ch] << "    " 
	   << setw(6)  << setprecision(1) <<  fixed << rms[ch]
	   << "    " << aoh[ch] << endl;
    }

  }else if(c->Keyword("rms",channel)){
    if ((channel<1)||(channel>36)){
      cout << "invalid channel number " << channel << endl;
      return;
    }
    //const int N=400;
    const int N=readChannel(*fed[fedSlot], channel, buffer)-200;
    //cout << "reads " << N << endl;
    double sum=0,sum2=0;
    for(int i=100; i<100+N; i++){
    //for(int i=100; i<myread; i++){
      //cout << "buffer " << i << ": " << buffer[i] << endl;
      sum+=double(buffer[i]);
      sum2+=double(buffer[i])*double(buffer[i]);
      //cout << "sum " << sum << endl;
      //cout << "sum2 " << sum2 << endl;
    }
    cout << setw(2) << channel << "   "
	 << setw(6) << setprecision(1) << fixed << sum/double(N) << "    " 
	 << setw(6)  << setprecision(1) <<  fixed << sqrt(sum2/double(N)-sum/double(N)*sum/double(N))
	 << "    " << aoh[channel] << endl;

  }else if(c->Keyword("event")){
    cout << fed[fedSlot]->readEventCounter() << endl;

  }else if(c->Keyword("reset")){
    if(fed[fedSlot]){
      cout << fed[fedSlot]->reset() << endl;
    }else{
      cout << "fed not initialized" << endl;
    }

  }else if(c->Keyword("reset","fifo")){
    PixFEDCard[fedSlot]->write("LRES",0x80000000); 
    cout << "reset fifo in slot " << fedSlot << endl;

  }else if(c->Keyword("set_offset_dac",channel,par)){
    if( (channel>0)&&(channel<37)){
      //PixelFEDCard& pf=fed[fedSlot]->getPixelFEDCard();
      //pf.offs_dac[channel-1]=par;
      //PixFEDCard[fedSlot]->offs_dac[channel-1]=par;
      fed[fedSlot]->set_offset_dacs();
    }else{
      cout << "illegal channel" << endl;
    }

  }else if(c->Keyword("get_opt_inadj",channel)){
    if( (channel>0) && (channel<4) ){
      //cout << "opt inadj =" << fed[fedSlot]->getPixelFEDCard().opt_inadj[channel-1] << endl;
      //cout << "opt inadj =" << PixFEDCard[fedSlot]->opt_inadj[channel-1] << endl;
      PixelFEDCard& pf=fed[fedSlot]->getPixelFEDCard();
      cout << "opt inadj =" << pf.opt_inadj[channel-1] << endl;
    }else{
      cout << "illegal channel (1-4)" << endl;
    }

  }else if(c->Keyword("set_opt_inadj",channel,par)){
    if( (channel>0) && (channel<4) && (par>=0) && (par<=15) ){
      PixelFEDCard& pf=fed[fedSlot]->getPixelFEDCard();
      pf.opt_inadj[channel-1]=par;
      fed[fedSlot]->set_opto_params();
    }else{
      cout << "illegal channel (1-4) or value (0-15)" << endl;
    }

  }else if(c->Keyword("set_BaselineCorr",channel,par)){
    if((channel>0)&&(channel<5)){
      fed[fedSlot]->set_BaselineCorr(channel, par);
    }else{
      cout << "baseline correction:  1: ch0-11, 2: ch12-23  3:  4:" << endl;
    }

  }else{
    cout << "what??  <" << *c << ">" << endl;
  }

  
}
 
// ======================================================================
int main(int argc, char *argv[]) {

  VMELock lock(1);
  lock.acquire();

  //int fullInit(0);
  bool bg=false;
 // default configuration variables
  int port=0;                      // port, 0= no port,    define with option -port
  string file="data/fed.ini";        // init file            define with option -file


  // parse command line arguments
  for(int i=1; i<argc; i++){
    if (strcmp(argv[i],"-port")==0){
      i++;
      try{
        port=atoi(argv[i]);
      }catch(...){
        cerr << "bad port number " << argv[i] << endl;
        exit(1);
      }
    }else if (strcmp(argv[i],"-init")==0){
      i++;
      if(i<argc){
        file=(argv[i]);
      }else{
        cerr << "file argument missing " << endl;
      }
    }else if(strcmp(argv[i],"-vmecaenpci")==0){
      busAdapter = new HAL::CAENLinuxBusAdapter::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718);
    }else if(strcmp(argv[i],"-vmecaenusb")==0){
      busAdapter = new HAL::CAENLinuxBusAdapter::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V1718);
      cout << "using usb adapter" << endl;
    }else{
      cerr << "usage: fed [-port <port>] [-vmecaenpci | -vmecaenusb] [-init <filename>]" << endl;
      exit(1);
    }
  }

  

  SimpleCommand* cmd=new SimpleCommand();
  if(cmd->Read(file.c_str())==0){
    do{ exec(cmd);  } while (cmd->Next());
  }
  lock.release();


  MultiplexingServer serv(cmd, bg);
  if(port){ serv.open(port); }
  cout << "done initializing server " << endl;


  // go
  while(serv.eventloop() ){
    lock.acquire();
    exec(cmd);
    lock.release();
  }

  return 0;
}

