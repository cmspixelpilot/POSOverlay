#include "HTMLFieldElement.hh"

#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace std;

namespace ttc {


  HTMLFieldElement::HTMLFieldElement(){
    
  }

  HTMLFieldElement::HTMLFieldElement(const FieldType type, const int flag,
                                     const std::string &name,
                                     const std::vector<std::string> &values, 
                                     const std::vector<std::string> &titles,
                                     const std::string &Options) :
    _flag(flag),_name(name), _values(values), _titles(titles),
    _checked(values.size(),false),
    _default(0), _type(type), _options(Options)  {
    if (_titles.size() != _values.size() || _values.size()==0 ||
        _checked.size() != _values.size()){
      stringstream my;
      my << "WARNING: HTMLFieldElement::HTMLFieldElement(): "
         <<"_titles.size() != _titles.size()="
         <<_titles.size()<<" _values.size()="<<_values.size()
         <<" _checked.size()="<<_checked.size()<<") (or size=0)";
      throw std::invalid_argument(my.str());
    }
    if (name.size()==0){
      stringstream my;
      my << "WARNING: HTMLFieldElement::HTMLFieldElement(): "
         <<"name = '"<<name<<"' name.size() = "<<name.size();
      throw std::invalid_argument(my.str());
    }
    SetOptions(Options);
  }

  HTMLFieldElement::HTMLFieldElement(const FieldType type, const int flag,
                                     const std::string &name,
                                     const std::vector<std::string> &values,
                                     const std::string &Options) : 
  _default(0) {
    HTMLFieldElement(type, flag, name, values, values, Options);
  }

  void HTMLFieldElement::Set(const FieldType type, const int flag,
                             const std::string &name,
                             const std::vector<std::string> &values, 
                             const std::vector<std::string> &titles,
                             const std::string &Options){
    _type = type;
    _flag = flag;
    _name = name;
    _values = values;
    _titles = titles;
    _checked.resize(_values.size(),false);
    SetOptions(Options);
    if (_titles.size() != _values.size() || _values.size()==0){
      stringstream my;
      my << "WARNING: HTMLFieldElement::Set(): "
         <<"_titles.size() != _values.size() ("
         <<_titles.size()<<" != "<<values.size()<<") or size=0";
      throw std::invalid_argument(my.str());
    }
    if (name.size()==0){
      stringstream my;
      my << "WARNING: HTMLFieldElement::Set(): "
         <<"name = '"<<name<<"' name.size() = "<<name.size();
      throw std::invalid_argument(my.str());
    }
  }

  void HTMLFieldElement::Set(const FieldType type, const int flag,
                             const std::string &name,
                             const std::vector<std::string> &values,
                             const std::string &Options){
    Set(type,flag,name,values,values,Options);
  }

  int HTMLFieldElement::GetDefaultInt() const {
    int myint;
    if (1 == sscanf(_strdefault.c_str(),"%d",&myint))
      return myint;
    else
      return -1;
  }

  const std::string &HTMLFieldElement::GetDefaultTitle() const {
    for (size_t i=0; i<_values.size(); ++i){
      if (_strdefault == _values[i]){
        return _titles[i];
      }
    }
    cout << "ERROR: HTMLFieldElement::GetDefaultTitle(): _strdefault=\""
         <<_strdefault<<"\" not found!" <<endl;
    return _strdefault;
  }

  bool HTMLFieldElement::HasValue(const std::string & value) const {
    return (find(_values.begin(), _values.end(), value)!=_values.end());
  }

  void HTMLFieldElement::SetDefault(const int32_t value){
//     char dummy[30];
//     sprintf(dummy,"%ld",value);
//     string val=dummy;
    stringstream g;
    g<<dec<<value;
    string val=g.str();
    if (HasValue(val)){
      SetDefault(val);
    }else{
      if (value>=0 && unsigned(value) < _values.size()){
        _default = value;
        _strdefault = _values[_default];
      }else{
        cout << "ERROR: HTMLFieldElement::SetDefault(int value="<<value<<"): "
             << "No element matches this value!"<<endl;
      }
    }
  }
  
