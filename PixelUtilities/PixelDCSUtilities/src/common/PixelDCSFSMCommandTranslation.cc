#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMCommandTranslation.h"

/**************************************************************************
 * Auxiliary class for translation between XDAQ and PVSS FSM commands;    *
 * as the translation between states is not unique:                       *
 *                                                                        *
 *  XDAQ command "Halt" translates into PVSS command "OFF"                *
 *   ("RECOVER") if FSM is in XDAQ state "Configured" ("Error"),          *
 *  PVSS command "STANDBY" translates into XDAQ command "Configure"       *
 *   ("Stop") if FSM is in XDAQ state "Halted" ("Running"))               *
 *                                                                        *
 * but depends on the current FSM state,                                  *
 * the translation table is composed of triplets                          *
 *  { XDAQ command, XDAQ state, PVSS command}                             *
 * instead of simple pairs                                                *
 *   { XDAQ command, PVSS command}                                        *
 *                                                                        *
 * (cf. https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=1412) *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/08/10 14:47:04 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>

void PixelDCSFSMCommandTranslation::writeTo(std::ostream& stream) const
{
  std::cout << "   " 
	    << std::setw(13) << xdaqCommand_
	    << std::setw(13) << xdaqState_
	    << std::setw(13) << pvssCommand_ << std::endl;
}
