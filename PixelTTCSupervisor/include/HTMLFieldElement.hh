#ifndef __HTMLFieldElement_h__
#define __HTMLFieldElement_h__

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
#include "HTMLTable.hh"


namespace ttc {
  
  class HTMLFieldElement {
  public:
    enum FieldType {DROPDOWNMENU=0, RADIOBUTTON=1, CHECKBOX=2};
    ~HTMLFieldElement(){ }
    HTMLFieldElement(); 
    HTMLFieldElement(const FieldType type, const int flag, 
                     const std::string &name,
                     const std::vector<std::string> &values,
                     const std::vector<std::string> &titles,
                     const std::string &Options="");
    HTMLFieldElement(const FieldType type, const int flag,
                     const std::string &name,
                     const std::vector<std::string> &values,
                     const std::string &Options="");
    void Set(const FieldType type, const int flag,
             const std::string &name,
             const std::vector<std::string> &values, 
             const std::vector<std::string> &titles,
             const std::string &Options="");
    void Set(const FieldType type, const int flag,
             const std::string &name,
             const std::vector<std::string> &values,
             const std::string &Options="");
    std::string GetName() const { return _name; }
    void SetName(const std::string &Name) { _name=Name; }
    void push_back(const std::string &value, const std::string &title="");
    bool HasValue(const std::string & value) const;
    void SetDefault(const std::string & value);
    void SetDefault(const int32_t value);
    void SetCheck(const unsigned int index, bool val);
    void SetCheck(const std::string &value, bool val);
    void Check(const unsigned int index){ SetCheck(index,true); }
    void Check(const std::string &value) { SetCheck(value,true); }
    void UnCheck(const unsigned int index){ SetCheck(index,false); }
    void UnCheck(const std::string &value){ SetCheck(value,false); }
    bool IsChecked(const unsigned int index) const;
    bool IsChecked(const std::string &value) const;
    void UnCheckAll();
    std::vector<std::string> AllVector() const;
    std::vector<std::string> CheckedVector() const;
    void SetOptions(const std::string &Options);
    const std::string &GetDefault() const { return _strdefault; }
    const std::string &GetDefaultTitle() const;
    int GetDefaultInt() const;
    size_t GetDefaultIndex() const { return _default; }
    bool IsRadioButton() const { return _type==RADIOBUTTON; }
    bool IsDropDownMenu() const { return _type==DROPDOWNMENU; }
    bool IsCheckBoxes() const { return _type==CHECKBOX; }
    void Write(std::ostream &out);
    bool FindString(const std::string &basestr, 
                    const std::string &str1, 
                    const std::string &str2="", 
                    const std::string &str3="") const;
    std::string RemovePrefix(const std::string &value) const;
    size_t N() const { return _values.size(); }
    size_t size() const { return N(); }
    std::string GetValue(const size_t i);
    std::string GetTitle(const size_t i);
    void SetTitle(const size_t i, const std::string &tit);

  private:
    int _flag;
    std::string _name;
    std::vector<std::string> _values;
    std::vector<std::string> _titles;
    std::vector<bool> _checked;
    std::string _strdefault;
    size_t _default;
    FieldType _type;
    std::string _options;
    bool _arrangeVertically;
    bool _TitleBeforeField;
    bool _BoldSelected;
    bool _UnderlineSelected;
    bool _GreenSelected;
    bool _BlueSelected;
    bool _RedSelected;

  };


} // namespace ttc

#endif // __HTMLFieldElement_h__
