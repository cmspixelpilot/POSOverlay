#include <map>
#include <string>
#include <iostream>
#include <iomanip>

#include <unistd.h>


#include "PowerSupply.h"
#include "PS_Channel.h"
#include "CAENHVWrapper.h"

#include "SimpleCommand.h"
//#include "SimpleServer.h"
#include "MultiplexingServer.h"

using namespace std;

//--------------------------------------------------------
PowerSupply* Sys;
PS_Channel* ps_channel[500]={0}; // 
int ps_numch=0;
PS_Slot * sl;
int defaultSlot=5;
// group+voltage to channel mapping, to be defined by "channel" commands
std::map<int, int> vdChannel;
std::map<int, int> vaChannel;
std::map<int, int> hv0Channel;
std::map<int, int> hv1Channel;
std::map<int, int> optoChannel;
std::map<shell_t, int> ccuChannel;
//--------------------------------------------------------





string getStatus(PS_Parameter* p){
 stringstream str;
//      if (p->ch_On()) {
// 	str << " ON ";
//       }
      if (p->ch_RampingUp()) {
	str << " RAMPING UP ";
      } 
      if (p->ch_RampingDown()) {
	str << " RAMPING DOWN ";
      }

      if (p->ch_OverCurrent()) {
	str << " OVER CURRENT ";
      }
      if (p->ch_OverVoltage()) {
	str << " OVER VOLTAGE ";
      }
      if (p->ch_UnderVoltage()) {
	str << " UNDER VOLTAGE ";
      }
      if (p->ch_MaxV()) {
	str << " MAX V ";
      }
      if ( p->ch_ExternalDisable()) { 
	str << " EXTERNAL DISABLE ";
      }    
      if ( p->ch_InternalTrip() ) { 
	str << " INTERNAL TRIP";
      } 
      if ( p->ch_CalibrationError()) {
	str << " CALIBRATION ERROR ";
      }
      if ( p->ch_Unplugged()) {
      	str << " UNPLUGGED ";
      }

      if (p->ch_OverVoltageProtection()) {
	str << " OVV PROTECTION ";
      }

      if (p->ch_OverTemperature()) {
	str << " OVER TEMPERATURE " ;
      }
      return str.str();
}

void setSCP( PowerSupply* Sys, const int slot, const int channel, const int parameter,const bool value){ 
  if ((channel>=0)&&(channel<500)){  //ps_numch
    Sys->fillChannel(*ps_channel[channel]);
    PS_Parameter* par=ps_channel[channel]->parameter(parameter);
    par->setBoolValue(value);
    Sys -> writeParam (*(ps_channel[channel]), par);
  }else{
    cout << "invalid channel " << channel << endl;
  }
};

string getSCP( PowerSupply* Sys, const int slot, const int channel, const int parameter){ 
  Sys->fillChannel(*ps_channel[channel]);
  string value(ps_channel[channel]->parameter(parameter)->stringValue());
  return value;
};

string printSCP( PowerSupply* Sys, const int slot, const int channel, const int parameter){ 
  Sys->fillChannel(*ps_channel[channel]);
  if (parameter<ps_channel[channel]->num_params()){
    string value(ps_channel[channel]->parameter(parameter)->toString());
    return value;
  }else{
    cout << "no such parameter" << endl;
    return "";
  }
};

string printSC( PowerSupply* Sys, const int slot, const int channel){ 
   Sys->fillChannel(*ps_channel[channel]);
   return ps_channel[channel]->toStringShort();
};


void powerA4602Channel( PowerSupply* Sys, const int slot, int channel, bool on){
  if(channel>0){
    if (on){
      cout << "switching on channel " << channel << "  [" << ps_channel[channel]->name() << "]" << endl;
      setSCP(Sys, slot, channel, 15,true); // enable 
      setSCP(Sys, slot, channel,  2,true); // power
    }else{
      cout << "switching off channel " << channel << "  [" << ps_channel[channel]->name() << "]" << endl;
      setSCP(Sys, slot, channel,  2,false);
    }
  }else{
    cout << "ccu channel undefined" << endl;
  }
}


//-----------------------------------------------------------
// control network name to index mapping
// group_t is defined in SimpleCommand.h, the index is a number in [0..63]
int getIndex(const group_t g){
  int idx=abs(g.phi)-1;
  if (g.phi<0)    idx+=16;
  if (g.z=='N')   idx+=8;
  if (g.layer==3) idx+=32;
  return idx;
}

shell_t getShellFromIndex(const int idx){
  shell_t s;
  if (idx &  8){  s.z='N';  }else{ s.z='P'; }
  if (idx & 16){  s.x='-';  }else{ s.x='+'; }
  return s;
}

