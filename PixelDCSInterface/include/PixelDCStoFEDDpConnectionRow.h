// $Id: PixelDCStoFEDDpConnectionRow.h,v 1.1 2007/03/17 16:46:20 veelken Exp $

/**************************************************************************
 * Auxiliary class for storage of "connection" statistics                 *
 * related to last DAC temperature values for a single Read-out Chip      *
 * (number of ADC values within/outside of band suitable for calibration; *
 *  in case the ADC value is outside suitable region,                     *
 *  the PixelFECSupervisor needs to be instructed to change               *
 *  the TempRange DAC of the concerned Read-out Chip,                     *
 *  cf. https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=1007) *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/03/17 16:46:20 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCStoFEDDpConnectionRow_h_
#define _PixelDCStoFEDDpConnectionRow_h_

#include <time.h>

class PixelDCStoFEDDpConnectionRow
{
	public:

  /*       PixelDCStoFEDDpConnectionRow() // default constructor; allows to use PixelDCStoFEDDpConnectionRow objects within std::map
	   { fedBoardId_ = 0; fedChannelId_ = 0; rocId_ = 0; numDpAccepted_ = 0; numDpRejected_ = 0; dpUpdatePriority_ = 0.;  }  */
	PixelDCStoFEDDpConnectionRow(unsigned int fedBoardId, unsigned int fedChannelId, unsigned int rocId)
	  : fedBoardId_(fedBoardId), fedChannelId_(fedChannelId), rocId_(rocId) { numDpAccepted_ = 0; numDpRejected_ = 0; dpUpdatePriority_ = 0.; }
	~PixelDCStoFEDDpConnectionRow() {}

	void incDpAccepted() { ++numDpAccepted_; }
	void incDpRejected() { ++numDpRejected_; }

	void setDpUpdatePriority(float dpValue, float lastDpValue, bool isWithinDeadband,
				 int numDp, time_t currentTime, time_t lastUpdate) const;

	unsigned int getBoardId() const { return fedBoardId_; }
	unsigned int getChannelId() const { return fedChannelId_; }
	unsigned int getRocId() const { return rocId_; }

	unsigned long getNumDpAccepted() const { return numDpAccepted_; }
	unsigned long getNumDpRejected() const { return numDpRejected_; }

	float getDpUpdatePriority() const { return dpUpdatePriority_; }

	protected:

	unsigned int fedBoardId_;
        unsigned int fedChannelId_;
	unsigned int rocId_;

	unsigned long numDpAccepted_;
	unsigned long numDpRejected_;

	mutable float dpUpdatePriority_;
};

bool operator < (const PixelDCStoFEDDpConnectionRow& arg1, const PixelDCStoFEDDpConnectionRow& arg2);

#endif
