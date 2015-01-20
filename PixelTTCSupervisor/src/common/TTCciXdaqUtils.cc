#include "TTCciXdaqUtils.hh"

using namespace std;

namespace ttc {
  
  void DropDownMenu(std::ostream &out, 
                    const std::string &name,
                    std::vector<std::string> &options,
                    size_t default_value){
    if (options.size()<1){
      stringstream myerr;
      myerr << "ERROR: DropDownMenu(): options.size()="<<options.size();
      throw std::invalid_argument(myerr.str());
    }
    if (default_value>=options.size()){
      cout << "ERROR: DropDownMenu(): default_value="<<default_value
           <<" is invalid, since options.size()="<<options.size()<<endl;
      default_value = 0;
    }
    out << "<select name=\""<<name<<"\">"<<endl;
    for (size_t i=0; i<options.size(); ++i){
      out << "<option value=\""<<i<<"\""
          <<(i==default_value?"selected":"")<<">"<<options[i]<<endl;
    }
    out << "</select>"<<endl;
  }

  void DropDownMenu(std::ostream &out, 
                    const std::string &name,
                    std::vector<std::string> &options,
                    std::vector<std::string> &values,
                    std::string default_value){
    if (options.size()<1){
      stringstream myerr;
      myerr << "ERROR: DropDownMenu(): options.size()="<<options.size();
      throw std::invalid_argument(myerr.str());
    }
    if (find(options.begin(), options.end(), default_value)==options.end()){
      cout << "ERROR: DropDownMenu(): default_value="<<default_value
           <<" is not found amongst the options!"<<endl;
    }
    if (options.size() != values.size()){
      if (options.size() > values.size()){
        cout << "WARNING: DropDownMenu(): options.size()="<<options.size()
             <<" but values.size()="<<values.size()<<endl;
      }else{
        stringstream myerr;
        myerr << "ERROR: DropDownMenu(): options.size()="<<options.size()
              <<" but values.size()="<<values.size();
        throw std::invalid_argument(myerr.str());
      }
    }
    out << "<select name=\""<<name<<"\">"<<endl;
    for (size_t i=0; i<options.size(); ++i){
      out << "<option value=\""<<values[i]<<"\""
          <<(values[i]==default_value?"selected":"")<<">"<<options[i]<<endl;
    }
    out << "</select>"<<endl;
  }




}// namespace ttc 
