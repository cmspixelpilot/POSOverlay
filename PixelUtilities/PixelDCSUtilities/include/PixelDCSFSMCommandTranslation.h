// $Id: PixelDCSFSMCommandTranslation.h,v 1.1 2007/08/10 14:47:03 veelken Exp $

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
 * Last update: $Date: 2007/08/10 14:47:03 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCSFSMCommandTranslation_h_
#define _PixelDCSFSMCommandTranslation_h_

#include <string>

class PixelDCSFSMCommandTranslation
{
	public:

        PixelDCSFSMCommandTranslation(const std::string xdaqCommand, const std::string& xdaqState, const std::string pvssCommand)
	  : xdaqCommand_(xdaqCommand), xdaqState_(xdaqState), pvssCommand_(pvssCommand) {}
        ~PixelDCSFSMCommandTranslation() {}

	const std::string& getXdaqCommand() const { return xdaqCommand_; }
	const std::string& getXdaqState() const { return xdaqState_; }
        const std::string& getPvssCommand() const { return pvssCommand_; }

	void writeTo(std::ostream& stream) const;

	protected:

	std::string xdaqCommand_;
	std::string xdaqState_;
	std::string pvssCommand_;
};
#endif
