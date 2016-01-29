#ifndef _DiagWrapper_h_
#define _DiagWrapper_h_

#include <string>

class DiagWrapper
{
  public:
    DiagWrapper();
    
    void reportError(std::string msg, int level);
    
    
};

#endif
