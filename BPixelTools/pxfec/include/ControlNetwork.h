#ifndef CONTROLNETWORK
#define CONTROLNETWORK

#include "PixelFECInterface/include/PixelFECInterface.h"
#include <string>
#include "Module.h"

class ControlNetwork{
 public:
  PixelFECInterface *interface;
  int mfec;
  int channel;
  int slot;// just informational
  std::string id;
  Module* m[32];

 ControlNetwork(
	       PixelFECInterface *FECInterface, 
	       int fecSlot,
	       int mfecNumber, 
	       int mfecChannel,
	       std::string groupName
	       );

 void Execute(SysCommand *command);
 int ctrlStatus(bool verbose=true);
 int getByte(int byte);
 int testHubId(const int id);
 void scanHubIds(); 
//##################################
 int testHubId(const int id, int mode);
 void scanHubIds(int mode);
 //##################################
 void init();

 int analyzeFecCsr(const int status, const bool verbose=true);

};
#endif
