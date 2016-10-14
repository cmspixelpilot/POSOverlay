// banger.exe board chtcp-2.0://localhost:10203?target=fed02:50001 $BUILD_HOME/pixel/PixelFEDInterface/dat/address_table.xml read pixfed_stat_regs.ttc.ec0

#include <iostream>
#include <iomanip>
#include "uhal/uhal.hpp"
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

void usage() {
  std::cerr << "Usage: banger.exe uri addr_table command additional_args\n"
            << "E.g.:\n"
            << "readaddr and writeaddr commands (addr_table can then be a dummy one, a good one is ${BUILD_HOME}/pixel/BPixelTools/utca/address_table.xml)\n"
            << "       banger.exe uri addr_table command readaddr address_in_hex\n"
            << "       banger.exe uri addr_table command writeaddr address_in_hex value_to_write\n"
            << "read and write commands (addr_table must have node_str defined in it)\n"
            << "       banger.exe uri addr_table command read node_str\n"
            << "       banger.exe uri addr_table command write node_str value_to_write\n"
            << "       banger.exe uri addr_table command readstr node_str\n";
  exit(1);
}

int main(int argc, char** argv) {
  uhal::setLogLevelTo(uhal::Error());

  if (argc < 5)
    usage();

  const std::string uri = argv[1];
  const std::string addr_table = std::string("file://") + argv[2];
  
  const std::string cmd = argv[3];
  const std::string node = argv[4];

  RegManager rm("board", uri, addr_table);

  if (cmd == "readaddr") {
    uint32_t addr = strtoul(node.c_str(), 0, 16);
    uint32_t val = rm.Read(addr);
    std::cout << "value @ 0x" << std::hex << addr << ": 0x" << val << std::dec << " = " << val << std::endl;
    std::cout << val << std::endl;
  }
  else if (cmd == "writeaddr") {
    if (argc < 6) usage();
    uint32_t addr = strtoul(node.c_str(), 0, 16);
    uint32_t val = strtoul(argv[5], 0, 16);
    std::cout << "value @ 0x" << std::hex << addr << ": 0x" << rm.Write(addr, val) << std::dec << std::endl;
    uint32_t val2 = rm.Read(addr);
    std::cout << "write val @ 0x" << std::hex << addr << ": 0x" << val << " readback 0x" << val2 << std::dec << std::endl;
  }
  else if (cmd == "read") {
    uint32_t val = rm.ReadReg(node);
    std::cout << "value: " << val << " = 0x" << std::hex << val << std::dec << std::endl;
    std::cout << val << std::endl;
  }
  else if (cmd == "write") {
    if (argc < 6) usage();
    uint32_t val = strtoul(argv[5], 0, 16);
    rm.WriteReg(node, val);
    uint32_t val2 = rm.ReadReg(node);
    std::cout << "write val 0x" << std::hex << val << " readback 0x" << val2 << std::dec << std::endl;
  }
  else if (cmd == "readstr")
    std::cout << "value as str: " << rm.ReadRegAsString(node) << std::endl;
}
