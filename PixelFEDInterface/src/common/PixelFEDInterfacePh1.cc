#include <cassert>
#include <iostream>
#include <iomanip>
#include <time.h>
//#include <unistd.h>

#include "PixelFEDInterface/include/PixelFEDInterfacePh1.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

using namespace std;

PixelFEDInterfacePh1::PixelFEDInterfacePh1(RegManager* const rm)
  : Printlevel(1),
    regManager(rm),
    slink64calls(0)
{
  num_SEU.assign(96, 0);
}

PixelFEDInterfacePh1::~PixelFEDInterfacePh1() {
}

int PixelFEDInterfacePh1::setup(const string& fn) {
  setPixelFEDCard(pos::PixelPh1FEDCard(fn));
  return setup();
}

int PixelFEDInterfacePh1::setup(pos::PixelPh1FEDCard& c) {
  setPixelFEDCard(c);
  return setup();
}

int PixelFEDInterfacePh1::setup() {
  fRegMapFilename[FMC0_Fitel0] = fitel_fn_base + "/FMCFITEL.txt";
  fRegMapFilename[FMC0_Fitel1] = fitel_fn_base + "/FMCFITEL.txt";
  fRegMapFilename[FMC1_Fitel0] = fitel_fn_base + "/FMCFITEL.txt";
  fRegMapFilename[FMC1_Fitel1] = fitel_fn_base + "/FMCFITEL.txt";

  LoadFitelRegMap(0, 0);
  LoadFitelRegMap(0, 1);
  LoadFitelRegMap(1, 0);
  LoadFitelRegMap(1, 1);

  std::vector<std::pair<std::string, uint32_t> > cVecReg = {
    {"pixfed_ctrl_regs.PC_CONFIG_OK",    0},
    {"pixfed_ctrl_regs.rx_index_sel_en", 0},
    {"pixfed_ctrl_regs.DDR0_end_readout", 0},
    {"pixfed_ctrl_regs.DDR1_end_readout", 0},
    {"pixfed_ctrl_regs.fitel_i2c_cmd_reset", 1}, // fitel I2C bus reset & fifo TX & RX reset
    {"pixfed_ctrl_regs.PACKET_NB", card.packet_nb}, // the FW needs to be aware of the true 32 bit workd Block size for some reason! This is the Packet_nb_true in the python script?!
    {"ctrl.ttc_xpoint_A_out3", 0}, // Used to set the CLK input to the TTC clock from the BP - 3 is XTAL, 0 is BP
    {"pixfed_ctrl_regs.TBM_MASK_1", card.cntrl_1},
    {"pixfed_ctrl_regs.TBM_MASK_2", card.cntrl_2},
    {"pixfed_ctrl_regs.TBM_MASK_3", card.cntrl_3},
    {"fe_ctrl_regs.fifo_config.channel_of_interest", card.TransScopeCh},
    {"pixfed_ctrl_regs.TRIGGER_SEL", 0},
    {"pixfed_ctrl_regs.data_type", 0}, // 2 = fake data?
    {"fe_ctrl_regs.decode_reg_reset", 1}, // init FE spy fifos etc JMTBAD take out if this doesn't work any more
    {"REGMGR_DISPATCH", 0}, // JMTBAD there were two separate WriteStackReg calls, take this out if it doesn't matter
    {"pixfed_ctrl_regs.fitel_i2c_cmd_reset", 0},
    {"pixfed_ctrl_regs.fitel_config_req", 0},
    {"pixfed_ctrl_regs.PC_CONFIG_OK", 1},
  };

  regManager->WriteStackReg(cVecReg);

  usleep(200000);

  int cDDR3calibrated = regManager->ReadReg("pixfed_stat_regs.ddr3_init_calib_done") & 1;

  ConfigureFitel(0, 0, true);
  ConfigureFitel(0, 1, true);
  //ConfigureFitel(1, 0, true);
  //ConfigureFitel(1, 1, true);

  fNthAcq = 0;

  getBoardInfo();

  //  readPhases(true, true);

  return cDDR3calibrated;
}

void PixelFEDInterfacePh1::loadFPGA() {
}


std::string PixelFEDInterfacePh1::getBoardType()
{
    // adapt me!
    std::string cBoardTypeString;

    uhal::ValWord<uint32_t> cBoardType = regManager->ReadReg( "board_id" );

    char cChar = ( ( cBoardType & 0xFF000000 ) >> 24 );
    cBoardTypeString.push_back( cChar );

    cChar = ( ( cBoardType & 0x00FF0000 ) >> 16 );
    cBoardTypeString.push_back( cChar );

    cChar = ( ( cBoardType & 0x0000FF00 ) >> 8 );
    cBoardTypeString.push_back( cChar );

    cChar = ( cBoardType & 0x000000FF );
    cBoardTypeString.push_back( cChar );

    return cBoardTypeString;

}

void PixelFEDInterfacePh1::getFEDNetworkParameters()
{
    std::cout << "MAC & IP Source: " << regManager->ReadReg("mac_ip_source") << std::endl;

    std::cout << "MAC Address: " << std::hex << regManager->ReadReg("mac_b5") << ":" << regManager->ReadReg("mac_b4") << ":" << regManager->ReadReg("mac_b3") << ":" << regManager->ReadReg("mac_b2") << ":" << regManager->ReadReg("mac_b1") << ":" << regManager->ReadReg("mac_b0") << std::dec << std::endl;
}

void PixelFEDInterfacePh1::getBoardInfo()
{
    std::cout << std::endl << "Board Type: " << getBoardType() << std::endl;
    getFEDNetworkParameters();
    std::string cBoardTypeString;

    uhal::ValWord<uint32_t> cBoardType = regManager->ReadReg( "pixfed_stat_regs.user_ascii_code_01to04" );

    char cChar = ( ( cBoardType & 0xFF000000 ) >> 24 );
    cBoardTypeString.push_back( cChar );

    cChar = ( ( cBoardType & 0x00FF0000 ) >> 16 );
    cBoardTypeString.push_back( cChar );

    cChar = ( ( cBoardType & 0x0000FF00 ) >> 8 );
    cBoardTypeString.push_back( cChar );

    cChar = ( cBoardType & 0x000000FF );
    cBoardTypeString.push_back( cChar );

    cBoardType = regManager->ReadReg( "pixfed_stat_regs.user_ascii_code_05to08" );
    cChar = ( ( cBoardType & 0xFF000000 ) >> 24 );
    cBoardTypeString.push_back( cChar );

    cChar = ( ( cBoardType & 0x00FF0000 ) >> 16 );
    cBoardTypeString.push_back( cChar );

    cChar = ( ( cBoardType & 0x0000FF00 ) >> 8 );
    cBoardTypeString.push_back( cChar );

    cChar = ( ( cBoardType & 0x00000000 ) );
    cBoardTypeString.push_back( cChar );

    std::cout << "Board Use: " << cBoardTypeString << std::endl;

    std::cout << "FW version IPHC : " << regManager->ReadReg( "pixfed_stat_regs.user_iphc_fw_id.fw_ver_nb" ) << "." << regManager->ReadReg( "pixfed_stat_regs.user_iphc_fw_id.archi_ver_nb" ) << "; Date: " << regManager->ReadReg( "pixfed_stat_regs.user_iphc_fw_id.fw_ver_day" ) << "." << regManager->ReadReg( "pixfed_stat_regs.user_iphc_fw_id.fw_ver_month" ) << "." << regManager->ReadReg( "pixfed_stat_regs.user_iphc_fw_id.fw_ver_year" ) <<  std::endl;
    std::cout << "FW version HEPHY : " << regManager->ReadReg( "pixfed_stat_regs.user_hephy_fw_id.fw_ver_nb" ) << "." << regManager->ReadReg( "pixfed_stat_regs.user_hephy_fw_id.archi_ver_nb" ) << "; Date: " << regManager->ReadReg( "pixfed_stat_regs.user_hephy_fw_id.fw_ver_day" ) << "." << regManager->ReadReg( "pixfed_stat_regs.user_hephy_fw_id.fw_ver_month" ) << "." << regManager->ReadReg( "pixfed_stat_regs.user_hephy_fw_id.fw_ver_year" ) << std::endl;

    std::cout << "FMC 8 Present : " << regManager->ReadReg( "status.fmc_l8_present" ) << std::endl;
    std::cout << "FMC 12 Present : " << regManager->ReadReg( "status.fmc_l12_present" ) << std::endl << std::endl;
}


