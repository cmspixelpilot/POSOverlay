// mostly by Nicolas Pierre

#ifndef PixelUtilities_PixeluTCAUtilities_RegManager_h
#define PixelUtilities_PixeluTCAUtilities_RegManager_h

#include <uhal/uhal.hpp>
#include "PixelUtilities/PixeluTCAUtilities/include/PixelUhalLogSetter.h"

/*!
 * \class RegManager
 * \brief Permit connection to given boards and r/w given registers
 */
class RegManager {
 private:
  PixelUhalLogSetter sLogSetter; /*!< whether uhal logging configured yet*/

 protected:
  std::string fUniqueId; /*!< used in prints*/
  bool fVerifyWrites; /*!< whether to verify writes*/
  bool fDebugPrints; /*!< print out mostly everything*/
  uhal::HwInterface fBoard; /*!< Board in use*/
 public:
  /*!
   * \brief Read from an address
   * \param pAddr the address
   */
  virtual uhal::ValWord<uint32_t> Read(const uint32_t pAddr);
  /*!
   * \brief Read from an address
   * \param pAddr the address
   * \param pMask the mask
   */
  virtual uhal::ValWord<uint32_t> Read(const uint32_t pAddr, const uint32_t pMask);
  /*!
   * \brief Write to an address
   * \param pAddr the address
   * \param pVal the value
   */
  virtual bool Write(const uint32_t pAddr, const uint32_t pVal);
  /*!
   * \brief Write to an address
   * \param pAddr the address
   * \param pVal the value
   * \param pMask the mask
   */
  virtual bool Write(const uint32_t pAddr, const uint32_t pVal, const uint32_t pMask);
  /*!
   * \brief Write a register
   * \param pRegNode : Node of the register to write
   * \param pVal : Value to write
   * \return boolean confirming the writing
   */
  virtual bool WriteReg(const std::string& pRegNode, const uint32_t& pVal);

  /*!
   * \brief Write a stack of registers
   * \param pVecReg : vector containing the registers and the associated values to write
   * \return boolean confirming the writing
   */
  virtual bool WriteStackReg(const std::vector<std::pair<std::string, uint32_t> >& pVecReg);

  /*!
   * \brief Write a block of values in a register
   * \param pRegNode : Node of the register to write
   * \param pValues : Block of values to write
   * \return boolean confirming the writing
   */
  virtual bool WriteBlockReg(const std::string& pRegNode, const std::vector< uint32_t >& pValues);

  /** \brief Write a block of values at a given address
   * \param uAddr 32-bit address
   * \param pValues : Block of values to write
   * \param bNonInc true if Write mode is non-incremental
   * \return boolean confirming the writing
   */
  virtual bool WriteBlockAtAddress(uint32_t uAddr, const std::vector< uint32_t >& pValues, bool bNonInc=false);

  /*!
   * \brief Read a value in a register
   * \param pRegNode : Node of the register to read
   * \return ValWord value of the register
   */
  virtual uhal::ValWord<uint32_t> ReadReg(const std::string& pRegNode);

  /*!
   * \brief Read two 32-bit words and return them as a 64-bit value
   * \param node_msb the Node for the 32 most significant bits
   * \param node_lsb the Node for the 32 least significant bits
   * \return uint64_t the 64-bit value msb << 32 | lsb
   */
  virtual uint64_t ReadRegsAs64(const std::string& node_msb, const std::string& node_lsb);

  /*!
   * \brief Read a value at a given address
   * \param uAddr 32-bit address
   * \param uMask 32-bit mask
   * \return ValWord value of the register
   */
  virtual uhal::ValWord<uint32_t> ReadAtAddress(uint32_t uAddr, uint32_t uMask=0xFFFFFFFF);

  /*!
   * \brief Read a block of values in a register
   * \param pRegNode : Node of the register to read
   * \param pBlocksize : Size of the block to read
   * \return ValVector block values of the register
   */
  virtual uhal::ValVector<uint32_t> ReadBlockReg(const std::string& pRegNode, const uint32_t& pBlocksize);

  virtual std::vector<uint32_t> ReadBlockRegValue(const std::string& pRegNode, const uint32_t& pBlocksize);

  /*!
   * \brief Read a reg as a 4-char string
   * \param pRegNode : Node of the register to read
   * \return std::string the string
   */
  std::string ReadRegAsString(const std::string& pRegNode);


  /*!
   * \brief constructor
   * \param puHalConfigFileName : path of the uHal Config File
   * \param pBoardId Board Id in the XML configuration file.
   */
  RegManager(const std::string& puHalConfigFileName, const std::string& pBoardId);

  /*!
   * \brief constructor
   * \param pBoardId Board Id
   * \param fURI : connection URI (ask a stupid question?)
   * \param fAddressTableFn : address table fn
   */
  RegManager(const std::string& pBoardId, const std::string& fURI, const std::string& fAddressTableFn);

  /*!
   * \brief destructor
   */
  //  virtual ~RegManager();

  /*!
   * \brief get the uHAL HW Interface
   */
  //uhal::HwInterface& getHardwareInterface() { return fBoard; }

  /*!
   * \brief get the uHAL node
   */
  //const uhal::Node& getUhalNode(const std::string& pStrPath) { return fBoard.getNode(pStrPath); }

  /*!
   * \brief set the dev flag for verifying what was written
   */
  void setVerifyWrites(bool v) { fVerifyWrites = v; }

  /*!
   * \brief set the flag for printing nearly everything
   */
  void setDebugPrints(bool v) { fDebugPrints = v; }

  /*!
   * \brief set unique id for debug prints (default is connection file name + board id)
   */
  void setUniqueId(const std::string& s) { fUniqueId = s; }
};

#endif
