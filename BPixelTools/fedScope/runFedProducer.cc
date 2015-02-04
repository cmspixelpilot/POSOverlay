#include <iostream>

#include "FedProducer.hh"

#include "TROOT.h"
#include "TApplication.h"

using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char **argv) {

  int readfile(0);
  char filename[200];
  // -- command line arguments
  for (int i = 0; i < argc; i++)  {
    if (!strcmp(argv[i],"-f")) {
      readfile = 1; 
      sprintf(filename, argv[++i]); 
    }
  }

  TApplication theApp("App", &argc, argv, 0, -1);
  FedProducer a;
  if (readfile) {
    std::cout << "reading from file " << filename << std::endl;
    a.readFile(filename);
  } else {
    a.randomSignal();
  }
  
}
