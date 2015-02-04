#include "ControlNetwork.h"
#include "Module.h"
#include "TBM.h"
#include "PixelFECInterface/include/PixelFECInterface.h"
#include "SysCommand.h"

using namespace std;

//extern void analyzeError(CVErrorCodes ret);

ControlNetwork::ControlNetwork(PixelFECInterface *FECInterface, int fecSlot, int mfecNumber, int mfecChannel,string groupName){
  interface=FECInterface;
  //unsigned long data = 0;
  //interface->getversion(&data); 
  //cout<<" version " <<data<<endl;
  slot=fecSlot;
  mfec=mfecNumber;
  channel=mfecChannel;
  id=groupName;
  init();
  for(int hubID=0; hubID<32; hubID++){
    //m[hubID]=new Module(32, hubID, this);
    m[hubID]=new Module(16, hubID, this);
  }
}


int ControlNetwork::ctrlStatus(bool verbose){
  unsigned long data = 0;
  int ret = interface->getfecctrlstatus(mfec,&data);
  if(ret != 0) {  // Error
    cout<<"Error in getstatus "<<hex<<ret<<dec<<endl;
    //analyzeError( CVErrorCodes(ret));
    cout<<" mfec= "<<mfec<<" status "<<hex<<data<<" "<<ret<<dec<<endl;   
    return -1;
  }
  int stat=analyzeFecCsr(data,verbose);
  return stat;
}

int ControlNetwork::getByte(int byte){
  int data;
  int ret = interface->getByteHubCount(mfec,channel,byte,&data);
  if(ret != 0) {  // Error
    cout<<"Error in getByte "<<hex<<ret<<dec<<endl;
    //analyzeError( CVErrorCodes(ret));
    cout<<" mfec= "<<mfec<<" status "<<hex<<data<<" "<<ret<<dec<<endl;   
    return -1;
  }
  return data;
}


// int ControlNetwork::testHubId(const int id){
//   const int tbmInternalPort=4;
//   const int tbmRead= 1;
//   const int tbmChannel=TBM::tbmA;
//   const int tbmRegister= 0; // doesn't really matter, 0..4
//   const int databyte=255; // according to TBM document needed when reading
//   interface->tbmcmd(mfec, channel, tbmChannel, id, tbmInternalPort, tbmRegister, databyte, tbmRead);
//   return ctrlStatus(false); // quiet mode
//   //return ctrlStatus(true); // verbose mode
// }

int ControlNetwork::testHubId(const int id){
  const int tbmInternalPort=4;
  const int tbmRead= 0;
  const int tbmChannel=TBM::tbmA;
  const int tbmRegister= 0; //
  const int databyte=0x01; //
  interface->tbmcmd(mfec, channel, tbmChannel, id, tbmInternalPort, tbmRegister, databyte, tbmRead);
  return ctrlStatus(false); // quiet mode
  //return ctrlStatus(true); // verbose mode
}

void ControlNetwork::scanHubIds(){
  for(int id=0; id<32; id++){
    if (testHubId(id)==0){
      cout << id << " ";
    }
  }
  cout << endl;
}


//###############################################################################################
int ControlNetwork::testHubId(const int id, int mode){ //mode: 0=tbm read, 1=tbm write, 2=roc, 3=rocinit, 4=roctrimload 

  const int tbmInternalPort=4;            //tbm 
  const int tbmChannel=TBM::tbmA;         //tbm
  const int tbmRegister= 0;               //tbm
  const int databyte=0x01;                //tbm 
  int portaddress = 0;                    //roc
  int mask=1; // 0-disable all pixels     //rocinit
  int trim = 0xf;                         //rocinit
  vector<unsigned char> maskTrims(4160);  //roctrimload
  int m =0;
  for(int icol=0;icol<52;icol++) {
    for(int irow=0;irow<80;irow++) {
      maskTrims[m]= char(0x80); // Enable + trim=0
      //maskTrims[m]= char(0x8F); // Enable + trim=F
      //maskTrims[m]= char(0x00); // Disable + trim=0
      //if(icol==10 && irow==10) maskTrims[m]= char(0x80); // select one pixel
      m++;
    }
  }

  if (mode==0) {
    interface->tbmcmd(mfec, channel, tbmChannel, id, tbmInternalPort, tbmRegister, databyte, 1);
  }
  else if (mode==1) {
    interface->tbmcmd(mfec, channel, tbmChannel, id, tbmInternalPort, tbmRegister, databyte, 0);
  }
  else if (mode==2) {
    for(int iroc=0;iroc<16;++iroc) {
      portaddress = iroc/4;  //
      interface->progdac(mfec, channel, id, portaddress, iroc, 0x19, 0); //Vcal 0
    }
  }
  else if (mode==3) {
    for(int iroc=0;iroc<16;iroc++) {
      portaddress = iroc/4; 
      interface->rocinit(mfec, channel, id, portaddress, iroc, mask, trim);
    }
    usleep(100000);  // usleep 
  }
  else if (mode==4) {
    for(int iroc=0;iroc<16;iroc++) {
      portaddress = iroc/4; 
      interface->roctrimload(mfec, channel, id, portaddress, iroc, maskTrims);
    }
    usleep(100000);  // usleep 
  }
  return ctrlStatus(false); // quiet mode
  //return ctrlStatus(true); // verbose mode
}