void PixelFEDInterfacePh1::disableFMCs()
{
    std::vector< std::pair<std::string, uint32_t> > cVecReg;
    cVecReg.push_back( { "fmc_pg_c2m", 0 } );
    cVecReg.push_back( { "fmc_l12_pwr_en", 0 } );
    cVecReg.push_back( { "fmc_l8_pwr_en", 0 } );
    regManager->WriteStackReg(cVecReg);
}

void PixelFEDInterfacePh1::enableFMCs()
{
    std::vector< std::pair<std::string, uint32_t> > cVecReg;
    cVecReg.push_back( { "fmc_pg_c2m", 1 } );
    cVecReg.push_back( { "fmc_l12_pwr_en", 1 } );
    cVecReg.push_back( { "fmc_l8_pwr_en", 1 } );
    regManager->WriteStackReg(cVecReg);
}

/*
void PixelFEDInterfacePh1::FlashProm( const std::string & strConfig, const char* pstrFile )
{
    checkIfUploading();

    fpgaConfig->runUpload( strConfig, pstrFile );
}

void PixelFEDInterfacePh1::JumpToFpgaConfig( const std::string & strConfig )
{
    checkIfUploading();

    fpgaConfig->jumpToImage( strConfig );
}

std::vector<std::string> PixelFEDInterfacePh1::getFpgaConfigList()
{
    checkIfUploading();
    return fpgaConfig->getFirmwareImageNames( );
}

void PixelFEDInterfacePh1::DeleteFpgaConfig( const std::string & strId )
{
    checkIfUploading();
    fpgaConfig->deleteFirmwareImage( strId );
}

void PixelFEDInterfacePh1::DownloadFpgaConfig( const std::string& strConfig, const std::string& strDest)
{
    checkIfUploading();
    fpgaConfig->runDownload( strConfig, strDest.c_str());
}

void PixelFEDInterfacePh1::checkIfUploading()
{
    if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
        throw Exception( "This board is uploading an FPGA configuration" );

    if ( !fpgaConfig )
        fpgaConfig = new CtaFpgaConfig( this );
}
*/

void PixelFEDInterfacePh1::LoadFitelRegMap(int cFMCId, int cFitelId) {
  const std::string& filename = fRegMapFilename[FitelMapNum(cFMCId, cFitelId)];
  std::ifstream file( filename.c_str(), std::ios::in );
  if (!file) {
    std::cerr << "The Fitel Settings File " << filename << " could not be opened!" << std::endl;
    assert(0);
  }

  std::string line, fName, fAddress_str, fDefValue_str, fValue_str, fPermission_str;
  FitelRegItem fRegItem;

  while (getline(file, line)) {
    if (line.find_first_not_of(" \t") == std::string::npos) continue;
    if (line[0] == '#' || line[0] == '*') continue;
    std::istringstream input(line);
    input >> fName >> fAddress_str >> fDefValue_str >> fValue_str >> fPermission_str;

    fRegItem.fAddress	 = strtoul(fAddress_str.c_str(),  0, 16);
    fRegItem.fDefValue	 = strtoul(fDefValue_str.c_str(), 0, 16);
    fRegItem.fValue	 = strtoul(fValue_str.c_str(),    0, 16);
    fRegItem.fPermission = fPermission_str[0];

    //std::cout << fName << " "<< +fRegItem.fAddress << " " << +fRegItem.fDefValue << " " << +fRegItem.fValue << std::endl;
    fRegMap[FitelMapNum(cFMCId, cFitelId)][fName] = fRegItem;
  }

  file.close();
}

void PixelFEDInterfacePh1::EncodeFitelReg( const FitelRegItem& pRegItem, uint8_t pFMCId, uint8_t pFitelId , std::vector<uint32_t>& pVecReq ) {
  pVecReq.push_back(  pFMCId  << 24 |  pFitelId << 20 |  pRegItem.fAddress << 8 | pRegItem.fValue );
}

void PixelFEDInterfacePh1::DecodeFitelReg( FitelRegItem& pRegItem, uint8_t pFMCId, uint8_t pFitelId, uint32_t pWord ) {
  //uint8_t cFMCId = ( pWord & 0xff000000 ) >> 24;
  //cFitelId = (  pWord & 0x00f00000   ) >> 20;
  pRegItem.fAddress = ( pWord & 0x0000ff00 ) >> 8;
  pRegItem.fValue = pWord & 0x000000ff;
}