bool invalidIndex(const int i){
  if((i<0)||(i>63)){
    cerr << "invalid group" << endl;
    return true;
  }
  return false;
}


void setChannelRange(int g1, int g2, int vdch, int vach, int hv0ch, int hv1ch,int optoch  ){
  for(int groupIdx=g1; groupIdx<g2; groupIdx++){
    if (vach>0)  {vaChannel[groupIdx]  =vach;}
    if (vdch>0)  {vdChannel[groupIdx]  =vdch;}
    if (hv0ch>0) {hv0Channel[groupIdx] =hv0ch;}
    if (hv1ch>0) {hv1Channel[groupIdx] =hv1ch;}
    if (optoch>0){optoChannel[groupIdx & 31]=optoch;}
  }
}

/* this function handles client requests passed on to it as a string and
  and sends a response to cout, which should have been redirected */

void exec(SimpleCommand* c){

  // group+voltage to channel mapping, to be defined by "channel" commands
//   static std::map<int, int> vdChannel;
//   static std::map<int, int> vaChannel;
//   static std::map<int, int> hv0Channel;
//   static std::map<int, int> hv1Channel;
//   static std::map<int, int> optoChannel;
//   static std::map<shell_t, int> ccuChannel;

  int slot,channel,parameter;
  static  int defaultGroup=-1;
  int vdch, vach, hv0ch, hv1ch, optoch;
  group_t group;
  shell_t shell;

  if(c->Keyword("map")){
    ps_numch=sl->numCh();
    for(int ch=0; ch<ps_numch; ch++){
      Sys->fillChannel(*ps_channel[ch]);

      string pw="";
      try{
 	pw=ps_channel[ch]->searchParameter("Pw")->stringValue();
	if (pw=="On"){
	  //pw="\033[30;41mOn\033[0m"; // black on red
	  pw="\033[31;49mOn\033[0m"; // red on white
	}
      }catch(runtime_error){
	pw="-";
      }

       string status="";
       try{
	 status=getStatus(ps_channel[ch]->searchParameter("Status"));
       }catch(runtime_error){
 	status="";
       }

      cout << setw(2) << ch << ") " << setw(10) << ps_channel[ch]->name() << " " << pw << " " << status << endl;
    }

  }else if(c->Keyword("off")){
    int numch=sl->numCh();
    for(int ch=0; ch<numch; ch++){
      Sys->fillChannel(*ps_channel[ch]);
      try{
 	PS_Parameter* par=ps_channel[ch]->searchParameter("Pw");
	if (par->boolValue()){
	  cout << "switching off " << ch << endl;
	  par->setBoolValue(false);
	  Sys -> writeParam (*(ps_channel[ch]), par);
	}
      }catch(runtime_error){
	cout << "runtime_error caught in off !!" << ch << endl;
      }
    }
    
  }else if(c->Keyword("channels",group, vdch, vach, hv0ch, hv1ch)){
    int groupIdx=getIndex(group);
    vaChannel[groupIdx]=vach;
    vdChannel[groupIdx]=vdch;
    hv0Channel[groupIdx]=hv0ch;
    hv1Channel[groupIdx]=hv1ch;
    optoChannel[groupIdx & 31]=-1;

  }else if(c->Keyword("channels",group, vdch, vach, hv0ch, hv1ch, optoch)){
    int groupIdx=getIndex(group);
    vaChannel[groupIdx]=vach;
    vdChannel[groupIdx]=vdch;
    hv0Channel[groupIdx]=hv0ch;
    hv1Channel[groupIdx]=hv1ch;
    optoChannel[groupIdx & 31]=optoch;

  }else if(c->Keyword("channels","*", vdch, vach, hv0ch, hv1ch, optoch)){
    setChannelRange(0,64,vdch, vach, hv0ch, hv1ch, optoch);

  }else if(c->Keyword("channels","*", vdch, vach, hv0ch, hv1ch)){
    setChannelRange(0,64,vdch, vach, hv0ch, hv1ch, -1);

  }else if(c->Keyword("channels","*L12", vdch, vach, hv0ch, hv1ch, optoch)){
    setChannelRange(0,32,vdch, vach, hv0ch, hv1ch, optoch);

  }else if(c->Keyword("channels","*L3", vdch, vach, hv0ch, hv1ch, optoch)){
    setChannelRange(32,64,vdch, vach, hv0ch, hv1ch, optoch);

  }else if(c->Keyword("channels")){
    for(int idx=0; idx<64; idx++){
      if (idx & 16){  cout << '-';  }else{ cout<<'+'; }
      cout <<(idx&7)+1; 
      if (idx &  8){  cout <<'N';  }else{ cout << 'P'; }
      if (idx<32){ cout << "L12";}else{cout <<"L3 ";}
      cout << setw(4) << vdChannel[idx] << setw(4)  << vaChannel[idx] << setw(4)  << hv0Channel[idx] << setw(4)  << hv1Channel[idx] << setw(4)  << optoChannel[idx] << endl;
    }

  }else if(c->Keyword("ccu",shell, optoch)){
    ccuChannel[shell]=optoch;

  }else if(c->Keyword("group",group)){
    defaultGroup=getIndex(group);
    cout << "default group is now "<< *c << endl;
    string s=c->getBuffer();
    c->setPrompt(s.substr(6,s.length())+">");

  }else if(c->Keyword("ccu","on")){
    // infer shell from default group
    if (invalidIndex(defaultGroup)) return;
    channel=ccuChannel[ getShellFromIndex(defaultGroup) ];
    powerA4602Channel(Sys, defaultSlot, channel, true);

  }else if(c->Keyword("ccu","off")){
    if (invalidIndex(defaultGroup)) return;
    channel=ccuChannel[ getShellFromIndex(defaultGroup) ];
    powerA4602Channel(Sys, defaultSlot, channel, false);

  }else if(c->Keyword("ccu",shell, "on")){
    channel=ccuChannel[shell];
    powerA4602Channel(Sys, defaultSlot,channel, true);

  }else if(c->Keyword("ccu",shell, "off")){
    channel=ccuChannel[shell];
    powerA4602Channel(Sys, defaultSlot,channel, false);

  }else if(c->Keyword("poff")){
    if (invalidIndex(defaultGroup)) return;
    setSCP(Sys,defaultSlot,vdChannel[defaultGroup],1,false);
    //setSCP(Sys,defaultSlot,vaChannel[defaultGroup],1,false);

  }else if(c->Keyword("pon")){
    if (invalidIndex(defaultGroup)) return;
    //setSCP(Sys,defaultSlot,vaChannel[defaultGroup],1,true);
    setSCP(Sys,defaultSlot,vdChannel[defaultGroup],1,true);

  }else if(c->Keyword("hv","off")){
    if (invalidIndex(defaultGroup)) return;
    setSCP(Sys,defaultSlot,hv0Channel[defaultGroup],1,false);
    setSCP(Sys,defaultSlot,hv1Channel[defaultGroup],1,false);

  }else if(c->Keyword("hv","on")){
    if (invalidIndex(defaultGroup)) return;
    setSCP(Sys,defaultSlot,hv0Channel[defaultGroup],1,true);
    setSCP(Sys,defaultSlot,hv1Channel[defaultGroup],1,true);

  }else if(c->Keyword("get","id")){
    if (invalidIndex(defaultGroup)) return;
    //cout << defaultGroup <<  " " << vdChannel[defaultGroup] << endl;
    cout << getSCP(Sys,defaultSlot,vdChannel[defaultGroup],8) << endl;

  }else if(c->Keyword("get","ia")){
    if (invalidIndex(defaultGroup)) return;
    cout << getSCP(Sys,defaultSlot,vaChannel[defaultGroup],1) << endl;

  }else if(c->Keyword("get","vd")){
    if (invalidIndex(defaultGroup)) return;
    cout << getSCP(Sys,defaultSlot,vdChannel[defaultGroup],3) << endl;

  }else if(c->Keyword("get","va")){
    if (invalidIndex(defaultGroup)) return;
    cout << getSCP(Sys,defaultSlot,vaChannel[defaultGroup],7) << endl;

  }else if(c->Keyword("get","pw")){
    if (invalidIndex(defaultGroup)) return;
    cout << getSCP(Sys,defaultSlot,vdChannel[defaultGroup],1) << endl;

  }else if(c->Keyword("get",slot,channel,parameter)){
    cout << getSCP(Sys,slot,channel,parameter) << endl;

  }else if(c->Keyword("printSCP",slot,channel,parameter)){
    cout << printSCP(Sys,slot,channel,parameter) << endl;

  }else if(c->Keyword("print",channel,parameter)){
    cout << printSCP(Sys,defaultSlot, channel,parameter) << endl;

  }else if(c->Keyword("print",channel)){
    cout << setw(2) << channel << ") " << ps_channel[channel]->name() << endl;
    cout << printSC(Sys,defaultSlot,channel) << endl;
    
  }else if(c->Keyword("print")){
    int numch=sl->numCh();
    for(int ch=0; ch<numch; ch++){
      PS_Channel* psc=ps_channel[ch];
      cout << psc->toStringShort() << endl;
    }

  }else if(c->Keyword("mb",shell, "on")){
    for(map<int,int>::iterator iter = optoChannel.begin(); iter != optoChannel.end(); iter++ ) {
      if ( (iter->second>-1) && (shell==getShellFromIndex(iter->first)) ) {
	channel=iter->second;
	powerA4602Channel(Sys, defaultSlot, channel, true);
      }
	  }

  }else if(c->Keyword("mb", shell, "off")){
    for( map<int,int>::iterator iter = optoChannel.begin(); iter != optoChannel.end(); iter++ ) {
      if ( (iter->second>-1) && (shell==getShellFromIndex(iter->first)) ) {
	channel=iter->second;
	powerA4602Channel(Sys, defaultSlot, channel, false);
      }
    }
 
  }else if(c->Keyword("mb", "on")){
    if (invalidIndex(defaultGroup)) return;
    channel=optoChannel[defaultGroup & 31];
    powerA4602Channel(Sys, defaultSlot, channel, true);

  }else if(c->Keyword("mb", "off")){
    if (invalidIndex(defaultGroup)) return;
    channel=optoChannel[defaultGroup % 31];
    powerA4602Channel(Sys, defaultSlot, channel, false);
 
  }else if(c->Keyword("get","imb")){
    if (invalidIndex(defaultGroup)) return;
    channel=optoChannel[defaultGroup % 31];
    if(channel>0){
      cout << getSCP(Sys,defaultSlot, channel,12) << endl;
    }else{
      cout << 0 << endl; // this is just a dummy response
    }

  }else if(c->Keyword("get","iccu")){
    if (invalidIndex(defaultGroup)) return;
    channel=ccuChannel[ getShellFromIndex(defaultGroup) ];
    if(channel>0){
      cout << getSCP(Sys,defaultSlot, channel,12) << endl;
    }else{
      cout << 0 << endl; // this is just a dummy response
    }

  }else{
    cerr << "what? [" << *c << "]" << endl;
  } 
}