void ControlNetwork::scanHubIds(int mode){
  for(int id=0; id<32; id++){
    if (testHubId(id , mode)==0){
      cout << id << " ";
    }
  }
  cout << endl;
}
//################################################################################################


void ControlNetwork::Execute(SysCommand *command){

  int *hubid;
  int *bit;

  if(command->type==SysCommand::kCN){
    // control network specific commands

    if (command->Keyword("hello")){
      cout << "my name is " << id << endl;

    }else if (command->Keyword("status")) {
      int stat=ctrlStatus(false);// quiet mode
      if (stat==0) {cout << "ok" << endl;}else{cout <<"error" << endl;}

    }else if (command->Keyword("print","status")) {
      ctrlStatus(true); //verbose version
      int btx=getByte(0); int hubtx=(btx>>3)&0x1F; int porttx=(btx&0x07);
      cout << "Tx : " << getByte(2) << " bytes " << "  hub " << hubtx << "  port " << porttx << endl;
      int brx=getByte(1); int hubrx=(brx>>3)&0x1F; int portrx=(brx&0x07);
      cout << "Rx : " << getByte(3) << " bytes " << "  hub " << hubrx << "  port " << portrx << endl;
    }else if (command->Keyword("version")) {
      unsigned long version;
      //int stat=interface->getversion(mfec, &version);
      int stat=interface->getversion(&version);
      if (stat==0) {cout << "firmware version " << version << endl;}else{cout <<"error" << endl;}

    }else if (command->Keyword("test","hub",&hubid)) {
      if (testHubId(*hubid)==0) { cout << "ok" << endl; }else{ cout << "error" << endl;}

    }else if (command->Keyword("scan","hubs")) {
      scanHubIds();

    }else if (command->Keyword("reset","doh")){
      interface->resetdoh(mfec, channel);

    }else if (command->Keyword("get","txhub")){
      cout << getByte(0) << endl;

    }else if (command->Keyword("get","rxhub")){
      cout << getByte(1) << endl;

    }else if (command->Keyword("get","txcount")){
      cout << getByte(2) << endl;

    }else if (command->Keyword("get","rxcount")){
      cout << getByte(3) << endl;

    }else if (command->Keyword("test","fiber")){
      int rda=0,rck=0;
      interface->testFiber(mfec,channel, &rda, &rck);
      if ((rda==0xffff)&&(rck==0xffff)){
	cout << " ok" << endl;
      }else{
	cout << " failed !!!      rda,rck = " <<  hex << showbase << rda << "  " << rck << dec  << endl;
      }
     
    }else if (command->Keyword("reset","roc")){         interface->injectrstroc(mfec, 1);
    }else if (command->Keyword("reset","roc",&bit)){    interface->injectrstroc(mfec, (*bit)&1); // is this needed??

    }else if (command->Keyword("reset","tbm")){         interface->injectrsttbm(mfec,1);
    }else if (command->Keyword("reset","tbm",&bit)){    interface->injectrsttbm(mfec,(*bit)&1);

    }else if (command->Keyword("disable","triggers")){  interface->disableexttrigger(mfec, 1);

    }else if (command->Keyword("enable","triggers")){   interface->disableexttrigger(mfec, 0);

    }else if (command->Keyword("inject","trigger")){    interface->injecttrigger(mfec, 1);
    }else if (command->Keyword("inject","cal")){        interface->injectrstcsr(mfec, 1);
    

    } //#####################################################
    else if (command->Keyword("local")){
      interface->disableexttrigger(mfec, 1);
      interface->loopcaltrigger(mfec,1);
    }
    else if (command->Keyword("external")){
      interface->disableexttrigger(mfec, 0);
      interface->loopcaltrigger(mfec,0);
    }
    else if (command->Keyword("scanhubs","tbmread")) {
      scanHubIds(0);
    }
    else if (command->Keyword("scanhubs","tbmwrite")) {
      scanHubIds(1);
    }
    else if (command->Keyword("scanhubs","roc")) {
      scanHubIds(2);
    }
    else if (command->Keyword("scanhubs","rocinit")) {
      scanHubIds(3);
    } 
    else if (command->Keyword("scanhubs","roctrimload")) {
      scanHubIds(4);
    }
    //#####################################################
    else{
      cout << "unknown ControlNetwork command "  << endl;
    }
  }else{
    m[command->module]->Execute(command);
  }
}


 
void ControlNetwork::init(){
  //unsigned long data = 0;
  // What does this realy do?
  // Internal triggers work for both settings
  // External: works for both? 
  interface->disableexttrigger(mfec,0);// 0 - Enable external(TTC) triggers

  // 1- Disable external(TTC) triggers
  /*   do we really need all this??
       cout<<" after enable/disable ext trig "<<endl;
       interface->getfecctrlstatus(mfec,&data);  
       cout<<"FEC Status = "<<hex<<data<<dec<<endl;
  */
  //cout<<" FEC cached registers "<<hex<<aFECInterface.getControlReg(mfec)<<" "
  //  <<  aFECInterface.getCsReg(mfec)<<dec<<endl;
    
  // Settings for internal triggers
  interface->enablecallatency(mfec, 0);  // 1=generate cal+trig, 0=cal only
  // Strange but for CAL only something has to be written into the COUNT register
  interface->callatencycount(mfec, 1); // cal-trig delay in bx
  //aFECInterface.callatencycount(i, 150); // cal-trig delay in bx


  // Reset 
  interface->injectrsttbm( mfec, 1);  // send reset-TBM
  //cout<<" after resettbm "<<endl;
  interface->injectrstroc( mfec, 1);  // send reset-ROC
  //cout<<" after reset tbm/roc "<<endl;

  // Try the new setup bits
  int ret = interface->FullBufRDaDisable(mfec,1); //1-set
  if(ret != 0) cout<<" error in set FullBufRDaDisbale "<<ret<<endl; //Error

  
  ctrlStatus();
}


