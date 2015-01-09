
/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores value of Vana and Vsf DAC used when taking last DAC ADC values *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/07 16:06:23 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelReadOutChipData_dacInfo_h_
#define _PixelReadOutChipData_dacInfo_h_

class PixelReadOutChipData_dacInfo
{
 public:
  PixelReadOutChipData_dacInfo(unsigned int Vana, unsigned int Vsf) 
   : Vana_(Vana), Vsf(Vsf) {}

  unsigned int getVana() const { return Vana_; }
  unsigned int getVsf() const { return Vsf_; }

 private:
  unsigned int Vana_;
  unsigned int Vsf_;
};

#endif
