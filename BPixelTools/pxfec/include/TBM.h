#ifndef TBMM
#define TBMM

class ControlNetwork;
#include "SysCommand.h"

class TBM
{
 public:
  static const int tbmA = 0xE; //  TBM-A 
  static const int tbmB = 0xF; //  TBM-B 

 private:
  int hubaddress;
  ControlNetwork * cn;
  int tbmAReg[8];
  int tbmBReg[8];

 public:
  TBM(const int aHubId, ControlNetwork *aCN);
  void Execute(SysCommand * cmd);
  void Init();
  int replace(const int oldValue, const int change, const int mask)const;
  int setTBM(const int tbmChannel, const int tbmRegister, const int value, const int mask=0);
  void setTBMs(const int tbmRegister, const int value, const int mask=0);
  void setTBMDAC(const int DACAddress, const int value);
  void setDelayTBMa(const int DACAddress, const int value);
  void setDelayTBMb(const int DACAddress, const int value);
  void setPLLDelayTBM(const int DACAddress, const int value);
  void setPKAMCountTBM(const int DACAddress, const int value);
  void setAutoResetTBM(const int DACAddress, const int value);
  int readTBM(const int tbmChannel, const int tbmRegister, int& value);
  int readStack(const int tbmChannel,const bool explain=false);
  int dump(const int tbmChannel);
};
#endif


