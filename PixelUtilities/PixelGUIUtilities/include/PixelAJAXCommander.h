// $Id: PixelAJAXCommander.h,v 1.1 2007/10/04 18:26:36 veelken Exp $

/*************************************************************************
 * Auxiliary class to create Javascript code for AJAX functionality      *
 * (dynamic updates of webpages without need to issue reload command     *
 *  by browser)                                                          *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/10/04 18:26:36 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelAJAXCommander_h_
#define _PixelAJAXCommander_h_

#include <string>

class PixelAJAXCommander 
{
 public:
  PixelAJAXCommander();
  virtual ~PixelAJAXCommander();

  static void printJavascriptHeader(std::ostream& stream);
  static void printJavascriptServerRequest(const std::string& url, std::ostream& stream);
  static void printJavascriptTrailer(std::ostream& stream);

};

#endif
