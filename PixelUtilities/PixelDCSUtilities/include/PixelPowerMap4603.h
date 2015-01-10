/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das                          			  *
  *************************************************************************/

#include <iostream>
#include <string>

enum TriVoltage {LV_UNDEFINED, LV_OFF, LV_ON_REDUCED, LV_ON};
enum BiVoltage {HV_UNDEFINED, HV_OFF, HV_ON};

class PixelPowerMap4603
{

 public:

  PixelPowerMap4603();
  PixelPowerMap4603(const PixelPowerMap4603& ppm);

  void setVoltage(unsigned int fb, unsigned int mp, unsigned int io, TriVoltage voltage, std::ostream &ostr=std::cout);
  void setVoltage(std::string, TriVoltage voltage, std::ostream &ostr=std::cout);
  TriVoltage getVoltage(const unsigned int fb, const unsigned int mp, const unsigned int io, std::ostream &ostr=std::cout) const;
  TriVoltage getVoltage(std::string, std::ostream &ostr=std::cout);  

  void setHVoltage(unsigned int fb, unsigned int mp, unsigned int io, BiVoltage voltage, std::ostream &ostr=std::cout);
  void setHVoltage(std::string, BiVoltage voltage, std::ostream &ostr=std::cout);
  BiVoltage getHVoltage(const unsigned int fb, const unsigned int mp, const unsigned int io, std::ostream &ostr=std::cout) const;
  BiVoltage getHVoltage(std::string, std::ostream &ostr=std::cout);  

  void setHVoff(); //mark all HV as off

  void init();


 private:

  bool pwCoordinate(std::string, unsigned int &fb, unsigned int &mp, unsigned int &io, std::ostream &ostr=std::cout);

  TriVoltage a4603_[2][2][2];
  BiVoltage a4603HV_[2][2][2];

};