///////////////////////////////////////////////////
int ControlNetwork::analyzeFecCsr(const int status, const bool verbose) {
  if (verbose){
    cout << "ControlNetwork " << id << ": slot " << slot << " mfec " << mfec << " channel " << channel;
    cout<<hex<< "  status=" << status<<dec<<endl;
  }
  if( channel== 1){
    if(verbose){
      if( (status & 0x0100) == 0x0100 )  cout<<" ch1 receive timeout "<<endl;
      if( (status & 0x0200) == 0x0200 )  cout<<" ch1 receive complete "<<endl;
      if( (status & 0x0400) == 0x0400 )  cout<<" ch1 hub address error "<<endl;
      if( (status & 0x0800) == 0x0800 )  cout<<" ch1 send started (not finished!) "<<endl;
      if( (status & 0x1000) == 0x1000 )  cout<<" ch1 receive count error "<<endl;
      if( (status & 0x2000) == 0x2000 )  cout<<" ch1 receive error "<<endl;
    }
    if( (status & 0x2000) == 0x2000 )  return 1;
    return 0;
  }else if (channel==2){
    if(verbose){
      if( (status & 0x01000000) == 0x01000000 )  cout<<" ch2 receive timeout "<<endl;
      if( (status & 0x02000000) == 0x02000000 )  cout<<" ch2 receive complete "<<endl;
      if( (status & 0x04000000) == 0x04000000 )  cout<<" ch2 hub address error "<<endl;
      if( (status & 0x08000000) == 0x08000000 )  cout<<" ch2 send started (not finished!) "<<endl;
      if( (status & 0x10000000) == 0x10000000 )  cout<<" ch2 receive count error "<<endl;
      if( (status & 0x20000000) == 0x20000000 )  cout<<" ch2 receive error "<<endl;
      
    }
    if( (status & 0x20000000) == 0x20000000 )  return 1;
    return 0;
  }else{
    cout << "illegal channel " << channel << "  in mfec " << mfec << endl;
    return 0;
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////
