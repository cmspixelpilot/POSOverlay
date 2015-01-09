#include <fstream>
#include <iostream.h>
#include <stdint.h>
#include <vector.h>

#include "SLinkDecoder.h"

int main(int argc, char *argv[])
{

  if (argc!=3) 
  {
    std::cout<<"Sorry, correct usage: SLink2Human inputFile.dmp outputFile.txt"<<std::endl; 
    return 0;
  }
  
  std::ifstream in(std::string(argv[1]).c_str(),ios::binary|ios::in); assert(in.good());
  std::ofstream out(std::string(argv[2]).c_str(), ios::out); assert(out.good());

  pos::SLinkDecoder sLinkDecoder(&in);

  pos::Word64 runNumber;
  if (sLinkDecoder.getNextWord64(runNumber)) {
    out<<"Run number according to insert in file is "<<runNumber.getWord()<<std::endl;
  } else {
    out<<"Could not read run number from the file. Its corrupted from the start!"<<std::endl;
    return 0;
  }
  
  pos::PixelSLinkEvent sLinkEvent;
  while (sLinkDecoder.getNextEvent(sLinkEvent)) {

    sLinkEvent.decodeEvent();    
    pos::PixelSLinkHeader eventHeader=sLinkEvent.getHeader();
    pos::PixelSLinkTrailer eventTrailer=sLinkEvent.getTrailer();
    std::vector<pos::PixelHit> hits=sLinkEvent.getHits();
    std::vector<pos::PixelError> errors=sLinkEvent.getErrors();

    unsigned int BOE=eventHeader.getBOE();
    unsigned int Evt_ty=eventHeader.getEvt_ty();
    unsigned int LV1_id=eventHeader.getLV1_id();
    unsigned int BX_id=eventHeader.getBX_id();
    unsigned int Source_id=eventHeader.getSource_id();
    unsigned int FOV=eventHeader.getFOV();
    unsigned int H=eventHeader.getH();

    out<<"SLink Header=0x"<<std::hex<<eventHeader.getWord64().getWord()<<std::dec;
    out<<", BOE="<<BOE;
    out<<", Evt_ty="<<Evt_ty;
    out<<", LV1_id="<<LV1_id;
    out<<", BX_id="<<BX_id;
    out<<", Source_id="<<Source_id;
    out<<", FOV="<<FOV;
    out<<", H="<<H<<std::endl;

    for (unsigned int ihit=0; ihit<hits.size(); ++ihit) {

      pos::PixelHit hit=hits.at(ihit);
      int FEDChannel=hit.getLink_id();
      int ROC=hit.getROC_id();
      int dcol=hit.getDCol_id();
      int pxl=hit.getPix_id();
      int row=hit.getRow();
      int column=hit.getColumn();
      int ADC=hit.getADC();

      out<<" Pixel Hit=0x"<<std::hex<<hit.getWord32().getWord()<<std::dec;
      out<<", FED Channel="<<FEDChannel;
      out<<", ROC Number ="<<ROC;
      out<<", DCol="<<dcol;
      out<<", Pxl="<<pxl;
      out<<", Row = "<<row;
      out<<", Column = "<<column;
      out<<", ADC = "<<ADC<<std::endl;

    }

    for (unsigned int ierror=0; ierror<errors.size(); ++ierror) {

      pos::PixelError error=errors.at(ierror);
      std::string errorCode=error.getErrorCode();
      unsigned int ttcrxEventNumber=error.getTTCrxEventNumber();
      unsigned int dataWord=error.getDataWord();

      out<<" Error=0x"<<std::hex<<error.getWord32().getWord()<<std::dec;
      out<<", Error Code="<<errorCode;
      out<<", TTCrx Event Number="<<ttcrxEventNumber;
      out<<", Data Word="<<dataWord;

      if (errorCode=="EventNumberError" || errorCode=="TrailerError") {
        unsigned int FEDChannel=error.getChannel();
        out<<", FED Channel="<<FEDChannel;
      }

      if (errorCode=="TimeOut" || errorCode=="TrailerError") {
        unsigned int errorDetail=error.getErrorDetail();
        out<<", Error Detail="<<errorDetail;
      }

      out<<std::endl;

    }

    unsigned int EOE=eventTrailer.getEOE();
    unsigned int Evt_lgth=eventTrailer.getEvt_lgth();
    unsigned int CRC=eventTrailer.getCRC();
    unsigned int EVT_stat=eventTrailer.getEVT_stat();
    unsigned int TTS=eventTrailer.getTTS();
    unsigned int T=eventTrailer.getT();

    out<<"SLink Trailer=0x"<<std::hex<<eventTrailer.getWord64().getWord()<<std::dec;
    out<<", EOE="<<EOE;
    out<<", Evt_lgth="<<Evt_lgth;
    out<<", CRC="<<CRC;
    out<<", EVT_stat="<<EVT_stat;
    out<<", TTS="<<TTS;
    out<<", T="<<T<<std::endl;

  }


return 0;

}