  void HTMLFieldElement::SetDefault(const std::string & value){
    if (value == _strdefault) return;
    _strdefault = value;
    _default = 0;
    bool found=false;
    for (size_t i=0; i<_values.size(); ++i){
      if (value == _values[i]){
        _default = i;
        found = true;
      }
    }
    if (!found){
      if (value == "UNDEFINED"){
        _default = 0;
        _strdefault = _values[_default];
      }else{
        stringstream my;
        my << "WARNING: HTMLFieldElement::SetDefault(): "
           <<"default value '"<<value<<"'  not found!";
        throw std::invalid_argument(my.str());      
      }
    }
  }
    
  bool HTMLFieldElement::IsChecked(const unsigned int index) const {
    if (_type != CHECKBOX){
        stringstream my;
        my << "WARNING: HTMLFieldElement::IsChecked(): Type NOT CHECKBOX!";
        cout << my.str();
        return false;
    }
    if (index < _checked.size()){
      return _checked[index];
    }
    stringstream my;
    my << "WARNING: HTMLFieldElement::IsChecked(): "
       <<"index="<<index<<" out of range! (max = "<<_checked.size()<<")";
    cout << my.str();
    return false;
  }

  bool HTMLFieldElement::IsChecked(const string &value) const {
    if (_type != CHECKBOX){
        stringstream my;
        my << "WARNING: HTMLFieldElement::IsChecked(): Type NOT CHECKBOX!";
        cout << my.str();
        return false;
    }
    const std::string value2 = RemovePrefix(value);
    for (size_t i=0; i<_values.size(); ++i){
      if (value == _values[i] || value2 == _values[i]){
        return _checked[i];
      }
    }
    stringstream my;
    my << "WARNING: HTMLFieldElement::IsChecked(): "
       <<"default value '"<<value<<"'  not found!";
    cout << my.str()<<endl;
    //throw std::invalid_argument(my.str());      
    return false;
  }

  void HTMLFieldElement::SetCheck(const unsigned int index, bool val){
    if (_type != CHECKBOX){
        stringstream my;
        my << "WARNING: HTMLFieldElement::SetCheck(): Type NOT CHECKBOX!";
        cout << my.str();
        return;
    }
    if (index < _checked.size()){
      _checked[index] = val;
    }else{
        stringstream my;
        my << "WARNING: HTMLFieldElement::SetCheck(): "
           <<"index="<<index<<" out of range! (max = "<<_checked.size()<<")";
        cout << my.str();
    }
  }

  void HTMLFieldElement::SetCheck(const std::string &value, bool val){
    if (_type != CHECKBOX){
        stringstream my;
        my << "WARNING: HTMLFieldElement::SetCheck(): Type NOT CHECKBOX!";
        cout << my.str();
        return;
    }
    bool found=false;
    const std::string value2 = RemovePrefix(value);
    for (size_t i=0; i<_values.size(); ++i){
      if (value == _values[i] || value2 == _values[i]){
        _checked[i] = val;
        found = true;
      }
    }
    if (!found){
      stringstream my;
      my << "WARNING: HTMLFieldElement::SetCheck(): "
         <<"value '"<<value<<"'  not found!";
      cout << my.str()<<endl;
      //throw std::invalid_argument(my.str());
    }
  }

  
  void HTMLFieldElement::UnCheckAll() {
    if (_type != CHECKBOX){
        stringstream my;
        my << "WARNING: HTMLFieldElement::UnCheckAll(): Type NOT CHECKBOX!";
        cout << my.str();
        return;
    }
    _checked.clear();
    _checked.resize(_values.size(), false);
  }
  
  std::vector<std::string> HTMLFieldElement::CheckedVector() const {
    std::vector<std::string> vec;
    if (_type != CHECKBOX){
        stringstream my;
        my << "WARNING: HTMLFieldElement::CheckedVector(): Type NOT CHECKBOX!";
        cout << my.str();
        return vec;
    }
    for (size_t i=0; i<_checked.size(); ++i){
      if (_checked[i]){
        vec.push_back(_values[i]);
      }
    }
    return vec;
  }
  
