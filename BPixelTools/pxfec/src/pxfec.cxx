#include "BPixelTools/pxfec/include/SysCommand.h"
#include "BPixelTools/tools/include/VMELock.h"
#include "PixelFECInterface/include/PixelFECInterface.h"

#include "VMEDevice.hh"
#include "VMEAddressTable.hh"
#include "VMEAddressTableASCIIReader.hh"
#include "CAENLinuxBusAdapter.hh"

#include <string>
#include <iostream>
#include <unistd.h>

#include "BPixelTools/tools/include/MultiplexingServer.h"
#include "BPixelTools/pxfec/include/ControlNetwork.h"

using namespace std;




//--------------------------------------------------
// some global variables needed for access to vme stuff inside of "exec"
//int32_t gBHandle=0;
PixelFECInterface* pixelFECInterface[22]={0};
ControlNetwork* cn[64]={0};

HAL::VMEAddressTable *addressTablePtr = 0;
HAL::CAENLinuxBusAdapter *busAdapter = 0;

//--------------------------------------------------


PixelFECInterface* initPixelFEC(int slot){

  unsigned long fecBase=0x08000000 * slot; 
  HAL::VMEDevice * VMEPtr =new HAL::VMEDevice(*addressTablePtr, *busAdapter, fecBase);

  int dummy_vmeslot=0; unsigned int dummy_feccrate=0; unsigned int dummy_fecslot = 0;
  PixelFECInterface* aFECInterface = new PixelFECInterface(VMEPtr,dummy_vmeslot,dummy_feccrate,dummy_fecslot);

  cout<<"Init FEC in slot "<< slot <<endl;

  // Set the FEC to Pixel mode = 4
  int ret = aFECInterface->setssid(4);  
  if(ret != 4) {  // Error
    cout<<"Error in setssid "<<hex<<ret<<dec<<endl;
    return NULL;
  }
  cout<<"Set FEC to pixel mode "<<endl;


  unsigned long data = 0;
  ret = aFECInterface->getversion(&data); //  
  cout<<"mFEC Firmware Version "<<data<<endl;;

  return aFECInterface;
}


//--------------------------------------------------
void exec(SysCommand* cmd){
  int *slot;
  int *mfec;
  int *sector;
  if(cmd->type==SysCommand::kNONE) return;
  if (cmd->type==SysCommand::kSYS){
    if( cmd->Keyword("fec",&slot) ){ 
      pixelFECInterface[*slot]=initPixelFEC(*slot);
    } else if (cmd->Keyword("mfec",&slot,&mfec,&sector)) {
      cn[*sector   ]=new ControlNetwork(pixelFECInterface[*slot], *slot, *mfec, 1, SysCommand::getCNname(*sector   ));
      cn[*sector+32]=new ControlNetwork(pixelFECInterface[*slot], *slot, *mfec, 2, SysCommand::getCNname(*sector+32));
    } else if (cmd->Keyword("mfec",&slot,&mfec,"*")) {
      ControlNetwork* L12=new ControlNetwork(pixelFECInterface[*slot], *slot, *mfec, 1, "*");
      ControlNetwork* L3 =new ControlNetwork(pixelFECInterface[*slot], *slot, *mfec, 2, "*");
      for(int s=0; s<32; s++){
	cn[s   ]=L12;
	cn[s+32]=L3;
      }
    }else{
      cout << "unknown system command " << endl;
    }
  }else if (cn[cmd->CN]!=NULL){
    cn[cmd->CN]->Execute(cmd);
  }else{
    cout << "ControlNetwork doesn't exist "<< cmd->CN <<  " " << SysCommand::getCNname(cmd->CN) <<endl;
  }
}


//--------------------------------------------------
int main(int argc, char **argv){

  // default configuration variables 
  int port=0;                      // port, 0= no port,    define with option -port
  int VMEBoard=1; //CAEN interface   define with option -vmecaenpci or -vmecaenusb
  string file="data/d.ini";        // init file            define with option -file


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
      VMEBoard=1; // Optical CAEN interface
    }else if(strcmp(argv[i],"-vmecaenusb")==0){
      VMEBoard=2; // USB CAEN interface
    }else{
      cerr << "usage: pxfec [-port <port>] [-vmecaenpci | -vmecaenusb] [-init <filename>]" << endl;
      exit(1);
    }
  }


  // Init VME  
  cout<<" Use HAL, get busadapter "<<endl;
  // Get the HAL bus adaptor
  if(VMEBoard==1) 
    //    busAdapter = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718); //optical
    busAdapter = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718,0,0,HAL::CAENLinuxBusAdapter::A3818) ;

  else if(VMEBoard==2) 
    busAdapter = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V1718); //usb 
  else {
    cout<<" VME interface not chosen "<<VMEBoard<<endl;
    exit(1);
  }
  HAL::VMEAddressTableASCIIReader reader("PFECAddressMap.dat");
  addressTablePtr =
    new HAL::VMEAddressTable( "PFEC address table", reader);

  VMELock lock(1);
 

  // init SysCommand
  int promptMode=3;  // use 3 to reflect the Network name in the prompt
  SysCommand* cmd=new SysCommand();
  if(cmd->Read(file.c_str())==0){
    do{
      lock.acquire();
      exec(cmd); 
      lock.release();
    } while (cmd->Next());
  }

  // init server
  MultiplexingServer serv(cmd);
  serv.setPrompt(cmd->TargetPrompt(promptMode,">"));
  if( port >0 ){
    serv.open(port);
  }


  // go
  while(serv.eventloop() ){
    lock.acquire();
    exec(cmd);
    lock.release();
    serv.setPrompt(cmd->TargetPrompt(promptMode,">"));
  }

  
}
