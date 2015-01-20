#include "HTMLTable.hh"

using namespace std;

namespace ttc {
  
  HTMLTable::HTMLTable(std::ostream &outstream,
                       const unsigned int border,
                       const unsigned int cellpadding,
                       const unsigned int cellspacing,
                       const std::string &bgdcolor,
                       const std::string &align,
                       const unsigned int widthpercentage) : 
    _inrow(false), _incell(false), _isopen(false),
    out(outstream), 
    _align("top"),
    _cell_align(""), 
    _cell_bold(false),
    _cell_bgdcolor(""),
    _cell_colspan(1),_cell_rowspan(1)
  {
    Set(border,cellpadding,cellspacing,bgdcolor,align,widthpercentage);
    Open();
  }

  HTMLTable::HTMLTable(std::ostream &outstream,
                       const std::string &style) : 
    _inrow(false), _incell(false), _isopen(false),
    out(outstream), 
    _align("top"),
    _cell_align(""), 
    _cell_bold(false),
    _cell_bgdcolor("")
  {
    Set(style);
    Open();
  }

  HTMLTable::~HTMLTable() { 
    if (_isopen){
      Close();
      cout << "WARNING: HTMLTable::~HTMLTable(): Destructor called, although "
           << "table was still open!"<<endl;
    }
  }

  void HTMLTable::Set(const std::string &style){
    if (FindString(style,"noborder","NOBORDER","NoBorder")){
      _border = "0";
    }else if (FindString(style,"border","BORDER","Border")){
      _border = "1";
    }
    
    if (FindString(style,"left","Left","LEFT")){
      _align = "left";
    }else if (FindString(style,"right","Right","RIGHT")){
      _align = "right";
    }else if (FindString(style,"center","Center","CENTER")){
      _align = "center";
    } 
   
    if (FindString(style,"lightgreen","Lightgreen","LIGHTGREEN")){
      _bgdcolor = "rgb(153, 255, 153)";
    }else if (FindString(style,"green","Green","GREEN")){
      _bgdcolor = "rgb(0, 153, 0)";
    }else if (FindString(style,"white","WHITE","White")){
      _bgdcolor = "rgb(255, 255, 255)";
    }else if (FindString(style,"yello","Yellow","YELLOW")){
      _bgdcolor = "rgb(255, 255, 153)";
    }else if (FindString(style,"lightgrey","LIGHTGREY","Lightgrey")){
      _bgdcolor = "rgb(204, 204, 204)";
    }else if (FindString(style,"grey","GREY","Grey")){
      _bgdcolor = "rgb(153, 153, 153)";
    }else if (FindString(style,"pink","Pink","PINK")){
      _bgdcolor = "rgb(255, 153, 255)";
    }else if (FindString(style,"orange","Orange","ORANGE")){
      _bgdcolor = "rgb(255, 204, 102)";
    }else if (FindString(style,"blue","BLUE","Blue")){
      _bgdcolor = "rgb(204, 255, 255)";
    }
  }

