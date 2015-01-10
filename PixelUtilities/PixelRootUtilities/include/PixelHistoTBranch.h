#ifndef _PixelHistoTBranch_h_
#define _PixelHistoTBranch_h_

#include <TBranch.h>
#include <TBasket.h>
#include <TMath.h>
#include <vector>

class PixelHistoTBranch : public TBranch{
 public:
	Int_t SetEntry(Long64_t entry = 0, unsigned int leafN = 0, float value = 0);
	Int_t AddToEntry(Long64_t entry = 0, unsigned int leafN = 0, float value = 0);
};

#endif
