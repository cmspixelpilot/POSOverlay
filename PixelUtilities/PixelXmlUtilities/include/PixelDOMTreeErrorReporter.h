/*! \file PixelDOMTreeErrorReporter.hh 
 *  \brief Header file of the class that provides trace-back information from
 *         Xerces parsing errors of an input <em>xml configuration file</em>.
 *
 *  If, during parsing of <em>xml configuration file</em>, a syntactic error is
 *  detected, one of the methods defined for this class are invoked to provide
 *  a formatted trace-back of the offending <em>xml</em> tag.
 *  
 *  \image html separator.gif
 *  \image latex separator.pdf
 */
 
#ifndef _PIXELDOMTREEERRORREPORTER_H_
#define _PIXELDOMTREEERRORREPORTER_H_

#include <xercesc/sax/ErrorHandler.hpp>
#include <string>
using namespace XERCES_CPP_NAMESPACE;
//------------------------------------------------------------------------------
class PixelDOMTreeErrorReporter : public ErrorHandler
{
 public:
  PixelDOMTreeErrorReporter() : fSawErrors(false){;}

  ~PixelDOMTreeErrorReporter(){;}

  // -----------------------------------------------------------------------
  //  Implementation of the error handler interface
  // -----------------------------------------------------------------------
  void warning    (const SAXParseException& toCatch);
  void error      (const SAXParseException& toCatch);
  void fatalError (const SAXParseException& toCatch);
  void resetErrors(void);

  // -----------------------------------------------------------------------
  //  Getter methods
  // -----------------------------------------------------------------------
  bool getSawErrors() const {return fSawErrors;}


private:
  // -----------------------------------------------------------------------
  //  fSawErrors
  //	  This is set if we get any errors, and is queryable via a getter
  //	  method. Its used by the main code to suppress output if there are
  //	  errors.
  // -----------------------------------------------------------------------
  bool fSawErrors;
  void printError(const SAXParseException& toCatch, std::string errorType, std::string mthn);
};

#endif
