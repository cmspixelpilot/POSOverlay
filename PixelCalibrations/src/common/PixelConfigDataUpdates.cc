/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2008 Cornell U.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd              					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelConfigDataUpdates.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;

PixelConfigDataUpdates::PixelConfigDataUpdates(std::vector<std::string> types){

  types_=types;

  for(unsigned int i=0;i<types.size();i++){
    aliases_.push_back(getAliases(types[i]));
  }

}


unsigned int PixelConfigDataUpdates::nTypes(){

  return types_.size();

}

std::string PixelConfigDataUpdates::type(unsigned int n){

  assert(n<types_.size());

  return types_[n];

}

std::vector<std::string> PixelConfigDataUpdates::aliases(unsigned int n){

  assert(n<aliases_.size());

  return aliases_[n];

}


std::vector<std::string> PixelConfigDataUpdates::getAliases(std::string type){

  return PixelConfigInterface::getVersionAliases(type);

}

void PixelConfigDataUpdates::print(){

  cout << "[PixelConfigDataUpdates::print()] npaths="<<types_.size()<<endl;
  for (unsigned int i=0;i<types_.size();i++){
    cout << "[PixelConfigDataUpdates::print()] path="<<types_[i]<<" ---  ";
    for (unsigned int j=0;j<aliases_[i].size();j++){
      cout <<aliases_[i][j]<<" ";
    }
    cout <<endl;
  }


}

std::vector<std::string> PixelConfigDataUpdates::getNames(){

  std::vector<std::string> tmp;
  for (unsigned int i=0;i<types_.size();i++){
    tmp.push_back(types_[i]);
    for (unsigned int j=0;j<aliases_[i].size();j++){
      tmp.push_back(types_[i]+aliases_[i][j]);
    }
  }
  return tmp;

}
