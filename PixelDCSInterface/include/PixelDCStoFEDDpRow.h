// $Id: PixelDCStoFEDDpRow.h,v 1.3 2007/12/03 11:24:22 veelken Exp $

/***************************************************************************
 * Base class for                                                          *
 *  PixelDCStoFEDDpCalibrationRow,                                         *
 *  PixelDCStoFEDDpFilterRow,                                              *
 *  PixelDCStoFEDDpValueRow,                                               *
 *  PixelDCStoFEDDpConnectionRow and                                       *
 *  PixelDCStoFEDDpNameRow                                                 *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/12/03 11:24:22 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#ifndef _PixelDCStoFEDDpRow_h_
#define _PixelDCStoFEDDpRow_h_

#include <string>

class PixelDCStoFEDDpRow
{
 public:

  PixelDCStoFEDDpRow(unsigned int fedBoardId, unsigned int fedChannelId, unsigned int rocId)
    : fedBoardId_(fedBoardId), fedChannelId_(fedChannelId), rocId_(rocId) {}
  virtual ~PixelDCStoFEDDpRow() {}
  
  unsigned int getBoardId() const { return fedBoardId_; }
  unsigned int getChannelId() const { return fedChannelId_; }
  unsigned int getRocId() const { return rocId_; }
  
  virtual void writeTo(std::ostream& stream) const = 0;
  
 protected:
  
  unsigned int fedBoardId_;
  unsigned int fedChannelId_;
  unsigned int rocId_;
};
#endif
