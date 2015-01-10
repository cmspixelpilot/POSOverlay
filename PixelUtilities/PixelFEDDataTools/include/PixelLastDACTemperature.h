#ifndef _PixelLastDACTemperature_h_
#define _PixelLastDACTemperature_h_

/*************************************************************************
 * Auxiliary class for TemperatureFIFODecoder,                           *
 * stores one last DAC Temperature ADC value                             *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/12 10:48:53 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

class PixelLastDACTemperature
{
 public:    
  PixelLastDACTemperature(unsigned int fedChannel, unsigned int readOutChipId, unsigned int dacValue)
    : fedChannel_(fedChannel), readOutChipId_(readOutChipId), dacValue_(dacValue) {}

  unsigned int getFEDChannel() const { return fedChannel_; }
  unsigned int getReadOutChipId() const { return readOutChipId_; }
  unsigned int getDACValue() const { return dacValue_; }

 private:
  unsigned int fedChannel_; 
  unsigned int readOutChipId_; 
  unsigned int dacValue_; 
};

#endif
