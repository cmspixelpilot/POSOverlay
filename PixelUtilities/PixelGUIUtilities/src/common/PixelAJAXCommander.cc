
#include "PixelAJAXCommander.h"

/*************************************************************************
 * Auxiliary class to create Javascript code for AJAX functionality      *
 * (dynamic updates of webpages without need to issue reload command     *
 *  by browser)                                                          *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/10/04 18:26:37 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include <iomanip>
//#include <stdio>
#include <iostream>

//using namespace std;

PixelAJAXCommander::PixelAJAXCommander()
{
//--- nothing to be done yet...
}

PixelAJAXCommander::~PixelAJAXCommander()
{
//--- nothing to be done yet...
}

void PixelAJAXCommander::printJavascriptHeader(std::ostream& stream)
{
//--- print Javascript code defining AJAX functionality
//    (code taken from http://www.w3schools.com/ajax/ajax_browsers.asp)

  //stream << "<head>" << std::endl;
  stream << "<script type=\"text/javascript\">" << std::endl;
  stream << std::endl;
  stream << "function ajaxFunction()" << std::endl;
  stream << "{" << std::endl;
  stream << "  var xmlHttp;" << std::endl;
  stream << "  try {" << std::endl;
//--- create XMLHttpRequest object for Firefox, Opera 8.0+, Safari browsers are used
  stream << "    xmlHttp = new XMLHttpRequest();" << std::endl;
  stream << "  } catch ( exception ) {" << std::endl;
  stream << "    try {" << std::endl;
//--- create ActiveXObject object for Internet Explorer 6.0+
  stream << "      xmlHttp = new ActiveXObject(\"Msxml2.XMLHTTP\");" << std::endl;
  stream << "    } catch ( exception ) {" << std::endl;
  stream << "      try {" << std::endl;
//--- create ActiveXObject object for Internet Explorer 5.5+
  stream << "        xmlHttp = new ActiveXObject(\"Microsoft.XMLHTTP\");" << std::endl;
  stream << "      } catch ( exception ) {" << std::endl;
//--- incompatible browser;
//    give-up to creating an xmlHttp object 
//    and print an error message instead
  stream << "        alert(\"Your browser does not support AJAX !\");" << std::endl;
  stream << "        return false;" << std::endl;
  stream << "      }" << std::endl;
  stream << "    }" << std::endl;
  stream << "  }" << std::endl;
  stream << std::endl;
}

void PixelAJAXCommander::printJavascriptServerRequest(const std::string& url, std::ostream& stream) 
{
  stream << "  xmlHttp.open(\"GET\", \"" << url << "\", true);" << std::endl;
  stream << "  xmlHttp.send(null);"<<std::endl;  
}

void PixelAJAXCommander::printJavascriptTrailer(std::ostream& stream) 
{
  //stream << "  t = setTimeout('ajaxFunction()', 500);" << std::endl;
  stream << "}" << std::endl;
  stream << std::endl;
  stream << "function checkTime(i)" << std::endl;
  stream << "{" << std::endl;
  stream << "  if ( i < 10 ) { " << std::endl;
  stream << "    i = \"0\" + i;" << std::endl;
  stream << "  }" << std::endl;
  stream << "  return i;" << std::endl;
  stream << "}" << std::endl;
  stream << std::endl;
  stream << "</script>" << std::endl;
  //stream << "</head>" << std::endl;
}
