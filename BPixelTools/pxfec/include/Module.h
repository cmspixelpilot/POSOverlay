#ifndef MODULE
#define MODULE

class ControlNetwork;
class SysCommand;
class TBM;
class Roc;


class Module{
 private:
  ControlNetwork *cn;
  int hubaddress;
  TBM* tbm;
  int nrocs;
  Roc* roc[16];
  //Roc* roc[32];
  
 public:
  Module(const int aNRocs, const int aHubId, ControlNetwork *aCN);
  void Execute(SysCommand *command);

};
#endif
