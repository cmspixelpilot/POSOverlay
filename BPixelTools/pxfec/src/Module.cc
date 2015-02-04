#include "Module.h"
#include "TBM.h"
#include "Roc.h"
#include "PixelFECInterface/include/PixelFECInterface.h"


Module::Module(const int aNRocs, const int aHubId, ControlNetwork *aCN){
  hubaddress=aHubId;
  tbm=new TBM(aHubId,aCN);
  nrocs=aNRocs;
  cn=aCN;
  for(int i=0; i<nrocs; i++){
    roc[i]=new Roc(i, hubaddress, (int) (i/4), aCN);
  }
}


void Module::Execute(SysCommand *command){
  switch(command->type){
  case SysCommand::kMOD:    cout << "no module commands implemented" << endl; break;
  case SysCommand::kTBM:    tbm->Execute(command); break;
  case SysCommand::kROC:    roc[command->roc]->Execute(command); break;
  default:      cout << "Module::Execute> how did we get here ?" << endl;
  }
}
 
