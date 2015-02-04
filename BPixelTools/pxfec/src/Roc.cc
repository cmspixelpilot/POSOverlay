#include "PixelFECInterface/include/PixelFECInterface.h"
#include "ControlNetwork.h"
#include "Roc.h"

Roc::Roc(const int aChipId, const int aHubId, const int aPortId,ControlNetwork *aCN){

  iroc=aChipId;
  hubaddress=aHubId;
  portaddress=aPortId;
  cn=aCN;
}


void Roc::Init(){

  // Clear cal
  cn->interface->clrcal(cn->mfec, cn->channel, hubaddress, portaddress, iroc);
  
  cout << "masking " << iroc << endl;
  // Mask all
  int trim = 15; // set all trims to 15 i.e. turn them off
  int mask = 0; // mask (disable) all pixels
  for(int idcol=0;idcol<26;idcol++) {  // Disable all dcols
    cn->interface->dcolenable(cn->mfec, cn->channel, hubaddress, portaddress, iroc, idcol,mask);//disable
  }
  
  // Disable all pixels
  cn->interface->rocinit(cn->mfec, cn->channel, hubaddress, portaddress, iroc, mask, trim);
  cout<<endl;
}


void Roc::setDAC(const int dac, const int value){
  if(dac>0){
    if(portaddress<4){
      cn->interface->progdac(cn->mfec, cn->channel, hubaddress, portaddress, iroc, dac, value); 
    }else{
      cn->interface->progdac(cn->mfec, cn->channel, hubaddress, 0, iroc, dac, value); 
      cn->interface->progdac(cn->mfec, cn->channel, hubaddress, 1, iroc, dac, value); 
      cn->interface->progdac(cn->mfec, cn->channel, hubaddress, 2, iroc, dac, value); 
      cn->interface->progdac(cn->mfec, cn->channel, hubaddress, 3, iroc, dac, value); 
    }
  }
}    



