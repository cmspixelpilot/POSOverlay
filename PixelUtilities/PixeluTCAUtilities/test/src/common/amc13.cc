// cd ../../.. ; make ; cd - ; g++ -std=c++0x -I${BUILD_HOME}/pixel -I/opt/cactus/include version.cc amc13.cc -L${BUILD_HOME}/pixel/lib -lPixeluTCAUtilities -I/opt/xdaq/include -I../../include -L/opt/xdaq/lib -lconfig -lxcept -lxdaq -lxdata -llog4cplus -lxgi -ltoolbox -lxerces-c -lxoap -lcgicc -lmimetic -lasyncresolv -lpeer -luuid -llogxmlappender -llogudpappender -o amc13.exe -O0 && ./amc13.exe
#include <iostream>
#include "PixelUtilities/PixeluTCAUtilities/include/PixelAMC13Interface.h"
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

void help() {
  std::cout << "c - CalSync\n"
            << "t - LevelOne\n"
            << "1 - ResetTBM\n"
            << "2 - ResetROC\n"
            << "f - dump measured clock frequency\n"
            << "l - dump measured clock frequency 10 times 1 per sec\n"
            << "m mask - Set AMC mask\n"
            << "b bx - Set cal BX (L1A trig is at 500)\n"
            << "d delay - Set L1A burst delay in orbits\n"
            << "r - (re)Configure -- must do after m, b, or d for them to take effect\n"
            << "q - quit\n"
            << "h - this message\n"
            << std::endl;
}

int main(int argc, char** argv) {
  //  assert(argc > 1);

  PixelUhalLogSetter r;

  PixelAMC13Interface a("chtcp-2.0://localhost:10203?target=amc13_T1:50001",
                        "chtcp-2.0://localhost:10203?target=amc13_T2:50001");

  a.SetMask("1-12");
  a.SetDebugPrints(1);
  a.SetCalBX(420);
  a.SetL1ABurstDelay(1000);

  a.Configure();

  help();

  while (1) {
    std::cout << "? ";
    char c;
    std::cin >> c;
    if (c == 'h') {
      a.DumpTriggers();
      a.DumpHistory();
    }
    else if (c == 'c') {
      a.CalSync();
    }
    else if (c == 't') {
      a.LevelOne();
    }
    else if (c == '1') {
      a.ResetTBM();
    }
    else if (c == '2') {
      a.ResetROC();
    }
    else if (c == 'f') {
      a.ClockFreq();
    }
    else if (c == 'l') {
      for (int i = 0; i < 10; ++i) {
        a.ClockFreq();
        sleep(1);
      }
    }
    else if (c == 'm') {
      std::string s;
      std::cin >> s;
      a.SetMask(s);
      std::cout << "YOU MUST MANUALLY RECONFIGURE WITH r" << std::endl;
    }
    else if (c == 'b') {
      unsigned v;
      std::cin >> v;
      a.SetCalBX(v);
      std::cout << "YOU MUST MANUALLY RECONFIGURE WITH r" << std::endl;
    }
    else if (c == 'd') {
      unsigned v;
      std::cin >> v;
      a.SetL1ABurstDelay(v);
      std::cout << "YOU MUST MANUALLY RECONFIGURE WITH r" << std::endl;
    }
    else if (c == 'r') {
      a.Configure();
    }
    else if (c == 'q') {
      break;
    }
    else if (c == 'h') {
      help();
    }
  }
}
