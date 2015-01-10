/*! \file PixelXmlReader.h
 *  \brief This is actually a collection of classes, each one implementing a
 *         particular item described in the <em>XML</em> configuration files.
 *   
 *  For more details check each individual class defintion whithin this file
 * 
 *  \image html separator.gif
 *  \image latex separator.pdf
*/

#ifndef _PIXELXMLREADER_H_
#define _PIXELXMLREADER_H_

#include <string>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/PlatformUtils.hpp>

class DOMDocument;
class PixelDOMTreeErrorReporter;

class PixelXmlReader{
 public:
  PixelXmlReader();
  ~PixelXmlReader();
  void readDocument(std::string xmlName);
  unsigned int getNumberOfBranches(std::string branchName);
  std::string  getXMLAttribute(std::string tagName, std::string attributeName); 
  std::string  getXMLAttribute(std::string branchName, std::string attributeName,XMLSize_t position); 
  std::string  getXMLAttribute(std::string branchName, std::string tagName, std::string attributeName,XMLSize_t position=0); 
  void         convertEnvVariables(std::string &name); 
  bool         tagExist(std::string tagName); 
 private:
  std::string getXMLAttribute(std::string tagName, std::string attributeName,XERCES_CPP_NAMESPACE::DOMElement *branch); 
  XERCES_CPP_NAMESPACE::DOMDocument     *doc_;
  XERCES_CPP_NAMESPACE::DOMElement      *root_;
  XERCES_CPP_NAMESPACE::XercesDOMParser *parser_;
  PixelDOMTreeErrorReporter             *errReporter_;
//			      const XMLSize_t i );
};

#endif
