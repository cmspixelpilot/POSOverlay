/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das                          			  *
  *************************************************************************/

#include <iostream>
#include <string>


enum BiVoltage {LV_UNDEFINED, LV_OFF, LV_ON};

class PixelPowerMap4602
{

 public:

  PixelPowerMap4602();

  void setVoltage(unsigned int fb, unsigned int mp, unsigned int io, BiVoltage voltage, std::ostream &ostr=std::cout);
  void setVoltage(std::string, BiVoltage voltage, std::ostream &ostr=std::cout);
  BiVoltage getVoltage(unsigned int fb, unsigned int mp, unsigned int io, std::ostream &ostr=std::cout);
  BiVoltage getVoltage(std::string, std::ostream &ostr=std::cout);

 private:

  bool pwCoordinate(std::string, unsigned int &fb, unsigned int &mp, unsigned int &io, std::ostream &ostr=std::cout);

  BiVoltage a4602_[2][2][2];

};
