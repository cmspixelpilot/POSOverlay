//--*-C++-*--
// $Id: PixelFEDBaselineCalibrationNew.h,v 1.1 2009/02/19 11:00:00 joshmt Exp $
/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2009, Cornell University		                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd, S. Das, J. Thompson					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDBaselineCalibrationNew_h_
#define _PixelFEDBaselineCalibrationNew_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "TFile.h"

class PixelFEDBaselineCalibrationNew: public PixelFEDCalibrationBase {
 public:

  PixelFEDBaselineCalibrationNew( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDBaselineCalibrationNew(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  unsigned int iteration_;
  float tolerance_;

  std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels_;
  std::map <unsigned int, std::map<unsigned int, std::vector<std::stringstream*> > > summary_long_;
  std::map <unsigned int, std::map<unsigned int, std::vector<unsigned int> > > summary_short_;
  //               fednumber              channel         iteration  summary

  //
  std::map <unsigned int, std::map<unsigned int, std::vector<unsigned long> > > buffer_;
  std::map <unsigned int, std::map<unsigned int, std::vector<bool> > > done_;

  std::map <unsigned int, std::map<unsigned int,int > > deltaChannelOffset_;
  std::map <unsigned int, std::map<unsigned int,int > > deltaInputOffset_;
  std::map <unsigned int, std::map<unsigned int,double > > meanBlack_;


  std::vector <unsigned int> targetBlack_;
  std::vector <double> opticalReceiverSlope_;
  std::vector <int> channelOffsetSlope_;

  TFile* outputFile_;
  PixelRootDirectoryMaker* dirMakerFED_;


};

#endif
