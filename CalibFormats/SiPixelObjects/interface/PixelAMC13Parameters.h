#ifndef PixelAMC13Parameters_h
#define PixelAMC13Parameters_h

namespace pos {
  class PixelAMC13Parameters {
  public:
    PixelAMC13Parameters() {};
    PixelAMC13Parameters(int crate,
                         const std::string& uriT1, const std::string& uriT2,
                         const std::string& addressT1, const std::string& addressT2,
                         const std::string slotMask,
                         unsigned calBX,
                         unsigned L1ADelay,
                         bool newWay,
                         bool verifyL1A)
      : crate_(crate),
        uriT1_(uriT1),
        uriT2_(uriT2),
        addressT1_(addressT1),
        addressT2_(addressT2),
        slotMask_(slotMask),
        calBX_(calBX),
        L1ADelay_(L1ADelay),
        newWay_(newWay),
        verifyL1A_(verifyL1A)
      {}

    int getCrate() const { return crate_; }
    const std::string& getUriT1() const { return uriT1_; }
    const std::string& getUriT2() const { return uriT2_; }
    const std::string& getAddressT1() const { return addressT1_; }
    const std::string& getAddressT2() const { return addressT2_; }
    const std::string& getSlotMask() const { return slotMask_; }
    unsigned getCalBX() const { return calBX_; }
    unsigned getL1ADelay() const { return L1ADelay_; }
    bool getNewWay() const { return newWay_; }
    bool getVerifyL1A() const { return verifyL1A_; }

    void setCrate(int x) { crate_ = x; }
    void setUriT1(const std::string& x) { uriT1_ = x; }
    void setUriT2(const std::string& x) { uriT2_ = x; }
    void setAddressT1(const std::string& x) { addressT1_ = x; }
    void setAddressT2(const std::string& x) { addressT2_ = x; }
    void setSlotMask(const std::string& x) { slotMask_ = x; }
    void setCalBX(unsigned x) { calBX_ = x; }
    void setL1ADelay(unsigned x) { L1ADelay_ = x; }
    void setNewWay(bool x) { newWay_ = x; }
    void setVerifyL1A(bool x) { verifyL1A_ = x; }

  private:
    int crate_;
    std::string uriT1_, uriT2_;
    std::string addressT1_, addressT2_;
    std::string slotMask_; // converted to uint downstream
    unsigned calBX_;
    unsigned L1ADelay_;
    bool newWay_;
    bool verifyL1A_;
  };
}

#endif
