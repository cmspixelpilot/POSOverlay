#ifndef BASECOMMAND
#define BASECOMMAND


/*************************************************************
 *
 * 
 *
 *
 *************************************************************/
#include <string>

class BaseCommand {
public:
  virtual int Parse(const std::string& cmd)=0;
  virtual int Next()=0;
  virtual bool Exit()=0;
  virtual bool Keyword(const std::string&  keyword)=0;
  virtual ~BaseCommand(){};
};


#endif