void PixelFEDInterfacePh1::i2cRelease(uint32_t pTries)
{
    uint32_t cCounter = 0;
    // release
    regManager->WriteReg("pixfed_ctrl_regs.fitel_config_req", 0);
    while (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") != 0)
    {
        if (cCounter > pTries)
        {
            std::cout << "Error, exceeded maximum number of tries for I2C release!" << std::endl;
            break;
        }
        else
        {
            usleep(100);
            cCounter++;
        }
    }
}

bool PixelFEDInterfacePh1::polli2cAcknowledge(uint32_t pTries)
{
    bool cSuccess = false;
    uint32_t cCounter = 0;
    // wait for command acknowledge
    while (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 0)
    {
        if (cCounter > pTries)
        {
            std::cout << "Error, polling for I2C command acknowledge timed out!" << std::endl;
            break;

        }
        else
        {
            usleep(100);
            cCounter++;
        }
    }

    // check the value of that register
    if (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 1)
    {
        cSuccess = true;
    }
    else if (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 3)
    {
        cSuccess = false;
    }
    return cSuccess;
}

bool PixelFEDInterfacePh1::WriteFitelBlockReg(std::vector<uint32_t>& pVecReq) {
  regManager->WriteReg("pixfed_ctrl_regs.fitel_i2c_addr", 0x4d);
  bool cSuccess = false;
  // write the encoded registers in the tx fifo
  regManager->WriteBlockReg("fitel_config_fifo_tx", pVecReq);
  // sent an I2C write request
  regManager->WriteReg("pixfed_ctrl_regs.fitel_config_req", 1);

  // wait for command acknowledge
  while (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 0) usleep(100);

  uint32_t cVal = regManager->ReadReg("pixfed_stat_regs.fitel_config_ack");
  //if (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 1)
  if (cVal == 1)
    {
      cSuccess = true;
    }
  //else if (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 3)
  else if (cVal == 3)
    {
      std::cout << "Error writing Registers!" << std::endl;
      cSuccess = false;
    }

  // release
  i2cRelease(10);
  return cSuccess;
}

bool PixelFEDInterfacePh1::ReadFitelBlockReg(std::vector<uint32_t>& pVecReq)
{
  regManager->WriteReg("pixfed_ctrl_regs.fitel_i2c_addr", 0x4d);
  bool cSuccess = false;
  //uint32_t cVecSize = pVecReq.size();

  // write the encoded registers in the tx fifo
  regManager->WriteBlockReg("fitel_config_fifo_tx", pVecReq);
  // sent an I2C write request
  regManager->WriteReg("pixfed_ctrl_regs.fitel_config_req", 3);

  // wait for command acknowledge
  while (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 0) usleep(100);

  uint32_t cVal = regManager->ReadReg("pixfed_stat_regs.fitel_config_ack");
  //if (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 1)
  if (cVal == 1)
    {
      cSuccess = true;
    }
  //else if (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 3)
  else if (cVal == 3)
    {
      cSuccess = false;
      std::cout << "Error reading registers!" << std::endl;
    }

  // release
  i2cRelease(10);

  // clear the vector & read the data from the fifo
  pVecReq = regManager->ReadBlockRegValue("fitel_config_fifo_rx", pVecReq.size());
  return cSuccess;
}

void PixelFEDInterfacePh1::ConfigureFitel(int cFMCId, int cFitelId , bool pVerifLoop )
{
  FitelRegMap& cFitelRegMap = fRegMap[FitelMapNum(cFMCId, cFitelId)];
    FitelRegMap::iterator cIt = cFitelRegMap.begin();

    while ( cIt != cFitelRegMap.end() )
    {
        std::vector<uint32_t> cVecWrite;
        std::vector<uint32_t> cVecRead;

        uint32_t cCounter = 0;

        for ( ; cIt != cFitelRegMap.end(); cIt++ )
        {
            if (cIt->second.fPermission == 'w')
            {
                EncodeFitelReg( cIt->second, cFMCId, cFitelId, cVecWrite );

                if ( pVerifLoop )
                {
                    FitelRegItem cItem = cIt->second;
                    cItem.fValue = 0;

                    EncodeFitelReg( cItem, cFMCId, cFitelId, cVecRead );
                }
#ifdef COUNT_FLAG
                fRegisterCount++;
#endif
                cCounter++;

            }
        }

        WriteFitelBlockReg(  cVecWrite );
        //usleep(20000);
#ifdef COUNT_FLAG
        fTransactionCount++;
#endif
        if ( pVerifLoop )
        {
            ReadFitelBlockReg( cVecRead );

            // only if I have a mismatch will i decode word by word and compare
            if ( cVecWrite != cVecRead )
            {
                bool cAllgood = false;
                int cIterationCounter = 1;
                while ( !cAllgood )
                {
                    if ( cAllgood ) break;

                    std::vector<uint32_t> cWrite_again;
                    std::vector<uint32_t> cRead_again;

                    auto cMismatchWord = std::mismatch( cVecWrite.begin(), cVecWrite.end(), cVecRead.begin() );

                    while ( cMismatchWord.first != cVecWrite.end() )
                    {


                        FitelRegItem cRegItemWrite;
                        DecodeFitelReg( cRegItemWrite, cFMCId, cFitelId, *cMismatchWord.first );
                        FitelRegItem cRegItemRead;
                        DecodeFitelReg( cRegItemRead, cFMCId, cFitelId, *cMismatchWord.second );

                        if ( cIterationCounter == 5 )
                        {
                            std::cout << "\nERROR !!!\nReadback value not the same after 5 Iteration for Register @ Address: 0x" << std::hex << int( cRegItemWrite.fAddress ) << "\n" << "Written Value : 0x" << int( cRegItemWrite.fValue ) << "\nReadback Value : 0x" << int( cRegItemRead.fValue ) << std::dec << std::endl;
                            std::cout << "Fitel Id : " << int( cFitelId ) << std::endl << std::endl;
                            std::cout << "Failed to write register in " << cIterationCounter << " trys! Giving up!" << std::endl;
                            std::cout << "---<-FMC<-fi---------<-a-----<-v" << std::endl;
                            std::cout << static_cast<std::bitset<32> >( *cMismatchWord.first ) << std::endl << static_cast<std::bitset<32> >( *cMismatchWord.second ) << std::endl << std::endl;
                        }

                        cMismatchWord = std::mismatch( ++cMismatchWord.first, cVecWrite.end(), ++cMismatchWord.second );

                        EncodeFitelReg( cRegItemWrite, cFMCId, cFitelId, cWrite_again );
                        cRegItemRead.fValue = 0;
                        EncodeFitelReg( cRegItemRead, cFMCId, cFitelId, cRead_again );
                    }

                    WriteFitelBlockReg( cWrite_again );
                    ReadFitelBlockReg( cRead_again );

                    if ( cWrite_again != cRead_again )
                    {
                        if ( cIterationCounter == 5 )
                        {
                            std::cout << "Failed to configure FITEL in " << cIterationCounter << " Iterations!" << std::endl;
                            break;
                        }
                        cVecWrite.clear();
                        cVecWrite = cWrite_again;
                        cVecRead.clear();
                        cVecRead = cRead_again;
                        cIterationCounter++;
                    }
                    else
                    {
                      //                        std::cout << "Managed to write all registers correctly in " << cIterationCounter << " Iteration(s)!" << RESET << std::endl;
                        cAllgood = true;
                    }
                }
            }
        }
    }
}

std::pair<bool, std::vector<double> > PixelFEDInterfacePh1::ReadADC( const uint8_t pFMCId, const uint8_t pFitelId) {
  // the Fitel FMC needs to be set up to be able to read the RSSI on a given Channel:
  // I2C register 0x1: set to 0x4 for RSSI, set to 0x5 for Die Temperature of the Fitel
  // Channel Control Registers: set to 0x02 to disable RSSI for this channel, set to 0x0c to enable RSSI for this channel
  // the ADC always reads the sum of all the enabled channels!
  //initial FW setup
  regManager->WriteReg("pixfed_ctrl_regs.fitel_i2c_cmd_reset", 1);

  std::vector<std::pair<std::string, uint32_t> > cVecReg;
  cVecReg.push_back({"pixfed_ctrl_regs.fitel_i2c_cmd_reset", 0});
  cVecReg.push_back({"pixfed_ctrl_regs.fitel_config_req", 0});
  cVecReg.push_back({"pixfed_ctrl_regs.fitel_i2c_addr", 0x77});

  regManager->WriteStackReg(cVecReg);

  //first, write the correct registers to configure the ADC
  //the values are: Address 0x01 -> 0x1<<6 & 0x1f
  //                Address 0x02 -> 0x1

  // Vectors for write and read data!
  std::vector<uint32_t> cVecWrite;
  std::vector<uint32_t> cVecRead;

  //encode them in a 32 bit word and write, no readback yet
  cVecWrite.push_back(  pFMCId  << 24 |  pFitelId << 20 |  0x1 << 8 | 0x5f );
  cVecWrite.push_back(  pFMCId  << 24 |  pFitelId << 20 |  0x2 << 8 | 0x01 );
  regManager->WriteBlockReg("fitel_config_fifo_tx", cVecWrite);

  // sent an I2C write request
  regManager->WriteReg("pixfed_ctrl_regs.fitel_config_req", 1);

  // wait for command acknowledge
  while (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 0) usleep(100);

  uint32_t cVal = regManager->ReadReg("pixfed_stat_regs.fitel_config_ack");
  bool success = cVal != 3;

  // release
  i2cRelease(10);

  //now prepare the read-back of the values
  uint8_t cNWord = 10;
  for (uint8_t cIndex = 0; cIndex < cNWord; cIndex++)
    {
      cVecRead.push_back( pFMCId << 24 | pFitelId << 20 | (0x6 + cIndex ) << 8 | 0 );
    }
  regManager->WriteReg("pixfed_ctrl_regs.fitel_i2c_addr", 0x4c);

  regManager->WriteBlockReg( "fitel_config_fifo_tx", cVecRead );
  // sent an I2C write request
  regManager->WriteReg("pixfed_ctrl_regs.fitel_config_req", 3);

  // wait for command acknowledge
  while (regManager->ReadReg("pixfed_stat_regs.fitel_config_ack") == 0) usleep(100);

  cVal = regManager->ReadReg("pixfed_stat_regs.fitel_config_ack");
  success = success && cVal != 3;

  // release
  i2cRelease(10);

  // clear the vector & read the data from the fifo
  cVecRead = regManager->ReadBlockRegValue("fitel_config_fifo_rx", cVecRead.size());

  // now convert to Voltages!
  std::vector<double> cLTCValues(cNWord / 2, 0);

  double cConstant = 0.00030518;
  // each value is hidden in 2 I2C words
  for (int cMeasurement = 0; cMeasurement < cNWord / 2; cMeasurement++)
    {
      // build the values
      uint16_t cValue = ((cVecRead.at(2 * cMeasurement) & 0x7F) << 8) + (cVecRead.at(2 * cMeasurement + 1) & 0xFF);
      uint8_t cSign = (cValue >> 14) & 0x1;

      //now the conversions are different for each of the voltages, so check by cMeasurement
      if (cMeasurement == 4)
        cLTCValues.at(cMeasurement) = (cSign == 0b1) ? (-( 32768 - cValue ) * cConstant + 2.5) : (cValue * cConstant + 2.5);

      else
        cLTCValues.at(cMeasurement) = (cSign == 0b1) ? (-( 32768 - cValue ) * cConstant) : (cValue * cConstant);
    }

  // now I have all 4 voltage values in a vector of size 5
  // V1 = cLTCValues[0]
  // V2 = cLTCValues[1]
  // V3 = cLTCValues[2]
  // V4 = cLTCValues[3]
  // Vcc = cLTCValues[4]
  //
  // the RSSI value = fabs(V3-V4) / R=150 Ohm [in Amps]
  //double cADCVal = fabs(cLTCValues.at(2) - cLTCValues.at(3)) / 150.0;
  return std::make_pair(success, cLTCValues);
}

void PixelFEDInterfacePh1::reset() {
  // JMTBAD from fedcard

  //  setup(); // JMTBAD
}

void PixelFEDInterfacePh1::resetFED() {
}

void PixelFEDInterfacePh1::armOSDFifo(int channel, int rochi, int roclo) {
}

uint32_t PixelFEDInterfacePh1::readOSDFifo(int channel) {
  return 0;
}

void prettyprintPhase(const std::vector<uint32_t>& pData, int pChannel)
{
    std::cout <<  "Fibre: " << std::setw(2) <<  pChannel + 1 << "    " <<
              std::bitset<1>( (pData.at( (pChannel * 4 ) + 0 ) >> 10 ) & 0x1 )   << "    " << std::setw(2) <<
              ((pData.at( (pChannel * 4 ) + 0 ) >> 5  ) & 0x1f )  << "    " << std::setw(2) <<
              ((pData.at( (pChannel * 4 ) + 0 )       ) & 0x1f ) << "    " <<
      std::bitset<32>( pData.at( (pChannel * 4 ) + 1 )) << "    " <<
              std::bitset<1>( (pData.at( (pChannel * 4 ) + 2 ) >> 31 ) & 0x1 )  << " " << std::setw(2) <<
              ((pData.at( (pChannel * 4 ) + 2 ) >> 23 ) & 0x1f)  << " " << std::setw(2) <<
              ((pData.at( (pChannel * 4 ) + 2 ) >> 18 ) & 0x1f)  << " " << std::setw(2) <<
              ((pData.at( (pChannel * 4 ) + 2 ) >> 13 ) & 0x1f ) << " " << std::setw(2) <<
              ((pData.at( (pChannel * 4 ) + 2 ) >> 8  ) & 0x1f ) << " " << std::setw(2) <<
              ((pData.at( (pChannel * 4 ) + 2 ) >> 5  ) & 0x7  ) << " " << std::setw(2) <<
              ((pData.at( (pChannel * 4 ) + 2 )       ) & 0x1f ) << std::endl;
}

void PixelFEDInterfacePh1::readPhases(bool verbose, bool override_timeout) {
    // Perform all the resets
    std::vector< std::pair<std::string, uint32_t> > cVecReg;
    cVecReg.push_back( { "fe_ctrl_regs.decode_reset", 1 } ); // reset deocode auto clear
    cVecReg.push_back( { "fe_ctrl_regs.decode_reg_reset", 1 } ); // reset REG auto clear
    cVecReg.push_back( { "fe_ctrl_regs.idel_ctrl_reset", 1} );
    regManager->WriteStackReg(cVecReg);
    cVecReg.clear();
    cVecReg.push_back( { "fe_ctrl_regs.idel_ctrl_reset", 0} );
    regManager->WriteStackReg(cVecReg);
    cVecReg.clear();

    // NOTE: here the register idel_individual_ctrl is the base address of the registers for all 48 channels. So each 32-bit word contains the control info for 1 channel. Thus by creating a vector of 48 32-bit words and writing them at the same time I can write to each channel without using relative addresses!

    // set the parameters for IDELAY scan
    std::vector<uint32_t> cValVec;
    for (uint32_t cChannel = 0; cChannel < 48; cChannel++)
        // create a Value Vector that contains the write value for each channel
        cValVec.push_back( 0x80000000 );
    regManager->WriteBlockReg( "fe_ctrl_regs.idel_individual_ctrl", cValVec );
    cValVec.clear();

    // set auto_delay_scan and set idel_RST
    for (uint32_t cChannel = 0; cChannel < 48; cChannel++)
        cValVec.push_back( 0xc0000000 );
    regManager->WriteBlockReg( "fe_ctrl_regs.idel_individual_ctrl", cValVec );
    cValVec.clear();

    // set auto_delay_scan and remove idel_RST
    for (uint32_t cChannel = 0; cChannel < 48; cChannel++)
        cValVec.push_back( 0x80000000 );
    regManager->WriteBlockReg( "fe_ctrl_regs.idel_individual_ctrl", cValVec );
    cValVec.clear();

    // some additional configuration
    cVecReg.push_back( { "fe_ctrl_regs.fifo_config.overflow_value", 0x700e0}); // set 192val
    cVecReg.push_back( { "fe_ctrl_regs.fifo_config.channel_of_interest", 8} ); // set channel for scope FIFO
    regManager->WriteStackReg(cVecReg);
    cVecReg.clear();

    // initialize Phase Finding
    regManager->WriteReg("fe_ctrl_regs.initialize_swap", 1);
    std::cout << "Initializing Phase Finding ..." << std::endl << std::endl;
    sleep(3);

    //here I might do the print loop again as Helmut does it in the latest version of the PixFED python script
    regManager->WriteReg("fe_ctrl_regs.initialize_swap", 0);

    sleep(3);

    std::cout <<  "Phase finding Results: " << std::endl;

    uint32_t cNChannel = 24;
    std::vector<uint32_t> cReadValues = regManager->ReadBlockRegValue( "idel_individual_stat_block", cNChannel * 4 );

    std::cout << "FIBRE CTRL_RDY CNTVAL_Hi CNTVAL_Lo   pattern:                     S H1 L1 H0 L0   W R" << std::endl;
    //for(uint32_t cChannel = 0; cChannel < 48; cChannel++){
    for (uint32_t cChannel = 0; cChannel < cNChannel; cChannel++)
    {
        prettyprintPhase(cReadValues, cChannel);
    }

    //std::this_thread::sleep_for( cWait );

    cVecReg.push_back( { "pixfed_ctrl_regs.PC_CONFIG_OK", 0} );
    regManager->WriteStackReg(cVecReg);
    cVecReg.clear();
    cVecReg.push_back( { "pixfed_ctrl_regs.PC_CONFIG_OK", 1} );
    regManager->WriteStackReg(cVecReg);
    cVecReg.clear();
}

std::vector<uint32_t> PixelFEDInterfacePh1::readTransparentFIFO()
{
    //regManager->WriteReg("fe_ctrl_regs.decode_reg_reset", 1);
    //std::vector<uint32_t> cFifoVec = ReadBlockRegValue( "fifo.bit_stream", 32 );
  //    std::cout << std::endl << BOLDBLUE <<  "Transparent FIFO: " << RESET << std::endl;

    //for (auto& cWord : cFifoVec)
    //std::cout << GREEN << std::bitset<30>(cWord) << RESET << std::endl;

    std::vector<uint32_t> cFifoVec;
    //std::cout << "DEBUG: Helmut's way:" << std::endl;
    for (int i = 0; i < 32; i++)
    {
        uint32_t cWord = regManager->ReadReg("fifo.bit_stream");
        cFifoVec.push_back(cWord);
//        std::cout << GREEN << std::bitset<30>(cWord) << RESET << std::endl;
//        for (int iBit = 29; iBit >= 0; iBit--)
//        {
//            if (std::bitset<30>(cWord)[iBit] == 0) std::cout << GREEN << "_";
//            else std::cout << "-";
//        }
    }
    //    std::cout << RESET << std::endl;
    return cFifoVec;
}

int PixelFEDInterfacePh1::drainTransparentFifo(uint32_t* data) {
  const size_t MAX_TRANSPARENT_FIFO = 1024;
  std::vector<uint32_t> v(readTransparentFIFO());
  const int ie(min(v.size(), MAX_TRANSPARENT_FIFO));
  for (int i = 0; i < ie; ++i)
    data[i] = v[i];
  return ie;
}

void prettyprintSpyFIFO(const std::vector<uint32_t>& pVec)
{
    uint32_t cMask = 0xf0;
    for (size_t i = 0; i < pVec.size(); ++i)
    {
      uint32_t cWord = pVec[i];
        if (cWord != 0)
        {
            if ((cWord & 0xff) != 0) std::cout << std::hex << (cWord & 0xff) << " " ;
            if (((cWord & cMask) >> 4) == 11 ) std::cout << " " << std::endl;
            if (((cWord & cMask) >> 4) == 6 ) std::cout << " " << std::endl;
            if (((cWord & cMask) >> 4) == 7 ) std::cout << " " << std::endl;
            if (((cWord & cMask) >> 4) == 15 ) std::cout << " " << std::endl;
        }

    }
}

std::vector<uint32_t> PixelFEDInterfacePh1::readSpyFIFO()
{
  
  std::vector<uint32_t> cSpy[2];
  const size_t N = 4096;
  for (int i = 0; i < 2; ++i) {
    { //while (1) {
      std::vector<uint32_t> tmp = regManager->ReadBlockRegValue(i == 0 ? "fifo.spy_A" : "fifo.spy_B", N);
      std::vector<uint32_t>::iterator it = std::find(tmp.begin(), tmp.end(), 0);
//      int l = it - tmp.begin();
//      if (l == 0)
//	break;
      cSpy[i].insert(cSpy[i].end(), tmp.begin(), it);
    }
  }

  std::cout  << std::endl << "TBM_SPY FIFO A (size " << cSpy[0].size() << "):" << std::endl;
  prettyprintSpyFIFO(cSpy[0]);
  std::cout << std::endl << "TBM_SPY FIFO B (size " << cSpy[1].size() << "):" << std::endl;
  prettyprintSpyFIFO(cSpy[1]);

//append content of Spy Fifo B to A and return
    return std::vector<uint32_t>();
//    std::vector<uint32_t> cAppendedSPyFifo = cSpyA;
//    cAppendedSPyFifo.insert(cSpyA.end(), cSpyB.begin(), cSpyB.end());
//    return cAppendedSPyFifo;
}


int PixelFEDInterfacePh1::drainSpyFifo(uint32_t* data) {
  const size_t MAX_SPY_FIFO = 1024;
  std::vector<uint32_t> v(readSpyFIFO());
  const int ie(min(v.size(), MAX_SPY_FIFO));
  for (int i = 0; i < ie; ++i)
    data[i] = v[i];
  return ie;
}

PixelFEDInterfacePh1::encfifo1 prettyprintFIFO1( const std::vector<uint32_t>& pFifoVec, const std::vector<uint32_t>& pMarkerVec, std::ostream& os)
{
  PixelFEDInterfacePh1::encfifo1 r;
    os << "----------------------------------------------------------------------------------" << std::endl;
    for (uint32_t cIndex = 0; cIndex < pFifoVec.size(); cIndex++ )
    {
        if (pMarkerVec.at(cIndex) == 8)
        {
            // Event Header
            os << std::dec << "    Header: " << "CH: " << ( (pFifoVec.at(cIndex) >> 26) & 0x3f ) << " ID: " <<  ( (pFifoVec.at(cIndex) >> 21) & 0x1f ) << " TBM_H: " <<  ( (pFifoVec.at(cIndex) >> 9) & 0xff ) << " EVT Nr: " <<  ( (pFifoVec.at(cIndex)) & 0xff )  << std::endl;
	    //assert(!r.found);
	    r.found = true;
	    r.event = pFifoVec.at(cIndex) & 0xff;
        }

        if (pMarkerVec.at(cIndex) == 12)
        {
            os << std::dec << "ROC Header: " << "CH: " << ( (pFifoVec.at(cIndex) >> 26) & 0x3f  ) << " ROC Nr: " <<  ( (pFifoVec.at(cIndex) >> 21) & 0x1f ) << " Status: " << (  (pFifoVec.at(cIndex)) & 0xff )  << std::endl;
        }

        if (pMarkerVec.at(cIndex) == 1)
        {
	  PixelFEDInterfacePh1::encfifo1hit h;
	  h.ch = (pFifoVec.at(cIndex) >> 26) & 0x3f;
	  if (h.ch == 45) h.ch = 33;
	  if (h.ch == 46) h.ch = 34;
	  h.roc = (pFifoVec.at(cIndex) >> 21) & 0x1f;
	  h.dcol = (pFifoVec.at(cIndex) >> 16) & 0x1f;
	  h.pxl = (pFifoVec.at(cIndex) >> 8) & 0xff;
	  h.ph = (pFifoVec.at(cIndex)) & 0xff;
	  r.hits.push_back(h);
            os  << std::dec << "            CH: " << ( (pFifoVec.at(cIndex) >> 26) & 0x3f ) << " ROC Nr: " <<  ( (pFifoVec.at(cIndex) >> 21) & 0x1f ) << " DC: " <<  ( (pFifoVec.at(cIndex) >> 16) & 0x1f ) << " PXL: " <<  ( (pFifoVec.at(cIndex) >> 8) & 0xff ) <<  " PH: " <<  ( (pFifoVec.at(cIndex)) & 0xff ) << std::endl;
        }

        if (pMarkerVec.at(cIndex) == 4)
        {
            // TBM Trailer
            os << std::dec << "   Trailer: " << "CH: " << ( (pFifoVec.at(cIndex) >> 26) & 0x3f ) << " ID: " <<  ( (pFifoVec.at(cIndex) >> 21) & 0x1f ) << " TBM_T2: " <<  ( (pFifoVec.at(cIndex) >> 12) & 0xff ) << " TBM_T1: " <<  ( (pFifoVec.at(cIndex)) & 0xff )  << std::endl;
        }

        if (pMarkerVec.at(cIndex) == 6)
        {
            // Event Trailer
            os << std::dec << "Event Trailer: " << "CH: " << ( (pFifoVec.at(cIndex) >> 26) & 0x3f ) << " ID: " <<  ( (pFifoVec.at(cIndex) >> 21) & 0x1f ) << " marker: " <<  ( (pFifoVec.at(cIndex)) & 0x1fffff )  << std::endl;
        }
    }
    os << "----------------------------------------------------------------------------------" << std::endl;
return r;
}

PixelFEDInterfacePh1::digfifo1 PixelFEDInterfacePh1::readFIFO1() {
  digfifo1 df;
  df.cFifo1A  = regManager->ReadBlockRegValue("fifo.spy_1_A", 2048);
  df.cMarkerA = regManager->ReadBlockRegValue("fifo.spy_1_A_marker", 2048);
  df.cFifo1B  = regManager->ReadBlockRegValue("fifo.spy_1_B", 2048);
  df.cMarkerB = regManager->ReadBlockRegValue("fifo.spy_1_B_marker", 2048);

  std::cout << std::endl <<  "FIFO 1 Channel A: " << std::endl;
  df.a = prettyprintFIFO1(df.cFifo1A, df.cMarkerA, std::cout);
  std::cout << std::endl << "FIFO 1 Channel B: " << std::endl;
  df.b = prettyprintFIFO1(df.cFifo1B, df.cMarkerB, std::cout);
  //  assert(df.a.event == 0 || df.b.event == 0 || df.a.event == df.b.event);

  return df;
}

int PixelFEDInterfacePh1::drainFifo1(uint32_t *data) {
  return 0;
}

int PixelFEDInterfacePh1::drainErrorFifo(uint32_t *data) {
  return 0;
}

int PixelFEDInterfacePh1::drainTemperatureFifo(uint32_t* data) {
  return 0;
}

int PixelFEDInterfacePh1::drainTTSFifo(uint32_t *data) {
  return 0;
}

void PixelFEDInterfacePh1::SelectDaqDDR( uint32_t pNthAcq )
{
    fStrDDR  = ( ( pNthAcq % 2 + 1 ) == 1 ? "DDR0" : "DDR1" );
    fStrDDRControl = ( ( pNthAcq % 2 + 1 ) == 1 ? "pixfed_ctrl_regs.DDR0_ctrl_sel" : "pixfed_ctrl_regs.DDR1_ctrl_sel" );
    fStrFull = ( ( pNthAcq % 2 + 1 ) == 1 ? "pixfed_stat_regs.DDR0_full" : "pixfed_stat_regs.DDR1_full" );
    fStrReadout = ( ( pNthAcq % 2 + 1 ) == 1 ? "pixfed_ctrl_regs.DDR0_end_readout" : "pixfed_ctrl_regs.DDR1_end_readout" );
}

void prettyprintTBMFIFO(const std::vector<uint32_t>& pData )
{
  std::cout << "Global TBM Readout FIFO: size " << pData.size() << std::endl;
    //now I need to do something with the Data that I read into cData
    int cIndex = 0;
    uint32_t cPreviousWord = 0;
    for ( size_t i = 0; i < pData.size(); ++i)
    {
      uint32_t cWord = pData[i];
      std::cout << std::setw(5) << i << ": " << std::hex << std::setw(8) << cWord << std::dec << std::endl;
        if (cIndex % 2 == 0)
            cPreviousWord = cWord;

        else if (cPreviousWord == 0x1)
        {
            //std::cout << cWord <<  std::endl;
            std::cout << "    Pixel Hit: CH: " << ((cWord >> 26) & 0x3f) << " ROC: " << ((cWord >> 21) & 0x1f) << " DC: " << ((cWord >> 16) & 0x1f) << " ROW: " << ((cWord >> 8) & 0xff) << " PH: " << (cWord & 0xff) << std::dec << std::endl;
        }
        //else if (cPreviousWord == 0x6)
        //{
        ////std::cout << cWord <<  std::endl;
        //std::cout << "Event Trailer: CH: " << ((cWord >> 26) & 0x3f) << " ID: " << ((cWord >> 21) & 0x1f) << " marker: " << (cWord & 0x1fffff) << " ROW: " << ((cWord >> 8) & 0xff) << " PH: " << (cWord & 0xff) << std::dec << std::endl;
        //}
        else if (cPreviousWord == 0x8)
        {
            //std::cout << cWord <<  std::endl;
            std::cout << "Event Header: CH: " << ((cWord >> 26) & 0x3f) << " ID: " << ((cWord >> 21) & 0x1f) << " TBM H: " << ((cWord >> 16) & 0x1f) << " ROW: " << ((cWord >> 9) & 0xff) << " EventNumber: " << (cWord & 0xff) << std::dec << std::endl;
        }
        else if (cPreviousWord == 0xC)
        {
            //std::cout << cWord <<  std::endl;
            std::cout << " ROC Header: CH: " << ((cWord >> 26) & 0x3f) << " ROC Nr: " << ((cWord >> 21) & 0x1f) << " Status : " << (cWord  & 0xff) << std::dec << std::endl;
        }
        else if (cPreviousWord == 0x4)
        {
            //std::cout << cWord <<  std::endl;
            std::cout << " TBM Trailer: CH: " << ((cWord >> 26) & 0x3f) << " ID: " << ((cWord >> 21) & 0x1f) << " TBM T2: " << ((cWord >> 12) & 0xff) << " TBM_T1: " << (cWord & 0xff) << std::dec << std::endl;
        }
        cIndex++;
    }
}

std::vector<uint32_t> PixelFEDInterfacePh1::ReadData(uint32_t cBlockSize)
{
    //    std::cout << "JJJ READ DATA " << cBlockSize << std::endl;
    //std::chrono::milliseconds cWait( 10 );
    // the fNthAcq variable is automatically used to determine which DDR FIFO to read - so it has to be incremented in this method!

    // first find which DDR bank to read
    SelectDaqDDR( fNthAcq );
    std::cout << "Querying " << fStrDDR << " for FULL condition!" << std::endl;

    uhal::ValWord<uint32_t> cVal;
    int tries = 0;
    int maxtries = 100;
    do
    {
        cVal = regManager->ReadReg( fStrFull );
        if ( cVal == 0 && tries++ < maxtries ) usleep(1000);
    }
    while ( cVal == 0 && tries < maxtries);
    std::cout << fStrDDR << " full: " << regManager->ReadReg( fStrFull ) << " after " << tries << " tries " << std::endl;

    // DDR control: 0 = ipbus, 1 = user
    regManager->WriteReg( fStrDDRControl, 0 );
    usleep(10000);
    std::cout << "Starting block read of " << fStrDDR << std::endl;

    std::vector<uint32_t> cData;
    for (int i = 0; i < 5; ++i) {
      std::vector<uint32_t> tmp = regManager->ReadBlockRegValue( fStrDDR, cBlockSize );
      cData.insert(cData.end(), tmp.begin(), tmp.end());
    }

    regManager->WriteReg( fStrDDRControl , 1 );
    usleep(10000);
    regManager->WriteReg( fStrReadout, 1 );
    usleep(10000);

    // full handshake between SW & FW
    while ( regManager->ReadReg( fStrFull ) == 1 )
      usleep(10000);
    regManager->WriteReg( fStrReadout, 0 );

    //prettyprintTBMFIFO(cData);
    fNthAcq++;
    return cData;
}


int PixelFEDInterfacePh1::spySlink64(uint64_t *data) {
  ++slink64calls;
  std::cout << "slink64call #" << slink64calls << std::endl;

  //  usleep(1000000);
  //sleep(10);
  readSpyFIFO();
  PixelFEDInterfacePh1::digfifo1 f = readFIFO1();
  data[0] = 0x5000000000000000;
  data[0] |= uint64_t(f.a.event & 0xffffff) << 32;
  data[0] |= uint64_t(41) << 8;
  size_t j = 1;
  for (size_t i = 0; i < f.a.hits.size(); ++i, ++j) {
    encfifo1hit h = f.a.hits[i];
    data[j] = (h.ch << 26) | (h.roc << 21) | (h.dcol << 16) | (h.pxl << 8) | h.ph;
    data[j] |= uint64_t(0x1b) << 53;
  }
  for (size_t i = 0; i < f.b.hits.size(); ++i, ++j) {
    encfifo1hit h = f.b.hits[i];
    data[j] = (h.ch << 26) | (h.roc << 21) | (h.dcol << 16) | (h.pxl << 8) | h.ph;
    data[j] |= uint64_t(0x1b) << 53;
  }
  data[j] = 0xa000000000000000;
  data[j] |= uint64_t((j+1)&0x3fff) << 32;
  ++j;

  std::cout << "my fake fifo3:\n";
  for (size_t i = 0; i < j; ++i)
    std::cout << std::hex << "0x" << std::setw(8) << data[i] << std::endl;

  regManager->WriteReg("fe_ctrl_regs.decode_reg_reset", 1);
  usleep(1000);
  regManager->WriteReg("pixfed_ctrl_regs.PC_CONFIG_OK",0);
  usleep(1000);
  regManager->WriteReg("pixfed_ctrl_regs.PC_CONFIG_OK",1);
  usleep(1000);

  //std::vector<uint32_t> cData = ReadData(1024);
  //cData = ReadData(1024);
  return int(j);
}

bool PixelFEDInterfacePh1::isWholeEvent(uint32_t nTries) {
  return false;
}

bool PixelFEDInterfacePh1::isNewEvent(uint32_t nTries) {
  return false;
}

int PixelFEDInterfacePh1::enableSpyMemory(const int enable) {
  // card.modeRegister ?
  return setModeRegister(card.modeRegister);
}

uint32_t PixelFEDInterfacePh1::get_VMEFirmwareDate() {
  uint32_t iwrdat=0;
  // read here
  cout<<"FEDID:"<<card.fedNumber<<" VME FPGA (update via jtag pins only) firmware date d/m/y "
      <<dec<<((iwrdat&0xff000000)>>24)<<"/"
      <<dec<<((iwrdat&0xff0000)>>16)<<"/"
      <<dec<<((((iwrdat&0xff00)>>8)*100)+(iwrdat&0xff))<<endl;
  return iwrdat;
}

uint32_t PixelFEDInterfacePh1::get_FirmwareDate(int chip) {
  uint32_t iwrdat=0;
  if(chip != 1) return 0;
  //read here
  cout<<"FEDID:"<<card.fedNumber<<" FPGA firmware date d/m/y "
      <<dec<<((iwrdat&0xff000000)>>24)<<"/"
      <<dec<<((iwrdat&0xff0000)>>16)<<"/"
      <<dec<<((((iwrdat&0xff00)>>8)*100)+(iwrdat&0xff))<<endl;
  return iwrdat;
}

bool PixelFEDInterfacePh1::loadFedIDRegister() {
  if(Printlevel&1)cout<<"Load FEDID register from DB 0x"<<hex<<card.fedNumber<<dec<<endl;
  return setFedIDRegister(card.fedNumber);
}

bool PixelFEDInterfacePh1::setFedIDRegister(const uint32_t value) {
  cout<<"Set FEDID register "<<hex<<value<<dec<<endl;
  // write here
  uint32_t got = getFedIDRegister();
  if (value != got) cout<<"soft FEDID = "<<value<<" doesn't match hard board FEDID = "<<got<<endl;
  return value == got;
}

uint32_t PixelFEDInterfacePh1::getFedIDRegister() {
  return 0;
}

bool PixelFEDInterfacePh1::loadControlRegister() {
  if(Printlevel&1)cout<<"FEDID:"<<card.fedNumber<<" Load Control register from DB 0x"<<hex<<card.Ccntrl<<dec<<endl;
  return setControlRegister(card.Ccntrl);
}

bool PixelFEDInterfacePh1::setControlRegister(uint32_t value) {
  if(Printlevel&1)cout<<"FEDID:"<<card.fedNumber<<" Set Control register "<<hex<<value<<dec<<endl;
  // write here
  card.Ccntrl=value; // stored this value   
  return false;
}

uint32_t PixelFEDInterfacePh1::getControlRegister() {
  return 0;
}

bool PixelFEDInterfacePh1::loadModeRegister() {
  if(Printlevel&1)cout<<"FEDID:"<<card.fedNumber<<" Load Mode register from DB 0x"<<hex<<card.Ccntrl<<dec<<endl;
  return setModeRegister(card.modeRegister);
}

bool PixelFEDInterfacePh1::setModeRegister(uint32_t value) {
  if(Printlevel&1)cout<<"FEDID:"<<card.fedNumber<<" Set Mode register "<<hex<<value<<dec<<endl;
  // write here
  card.modeRegister=value; // stored this value   
  return false;
}

uint32_t PixelFEDInterfacePh1::getModeRegister() {
  return 0;
}

void PixelFEDInterfacePh1::set_PrivateWord(uint32_t pword) {
}

void PixelFEDInterfacePh1::resetSlink() {
}

bool PixelFEDInterfacePh1::checkFEDChannelSEU() {
  /*
    Check to see if the channels that are currently on match what we expect. If not
    increment the counter and return true. Note that this assumes that the method won't
    be called again until the SEU is fixed. Otherwise, the counter will be incremented multiple
    times for the same SEU.
  */
  bool foundSEU = false;
  uint64_t enbable_current_i = 0;
  // read here
  enbable_t enbable_current(enbable_current_i);

  // Note: since N_enbable_expected is bitset<9>, this only compares the first 9 bits
  if (enbable_current != enbable_expected && enbable_current != enbable_last) {
    foundSEU = true;
    cout << "Detected FEDChannel SEU in FED " << card.fedNumber << endl;
    cout << "Expected " << enbable_expected << " Found " << enbable_current << " Last " << enbable_last << endl;
    incrementSEUCountersFromEnbableBits(num_SEU, enbable_current, enbable_last);
  }

  enbable_last = enbable_current;

  return foundSEU;
}

void PixelFEDInterfacePh1::incrementSEUCountersFromEnbableBits(vector<int> &counter, enbable_t current, enbable_t last) {
  for(size_t i = 0; i < current.size(); i++)
    if (current[i] != last[i])
      counter[i]++;
}

bool PixelFEDInterfacePh1::checkSEUCounters(int threshold) {
  /*
    Check to see if any of the channels have more than threshold SEUs.
    If so, return true and set expected enbable bit for that channel
    to off.
    Otherwise, return false
  */
  bool return_val = false;
  cout << "Checking for more than " << threshold << " SEUs in FED " << card.fedNumber << endl;
  cout << "Channels with too many SEUs: ";
  for (size_t i=0; i<48; i++)
    {
      if (num_SEU[i] >= threshold) {
        enbable_expected[i] = 1; // 1 is off
        cout << " " << 1+i << "(" << num_SEU[i] << ")";
        return_val = true;
      }
    }
  if (return_val) {
    cout << ". Disabling." << endl;
    cout << "Setting runDegraded flag for FED " << card.fedNumber << endl;
    runDegraded_ = true;
  } else cout << endl;
  return return_val;
}

void PixelFEDInterfacePh1::resetEnbableBits() {
  // Get the current values of higher bits in these registers, so we can leave them alone

  //uint64_t OtherConfigBits = 0;
  //uint64_t enbable_exp = 0; //enbable_expected.to_ullong(); 
  //for (int i = 0; i < 48; ++i)
  //  if (enbable_expected[i])
  //    enbable_exp |= 1<<i;
  //uint64_t write = OtherConfigBits | enbable_exp;
  //  write;
}

void PixelFEDInterfacePh1::storeEnbableBits() {
  enbable_expected = masks_to_enbable(card.cntrl_1, card.cntrl_2, card.cntrl_3);
  enbable_last = enbable_expected;
}

void PixelFEDInterfacePh1::resetSEUCountAndDegradeState(void) {
  cout << "reset SEU counters and the runDegrade flag " << endl;
  // reset the state back to running 
  runDegraded_ = false;
  // clear the count flag
  num_SEU.assign(96, 0);
  // reset the expected state to default
  storeEnbableBits();
}

void PixelFEDInterfacePh1::testTTSbits(uint32_t data, int enable) {
  //will turn on the test bits indicate in bits 0...3 as long as bit 31 is set
  //As of this writing, the bits indicated are: 0(Warn), 1(OOS), 2(Busy), 4(Ready)
  //Use a 1 or any >1 to enable, a 0 or <0 to disable
  if (enable>0)
    data = (data | 0x80000000) & 0x8000000f;
  else
    data = data & 0xf;
  // write here
}

void PixelFEDInterfacePh1::setXY(int X, int Y) {
}

int PixelFEDInterfacePh1::getXYCount() {
  return 0;
}

void PixelFEDInterfacePh1::resetXYCount() {
}

int PixelFEDInterfacePh1::getNumFakeEvents() {
  return 0;
}

void PixelFEDInterfacePh1::resetNumFakeEvents() {
}

uint32_t PixelFEDInterfacePh1::readEventCounter() {
  return 0;
}

uint32_t PixelFEDInterfacePh1::getFifoStatus() {
  return 0;
}

uint32_t PixelFEDInterfacePh1::linkFullFlag() {
  return 0;
}

uint32_t PixelFEDInterfacePh1::numPLLLocks() {
  return 0;
}

uint32_t PixelFEDInterfacePh1::getFifoFillLevel() {
  return 0;
}

uint64_t PixelFEDInterfacePh1::getSkippedChannels() {
  return 0;
}

uint32_t PixelFEDInterfacePh1::getErrorReport(int ch) {
  return 0;
}

uint32_t PixelFEDInterfacePh1::getTimeoutReport(int ch) {
  return 0;
}

int PixelFEDInterfacePh1::TTCRX_I2C_REG_READ(int Register_Nr) {
  return 0;
}

int PixelFEDInterfacePh1::TTCRX_I2C_REG_WRITE(int Register_Nr, int Value) {
  return 0;
}
