//
// Author: Wolfram Erdmann
//
// Class provides functionalities to run commands from a file or the command line
// 


#ifndef SYSCOMMAND
#define SYSCOMMAND


/*************************************************************
 *
 * 
 *
 *
 *************************************************************/

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//#ifdef DERIVE_FROM_BASE
#include "BPixelTools/tools/include/BaseCommand.h"

class SysCommand:public BaseCommand {
//#else
//class SysCommand{
//#endif


 public:
  
  enum {kNONE, kTB, kTBM, kROC, kSYS, kCN, kMOD};
  static const int nModuleMax=32;
  static const int nRocMax=16;
  static const int nCNMax=64;
  static const int nArgMax=10;


  // visible part of a single instruction
  int type;                 // what kind of target kTB,kTBM,kROC
  int CN;                   // control network
  int module;               // module id
  int roc;                  // roc id (when type=kROC)
  int narg;                 // number of arguments
  char* carg[nArgMax]    ;  // keyword or NULL
  int* iarg[nArgMax];       // integer list
  int verbose;

  // methods
  SysCommand();

  //#ifdef DERIVE_FROM_BASE
  int Parse(const std::string& s){return Parse(s.c_str());}
  bool Keyword(const string& keyword){return Keyword(keyword.c_str());}
  //#endif

  int Next();
  bool Exit();
  int Parse(const char* line);
  int Read(const char* fileName);
  void Print();
  char* TargetPrompt(int mode, const char* sep);
  int GetTargetRoc(int *pModule, int *pRoc);
  char* toString();
  void RocsDone();
  bool IncludesRoc(int rocID);
  
  bool TargetIsTB();
  bool TargetIsTBM();
  bool TargetIsROC();
  bool Keyword(const char* keyword);
  bool Keyword(const char* keyword, int** value);
  bool Keyword(const char* keyword, int** value1, int** value2);
  bool Keyword(const char* keyword, int** value1, int** value2, int** value3);
  bool Keyword(const char* keyword, int** value1, int** value2, const char* keyword1);
  bool Keyword(const char* keyword, const char* keyword1);
  bool Keyword(const char* keyword, const char* keyword1, int** value);
 private:

  static const int ibufsize=1000;
  int ibuf[ibufsize]; // holds all integers 
  static const int cbufsize=200; 
  char cbuf[cbufsize]; //holds keywords + terminating '\0's

  struct target_t{
    int type;       // kTB,kTBM,kROC
    int nCN;        // number of control networks
    int CN[nCNMax]; // control network id (0,1,2,...)
    int nModule;    // number of target modules
    int hub[nModuleMax];  // list of hub ID's
    int nRoc;       // length of target list
    int id[16];     // id's of targets (used for ROCs only)
  };
  target_t target, defaultTarget;

  int isIterated[nArgMax];
  int temp[nArgMax];
  int iCN;
  int iHub;
  int iRoc;

  // file stack
  int nOpen;
  static const int nFileMax=3;
  ifstream* fileStack[nFileMax];
  ifstream* inputFile;
  int Getline(char* line, int n);

  bool exitFlag;
  const char* GetWord(const char *s, int* l);
  bool ParseNumberOrAlias(const char* word, const int &l, int * id);
  int StrToI(const char* word, const int len, int* v);
  int PrintList(char* buf, int n, int v[]);
  int PrintTarget(char* buf, target_t* t, int mode);
  
 public:
  static int getCNidx(const std::string&);
  static std::string getCNname(const int);
};



#endif
