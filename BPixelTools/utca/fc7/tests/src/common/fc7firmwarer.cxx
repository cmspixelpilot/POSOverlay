#include <time.h>
#include <unistd.h>
#include <iostream>
#include <boost/program_options.hpp>
#include "uhal/uhal.hpp"
#include "uhal/log/log.hpp"
#include "fc7/Firmware.hpp"
#include "fc7/MmcPipeInterface.hpp"

class BitstreamWriter {
public:
  BitstreamWriter(boost::program_options::variables_map lVariablesMap) {
    if (lVariablesMap.count("dumpfn")) {
      to_file = true;
      fn = lVariablesMap["dumpfn"].as<std::string>();
    }
    else
      to_file = false;
  }

  void write_to_file(const std::vector<uint8_t>& bs) {
    FILE* fw_f = fopen(fn.c_str(), "wb");
    assert(fw_f);
    for (size_t i = 0, ie = bs.size(); i < ie; ++i)
      fwrite(&bs[i], 1, 1, fw_f);
    fclose(fw_f);
  }

  void write_to_stdout(const std::vector<uint8_t>& bs) {
    assert(bs.size() % 2 == 0);
    std::cout << std::hex << std::setfill('0');
    for (size_t i = 0, ie = bs.size(); i < ie; i += 2) {
      std::cout << std::setw(2) << +bs[i]
                << std::setw(2) << +bs[i+1] << " ";
      if (i % 64 == 0)
        std::cout << "\n";
    }
    std::cout << std::setfill(' ') << std::dec << std::endl;
  }

  void write(const std::vector<uint8_t>& bs) {
    if (to_file)
      write_to_file(bs);
    else
      write_to_stdout(bs);
  }

private:
  bool to_file;
  std::string fn;
};

