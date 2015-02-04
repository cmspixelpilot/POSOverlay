#ifndef ROC
#define ROC

#include "ControlNetwork.h"
#include "SysCommand.h"

class Roc{

  int iroc;
  int portaddress;
  int hubaddress;
  ControlNetwork * cn;

public:
  Roc(const int aChipId, const int aHubId, const int aPortId, ControlNetwork * aCN);
  void Execute(SysCommand* command);
  void Init();
  void setDAC(const int dac, const int value);
};


#endif