  std::vector<std::string> HTMLFieldElement::AllVector() const {
    return _values;
  }
  
  
  void HTMLFieldElement::SetOptions(const std::string &Options){
    _options = Options;
    _arrangeVertically = false; // for Radio & Checkbox only
    _TitleBeforeField = false; // for Radio & Checkbox only
    _BoldSelected = false; // for Radio & Checkbox only
    _UnderlineSelected = false; // for Radio & Checkbox only
    _GreenSelected = false; // for Radio & Checkbox only
    _BlueSelected = false; // for Radio & Checkbox only
    _RedSelected = false; // for Radio & Checkbox only
    if (FindString(_options, "Vert", "vert", "VERT")){
      _arrangeVertically = true;
    }
    if (FindString(_options, "Horiz", "horiz", "HORIZ")){
      _arrangeVertically = false;
    }
    if (FindString(_options, "Tit", "tit", "TIT")){
      _TitleBeforeField = true;
    }
    if (FindString(_options, "Bold", "bold", "Bold")){
      _BoldSelected = true;
    }
    if (FindString(_options, "Underl", "underl", "UNDERL")){
      _UnderlineSelected = true;
    }
    if (FindString(_options, "Green", "green", "GREEN")){
      _GreenSelected = true;
    }
    if (FindString(_options, "Blue", "blue", "BLUE")){
      _BlueSelected = true;
    }
    if (FindString(_options, "RED", "red", "Red")){
      _RedSelected = true;
    }
  }

  void HTMLFieldElement::Write(std::ostream &out){
    if (_values.size()==0){
      stringstream my;
      my << "ERROR: HTMLFieldElement::Write(): _values.size()="
         <<_values.size()<<"!";
      throw std::invalid_argument(my.str());
    }
    switch(_type){
    case DROPDOWNMENU:
      out << "<select name=\""<<_name<<"\" width=\"20\">"<<endl;
      for (size_t i=0; i<_values.size(); ++i){
        out << "<option value=\""<<_values[i]<<"\""
            <<(i==_default?"selected":"")<<">"<<_titles[i]<<endl;
      }
      out << "</select>"<<endl;
      break;
    case RADIOBUTTON:
      {
        for (size_t i=0; i<_values.size(); ++i){
          if (i==_default){
            if (_BoldSelected)
              out << "<span style=\"font-weight: bold;\">";
            if (_UnderlineSelected)
              out << "<span style=\"text-decoration: underline;\">";
            if (_GreenSelected)
              out << "<span style=\"color: rgb(51, 204, 0);\">";
            if (_BlueSelected)
              out << "<span style=\"color: rgb(51, 51, 255);\">";
            if (_RedSelected)
              out << "<span style=\"color: rgb(255, 0, 0);\">";
          }
          if (_TitleBeforeField) out << cgicc::label(_titles[i]+std::string(" "));
          if (i==_default){
            out << cgicc::input().set("type", "radio")
              .set("name",_name).set("value",_values[i])
              .set("checked","checked");
          }else{
            out << cgicc::input().set("type", "radio")
              .set("name",_name).set("value",_values[i]);
          }
          if (!_TitleBeforeField) out << cgicc::label(_titles[i]+std::string(" "));
          if (_arrangeVertically) out << "<br>";
          if (i==_default){
            for (int k=0; k<(int(_UnderlineSelected)+int(_BoldSelected)
                             +int(_GreenSelected)+int(_BlueSelected)
                             +int(_RedSelected)); ++k) { out << "</span>"; }
          }
          out << endl;
        }
      }
      break;
    case CHECKBOX:
      {
        const bool usetable=(_values.size()>16?true:false);
        HTMLTable *tab=0;
        if (usetable){
          tab=new HTMLTable(out,0,1,1,"","");
        }
        for (size_t i=0; i<_values.size(); ++i){
          if (usetable && i%16==0 && tab) tab->NewCell();
          if (_checked[i]){
            if (_BoldSelected) 
              out << "<span style=\"font-weight: bold;\">";
            if (_UnderlineSelected) 
              out << "<span style=\"text-decoration: underline;\">";
            if (_GreenSelected) 
              out << "<span style=\"color: rgb(51, 204, 0);\">";
            if (_BlueSelected) 
              out << "<span style=\"color: rgb(51, 51, 255);\">";
            if (_RedSelected)
              out << "<span style=\"color: rgb(255, 0, 0);\">";
          }

          if (_TitleBeforeField) out << cgicc::label(_titles[i]+std::string(" "));
          if (_checked[i]){
            out << cgicc::input().set("type", "checkbox")
              .set("name",_name+"__"+_values[i]).set("checked","checked");
          }else{
            out << cgicc::input().set("type", "checkbox")
              .set("name",_name+"__"+_values[i]);
          }
          if (!_TitleBeforeField) out << cgicc::label(_titles[i]+std::string(" "));
          if (_arrangeVertically) out << "<br>";
          if (_checked[i]){
            for (int k=0; k<(int(_UnderlineSelected)+int(_BoldSelected)
                             +int(_GreenSelected)+int(_BlueSelected)
                             +int(_RedSelected)); ++k) { out << "</span>"; }
          }
          out << endl;
        }
        if (usetable  && tab){
          tab->Close();
          delete tab;
        }

      }
      break;
    default:
      stringstream my;
      my << "ERROR: HTMLFieldElement::Write(): "<<"ERROR!";
      throw std::invalid_argument(my.str());      
    }
  }