int main(int argc, char** argv) {
  uhal::setLogLevelTo(uhal::Warning());

  boost::program_options::variables_map lVariablesMap;
  {
    boost::program_options::options_description lOptions ( "Commandline parameters" );
    lOptions.add_options()
      ( "help,h", "Produce this help message" )
      ( "ip,i", boost::program_options::value<std::string>() , "IP address of target board -- required" )
      ( "switch,s", boost::program_options::value<std::string>() , "Reboot using image on SD card" )
      ( "menu,m", boost::program_options::value<std::string>()->implicit_value("yes")->default_value("") , "Reboot using image on SD card with interactive menu" )
      ( "dump,d", boost::program_options::value<std::string>(), "Dump file from sdcard" )
      ( "dumpbin,b", boost::program_options::value<std::string>(), "Dump local bin file" )
      ( "dumpbit,t", boost::program_options::value<std::string>(), "Dump local bit file" )
      ( "dumpbitswapped", boost::program_options::value<std::string>(), "Dump local bit file, swapped" )
      ( "dumpfn", boost::program_options::value<std::string>(), "Dump output file (above dumps default to stdout)" );

    try {
      boost::program_options::store ( boost::program_options::parse_command_line ( argc, argv, lOptions ), lVariablesMap );
      boost::program_options::notify ( lVariablesMap );

      if (lVariablesMap.count("help")) {
        uhal::log ( uhal::Info , "Usage: " , argv[0] , " [OPTIONS]" );
        uhal::log ( uhal::Info , lOptions );
        return 0;
      }
    }
    catch ( std::exception& e ) {
      uhal::log ( uhal::Error , "Error: " , uhal::Quote ( e.what() ) );
      uhal::log ( uhal::Error , "Usage: " , argv[0] , " [OPTIONS]" );
      uhal::log ( uhal::Error , lOptions );
      return 1;
    }
  }


  std::string lPassword("RuleBritannia");

  bool using_board = false;
  std::auto_ptr<uhal::HwInterface> lBoard;
  std::auto_ptr<fc7::MmcPipeInterface> lNode;
  std::vector<std::string> lFilesOnSD;
  BitstreamWriter bw(lVariablesMap);

  if (lVariablesMap.count("ip")) {
    using_board = true;
    std::string ip = lVariablesMap["ip"].as<std::string>();
    std::cout << "Connecting to " << ip << std::endl;
    std::string lURI("ipbusudp-2.0://" + ip + ":50001");
    lBoard.reset(new uhal::HwInterface(uhal::ConnectionManager::getDevice("Board", lURI, "file://fc7_mmc_interface.xml")));
    lNode.reset(new fc7::MmcPipeInterface(lBoard->getNode<fc7::MmcPipeInterface>("buf_test")));

    std::cout << "List of files on sd card:\n";
    lFilesOnSD = lNode->ListFilesOnSD();
    for (size_t i = 0, ie = lFilesOnSD.size(); i < ie; ++i)
      std::cout << std::setw(2) << i << ") " << lFilesOnSD[i] << std::endl;
  }

  if (lVariablesMap.count("switch")) {
    if (!using_board) {
      uhal::log(uhal::Error, "Error: --switch needs --ip");
      return 1;
    }

    std::string switch_to = lVariablesMap["switch"].as<std::string>();
    std::cout << "Switch to image " << switch_to << "; two or three timeouts normal next" << std::endl;

    if (std::find(lFilesOnSD.begin(), lFilesOnSD.end(), switch_to) == lFilesOnSD.end()) {
      uhal::log ( uhal::Error , "Error: didn't find file on sd card" );
      return 1;
    }

    lNode->RebootFPGA(switch_to, lPassword);
  }
  else if (lVariablesMap.count("menu") && lVariablesMap["menu"].as<std::string>() != "") {
    
    if (!using_board) {
      uhal::log(uhal::Error, "Error: --menu needs --ip");
      return 1;
    }

    std::cout << "Interactive menu mode: switch to which?\n";
    int which = -1;
    std::cin >> which;
    if (which >= 0 && which < int(lFilesOnSD.size())) {
      std::string switch_to = lFilesOnSD[which];
      std::cout << "Switch to image " << switch_to << "; two or three timeouts normal next" << std::endl;
      lNode->RebootFPGA(switch_to, lPassword);
    }
  }
  else if (lVariablesMap.count("dump")) {
    if (!using_board) {
      uhal::log(uhal::Error, "Error: --dump needs --ip");
      return 1;
    }

    std::string to_dump = lVariablesMap["dump"].as<std::string>();
    std::cout << "Dump image " << to_dump << " from sd card" << std::endl;
    if (std::find(lFilesOnSD.begin(), lFilesOnSD.end(), to_dump) == lFilesOnSD.end()) {
      uhal::log(uhal::Error, "Error: didn't find file on sd card" );
      return 1;
    }
        
    fc7::XilinxBitStream dump = lNode->FileFromSD(to_dump);
    std::cout << dump;
    bw.write(dump.Bitstream());
  }
  else if (lVariablesMap.count("dumpbin")) {
    std::string to_dump = lVariablesMap["dumpbin"].as<std::string>();
    std::cout << "Dump image " << to_dump << " from file" << std::endl;

    fc7::XilinxBinFile dump(to_dump);
    std::cout << dump;
    bw.write(dump.Bitstream());
  }
  else if (lVariablesMap.count("dumpbit")) {
    std::string to_dump = lVariablesMap["dumpbit"].as<std::string>();
    std::cout << "Dump image " << to_dump << " from file" << std::endl;

    fc7::XilinxBitFile dump(to_dump);
    std::cout << dump;
    bw.write(dump.Bitstream());
  }
  else if (lVariablesMap.count("dumpbitswapped")) {
    std::string to_dump = lVariablesMap["dumpbitswapped"].as<std::string>();
    std::cout << "Dump image " << to_dump << " from file" << std::endl;

    fc7::XilinxBitFile dump(to_dump);
    dump.BitSwap();
    std::cout << dump;
    bw.write(dump.Bitstream());
  }
}