void Roc::Execute(SysCommand *command){
  if(command->type==SysCommand::kROC){
    //cout << "executing roc command " << iroc << " " << hubaddress <<  endl;

    if(command->narg==0) return;
    int *value1, *value2;
    static int trimBits=15;
  
    if(command->Keyword("mask")){
      cout << "masking roc " << iroc << "  on hub " << hubaddress << endl;
      cn->interface->rocinit(cn->mfec, cn->channel, hubaddress, portaddress, iroc, 0, trimBits); //mask,trim
    }else if(command->Keyword("unmask")){
      cout << "unmasking roc " << iroc << "  on hub " << hubaddress << endl;
      cn->interface->rocinit(cn->mfec, cn->channel, hubaddress, portaddress, iroc, 1, trimBits); //mask,trim
    }else if(command->Keyword("trim",&value1)){
      trimBits=*value1;
      cn->interface->rocinit(cn->mfec, cn->channel, hubaddress, portaddress, iroc, 0, *value1); //mask,trim
    }else if(command->Keyword("clrcal")||command->Keyword("cald")){
      cn->interface->clrcal(cn->mfec, cn->channel, hubaddress, portaddress, iroc);
    }else if(command->Keyword("fullspeed")){     setDAC(0xFD,  0);
    }else if(command->Keyword("halfspeed")){     setDAC(0xFD,  1);
    }else if(command->Keyword("arm", &value1, &value2)){
      for(int * j=value1; (*j)>=0; j++){
	for(int * k=value2; (*k)>=0; k++){
	  cout << "roc # = "  << iroc << endl;
	  cout << "value1 = "  << *j << " value2 = " << *k << endl;
	  // enable dcol
	  cn->interface->dcolenable(cn->mfec, cn->channel, hubaddress, portaddress, iroc, int((*j)/2), 1);//enable
	  // Enable only 1 pixel
	  cn->interface->progpix1(cn->mfec, cn->channel, hubaddress, portaddress, iroc, *j, *k, 1, trimBits); //mask,trim
	  // Cal enable for one pixel
	  cn->interface->calpix( cn->mfec, cn->channel, hubaddress, portaddress, iroc, *j, *k, 1); // cal
	}
      } 
    }else if(command->Keyword("pixe", &value1, &value2)){
      for(int * j=value1; (*j)>=0; j++){
	for(int * k=value2; (*k)>=0; k++){
	  // enable dcol
	  cn->interface->dcolenable(cn->mfec, cn->channel, hubaddress, portaddress, iroc, int((*j)/2), 1);//enable
	  // Enable only 1 pixel
	  cn->interface->progpix1(cn->mfec, cn->channel, hubaddress, portaddress, iroc, *j, *k, 1, 0); //mask,trim
	}
      } 
    }else if(command->Keyword("pixd", &value1, &value2)){
      for(int * j=value1; (*j)>=0; j++){
	for(int * k=value2; (*k)>=0; k++){
	  // disable 1 pixel
	  cn->interface->progpix1(cn->mfec, cn->channel, hubaddress, portaddress, iroc, *j, *k, 0, 0); //mask,trim
	}
      } 
    }else if(command->Keyword("cole", &value1)){
      for(int * j=value1; (*j)>=0; j++){
	cn->interface->dcolenable(cn->mfec, cn->channel, hubaddress, portaddress, iroc, int((*j)/2), 1);//enable
      } 
    }else if(command->Keyword("cold", &value1)){
      for(int * j=value1; (*j)>=0; j++){
	cn->interface->dcolenable(cn->mfec, cn->channel, hubaddress, portaddress, iroc, int((*j)/2), 0);//enable
      } 
    }
    else if(command->Keyword("port",      &value1)){ portaddress=*value1; }
    else if(command->Keyword("port")){ cout << portaddress << endl; }
    else if(command->Keyword("Vdig",      &value1)){ setDAC(0x01, *value1); }
    else if(command->Keyword("Vana",      &value1)){ setDAC(0x02, *value1); }
    else if(command->Keyword("Vsh",       &value1)){ setDAC(0x03, *value1); }
    else if(command->Keyword("Vcomp",     &value1)){ setDAC(0x04, *value1); }
    else if(command->Keyword("Vleak_comp",&value1)){ setDAC(0x05, *value1); }
    else if(command->Keyword("VrgPr",     &value1)){ setDAC(0x06, *value1); }
    else if(command->Keyword("VwllPr",    &value1)){ setDAC(0x07, *value1); }
    else if(command->Keyword("VrgSh",     &value1)){ setDAC(0x08, *value1); }
    else if(command->Keyword("VwllSh",    &value1)){ setDAC(0x09, *value1); }
    else if(command->Keyword("VhldDel",   &value1)){ setDAC(0x0A, *value1); }
    else if(command->Keyword("Vtrim",     &value1)){ setDAC(0x0B, *value1); }
    else if(command->Keyword("VthrComp",  &value1)){ setDAC(0x0C, *value1); }
    else if(command->Keyword("VIBias_Bus",&value1)){ setDAC(0x0D, *value1); }
    else if(command->Keyword("Vbias_sf",  &value1)){ setDAC(0x0E, *value1); }
    else if(command->Keyword("VoffsetOp", &value1)){ setDAC(0x0F, *value1); }
    else if(command->Keyword("VIbiasOp",  &value1)){ setDAC(0x10, *value1); }
    else if(command->Keyword("PHOffset",  &value1)){ setDAC(0x11, *value1); }
    else if(command->Keyword("VIon",      &value1)){ setDAC(0x12, *value1); }
    else if(command->Keyword("Vcomp_ADC", &value1)){ setDAC(0x13, *value1); }
    else if(command->Keyword("PHScale",   &value1)){ setDAC(0x14, *value1); }
    else if(command->Keyword("VIbias_roc",&value1)){ setDAC(0x15, *value1); }
    else if(command->Keyword("VIColOr",   &value1)){ setDAC(0x16, *value1); }
    else if(command->Keyword("Vnpix",     &value1)){ setDAC(0x17, *value1); }
    else if(command->Keyword("VSumCol",   &value1)){ setDAC(0x18, *value1); }
    else if(command->Keyword("Vcal",      &value1)){ setDAC(0x19, *value1); }
    else if(command->Keyword("CalDel",    &value1)){ setDAC(0x1A, *value1); }
    else if(command->Keyword("RangeTemp", &value1)){ setDAC(0x1B, *value1); }
    else if(command->Keyword("WBC",       &value1)){ setDAC(0xFE, *value1); }
    else if(command->Keyword("CtrlReg",   &value1)){ setDAC(0xFD, *value1); }
    else if(command->Keyword("ReadBack",   &value1)){ setDAC(0xFF, *value1); }
    else{
      cout << "unknown roc command " << endl;; //command.Print();
      //sock.flush();
    }
  }else{
    cout << "how did we get here?" << endl;
  }
};
