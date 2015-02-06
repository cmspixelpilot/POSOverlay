// $Id: PixelDCStoFEDDpTable.h,v 1.3 2007/12/03 11:24:22 veelken Exp $

/***************************************************************************
 * Base class for                                                          *
 *  PixelDCStoFEDDpCalibrationTable,                                       *
 *  PixelDCStoFEDDpFilterTable,                                            *
 *  PixelDCStoFEDDpValueTable and                                          *
 *  PixelDCStoFEDDpNameTable                                               *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/12/03 11:24:22 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#ifndef _PixelDCStoFEDDpTable_h_
#define _PixelDCStoFEDDpTable_h_

#include <map>

#include "xdaq/exception/Exception.h"
#include "xdata/Table.h"

template<class C> class PixelDCStoFEDDpTable
{

//---------------------------------------------------------------------------------------------------
//
// NOTE: for templated classes, **all** member functions must be **defined** (not only **declared**) in the header,
//       such that the compiler can "see" them during compile-time
//       (otherwise, will get program abort at run-time due to unresolved symbols)
//
//---------------------------------------------------------------------------------------------------

 public:
  PixelDCStoFEDDpTable() {}
  virtual ~PixelDCStoFEDDpTable()
  {
    for ( typename std::map<unsigned int, std::map<unsigned int, std::map <unsigned int, C*> > >::const_iterator it1 = table_.begin();
	  it1 != table_.end(); ++it1 ) {
      const std::map<unsigned int, std::map <unsigned int, C*> > table2 = it1->second;
      for ( typename std::map<unsigned int, std::map <unsigned int, C*> >::const_iterator it2 = table2.begin();
	    it2 != table2.end(); ++it2 ) {
	const std::map <unsigned int, C*> table3 = it2->second;
	for ( typename std::map <unsigned int, C*>::const_iterator it3 = table3.begin();
	      it3 != table3.end(); ++it3 ) {
	  if ( it3->second != NULL ) delete it3->second;
	}
      }
    }
  }
  
  virtual C& getRow(unsigned int fedBoardId, unsigned int fedChannelId, unsigned int rocId) throw (xdaq::exception::Exception)
  {
    if ( table_[fedBoardId][fedChannelId][rocId] == NULL ) {
      XCEPT_RAISE (xdaq::exception::Exception, "Row not initialized");
    } else {
      return (*table_[fedBoardId][fedChannelId][rocId]);
    }
  }
  
  virtual void writeTo(std::ostream &stream) const
  {
//--- print header
    printHeader(stream);

//--- print data
//    contained in rows
    for ( typename std::map<unsigned int, std::map<unsigned int, std::map <unsigned int, C*> > >::const_iterator it1 = table_.begin();
	  it1 != table_.end(); ++it1 ) {
      const std::map<unsigned int, std::map <unsigned int, C*> > table2 = it1->second;
      for ( typename std::map<unsigned int, std::map <unsigned int, C*> >::const_iterator it2 = table2.begin();
	    it2 != table2.end(); ++it2 ) {
	const std::map <unsigned int, C*> table3 = it2->second;
	for ( typename std::map <unsigned int, C*>::const_iterator it3 = table3.begin();
	      it3 != table3.end(); ++it3 ) {
	  if ( it3->second != NULL ) it3->second->writeTo(stream);
	}
      }
    }
  }
  
 protected:
  
  virtual void initializeTableData(xdata::Table& table) throw (xdaq::exception::Exception)
  {
    std::vector<std::string> columns = table.getColumns();
    
    unsigned long numColumns = columns.size();
    if ( numColumns != numDataMembers_ ) {
      XCEPT_RAISE (xdaq::exception::Exception, "Invalid number of Columns");
    }
    
    unsigned long numRows = table.getRowCount();
    for ( unsigned long rowIndex = 0; rowIndex < numRows; ++rowIndex ) {
      
      std::vector<std::string>::iterator columnIterator = columns.begin();
      for ( unsigned long columnIndex = 0; columnIndex < numColumns;  ++columnIndex, ++columnIterator ) {
	std::string entry = table.getValueAt(rowIndex, *columnIterator)->toString();
	std::string columnName = (*columnIterator);
	//std::string columnType = table.getColumnType(*columnIterator);
	
	if ( columnName == "FEDBOARDID" ) {
	  newRow_fedBoardId_ = atoi(entry.data());
	} else if ( columnName == "FEDCHANNELID" ) {
	  newRow_fedChannelId_ = atoi(entry.data());
	} else if ( columnName == "ROCID" ) {
	  newRow_rocId_ = atoi(entry.data());
	} else {
	  initializeRowData(columnName, entry);
	}
      }
      
      if ( table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] == NULL ) {
	createRow(); 
      } else {
	XCEPT_RAISE (xdaq::exception::Exception, "Row defined twice");
      }
    }
    
    writeTo(std::cout);
  }
  
  virtual void initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception) = 0;
  virtual void createRow() = 0;
  
  virtual void printHeader(std::ostream& stream) const = 0;
  
  std::map<unsigned int, std::map<unsigned int, std::map <unsigned int, C*> > > table_;
  
  unsigned int newRow_fedBoardId_;
  unsigned int newRow_fedChannelId_;
  unsigned int newRow_rocId_;
  
  unsigned int numDataMembers_;
};

#endif