  bool HTMLFieldElement::FindString(const std::string &basestr, 
                                    const std::string &str1, 
                                    const std::string &str2, 
                                    const std::string &str3) const {
    if (str1.size()>0){
      if (search(basestr.begin(), basestr.end(), 
                        str1.begin(), str1.end()) != basestr.end()){
        return true; // found str1 in basestr
      }
    }
    if (str2.size()>0){
      if (search(basestr.begin(), basestr.end(), 
                 str2.begin(), str2.end()) != basestr.end()){
       return true; // found str2 in basestr
      }
    }
    if (str3.size()>0){
      if (search(basestr.begin(), basestr.end(), 
                 str3.begin(), str3.end()) != basestr.end()){
        return true; // found str3 in basestr
      }
    }
    return false; // nothing found
  }

  std::string HTMLFieldElement::GetValue(const size_t i){ 
    if (i<N())
      return _values[i]; 
    else{
      cout << "ERROR: HTMLFieldElement::GetValue(i="<<i
           <<"): Index out of range (name="<<GetName()<<")!"<<endl;
      return string("");
    }
  }

  std::string HTMLFieldElement::GetTitle(const size_t i){ 
    if (i<N())
      return _titles[i]; 
    else{
      cout << "ERROR: HTMLFieldElement::GetTitle(i="<<i
           <<"): Index out of range (name="<<GetName()<<")!"<<endl;
      return string("");
    }
  }
    
  void HTMLFieldElement::SetTitle(const size_t i, const std::string &tit){
    if (i<N()){
      _titles[i] = tit;
    }else{
      cout << "ERROR: HTMLFieldElement::SetTitle(i="<<i
           <<", \""<<tit<<"\"): Index out of range (name="
           <<GetName()<<")!"<<endl;
    }
  }

  void HTMLFieldElement::push_back(const std::string &value, 
                                   const std::string &title){
    _values.push_back(value);
    _titles.push_back(title.size()>0?title:value);
    _checked.push_back(false);
  }

  std::string HTMLFieldElement::RemovePrefix(const std::string &value) const {
    string out=value; 
    size_t begin=value.find(string("__"));
    begin += (string("__")).size();
    if (begin >= value.size()) return out;
    out.clear();
    for (size_t i=begin; i<value.size(); ++i){
      out.push_back(value[i]);
    }
    return out;
  }

} // html
