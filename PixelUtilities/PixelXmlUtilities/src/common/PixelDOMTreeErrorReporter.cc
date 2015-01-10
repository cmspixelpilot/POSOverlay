/*! \file PixelDOMTreeErrorReporter.cc 
 *  \brief Source code of the class that provides trace-back information from
 *         Xerces parsing errors of an input <em>xml configuration file</em>.
 *
 *  If, during parsing of <em>xml configuration file</em>, a syntactic error is
 *  detected, one of the methods defined for this class are invoked to provide
 *  a formatted trace-back of the offending <em>xml</em> tag.
 *  
 *  \image html separator.gif
 *  \image latex separator.pdf
 */
 
#include "PixelANSIColors.h"
#include "PixelDOMTreeErrorReporter.h"
#include <iostream>
#include <string>
#include <sstream>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/SAXParseException.hpp>

using namespace std;

//------------------------------------------------------------------------------
void PixelDOMTreeErrorReporter::warning(const SAXParseException& toCatch){
  string mthn = "[PixelDOMTreeErrorReporter::warning()]\t";
  fSawErrors = true;
  printError(toCatch,"Warning",mthn);
  exit(0) ;
}

//------------------------------------------------------------------------------
void PixelDOMTreeErrorReporter::error(const SAXParseException& toCatch)
{
  string mthn = "[PixelDOMTreeErrorReporter:error()]\t";
  fSawErrors = true;
  printError(toCatch,"Error",mthn);
  exit(0) ;
}

//------------------------------------------------------------------------------
void PixelDOMTreeErrorReporter::fatalError(const SAXParseException& toCatch){
  string mthn = "[PixelDOMTreeErrorReporter:fatalError()]\t";
  fSawErrors = true;
  printError(toCatch,"Fatal error",mthn);
  exit(0) ;
}

//------------------------------------------------------------------------------
void PixelDOMTreeErrorReporter::resetErrors(){
  fSawErrors = false;
}

//------------------------------------------------------------------------------
void PixelDOMTreeErrorReporter::printError(const SAXParseException& toCatch, std::string errorType, std::string mthn){
  stringstream message("");
//  message.str("") ;
  message << mthn
  	  << ACRed << ACBold 
  	  << errorType << " at file \"" 
          << ACReverse 
          << XMLString::transcode(toCatch.getSystemId())
          << ACPlain << ACRed << ACBold 
          << "\", line " 
          << ACBlue  << ACBold 
          << toCatch.getLineNumber()   
          << ACRed   << ACBold 
          << ", column " 
          << ACBlue  << ACBold  
          << toCatch.getColumnNumber() 
          << ACPlain
	  << endl
  	  << mthn
          << ACBlue  << ACBold 
          << "Message: " 
          << ACPlain 
          << XMLString::transcode(toCatch.getMessage()) 
          << " (check against dtd file)"
          << "\n\n" ;
  cout << message.str() << endl;
}


