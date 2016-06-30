#include <unistd.h>
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

RegManager::RegManager(const std::string& puHalConfigFileName, const std::string& pBoardId)
  : fUniqueId(pBoardId),
    fVerifyWrites(false),
    fDebugPrints(false),
    fBoard(uhal::ConnectionManager(puHalConfigFileName).getDevice(pBoardId))
{}

RegManager::RegManager(const std::string& pBoardId, const std::string& pURI, const std::string& pAddressTableFn)
  : fUniqueId(pBoardId),
    fVerifyWrites(false),
    fDebugPrints(false),
    fBoard(uhal::ConnectionManager::getDevice(pBoardId, pURI, pAddressTableFn))
{}

uhal::ValWord<uint32_t> RegManager::Read(const uint32_t pAddr) {
  uhal::ValWord<uint32_t> v = fBoard.getClient().read(pAddr);
  fBoard.dispatch();
  return v;
}

uhal::ValWord<uint32_t> RegManager::Read(const uint32_t pAddr, const uint32_t pMask) {
  uhal::ValWord<uint32_t> v = fBoard.getClient().read(pAddr, pMask);
  fBoard.dispatch();
  return v;
}

bool RegManager::Write(const uint32_t pAddr, const uint32_t pVal) {
  fBoard.getClient().write(pAddr, pVal);
  fBoard.dispatch();

  if (fVerifyWrites) {
    uhal::ValWord<uint32_t> reply = fBoard.getClient().read(pAddr);
    fBoard.dispatch();
            
    if (!reply.valid()) {
      std::cout << fUniqueId << " Read invalid for addr " << pAddr << std::endl;
      return false;
    }
    else if (reply.value() == pVal) {
      std::cout << fUniqueId << " Values written correctly !" << " addr 0x" << std::hex << pAddr << " = 0x" << pVal << std::dec << std::endl;
    }
    else {
      std::cout << fUniqueId << " Values written INCORRECTLY !" << " addr 0x" << std::hex << pAddr << " = 0x" << reply.value() << " != " << pVal << std::dec << std::endl;
      return false;
    }
  }

  return true;
}

bool RegManager::Write(const uint32_t pAddr, const uint32_t pVal, const uint32_t pMask) {
  fBoard.getClient().write(pAddr, pVal, pMask);
  fBoard.dispatch();

  if (fVerifyWrites) {
    uhal::ValWord<uint32_t> reply = fBoard.getClient().read(pAddr, pMask);
    fBoard.dispatch();
            
    if (!reply.valid()) {
      std::cout << fUniqueId << " Read invalid for addr " << pAddr << std::endl;
      return false;
    }
    else if (reply.value() == pVal) {
      std::cout << fUniqueId << " Values written correctly !" << " addr 0x" << std::hex << pAddr << " = 0x" << pVal << std::dec << std::endl;
    }
    else {
      std::cout << fUniqueId << " Values written INCORRECTLY !" << " addr 0x" << std::hex << pAddr << " = 0x" << reply.value() << " != " << pVal << std::dec << std::endl;
      return false;
    }
  }

  return true;
}

bool RegManager::WriteReg(const std::string& pRegNode, const uint32_t& pVal) {
  if (fDebugPrints)
    std::cout << fUniqueId << " WriteReg " << pRegNode << " 0x" << std::hex << pVal << std::dec << std::endl;

  fBoard.getNode(pRegNode).write(pVal);
  fBoard.dispatch();

  if (fVerifyWrites) {
    uhal::ValWord<uint32_t> reply = fBoard.getNode(pRegNode).read();
    fBoard.dispatch();
            
    if (!reply.valid()) {
      std::cout << fUniqueId << " Read invalid for node " << pRegNode << std::endl;
      return false;
    }
    else if (reply.value() == pVal) {
      std::cout << fUniqueId << " Values written correctly !" << pRegNode << " = " << pVal << std::endl;
    }
    else {
      std::cout << fUniqueId << " Values written INCORRECTLY !" << pRegNode << " = " << reply.value() << " != " << pVal << std::endl;
      return false;
    }
  }

  return true;
}

