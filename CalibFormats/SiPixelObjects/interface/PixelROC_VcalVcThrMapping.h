

/*************************************************************************
 * Auxiliary class for mapping the relation bewteen VCal and VcThr per   *
 * each ROC                                                              *
 *                                                                       *
 * Author: Annapaola de Cosa, UZH                                        *
 *                                                                       *
 * Last update: $Date: 2014/04/04                                        *
 *************************************************************************/

#ifndef _PixelROC_VcalVcThrMapping_h_
#define _PixelROC_VcalVcThrMapping_h_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <sstream>
#include <fstream>
#include <stdlib.h>

class PixelROC_VcalVcThrMapping
{

 private:
  //PixelROC_VcalVcThrMapping(const PixelROC_VcalVcThrMapping&); 
  static PixelROC_VcalVcThrMapping* mapInstance;
  static bool inited;
 //  static PixelROC_VcalVcThrMapping* mapInstance;
 
 protected:
  PixelROC_VcalVcThrMapping();
  //virtual ~PixelROC_VcalVcThrMapping();
  std::map<std::string, std::vector<float> > rocmap;

 public:
  static PixelROC_VcalVcThrMapping* Instance();
  static std::vector<float>  getValue(std::string const &key);  

};
#endif
