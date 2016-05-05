#ifndef _AMC13_INTERFACE_H__
#define _AMC13_INTERFACE_H__

#include "amc13/AMC13.hh"

class PixelAMC13Interface {
 public:
  struct BGO {
    BGO() : fUse(false) {}
    BGO(uint32_t pCommand, bool pRepeat, int pPrescale, int pBX)
    : fUse(true), fCommand(pCommand), fRepeat(pRepeat), fPrescale(pPrescale), fBX(pBX)
    {}
    bool fUse;
    unsigned fCommand;
    bool fRepeat;
    unsigned fPrescale;
    unsigned fBX;
  };

  struct Trigger {
    Trigger() : fEnabled(false) {}
    Trigger(int pMode, uint32_t pBurst, uint32_t pRate, int pRules)
    : fEnabled(true), fMode(pMode), fBurst(pBurst), fRate(pRate), fRules(pRules)
    {}
    bool fEnabled; // whether to set it up
    int fMode; //mode 0 = periodic trigger every rate orbits at BX=500
               //mode 1 = periodic trigger every rate BX
               //mode 2 = random trigger at rate Hz
    uint32_t fBurst;
    uint32_t fRate;
    int fRules;
  };

  uint32_t getTTCHistoryItemAddress( int item);

  PixelAMC13Interface(const std::string& uriT1, const std::string& addressT1, const std::string& uriT2, const std::string& addressT2, const std::string& amcMask);
  ~PixelAMC13Interface();

  amc13::AMC13* getAMC13() { return fAMC13; }
  void SetBGO(size_t i, BGO b);
  BGO GetBGO(size_t i);
  void SetAMCMask(uint32_t v) { fAMCMask = v; }
  void SetDebugPrints(bool v) { fDebugPrints = v; }
  void SetTTCSimulator(bool v) { fSimulate = v; }
  void SetTrigger(Trigger v) { fTrigger = v; }

  void DoResets();

  void Configure();
  void Halt();
  void Reset();

  void StartL1A();
  void StopL1A();
  void BurstL1A();

  void ClearL1AHistory();
  void ClearTTCHistory();
  void ClearTTCHistoryFilter();
  void DumpHistory();
  void DumpTriggers();

  void ConfigureBGO(size_t i, BGO b);
  void FireBGOs(unsigned which);

 private:
  amc13::AMC13* fAMC13;
  uint32_t fAMCMask;
  bool fDebugPrints;

  typedef std::map<size_t, BGO> BGOs;
  BGOs fBGOs;

  Trigger fTrigger;
  bool fSimulate;
};

#endif
