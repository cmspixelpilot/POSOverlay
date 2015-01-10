#include "PixelXmlReader.h"
#include "PixelDOMTreeErrorReporter.h"

#include <iostream>
#include <sstream>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/framework/URLInputSource.hpp>

using namespace XERCES_CPP_NAMESPACE;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////
PixelXmlReader::PixelXmlReader(){
  string mthn = "[PixelXmlReader::PixelXmlReader()]\t"; 
  try {
    XMLPlatformUtils::Initialize();
  } 
  catch (const XMLException& toCatch) {
    cout << mthn << "Exception occurred during XERCES parser initialization!" << endl;
  }
  
  parser_ = new XercesDOMParser();
  parser_->setValidationScheme(XercesDOMParser::Val_Always);
  parser_->setDoNamespaces(false);
  errReporter_ = new PixelDOMTreeErrorReporter();
  parser_->setErrorHandler(errReporter_);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
PixelXmlReader::~PixelXmlReader(){
  if(parser_){
    delete parser_;
  }
  if(errReporter_){
    delete errReporter_;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelXmlReader::readDocument(std::string xmlName){
  string mthn = "[PixelXmlReader::readDocument()]\t"; 
  try {
//     cout << mthn << "Configuration file: " << xmlName << endl;
     parser_->parse(xmlName.c_str());
   } 
   catch (...) {
     cout << mthn << "Unexpected exception during parsing of XML file." << endl;
     XMLPlatformUtils::Terminate();
   }
   // Get a handle on the document
   doc_ = parser_->getDocument();
 
   root_ = doc_->getDocumentElement();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int PixelXmlReader::getNumberOfBranches(std::string branchName){
  DOMNodeList * branches = root_->getElementsByTagName(XMLString::transcode(branchName.c_str()));
  if(branches == 0){
    cout << "[PixelXmlReader::getNumberOfBranches()]\tCan't find branch: " << branchName << endl;
    return 0;
  }
  return branches->getLength();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
std::string PixelXmlReader::getXMLAttribute(std::string tagName, std::string attributeName,XERCES_CPP_NAMESPACE::DOMElement *branch){
//   string mthn = "[PixelXmlReader::getXMLAttribute()]\t";
  string  retVal           = string("***** CAN'T FIND TAG ") + tagName + " *****";
  XMLCh * tagNameXML       = XMLString::transcode(tagName.c_str());
  XMLCh * attributeNameXML = XMLString::transcode(attributeName.c_str());
 
  if( branch->getElementsByTagName(tagNameXML) != 0 ) {
    if( branch->getElementsByTagName(tagNameXML)->item(0) != 0 ) {
      if( branch->getElementsByTagName(tagNameXML)->item(0)
	      																				  ->getAttributes()
	      																				  ->getNamedItem(attributeNameXML) != 0 ){
	      retVal = XMLString::transcode(branch->getElementsByTagName(tagNameXML)->item(0)
					        																														->getAttributes()
					        																														->getNamedItem(attributeNameXML)
					        																														->getNodeValue());
      }
    }
  }
  return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
std::string PixelXmlReader::getXMLAttribute(std::string tagName, std::string attributeName){
  return getXMLAttribute(tagName,attributeName,root_);	       
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
std::string PixelXmlReader::getXMLAttribute(std::string branchName, std::string attributeName,XMLSize_t position){
  XMLCh * attributeNameXML = XMLString::transcode(attributeName.c_str());
  DOMNodeList * branches = root_->getElementsByTagName(XMLString::transcode(branchName.c_str()));
  if(branches == 0){
    return string("***** CAN'T FIND BRANCH ") + branchName + " *****";
  }
  DOMElement * childBranch = (DOMElement *)branches->item(position);
  if(childBranch == 0){
    stringstream retVal;
    retVal << "***** CAN'T FIND BRANCH " <<  branchName << " AT POSITION " << position << " *****";
    return retVal.str();
  }
  
// 	string retVal= XMLString::transcode(childBranch->getAttributes()
// 			      														 ->getNamedItem(attributeNameXML)
// 			      														 ->getNodeValue());
//   return retVal;
	return XMLString::transcode(childBranch->getAttributes()
			      														 ->getNamedItem(attributeNameXML)
			      														 ->getNodeValue());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
std::string PixelXmlReader::getXMLAttribute(std::string branchName, std::string tagName, std::string attributeName,XMLSize_t position){

  DOMNodeList * branches = root_->getElementsByTagName(XMLString::transcode(branchName.c_str()));
  if(branches == 0){
    return string("***** CAN'T FIND BRANCH ") + branchName + " *****";
  }
  DOMElement * childBranch = (DOMElement *)branches->item(position);
  if(childBranch == 0){
    stringstream retVal;
    retVal << "***** CAN'T FIND BRANCH " <<  branchName << " AT POSITION " << position << " *****";
    return retVal.str();
  }
 
  return getXMLAttribute(tagName,attributeName,childBranch);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelXmlReader::convertEnvVariables(std::string &name){
	string mthn = "[PixelXmlReader::convertEnvVariables()]\t";
	int dollarPos = -1;
	int slashPos = 0;
	string retVal;
	while( (dollarPos = name.find('$',dollarPos+1)) >= 0){
		slashPos = name.find('/',dollarPos+1);
		if(slashPos < 0){
			slashPos = name.length()-1;
		}
		string envName = name.substr(dollarPos+1,slashPos-dollarPos-1);
//		cout << mthn << "Env:" << envName << endl;
		string env = envName;
	  if(getenv(envName.c_str()) != 0){
		  env = getenv(envName.c_str());
	  }
	  else{
		  cout << mthn << "The "<< envName << " environment variable has not been set!" << endl;
  	}
		name.erase(dollarPos,slashPos-dollarPos);
//		cout << mthn << "Erased:" << name << endl;
		name.insert(dollarPos,env);
//		cout << mthn << "Inserted:" << name << endl;
	}
//  cout << mthn << "Final Name:" << name << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool PixelXmlReader::tagExist(std::string tagName){
  DOMNodeList * branches = root_->getElementsByTagName(XMLString::transcode(tagName.c_str()));
//	cout << "[PixelXmlReader::tagExist()]\t" << branches  << " tagname " << tagName << " Number of branches " << branches->getLength() << endl;
  if(branches == 0){
    return false;
  }
	else if(branches->getLength() == 0){
		return false;
	}
	return true;
}
