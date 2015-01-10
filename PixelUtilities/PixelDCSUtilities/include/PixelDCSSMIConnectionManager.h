// $Id: PixelDCSSMIConnectionManager.h,v 1.1 2009/07/29 14:22:16 joshmt Exp $

/**************************************************************************
 * Class used for intersession management of connections                  *
 * to the SMI/PSX server. Used by:                                        *
 *   PixelDCSFSMInterface                                                 *
 *                                                                        *
 * Author: Joshua Thompson, Cornell			 	          *
 *                                                                        *
 * Last update: $Date: 2009/07/29 14:22:16 $ (UTC)                        *
 *          by: $Author: joshmt $                                        *
 **************************************************************************/

#ifndef _PixelDCSSMIConnectionManager_h_
#define _PixelDCSSMIConnectionManager_h_

#include <iostream>
#include <fstream>

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSMICommander.h"

class PixelDCSSMIConnectionManager
{
 public:
  PixelDCSSMIConnectionManager();
  ~PixelDCSSMIConnectionManager();

  void disconnectOldConnections( PixelDCSSMICommander* smiCommander );

  void addOpenConnection(const std::string connectionId);
	
 private:
    std::vector<std::string> getOpenConnections();
    void clearConnectionFile();

    std::ofstream *outputFile_;

};

#endif
