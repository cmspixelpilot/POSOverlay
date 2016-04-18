#include "BPixelTools/pxfec/include/SysCommand.h"
#include "BPixelTools/tools/include/VMELock.h"
#include "PixelFECInterface/include/PixelPh1FECInterface.h"


#include <string>
#include <iostream>
#include <unistd.h>

#include "BPixelTools/tools/include/MultiplexingServer.h"
#include "BPixelTools/pxfec/include/ControlNetwork.h"

using namespace std;




//--------------------------------------------------
// some global variables needed for access to vme stuff inside of "exec"
//int32_t gBHandle=0;
pos::PixelFECConfigInterface* pixelFECInterface[22]={0};
ControlNetwork* cn[64]={0};

PixelPh1FECInterface* initPixelFEC(int slot){
    
    unsigned long fecBase=0x08000000 * slot;
    cout << fecBase << endl;
    std::string build_home(getenv("BUILD_HOME"));
    std::string datbase_ = build_home + "/pixel/PixelFECInterface/dat/";
    std::string uri = "chtcp-2.0://131.225.180.40:10203?target=192.168.1.169:50001";
    RegManager * pRegManager = new RegManager("board", uri, "file://" + datbase_ + "address_table.xml");
    
    int dummy_vmeslot=0; unsigned int dummy_feccrate=0; unsigned int dummy_fecslot = 0;
    PixelPh1FECInterface* aFECInterface = new PixelPh1FECInterface(pRegManager,dummy_vmeslot,dummy_feccrate,dummy_fecslot);

    cout<<"Init FEC in slot "<< slot <<endl;
    
    const bool glib = false;
    
    // tell the firmware explicitly to do all these actions. Arguments for all these functions are: mfec and set bit 0 or 1. There is no need for the fecchannel, it applied to both channels

// JMT GLIB?
//   aFECInterface->enableinternalclk();
//    std::cout << "Switching to FCLKA" << std::endl;
//    aFECInterface->disableinternalclk();

	if (glib)
      aFECInterface->switchclk(0x10BA3071);
	else
      aFECInterface->switchclk(0xFF7CFFD8);

    // disable AMC13 data

//    int disableexttrigger_ = aFECInterface->disableexttrigger(1,1);
//    cout <<"disableexttrigger_ " << disableexttrigger_ << endl;
//
//    int loopnormtrigger_ = aFECInterface->loopnormtrigger(1,1);
//    cout <<"loopnormtrigger_ " << loopnormtrigger_ << endl;
//
//    int injectrstroc_ = aFECInterface->injectrstroc(1,1);
//    cout <<"injectrstroc_ " << injectrstroc_ << endl;
//
//    int injectrsttbm_ = aFECInterface->injectrsttbm(1,1);
//    cout <<"injectrsttbm_ " << injectrsttbm_ << endl;
//
//    // enable external AMC13 trigger again

//    int disableexttrigger_ = aFECInterface->disableexttrigger(1,0);
//    cout <<"disableexttrigger_ " << disableexttrigger_ << endl;

  
//   int calpix_ = aFECInterface->calpix(1,1,15, 7, 1,1,1,1, true);

  	    
   int myData = 0; 
     int readbyte = aFECInterface->getByteHubCount(1, 1, 1, &myData);

    cout <<"myData " << hex << myData << hex << " "<<readbyte << dec <<  endl;


cout <<"bla bla"<<endl;
   
    //   aFECInterface->haltest();

  cout <<" ---end of cunstructor--- " << endl;
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
