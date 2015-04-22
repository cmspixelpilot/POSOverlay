#ifndef _PixelHistoTransactionKeywords_h_
#define _PixelHistoTransactionKeywords_h_
#include <TString.h>

namespace pixelHistoTransactionKeywords{
  //static int           requestLengthTK           = 2000;
  static const TString getTK                     = "get=";
  static const TString producerUpdateFileTK      = "producerUpdateFile";
  static const TString producerUpdateContentTK   = "producerUpdateContent=";
  static const TString consumerRefreshTK         = "consumerRefresh"; 
  static const TString identifyTK                = "identify"; 
  static const TString producerIdentifierTK      = "producerIdentifier"; 
  static const TString consumerIdentifierTK      = "consumerIdentifier"; 
  static const TString enableAutoRefreshTK       = "enableAutoRefresh"; 
  static const TString disableAutoRefreshTK      = "disableAutoRefresh"; 
  static const TString requestFileListTK         = "requestFileList"; 
  static const TString requestFileContentTK      = "requestFileContent=";
/////////////////////////////////////////////////////////////////////////  
  static const TString errorTK                   = "error"; 
  static const TString errorObjectNotFoundTK     = "errorObjectNotFound"; 
  static const TString errorDirecortyRequestedTK = "errorDirectoryRequested"; 

}

#endif
