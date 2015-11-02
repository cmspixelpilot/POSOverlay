/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund			  *
  *************************************************************************/

#include "PixelPowerMap4603.h"
#include <assert.h>

PixelPowerMap4603::PixelPowerMap4603()
{
  init();
}

PixelPowerMap4603::PixelPowerMap4603(const PixelPowerMap4603& ppm) {
  for (unsigned int i=0; i<2; ++i) {
    for (unsigned int j=0; j<2; ++j) {
      for (unsigned int k=0; k<2; ++k) {
	a4603_[i][j][k]=ppm.getVoltage(i,j,k);
	a4603HV_[i][j][k]=ppm.getHVoltage(i,j,k);
      }
    }
  }  
}

void PixelPowerMap4603::init() {
  for (unsigned int i=0; i<2; ++i) {
    for (unsigned int j=0; j<2; ++j) {
      for (unsigned int k=0; k<2; ++k) {
	a4603_[i][j][k]=LV_UNDEFINED;
	a4603HV_[i][j][k]=HV_UNDEFINED;
      }
    }
  }
}

bool PixelPowerMap4603::pwCoordinate(std::string coord, unsigned int &fb, unsigned int &mp, unsigned int &io, std::ostream &ostr)
{
  bool worked=true;

  if (coord[0]=='F' || coord[0] == 'P') {
    fb=0;
  } else if (coord[0]=='B') {
    fb=1;
  } else {
    ostr<<"PixelPowerMap4603::setTriVoltage - First letter of "<<coord<<" neither F nor P nor B!"<<std::endl;
    worked=false;
  }

  if (coord[6]=='m') {
    mp=0;
  } else if (coord[6]=='p') {
    mp=1;
  } else {
    ostr<<"PixelPowerMap4603::setTriVoltage - Seventh letter of "<<coord<<" neither m nor p!"<<std::endl;
    worked=false;
  }

  if (coord[7]=='I') {
    io=0;
  } else if (coord[7]=='O') {
    io=1;
  } else {
    ostr<<"PixelPowerMap4603::setTriVoltage - Eighth letter of "<<coord<<" neither I nor O!"<<std::endl;
    worked=false;
  }

  return worked;
}

void PixelPowerMap4603::setVoltage(unsigned int fb, unsigned int mp, unsigned int io, TriVoltage voltage, std::ostream &ostr)
{
  if ((fb<2) && (mp<2) && (io<2)) {
    a4603_[fb][mp][io]=voltage;
  } else {
    ostr<<"PixelPowerMap4603::setVoltage - Coordinates fb = "<<fb<<", mp = "<<mp<<", and io = "<<io<<" is not possible!"<<std::endl;
    assert(0);
  }
}

void PixelPowerMap4603::setVoltage(std::string coord, TriVoltage voltage, std::ostream &ostr)
{
  unsigned int fb=0, mp=0, io=0;
  bool worked=pwCoordinate(coord, fb, mp, io, ostr);
  if (worked==true) {
    setVoltage(fb, mp, io, voltage, ostr);
    a4603_[fb][mp][io]=voltage;
  } else {
    ostr<<"PixelPowerMap4603::setTriVoltage - Did not set new TriVoltage because coordinate is problematic."<<std::endl;
  }    
}

TriVoltage PixelPowerMap4603::getVoltage(const unsigned int fb, const unsigned int mp, const unsigned int io, std::ostream &ostr) const
{
  if ((fb<2) && (mp<2) && (io<2)) {
    return a4603_[fb][mp][io]; 
  } else {
    ostr<<"PixelPowerMap4603::setVoltage - Coordinates fb = "<<fb<<", mp = "<<mp<<", and io = "<<io<<" is not possible!"<<std::endl;
    assert(0);
  }
}

TriVoltage PixelPowerMap4603::getVoltage(std::string coord, std::ostream &ostr)
{
  unsigned int fb=0, mp=0, io=0;
  bool worked=pwCoordinate(coord, fb, mp, io, ostr);
  if (worked==true) {
    return getVoltage(fb, mp, io, ostr);
  } else {
    ostr<<"PixelPowerMap4603::setTriVoltage - Did not get new TriVoltage because coordinate is problematic."<<std::endl;
    return LV_UNDEFINED;
  }
}

void PixelPowerMap4603::setHVoltage(unsigned int fb, unsigned int mp, unsigned int io, BiVoltage voltage, std::ostream &ostr)
{
  if ((fb<2) && (mp<2) && (io<2)) {
    a4603HV_[fb][mp][io]=voltage;
  } else {
    ostr<<"PixelPowerMap4603::setHVoltage - Coordinates fb = "<<fb<<", mp = "<<mp<<", and io = "<<io<<" is not possible!"<<std::endl;
    assert(0);
  }
}

void PixelPowerMap4603::setHVoltage(std::string coord, BiVoltage voltage, std::ostream &ostr)
{
  unsigned int fb=0, mp=0, io=0;
  bool worked=pwCoordinate(coord, fb, mp, io, ostr);
  if (worked==true) {
    setHVoltage(fb, mp, io, voltage, ostr);
    a4603HV_[fb][mp][io]=voltage; //JMT -- i don't understand why this line is here. it seems harmless, but redundant
  } else {
    ostr<<"PixelPowerMap4603::setHVoltage - Did not set new HVoltage because coordinate is problematic."<<std::endl;
  }    
}

BiVoltage PixelPowerMap4603::getHVoltage(const unsigned int fb, const unsigned int mp, const unsigned int io, std::ostream &ostr) const
{
  if ((fb<2) && (mp<2) && (io<2)) {
    return a4603HV_[fb][mp][io]; 
  } else {
    ostr<<"PixelPowerMap4603::setHVoltage - Coordinates fb = "<<fb<<", mp = "<<mp<<", and io = "<<io<<" is not possible!"<<std::endl;
    assert(0);
  }
}

BiVoltage PixelPowerMap4603::getHVoltage(std::string coord, std::ostream &ostr)
{
  unsigned int fb=0, mp=0, io=0;
  bool worked=pwCoordinate(coord, fb, mp, io, ostr);
  if (worked==true) {
    return getHVoltage(fb, mp, io, ostr);
  } else {
    ostr<<"PixelPowerMap4603::setHVoltage - Did not get HVoltage because coordinate is problematic."<<std::endl;
    return HV_UNDEFINED;
  }
}

void PixelPowerMap4603::setHVoff() {
  for (unsigned int i=0; i<2; ++i) {
    for (unsigned int j=0; j<2; ++j) {
      for (unsigned int k=0; k<2; ++k) {
	a4603HV_[i][j][k]=HV_OFF;
      }
    }
  }
}
