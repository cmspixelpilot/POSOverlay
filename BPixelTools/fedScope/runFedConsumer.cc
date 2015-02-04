#include "FedConsumer.hh"

#include "TROOT.h"
#include "TApplication.h"


// ----------------------------------------------------------------------
int main(int argc, char **argv) {
  char server[100]="localhost";
  if (argc>1){
    strncpy(server,argv[1],100);
  }
  TApplication theApp("App", &argc, argv, 0, -1);
  new FedConsumer(server);
  theApp.Run();
}
