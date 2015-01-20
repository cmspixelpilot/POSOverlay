#ifndef H_HTMLTable_h
#define H_HTMLTable_h

//#include "cgicc/CgiDefs.h"
//#include "cgicc/Cgicc.h"
//#include "cgicc/HTTPHTMLHeader.h"
//#include "cgicc/HTMLClasses.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace ttc {

  class HTMLTable {
  public:
    HTMLTable(std::ostream &outstream,
              const unsigned int border=1,
              const unsigned int cellpadding=2,
              const unsigned int cellspacing=2,
              const std::string &bgdcolor="",
              const std::string &align="",
              const unsigned int widthpercentage=0);
    HTMLTable(std::ostream &outstream,
              const std::string &style);
    ~HTMLTable();
    void Set(const unsigned int border=1,
             const unsigned int cellpadding=2,
             const unsigned int cellspacing=2,
             const std::string &bgdcolor="",
             const std::string &align="",
             const unsigned int widthpercentage=0);
    void Set(const std::string &style);
    void SetCellStyle(const std::string &style);
    void ClearCellStyle();
    bool FindString(const std::string &basestr, 
                    const std::string &str1, 
                    const std::string &str2="", 
                    const std::string &str3="") const;
    void NewCell(const std::string &style="", 
                 const size_t colspan=1, const size_t rowspan=1);
    void NewRow();
    void CloseRow(){ out << "</tr>"; _inrow=false; }
    void Open();
    void Close();
    

  private:
    bool _inrow;
    bool _incell;
    bool _isopen;
    std::ostream &out;
    std::string _border;
    std::string _padding;
    std::string _spacing;
    std::string _bgdcolor;
    std::string _align;
    size_t _widthpercentage;
    std::string _cell_vertical;
    std::string _cell_align;
    bool _cell_bold;
    std::string _cell_bgdcolor;
    size_t _cell_colspan;
    size_t _cell_rowspan;
  };

} // namespace ttc

#endif //H_HTMLTable_h
