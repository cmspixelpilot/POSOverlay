/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund			  *
  *************************************************************************/

#include "PixelPowerMap4602.h"
#include <assert.h>

PixelPowerMap4602::PixelPowerMap4602()
{
  for (unsigned int i=0; i<2; ++i) {
    for (unsigned int j=0; j<2; ++j) {
      for (unsigned int k=0; k<2; ++k) {
	a4602_[i][j][k]=LV_UNDEFINED;
      }
    }
  }
}

bool PixelPowerMap4602::pwCoordinate(std::string coord, unsigned int &fb, unsigned int &mp, unsigned int &io, std::ostream &ostr)
{
  bool worked=true;

  if (coord[0]=='F' || coord[0] == 'P') {
    fb=0;
  } else if (coord[0]=='B') {
    fb=1;
  } else {
    ostr<<"PixelPowerMap4602::setBiVoltage - First letter of "<<coord<<" neither F nor P nor B!"<<std::endl;
    worked=false;
  }

  if (coord[6]=='m') {
    mp=0;
  } else if (coord[6]=='p') {
    mp=1;
  } else {
    ostr<<"PixelPowerMap4602::setBiVoltage - Seventh letter of "<<coord<<" neither m nor p!"<<std::endl;
    worked=false;
  }

  if (coord[7]=='I') {
    io=0;
  } else if (coord[7]=='O') {
    io=1;
  } else {
    ostr<<"PixelPowerMap4602::setBiVoltage - Eighth letter of "<<coord<<" neither I nor O!"<<std::endl;
    worked=false;
  }

  return worked;
}

void PixelPowerMap4602::setVoltage(unsigned int fb, unsigned int mp, unsigned int io, BiVoltage voltage, std::ostream &ostr)
{
  if ((fb<2) && (mp<2) && (io<2)) {
    a4602_[fb][mp][io]=voltage;
  } else {
    ostr<<"PixelPowerMap4602::setVoltage - Coordinates fb = "<<fb<<", mp = "<<mp<<", and io = "<<io<<" is not possible!"<<std::endl;
    assert(0);
  }
}

void PixelPowerMap4602::setVoltage(std::string coord, BiVoltage voltage, std::ostream &ostr)
{
  unsigned int fb=0, mp=0, io=0;
  bool worked=pwCoordinate(coord, fb, mp, io, ostr);
  if (worked==true) {
    setVoltage(fb, mp, io, voltage, ostr);
  } else {
    ostr<<"PixelPowerMap4602::setBiVoltage - Did not set new BiVoltage because coordinate is problematic."<<std::endl;
  }    
}

BiVoltage PixelPowerMap4602::getVoltage(unsigned int fb, unsigned int mp, unsigned int io, std::ostream &ostr)
{
  if ((fb<2) && (mp<2) && (io<2)) {
    return a4602_[fb][mp][io]; 
  } else {
    ostr<<"PixelPowerMap4602::setVoltage - Coordinates fb = "<<fb<<", mp = "<<mp<<", and io = "<<io<<" is not possible!"<<std::endl;
    assert(0);
  }
}

BiVoltage PixelPowerMap4602::getVoltage(std::string coord, std::ostream &ostr)
{
  unsigned int fb=0, mp=0, io=0;
  bool worked=pwCoordinate(coord, fb, mp, io, ostr);
  if (worked==true) {
    return getVoltage(fb, mp, io, ostr);
  } else {
    ostr<<"PixelPowerMap4602::getBiVoltage - Did not get new BiVoltage because coordinate is problematic."<<std::endl;
    return LV_UNDEFINED;
  }
}