  void HTMLTable::Set(const unsigned int border,
                      const unsigned int cellpadding,
                      const unsigned int cellspacing,
                      const std::string &bgdcolor,
                      const std::string &align,
                      const unsigned int widthpercentage){
    {
      stringstream s; s<<border;
      _border = s.str();
    }
    {
      stringstream s; s<<cellpadding;
      _padding = s.str();
    }
    {
      stringstream s; s<<cellspacing;
      _spacing = s.str();
    }
    {
      stringstream s; 
      if (widthpercentage<=100)
        _widthpercentage = widthpercentage;
      else{
        cout << "WARNING: HTMLTable::Set(): Invalid value for arg "
             <<"widthpercentage="<< widthpercentage
             <<". Should be in (0..100]."<<endl;
      }
    }
    { 
      if (bgdcolor.size()==0){
        _bgdcolor = "rgb(255,255,255)";
      }else if(FindString(bgdcolor,"rgb(")){
        _bgdcolor = bgdcolor;
      }else if (FindString(bgdcolor,"lightgreen","Lightgreen","LIGHTGREEN")){
        _bgdcolor = "rgb(153, 255, 153)";
      }else if (FindString(bgdcolor,"green","Green","GREEN")){
        _bgdcolor = "rgb(0, 153, 0)";
      }else if (FindString(bgdcolor,"white","WHITE","White")){
        _bgdcolor = "rgb(255, 255, 255)";
      }else if (FindString(bgdcolor,"yello","Yellow","YELLOW")){
        _bgdcolor = "rgb(255, 255, 153)";
      }else if (FindString(bgdcolor,"lightgrey","LIGHTGREY","Lightgrey")){
        _bgdcolor = "rgb(204, 204, 204)";
      }else if (FindString(bgdcolor,"grey","GREY","Grey")){
        _bgdcolor = "rgb(153, 153, 153)";
      }else if (FindString(bgdcolor,"pink","Pink","PINK")){
        _bgdcolor = "rgb(255, 153, 255)";
      }else if (FindString(bgdcolor,"orange","Orange","ORANGE")){
        _bgdcolor = "rgb(255, 204, 102)";
      }else if (FindString(bgdcolor,"blue","BLUE","Blue")){
        _bgdcolor = "rgb(204, 255, 255)";
      }else{
        cout << "WARNING: HTMLTable::Set(): Unkown value for background "
             <<"color: '"<< bgdcolor <<"'"<<endl;
        _bgdcolor = "rgb(255,255,255)";
      }
    }
    { 
      if (align.size()==0){
        _align = "left";
      }else if (FindString(align,"left","Left","LEFT")){
        _align = "left";
      }else if (FindString(align,"right","Right","RIGHT")){
        _align = "right";
      }else if (FindString(align,"center","Center","CENTER")){
        _align = "center";
      }else{
        cout << "WARNING: HTMLTable::Set(): Unkown value for alignment: "
             <<"'"<< align <<"'"<<endl;
        _align = "left";
      }
    }
    //_cellstyle = "";
  }
  
