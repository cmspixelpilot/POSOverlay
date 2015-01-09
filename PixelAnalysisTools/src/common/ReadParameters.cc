#include "ReadParameters.h"
#include <stdlib.h>


using namespace std;


// *** Public ***

ReadParameters::ReadParameters(const char* InputFileName)
{
  in.open(InputFileName);

  if(!in.good())
    {
      cout << "[ReadParameters::ReadFromFile]\tError opening file : " << InputFileName << endl;
      exit (1);
    }
}

void ReadParameters::ReadFromFile (vector<string>* ParVector)
{
  int    n = 0;
  char   ReadRows[256];
  string stringa;

  while(in.good())
    {
      in.getline(ReadRows,255);
      stringa = ReadRows;
      if ((stringa.length() != 0) && (stringa[0] != '#'))
      ParVector->push_back(stringa);
      n++;
    }
}


// *** Private ***
