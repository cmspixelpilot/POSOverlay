// need to fix the Makefile and the stupid version.cc shit
// cd $BUILD_HOME/pixel/PixelUtilities/PixeluTCAUtilities/test/src/common/ ; g++ -I${BUILD_HOME}/pixel -I/opt/cactus/include -L/opt/cactus/lib -lboost_system banger.cc -lcactus_uhal_uhal ../../../src/common/RegManager.cc -o banger.exe ; cd -

#include <iostream>
#include <iomanip>
#include "uhal/uhal.hpp"
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

int main(int argc, char** argv) {
  uhal::setLogLevelTo(uhal::Error());

  assert(argc >= 6);
  const std::string board = argv[1];
  const std::string uri = argv[2];
  const std::string addr_table = argv[3];
  const std::string cmd = argv[4];
  const std::string node = argv[5];

  RegManager rm(board, uri, addr_table);
  //std::cout << "board: " << board << std::endl;
  //std::cout << "uri: " << uri << std::endl;
  //std::cout << "addr_table: " << addr_table << std::endl;
  //std::cout << "cmd: " << cmd << std::endl;
  //std::cout << "node: " << node << std::endl;

  if (cmd == "read")
    std::cout << "value: 0x" << std::hex << rm.ReadReg(node) << std::dec << std::endl;
  else if (cmd == "readstr")
    std::cout << "value as str: " << rm.ReadRegAsString(node) << std::endl;
  else if (cmd == "write") {
    assert(argc >= 7);
    uint32_t val = strtoul(argv[6], 0, 16);
    rm.WriteReg(node, val);
    uint32_t val2 = rm.ReadReg(node);
    std::cout << "write val 0x" << std::hex << val << " readback 0x" << val2 << std::dec << std::endl;
  }
      
}
