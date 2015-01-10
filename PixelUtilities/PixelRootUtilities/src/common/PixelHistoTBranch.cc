#include "PixelUtilities/PixelRootUtilities/include/PixelHistoTBranch.h"
#include <string>
#include <iostream>

using namespace std;

Int_t PixelHistoTBranch::SetEntry(Long64_t entry, unsigned int leafN, float value){
//  string mthn = "[MyTBranch::SetEntry()]\t";
	if (TestBit(kDoNotProcess)) {
     return 0;
  }
  if ((entry >= fEntryNumber)) {
     return 0;
  }
  Int_t nbytes = 0;
  Long64_t first = fBasketEntry[fReadBasket];
  Long64_t last = 0;
  if (fReadBasket == fWriteBasket) {
     last = fEntryNumber - 1;
  } else {
     last = fBasketEntry[fReadBasket+1] - 1;
  }
  // Are we still in the same ReadBasket?
  if ((entry < first) || (entry > last)) {
     fReadBasket = TMath::BinarySearch(fWriteBasket + 1, fBasketEntry, entry);
     first = fBasketEntry[fReadBasket];
  }
  // We have found the basket containing this entry.
  // make sure basket buffers are in memory.
  TBasket* basket = (TBasket*) fBaskets.UncheckedAt(fReadBasket);
  if (!basket) {
     basket = GetBasket(fReadBasket);
     if (!basket) {
        return -1;
     }
  }
  basket->PrepareBasket(entry);
  TBuffer* buf = basket->GetBufferRef();
//  buf->Dump();
	// This test necessary to read very old Root files (NvE).
  if (!buf) {
     TFile* file = GetFile(0);
     basket->ReadBasketBuffers(fBasketSeek[fReadBasket], fBasketBytes[fReadBasket], file);
     buf = basket->GetBufferRef();
  }
  // Set entry offset in buffer and read data from all leaves.
  buf->ResetMap();
  if (!buf->IsReading()) {
     basket->SetReadMode();
  }
  Int_t* entryOffset = basket->GetEntryOffset();
  Int_t bufbegin = 0;
  if (entryOffset) {	   
     bufbegin = entryOffset[entry-first];
  } else {
     bufbegin = basket->GetKeylen() + ((entry - first) * basket->GetNevBufSize());
  }
//	cout << mthn << "BufBegin: " << bufbegin << endl;
  buf->SetBufferOffset(bufbegin);
  Int_t* displacement = basket->GetDisplacement();
  if (displacement) {
     buf->SetBufferDisplacement(displacement[entry-first]);
  } else {
     buf->SetBufferDisplacement();
  }
	float readValue;
  for (Int_t i = 0; i < fNleaves; ++i) {
		if((unsigned int)i==leafN){
	    *buf << value;
			break;
		}
		else{
			*buf >> readValue;
		}
	}
	nbytes = buf->Length() - bufbegin;
	return nbytes;
}

Int_t PixelHistoTBranch::AddToEntry(Long64_t entry, unsigned int leafN, float value){
  string mthn = "[MyTBranch::AddToEntry()]\t";
	if (TestBit(kDoNotProcess)) {
     return 0;
  }
  if ((entry >= fEntryNumber)) {
     return 0;
  }
  Int_t nbytes = 0;
  Long64_t first = fBasketEntry[fReadBasket];
  Long64_t last = 0;
  if (fReadBasket == fWriteBasket) {
     last = fEntryNumber - 1;
  } else {
     last = fBasketEntry[fReadBasket+1] - 1;
  }
  // Are we still in the same ReadBasket?
  if ((entry < first) || (entry > last)) {
     fReadBasket = TMath::BinarySearch(fWriteBasket + 1, fBasketEntry, entry);
     first = fBasketEntry[fReadBasket];
  }
  // We have found the basket containing this entry.
  // make sure basket buffers are in memory.
  TBasket* basket = (TBasket*) fBaskets.UncheckedAt(fReadBasket);
  if (!basket) {
     basket = GetBasket(fReadBasket);
     if (!basket) {
        return -1;
     }
  }
  basket->PrepareBasket(entry);
  TBuffer* buf = basket->GetBufferRef();
//  buf->Dump();
	// This test necessary to read very old Root files (NvE).
  if (!buf) {
     TFile* file = GetFile(0);
     basket->ReadBasketBuffers(fBasketSeek[fReadBasket], fBasketBytes[fReadBasket], file);
     buf = basket->GetBufferRef();
  }
  // Set entry offset in buffer and read data from all leaves.
  buf->ResetMap();
	//CHANGED BY LORE
  if (!buf->IsReading()) {
     basket->SetReadMode();
  }
  Int_t* entryOffset = basket->GetEntryOffset();
  Int_t bufbegin = 0;
  if (entryOffset) {	   
     bufbegin = entryOffset[entry-first];
  } else {
     bufbegin = basket->GetKeylen() + ((entry - first) * basket->GetNevBufSize());
  }
//	cout << mthn << "BufBegin: " << bufbegin << endl;
  buf->SetBufferOffset(bufbegin);
  Int_t* displacement = basket->GetDisplacement();
  if (displacement) {
     buf->SetBufferDisplacement(displacement[entry-first]);
  } else {
     buf->SetBufferDisplacement();
  }
	vector<float> bufferValue;
  for (Int_t i = 0; i < fNleaves; ++i) {
		bufferValue.push_back(1);
		(*buf) >> bufferValue[i];
//		cout << mthn << "i: " << i << " value: " << bufferValue[i] << endl;
		if((unsigned int)i==leafN){
		  break;
		}
	}
//	(*buf) >> bufferValue;

	buf->SetBufferOffset(bufbegin);
  displacement = basket->GetDisplacement();
  if (displacement) {
     buf->SetBufferDisplacement(displacement[entry-first]);
  } else {
     buf->SetBufferDisplacement();
  }
  for (Int_t i = 0; i < fNleaves; ++i) {
		if((unsigned int)i==leafN){
			(*buf) << bufferValue[i] + value;
			break;
		}
		else{
			(*buf) << bufferValue[i];
		}
	}
//	*buf << (bufferValue + value);
  
	nbytes = buf->Length() - bufbegin;
	return nbytes;
}