  bool HTMLTable::FindString(const std::string &basestr, 
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

  void HTMLTable::ClearCellStyle(){
    _cell_align = "";
    _cell_vertical = "";
    _cell_bold=false;
    _cell_bgdcolor = "";
  }

  void HTMLTable::SetCellStyle(const std::string &style){
    if (FindString(style,"center","CENTER","Center")){
      _cell_align = "text-align: center;";
    }else if (FindString(style,"left","LEFT","Left")){
      _cell_align = "text-align: left;";
    }else if (FindString(style,"right","RIGHT","Right")){
      _cell_align = "text-align: right;";
    }
            
    if (FindString(style,"bottom","BOTTOM","Bottom")){
      _cell_vertical = "vertical-align: bottom;";
    }else if (FindString(style,"middle","Middle","MIDDLE")){
      _cell_vertical = "vertical-align: middle;";
    }else if (FindString(style,"top","Top","TOP")){
      _cell_vertical = "vertical-align: top;";
    }
    
    if (FindString(style,"bold","Bold","BOLD")){
      _cell_bold=true;
    }else{
      _cell_bold=false;
    }
    
    if (FindString(style,"lightgreen","Lightgreen","LIGHTGREEN")){
      _cell_bgdcolor = "background-color: rgb(220, 255, 220);";
    }else if (FindString(style,"green","Green","GREEN")){
      _cell_bgdcolor = "background-color: rgb(153, 255, 153);";
    }else if (FindString(style,"lightorange","LIGHTORANGE","Lightorange")){
      _cell_bgdcolor = "background-color: rgb(255, 235, 193);";
    }else if (FindString(style,"orange","ORANGE","Orange")){
      _cell_bgdcolor = "background-color: rgb(255, 204, 102);";
    }else if (FindString(style,"white","WHITE","White")){
      _cell_bgdcolor = "background-color: rgb(255, 255, 255);";
    }else if (FindString(style,"lightyellow","Lightyellow","LIGHTYELLOW")){
      _cell_bgdcolor = "background-color: rgb(255, 255, 224);";
    }else if (FindString(style,"yello","Yellow","YELLOW")){
      _cell_bgdcolor = "background-color: rgb(255, 255, 153);";
    }else if (FindString(style,"lightgrey","LIGHTGREY","Lightgrey")){
      _cell_bgdcolor = "background-color: rgb(204, 204, 204);";
    }else if (FindString(style,"grey","GREY","Grey")){
      _cell_bgdcolor = "background-color: rgb(153, 153, 153);";
    }else if (FindString(style,"pink","Pink","PINK")){
      _cell_bgdcolor = "background-color: rgb(255, 153, 255);";
    }else if (FindString(style,"lightblue","LIGHTBLUE","LightBlue")){
      _cell_bgdcolor = "background-color: rgb(224, 255, 255);";
    }else if (FindString(style,"blue","BLUE","Blue")){
      _cell_bgdcolor = "background-color: rgb(51, 204, 255);";
    }else if (FindString(style,"lightred","LIGHTRED","LightRed")){
      //_cell_bgdcolor = "background-color: rgb(51, 204, 255);";
      _cell_bgdcolor = "background-color: rgb(255, 234, 234);";
    }else if (FindString(style,"red","RED","Red")){
      _cell_bgdcolor = "background-color: rgb(255, 68, 68);";
    }
  }

  void HTMLTable::NewCell(const std::string &style,
                          const size_t colspan, const size_t rowspan){
    if(!_isopen){
      cout << "ERROR: HTMLTable::NewCell(): Table is not open!"<<endl;
      return;
    }
    SetCellStyle(style);
    if (_incell) out<<"</td>"<<endl;
    _incell = true;
    if (!_inrow) out << "<tr>"<<endl;
    _inrow = true;
    string mystyle;
    if (colspan==0 || rowspan==0){
      cout << "ERROR: HTMLTable::NewCell(): colspan / rowspan="
           <<colspan<<" / "<< rowspan <<" out of range! Should not be 0!"
           <<endl;
    }
    if ((colspan>1 || rowspan>1) && (colspan>0 && rowspan>0)){
      stringstream mys;
      mys << " colspan=\""<<int(colspan)<<"\" rowspan=\""
          <<int(rowspan)<<"\"";
      mystyle += mys.str();
    }
    mystyle +=" style=\"";
    if (_cell_bgdcolor.size()>0) mystyle += _cell_bgdcolor;
    if (_cell_vertical.size()>0) mystyle += (" "+_cell_vertical);
    if (_cell_bold) mystyle += " font-weight: bold;";
    if (_cell_align.size()>0) mystyle += (" "+_cell_align);
    mystyle += "\"";
    if (mystyle.size()<10) mystyle = string("");
    out << "<td"<<mystyle<<">"<<endl;
  }

  void HTMLTable::NewRow(){
    if(!_isopen){
      cout << "ERROR: HTMLTable::NewRow(): Table is not open!"<<endl;
      return;
    }
    if (_incell){
      out<<"</td>"<<endl;
      _incell = false;
    }
    if (_inrow) out << "</tr>"<<endl;
    _inrow = true;
    out<<"<tr>"<<endl;
  }

  void HTMLTable::Open(){
    if(_isopen){
      cout << "ERROR: HTMLTable::Open(): Table is already open!"<<endl;
      return;
    }
    _isopen=true;
    out << "<table cellpadding=\""<<_padding
        <<"\" cellspacing=\""<<_spacing
        <<"\" border=\""<<_border
        <<"\" style=\""
        <<"text-align: "<<_align<<";";
    if (_widthpercentage > 0){
      out <<" width: "<<_widthpercentage<<"%;";
    }
    out <<" background-color: "<<_bgdcolor<<";"
        <<"\">"<<endl;

    //out <<cgicc::tbody()<<endl;
    out << "<tbody>" <<endl;
  }

  void HTMLTable::Close(){
    if(!_isopen){
      cout << "ERROR: HTMLTable::Close(): Table is not open!"<<endl;
      return;
    }
    if (_incell) out<<"</td>"<<endl;
    if (_inrow) out<<"</tr>"<<endl;
    out << "</tbody>" <<endl<<"</table>"<<endl;
    _isopen = false;
    _incell=false;
    _inrow=false;
  }

} // namespace ttc
