// cd ../../.. ; make ; cd - ; g++ -std=c++0x -I${BUILD_HOME}/pixel -I/opt/cactus/include version.cc amc13.cc -L${BUILD_HOME}/pixel/lib -lPixeluTCAUtilities -I/opt/xdaq/include -I../../include -L/opt/xdaq/lib -lconfig -lxcept -lxdaq -lxdata -llog4cplus -lxgi -ltoolbox -lxerces-c -lxoap -lcgicc -lmimetic -lasyncresolv -lpeer -luuid -llogxmlappender -llogudpappender -o amc13.exe -O0 && ./amc13.exe
#include <iostream>
#include "PixelUtilities/PixeluTCAUtilities/include/PixelAMC13Interface.h"
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

int main(int argc, char** argv) {
  //  assert(argc > 1);

  RegManagerUhalLogSetter r;

  PixelAMC13Interface a("chtcp-2.0://localhost:10203?target=amc13_T1:50001",
                        "/opt/cactus/etc/amc13/AMC13XG_T1.xml",
                        "chtcp-2.0://localhost:10203?target=amc13_T2:50001",
                        "/opt/cactus/etc/amc13/AMC13XG_T2.xml",
                        "1-12");

  a.SetDebugPrints(1);
  a.SetTTCSimulator(true);
  PixelAMC13Interface::Trigger trig(0, 1, 100, 0);
  //  trig.fEnabled = false;
  a.SetTrigger(trig);
  a.SetBGO(0, PixelAMC13Interface::BGO(0x2c, true, 0, 420));
  a.SetBGO(1, PixelAMC13Interface::BGO(0x14, false, 0, 1));
  a.SetBGO(2, PixelAMC13Interface::BGO(0x1c, false, 0, 1));
  //a.SetBGO(0, PixelAMC13Interface::BGO(0x2, false, 0, 123));

  a.Configure();

  while (1) {
    std::cout << "Press something... ";
    char c;
    std::cin >> c;
    if (c == 'h') {
      //a.getAMC13()->write(amc13::AMC13Simple::T1, "CONF.TTC.ENABLE_INTERNAL_L1A", true);
      a.DumpTriggers();
      a.DumpHistory();
      //      a.getAMC13()->write(amc13::AMC13Simple::T1, "CONF.TTC.ENABLE_INTERNAL_L1A", false);
    }
    else if (c == 't') {
//      a.StartL1A();
//      usleep(90000);
//      a.StopL1A();
      a.BurstL1A();
    }
    else if (c == 'f') {
      std::cout << "clock freq: " << a.getAMC13()->read(amc13::AMC13Simple::T2, "STATUS.TTC.CLK_FREQ") * 50 << std::endl;
    }
    else if (c >= '1' && c <= '7') {
      unsigned w(c - '0');
      std::cout << "fire bgos " << w << std::endl;
      a.FireBGOs(w);
    }
    else if (c == 'q') {
      break;
    }
  }
}
