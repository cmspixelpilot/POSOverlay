#ifndef SIMPLECOMMAND
#define SIMPLECOMMAND


/*************************************************************
 *
 * 
 *
 *
 *************************************************************/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "BaseCommand.h"

using namespace std;

struct sector_t{
  int phi;  // +/-1 .. +/-8
  char z;   // P or N
};

struct group_t{
  int phi;  // +/-1 .. +/-8
  char z;   // P or N
  int layer;  // [1,3]  1=1&2, 3=3
};

struct shell_t{
  char z; // P or N
  char x; // + or -
  bool operator==(const shell_t&  s1) const{ return (s1.z==z)&&(s1.x==x);}
  bool operator<(const shell_t& s1) const{ return (s1.z<z) || ((s1.z==z)&&(s1.x<x));}
};

class SimpleCommand:public BaseCommand {



public:

  enum {kNone, kInt, kFloat, kString, kSector, kGroup, kShell};

  int verbose;
  
  // methods
  SimpleCommand();
  ~SimpleCommand(){};
  int Parse(const string& cmd);
  int Next();
  int Read(const string& fileName);
  //char* TargetPrompt(int mode, const char* sep);
  // In SimpleCommand this is just a convenient way to pass the prompt between server and application
  // the command doesn't really touch the prompt string
  void setPrompt(const char* s){prompt=s;};
  void setPrompt(const string& s){prompt=s;};
  std::string getPrompt(){return prompt;};
  
  bool Exit();
  bool Keyword(const string&  keyword);
  bool Keyword(const string&  keyword, int& value);
  bool Keyword(const string&, float& value);
  bool Keyword(const string&  keyword, const string& keyword2);
  bool Keyword(const string&  keyword, const string& keyword2, int& value);
  int Keyword(const string& keyword1, const vector<string>& keyword2);
  int Keyword(const string& keyword1, const vector<string>& keyword2, int& value);
  bool Keyword(const string&, int& value1, int& value2);
  bool Keyword(const string&, int& value1, int& value2, int& value3);
  bool Keyword(const string&, int& value1, int& value2, int& value3, int& value4, int& value5, int& value6);
  bool Keyword(const string&, int& value1, int& value2, const string&);

  bool Keyword(const string&  keyword, float& value1, float& value2);

  bool Keyword(const string&, sector_t& sect);
  bool Keyword(const string&, sector_t& sect, const string&, int& lay);
  bool Keyword(const string&, int& value1, int& value2, sector_t& sect);
  bool Keyword(const string&, group_t& sect);
  bool Keyword(const string&, group_t& sect, int& value1, int& value2, int& value3, int& value4);
  bool Keyword(const string&, group_t& sect, int& value1, int& value2, int& value3, int& value4, int& value5);
  bool Keyword(const string&, const string&, int& value1, int& value2, int& value3, int& value4);
  bool Keyword(const string&, const string&, int& value1, int& value2, int& value3, int& value4, int& value5);

  bool Keyword(const string&, shell_t& , int& value);
  bool Keyword(const string&, shell_t& , const string& keyword2);

  string buffer;
  string getBuffer(){return buffer;};

  // a non-member function. must be a friend in order to have access
  friend std::ostream& operator << (std::ostream &os, const SimpleCommand& c);

 private:

  struct item{
    string token;
    float f;
    int i;
    sector_t sector;
    group_t group;
    shell_t shell;
    int type;
  };
  vector<item> tokens;  // global // visible part

  // command stack
  vector<vector<item> > cmdStack;
  int Pop();

  // file stack
  int nOpen;
  static const int nFileMax=3;
  ifstream* fileStack[nFileMax];
  ifstream* inputFile;
  int Getline(char* line, int n);

  bool exitFlag;
  const char* GetWord(const char *s, int* l);
  int StrToI(const char* word, const int len, int* v);
  int StrToF(const char* word, const int len, float* v);
  //bool ParseNumberOrAlias(const char* word, const int &l, int *id);
  int PrintList(char* buf, int n, int v[]);
  string CharToStr(const char* word, const int len);
  //int PrintTarget(char* buf, target_t* t, int mode);
  //int getCNidx(const std::string& s);
  //std::string getCNname(const int idx);

  std::string prompt;
};


#endif
