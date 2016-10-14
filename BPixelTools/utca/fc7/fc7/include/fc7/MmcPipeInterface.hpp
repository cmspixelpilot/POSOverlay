#ifndef _fc7_MmcPipeInterface_hpp_
#define _fc7_MmcPipeInterface_hpp_

// FC7 Headers
#include "uhal/DerivedNode.hpp"
#include "uhal/log/exception.hpp"

#include "fc7/Firmware.hpp"

#include <string>


namespace uhal
{
  namespace exception
  {
    ExceptionClass ( TextExceedsSpaceAvailable , "Text exceeds space available for it in the MMC" );
    ExceptionClass ( ReplyIndicatesError , "Reply value from MMC indicates an error" );
    ExceptionClass ( GoldenImageIsInvolateError , "An attempt was made to modify the inviolate boot image" );
  }
}

namespace fc7
{


  /*!
   * @class MmcPipeInterfaceNode
   * @brief
   *
   * To fill
   *
   * @author Andrew Rose
   * @date February 2014
   */

  class MmcPipeInterface : public uhal::Node
  {
      UHAL_DERIVEDNODE ( MmcPipeInterface );
    public:

      // PUBLIC METHODS
      MmcPipeInterface ( const uhal::Node& ) ;
      virtual ~MmcPipeInterface();

    public:
      void SetDummySensor ( const uint8_t& aValue );

      void FileToSD ( const std::string& aFilename, Firmware& aFirmware );
      XilinxBitStream FileFromSD ( const std::string& aFilename );

      void RebootFPGA ( const std::string& aFilename , const std::string& aPassword );
      void BoardHardReset ( const std::string& aPassword );
      void DeleteFromSD ( const std::string& aFilename , const std::string& aPassword );

      std::vector< std::string > ListFilesOnSD ( );

    public:
      void UpdateCounters();

      const uint16_t& FPGAtoMMCDataAvailable();
      const uint16_t& FPGAtoMMCSpaceAvailable();
      const uint16_t& MMCtoFPGADataAvailable();
      const uint16_t& MMCtoFPGASpaceAvailable();

    private:
      void Send ( const uint32_t& aHeader );
      void Send ( const uint32_t& aHeader , const uint32_t& aSizeInWords , const uint32_t* aPayload );
      void Send ( const uint32_t& aHeader , const uint32_t& aSizeInBytes , const char* aPayload );

      std::vector< uint32_t > Receive ( );

      std::string ConvertString ( std::vector< uint32_t >::const_iterator aStart , const std::vector< uint32_t >::const_iterator& aEnd );

    private:
      void SetTextSpace ( const std::string& aStr );
      void EnterSecureMode ( const std::string& aPassword );
      std::string GetTextSpace ( );

    private:
      uint16_t mFPGAtoMMCDataAvailable;
      uint16_t mFPGAtoMMCSpaceAvailable;
      uint16_t mMMCtoFPGADataAvailable;
      uint16_t mMMCtoFPGASpaceAvailable;
  };

}

#endif  /* _fc7_MmcPipeInterface_hpp_ */


