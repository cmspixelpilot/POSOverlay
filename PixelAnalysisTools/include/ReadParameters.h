#ifndef _READPARAMETERS
#define _READPARAMETERS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


using namespace std;


class ReadParameters {

 public:
  
  ReadParameters (const char* InputFileName);

  ~ReadParameters () {;};

  void ReadFromFile (vector<string>* ParVector);


 private:

  ifstream in;

};

#endif
