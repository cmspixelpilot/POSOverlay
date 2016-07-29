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
            << "clrttch - clear TTC history\n"
            << "clrttcf - clear TTC history filters\n"
            << "ec0 - send ec0\n"
            << "oc0 - send oc0\n"
            << "r - (re)Configure -- must do after m, b, or d for them to take effect\n"
            << "q - quit\n"
            << "h - this message\n"
            << std::endl;
}

int main(int argc, char** argv) {
  
  PixelUhalLogSetter r;

  PixelAMC13Interface a("chtcp-2.0://localhost:10203?target=amc13_T1:50001",
                        "/opt/cactus/etc/amc13/AMC13XG_T1.xml",
                        "chtcp-2.0://localhost:10203?target=amc13_T2:50001",
                        "/opt/cactus/etc/amc13/AMC13XG_T2.xml"
                        );

  a.SetDebugPrints(1);

  help();

  while (1) {
    std::cout << "? ";
    std::string c;
    std::cin >> c;
    if (c == "h") {
      a.DumpTriggers();
      a.DumpTTCHistory();
    }
    else if (c == "c") {
      a.CalSync();
    }
    else if (c == "t") {
      a.LevelOne();
    }
    else if (c == "1") {
      a.ResetTBM();
    }
    else if (c == "2") {
      a.ResetROC();
    }
    else if (c == "f") {
      a.GetClockFreq();
    }
    else if (c == "l") {
      for (int i = 0; i < 10; ++i) {
        a.GetClockFreq();
        sleep(1);
      }
    }
    else if (c == "m") {
      std::string s;
      std::cin >> s;
      a.SetMask(s);
      std::cout << "YOU MUST MANUALLY RECONFIGURE WITH r" << std::endl;
    }
    else if (c == "b") {
      unsigned v;
      std::cin >> v;
      a.SetCalBX(v);
      std::cout << "YOU MUST MANUALLY RECONFIGURE WITH r" << std::endl;
    }
    else if (c == "d") {
      unsigned v;
      std::cin >> v;
      a.SetL1ADelay(v);
      std::cout << "YOU MUST MANUALLY RECONFIGURE WITH r" << std::endl;
    }
    else if (c == "clrttch") {
      a.ClearTTCHistory();
    }
    else if (c == "clrttcf") {
      a.ClearTTCHistoryFilter();
    }
    else if (c == "ec0") {
      a.Get()->sendLocalEvnOrnReset(true, false);
    }
    else if (c == "oc0") {
      a.Get()->sendLocalEvnOrnReset(false, true);
    }
    else if (c == "r") {
      a.Configure();
    }
    else if (c == "q") {
      break;
    }
    else if (c == "h") {
      help();
    }
  }
}
