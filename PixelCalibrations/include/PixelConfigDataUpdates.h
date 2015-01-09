/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2008 Cornell U.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd              					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelConfigDataUpdates_h_
#define _PixelConfigDataUpdates_h_

#include <vector>
#include <string>

class PixelConfigDataUpdates{

 public:

  PixelConfigDataUpdates(std::vector<std::string>);

  virtual ~PixelConfigDataUpdates(){};

  unsigned int nTypes();
  std::string type(unsigned int n);

  std::vector<std::string> aliases(unsigned int n);

  std::vector<std::string> getNames();

  void print();

 private:

  std::vector<std::string> getAliases(std::string type);

  std::vector<std::string> types_;
  std::vector<std::vector<std::string> > aliases_;

};

#endif
