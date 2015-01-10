/*************************************************************************
 * Auxiliary class for LastDACFIFODecoder,                               *
 * stores one last DAC value and ROC ID information                      *
 *                                                                       *
 * Author: James Zabel, Rice University                                  *
 *                                                                       *
 * Last update: $Date: 2010/08/31 18:41:24 $ (UTC)                       *
 *          by: $Author: jzabel $                                         *
 *************************************************************************/

#ifndef _PixelLastDACReading_h_
#define _PixelLastDACReading_h_

class PixelLastDACReading
{
  public:
    PixelLastDACReading(unsigned int fedChannel, unsigned int readOutChipId, unsigned int dacValue)
      : fedChannel_(fedChannel), readOutChipId_(readOutChipId), dacValue_(dacValue) {}
    unsigned int getFEDChannel() const {return fedChannel_;}
    unsigned int getReadOutChipId() const {return readOutChipId_;}
    unsigned int getDACValue() const {return dacValue_;}

  private:
    unsigned int fedChannel_;
    unsigned int readOutChipId_;
    unsigned int dacValue_;
};

#endif