bool RegManager::WriteStackReg(const std::vector< std::pair<std::string, uint32_t> >& pVecReg) {
  if (fDebugPrints)
    std::cout << fUniqueId << " WriteStackReg:\n";

  for (size_t v = 0; v < pVecReg.size(); v++) {
    if (fDebugPrints)
      std::cout << "\t" << fUniqueId << pVecReg[v].first << " 0x" << std::hex << pVecReg[v].second << std::dec << std::endl;

    if (pVecReg[v].first == "REGMGR_DISPATCH") {
      fBoard.dispatch();
      usleep(pVecReg[v].second);
    }
    else
      fBoard.getNode(pVecReg[v].first).write(pVecReg[v].second);
  }

  fBoard.dispatch();

  if (fVerifyWrites) {
    size_t cNbOK = 0;
            
    for (size_t v = 0; v < pVecReg.size(); v++) {
      uhal::ValWord<uint32_t> reply = fBoard.getNode(pVecReg[v].first).read();
      fBoard.dispatch();
                
      if (!reply.valid())
	std::cout << fUniqueId << " Read invalid for node " << pVecReg[v].first << std::endl;
      else if (reply.value() == pVecReg[v].second) {
	std::cout << fUniqueId << " Values written correctly !" << pVecReg[v].first << " = " << pVecReg[v].second << std::endl;
	++cNbOK;
      }
      else
	std::cout << fUniqueId << " Values written INCORRECTLY !" << pVecReg[v].first << " = " << reply.value() << " != " << pVecReg[v].second << std::endl;
    }
            
    if (cNbOK == pVecReg.size())
      std::cout << "All " << cNbOK << " values written correctly !" << std::endl;
    else {
      std::cout << "Only " << cNbOK << "/" << pVecReg.size() << " values written correctly !" << std::endl;
      return false;
    } 
  }

  return true;
}
    
bool RegManager::WriteBlockReg(const std::string& pRegNode, const std::vector<uint32_t>& pValues) {
  if (fDebugPrints) {
    std::cout << fUniqueId << " WriteBlockReg " << pRegNode << ":\n";
    for (size_t i = 0; i < pValues.size(); ++i)
      std::cout << "\t" << fUniqueId << " 0x" << std::hex << pValues[i] << std::dec << std::endl;
  }

  fBoard.getNode(pRegNode).writeBlock(pValues);
  fBoard.dispatch();
        
  bool cWriteCorr = true;
  if (fVerifyWrites) {
    int cErrCount = 0;
            
    uhal::ValVector<uint32_t> cBlockRead = fBoard.getNode(pRegNode).readBlock(pValues.size());
    std::cout << fBoard.getNode(pRegNode).getAddress () <<" size " << fBoard.getNode(pRegNode).getSize() << std::endl;
    fBoard.dispatch();
            
    for (size_t i = 0; i != cBlockRead.size(); i++) {
      if (cBlockRead[i] != pValues.at(i)) {
	cWriteCorr = false;
	cErrCount++;
      }
    }
            
    std::cout << "Block Write finished !!\n" << cErrCount << " values failed to write !" << std::endl;
  }
        
  return cWriteCorr;
}
    
