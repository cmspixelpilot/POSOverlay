// mostly by Nicolas Pierre

#ifndef PixelUtilities_PixeluTCAUtilities_RegManager_h
#define PixelUtilities_PixeluTCAUtilities_RegManager_h

#include <uhal/uhal.hpp>

/*!
 * \class RegManager
 * \brief Permit connection to given boards and r/w given registers
 */
class RegManager {
 protected:
  const std::string fUHalConfigFileName;         /*!< path of the uHal Config File*/
  const std::string fBoardId; /*!< board id from connection file*/
  std::string fUniqueId; /*!< used in prints*/
  bool fVerifyWrites; /*!< whether to verify writes*/
  bool fDebugPrints; /*!< print out mostly everything*/
  uhal::ConnectionManager* fCM; /*!< connection manager*/
  uhal::HwInterface* fBoard;         /*!< Board in use*/
        
 public:
  /*!
   * \brief Write a register
   * \param pRegNode : Node of the register to write
   * \param pVal : Value to write
   * \return boolean confirming the writing
   */
  virtual bool WriteReg( const std::string& pRegNode, const uint32_t& pVal );
  /*!
   * \brief Write a stack of registers
   * \param pVecReg : vector containing the registers and the associated values to write
   * \return boolean confirming the writing
   */
  virtual bool WriteStackReg( const std::vector<std::pair<std::string, uint32_t> >& pVecReg );
  /*!
   * \brief Write a block of values in a register
   * \param pRegNode : Node of the register to write
   * \param pValues : Block of values to write
   * \return boolean confirming the writing
   */
  virtual bool WriteBlockReg( const std::string& pRegNode, const std::vector< uint32_t >& pValues );
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
  virtual uhal::ValWord<uint32_t> ReadReg( const std::string& pRegNode );
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
  virtual uhal::ValVector<uint32_t> ReadBlockReg( const std::string& pRegNode, const uint32_t& pBlocksize );
  /*!
   * \brief Time Out for sending the register/value stack in the writting.
   * \brief It has only to be set in a detached thread from the one you're working on
   */

  virtual std::vector<uint32_t> ReadBlockRegValue( const std::string& pRegNode, const uint32_t& pBlocksize );

 public:
  // Connection w uHal
  /*!
   * \brief Constructor of the RegManager class
   * \param puHalConfigFileName : path of the uHal Config File
   * \param pBoardId Board Id in the XML configuration file.
   */
  RegManager(const std::string& puHalConfigFileName, const std::string& pBoardId);
  /*!
   * \brief Destructor of the RegManager class
   */
  virtual ~RegManager();
  /*!
   * \brief get the uHAL HW Interface
   */
  uhal::HwInterface* getHardwareInterface() const {
    return fBoard;
  }
  /*!
   * \brief get the uHAL node
   */
  const uhal::Node& getUhalNode( const std::string& pStrPath ) {
    return fBoard->getNode( pStrPath );
  }
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