int main(int argc, char *argv[])
{
  
  string file="config.ini";  // init file, (should define the channel maping)

  //look at command line options
  bool bg=false;
  int port=0;
  for(int i=1; i<argc; i++){
    if(strcmp(argv[i],"-bg")==0){ bg=true;}
    else if(strcmp(argv[i],"-port")==0){
      if((i+1)<argc){
        port=atoi(argv[i+1]);
        i++;
      }
    }else if(strcmp(argv[i],"-slot")==0){
      if((i+1)<argc){
        defaultSlot=atoi(argv[i+1]);
	file="config"+((string) argv[i+1])+".ini";  // init file
        i++;
      }
    }else{
      cout << "usage: server [-bg] [-port <port>] [-slot <slot>]" << endl;
      exit(1);
    }
  }

  
  // init Caen stuff
  //Sys=initPS();
  const char SystemName[60] = "System0";
  //const char IPAddress[60]  = "129.129.202.35";
  const char IPAddress[60]  = "10.176.11.55";
  const char UserName[60]   = "admin";
  const char PassWd[60]     = "admin";
  
  cout << "System Name: "<< SystemName << endl;
  cout << "IP Address: " << IPAddress  << endl;
  cout << "User Name: "  << UserName   << endl;
  cout << "Password: "   << PassWd     << endl;
  cout << "==========================================\n";
  
  // Stelle Verbindung zum System via TCP/IP her
  Sys = new PowerSupply(SystemName, IPAddress, UserName, PassWd);
  PS_CrateMap* SysCrateMap = Sys->getCrateMap();
  Sys->fillCrateSlots(*SysCrateMap);
  // done

  sl=Sys->getSlot(defaultSlot);
  int numch=sl->numCh();
  cout << "slot "<< defaultSlot <<" has " << numch << " channels " << endl;
  for(unsigned short channel=0; channel<numch; channel++){
      ps_channel[channel]=Sys->getChannel(defaultSlot, channel);
      cout << setw(2) << channel << ") " << ps_channel[channel]->name() << endl;  
  }
  

  SimpleCommand* cmd=new SimpleCommand();
  if(cmd->Read(file.c_str())==0){
    do{ exec(cmd); } while (cmd->Next());
  }


  MultiplexingServer serv(cmd, bg);
  if(port){ serv.open(port); }

  serv.setPrompt(cmd->getPrompt());
  // go
  while(serv.eventloop() ){
    exec(cmd);
    serv.setPrompt(cmd->getPrompt());
  }

}