bool RegManager::WriteBlockAtAddress(uint32_t uAddr, const std::vector<uint32_t>& pValues, bool bNonInc) {
  if (fDebugPrints) {
    std::cout << fUniqueId << " WriteBlockReg(NonInc=" << bNonInc << ") @ 0x" << std::hex << uAddr << std::dec << ":\n";
    for (size_t jmt = 0; jmt < pValues.size(); ++jmt)
      std::cout << "\t" << fUniqueId << " 0x" << std::hex << pValues[jmt] << std::dec << std::endl;
  }

  fBoard.getClient().writeBlock(uAddr, pValues, bNonInc ? uhal::defs::NON_INCREMENTAL : uhal::defs::INCREMENTAL);
  fBoard.dispatch();
        
  bool cWriteCorr = true;
  if (fVerifyWrites) {
    int cErrCount = 0;
            
    uhal::ValVector<uint32_t> cBlockRead = fBoard.getClient().readBlock(uAddr, pValues.size(), bNonInc ? uhal::defs::NON_INCREMENTAL : uhal::defs::INCREMENTAL);
    fBoard.dispatch();
            
    for (size_t i = 0; i != cBlockRead.size(); i++) {
      if (cBlockRead[i] != pValues.at(i)) {
	cWriteCorr = false;
	cErrCount++;
      }
    }
            
    std::cout << "BlockWriteAtAddress finished !!\n" << cErrCount << " values failed to write !" << std::endl;
  }
        
  return cWriteCorr;
}

uhal::ValWord<uint32_t> RegManager::ReadReg(const std::string& pRegNode) {
  uhal::ValWord<uint32_t> cValRead = fBoard.getNode(pRegNode).read();
  fBoard.dispatch();

  if (fDebugPrints) {
    std::cout << fUniqueId << " ReadReg " << pRegNode << " valid? " << cValRead.valid();
    if (cValRead.valid())
      std::cout << "  0x" << std::hex << cValRead << std::dec;
    std::cout << std::endl;
  }
        
  return cValRead;
}

uint64_t RegManager::ReadRegsAs64(const std::string& node_msb, const std::string& node_lsb) {
  uhal::ValWord<uint32_t> msb = fBoard.getNode(node_msb).read();
  uhal::ValWord<uint32_t> lsb = fBoard.getNode(node_lsb).read();
  fBoard.dispatch();
  return uint64_t(msb.value()) << 32 | lsb.value();
}
    
uhal::ValWord<uint32_t> RegManager::ReadAtAddress(uint32_t uAddr, uint32_t uMask) {
  uhal::ValWord<uint32_t> cValRead = fBoard.getClient().read(uAddr, uMask);
  fBoard.dispatch();

  if (fDebugPrints) {
    std::cout << fUniqueId << " ReadAtAddress @ 0x" << std::hex << uAddr << " mask 0x" << uMask << std::dec << " valid? " << cValRead.valid();
    if (cValRead.valid())
      std::cout << "\t" << fUniqueId << " 0x" << std::hex << cValRead << std::dec;
    std::cout << std::endl;
  }
        
  return cValRead;
}
    
uhal::ValVector<uint32_t> RegManager::ReadBlockReg(const std::string& pRegNode, const uint32_t& pBlockSize) {
  uhal::ValVector<uint32_t> cBlockRead = fBoard.getNode(pRegNode).readBlock(pBlockSize);
  fBoard.dispatch();

  if (fDebugPrints) {
    std::cout << fUniqueId << " ReadBlockReg " << pRegNode << " blocksize " << pBlockSize << " valid? " << cBlockRead.valid() << ":";
    if (cBlockRead.valid())
      for (size_t jmt = 0; jmt < cBlockRead.size(); ++jmt)
	std::cout << "\t" << fUniqueId << " 0x" << std::hex << cBlockRead[jmt] << std::dec << std::endl;
    std::cout << std::endl;
  }
        
  return cBlockRead;
}

std::vector<uint32_t> RegManager::ReadBlockRegValue(const std::string& pRegNode, const uint32_t& pBlocksize) {
  uhal::ValVector<uint32_t> valBlock = ReadBlockReg(pRegNode, pBlocksize);
  //if (valBlock.valid())
  std::vector<uint32_t> vBlock = valBlock.value();
  return vBlock;
}

std::string RegManager::ReadRegAsString(const std::string& pRegNode) {
  uint32_t v = ReadReg(pRegNode).value();
  std::string s;
  for (int i = 0; i < 4; ++i) {
    int off = (3-i)*8;
    s.push_back(char((v & (0xFF << off)) >> off));
  }
  return s;
}
