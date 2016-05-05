#ifndef _AMC13_INTERFACE_H__
#define _AMC13_INTERFACE_H__

#include "amc13/AMC13.hh"

class PixelAMC13Interface {
 public:
  struct BGO {
    BGO() : fUse(false) {}
    BGO(uint32_t pCommand, bool pSingle, int pPrescale, int pBX)
    : fUse(true),
      fCommand(pCommand),
      isLong(fCommand & 0xFFFFFF00),
      single(pSingle),
      fPrescale(pPrescale),
      fBX(pBX)
    {}

    bool fUse;
    unsigned fCommand;
    bool isLong;
    bool single;
    unsigned fPrescale;
    unsigned fBX;
  };

  PixelAMC13Interface(const std::string& uriT1, const std::string& uriT2);
  PixelAMC13Interface(const std::string& uriT1, const std::string& addressT1, const std::string& uriT2, const std::string& addressT2);
  ~PixelAMC13Interface();

  amc13::AMC13* Get() { return fAMC13; }

  void SetMask(uint32_t v) { fMask = v; }
  void SetMask(const std::string& v) { fMask = fAMC13->parseInputEnableList(v, true); }
  void SetDebugPrints(bool v) { fDebugPrints = v; }
  void SetCalBX(unsigned v) { fCalBX = v; }
  void SetL1ABurstDelay(unsigned v) { fL1ABurstDelay = v; }

  void DoResets();

  void Configure();
  void Halt();
  void Reset();

  void CalSync();
  void LevelOne();
  void ResetTBM();
  void ResetROC();

  uint32_t ClockFreq();

  void ClearL1AHistory();
  void ClearTTCHistory();
  void ClearTTCHistoryFilter();
  uint32_t getTTCHistoryItemAddress( int item);
  void DumpHistory();
  void DumpTriggers();

  void ConfigureBGO(unsigned i, BGO b);
  void FireBGO(unsigned which);

 private:
  amc13::AMC13* fAMC13;
  uint32_t fMask;
  bool fDebugPrints;
  uint32_t fCalBX;
  uint32_t fL1ABurstDelay;
};

#endif
