#include "JsonList.h"
#include <TList.h>
#include <TObject.h>

using namespace std;

JsonList::JsonList(string listener,string producer){
  listener_ = listener;
	producer_ = producer;
}

JsonList::JsonList(string thisName, string parentId,string listener,bool leaf,string producer){
	jsonId_.set(parentId+thisName,thisName,thisName,leaf,producer);
	listener_ = listener;
	producer_ = producer;
}

JsonList::~JsonList(){
	for(multimap<JsonIdentifiers,JsonList*>::iterator it=children_.begin(); it != children_.end(); ++it){
		if(it->second != 0){
			delete it->second;
		}
	}
}

ostream &operator <<(ostream &str,JsonList &j) {
	bool alreadyChildren  = false;
  unsigned int childN   = 1;
	unsigned int lastOnes = 0;
	std::string  currentId="";
	for(multimap<JsonIdentifiers,JsonList*>::iterator it=j.children_.begin(); it != j.children_.end(); ++it, childN++){
//	  cout << childN << " : " << j.children_.count(it->first) << endl;
		if(currentId != it->first.getId()){
		  if(currentId != ""){
				str << ",";
			}
			currentId = it->first.getId();
			alreadyChildren = false;
			lastOnes = 0;
			childN = 1;
		}
		if(childN-lastOnes>1){
		  str << ",";
	  }
    if(!alreadyChildren){
  	  str << "{id:'";
			if(it->first.isLeaf() 
			   && it->first.getId().find(it->first.getProducer()) != 0){
        str << it->first.getProducer();
			}
			str << it->first.getId() << "',text:'" << it->first.getText() << "'";
  		if(!(j.children_.count(it->first) == 1 && it->second == 0)){
  			str << ",children: [";
  			alreadyChildren = true;
			}
			else{
			  if(it->first.isLeaf()){
				  str << ",leaf:true,listeners: {" << j.listener_ << "}";
				}
				str << "}";
			}
		}
		if(it->second == 0){
 		  lastOnes++;
			continue;
		}
	  str << *(it->second);
		if(alreadyChildren && childN == j.children_.count(it->first)){
  		str << "]}";
			childN = 0;
			alreadyChildren = false;
			lastOnes = 0;
		}
	}
	if(j.children_.size() == 0){
	  if(j.jsonId_.isLeaf()){
		  str << "{id:'" << j.jsonId_.getProducer() << j.jsonId_.getId() << "',text:'" << j.jsonId_.getText() << "',leaf:true,listeners: {" << j.listener_ << "}}";
	  }
	  else{
	    str << "{id:'" << j.jsonId_.getId() << "',text:'" << j.jsonId_.getText() << "'}";
	  }
	}

	return str;
}


void JsonList::add(string path,string id){
	string mthn = "[JasonList::add()]\t";
	string childName = "";
	int slash;
	//Look for thisName
	if( ( slash=path.substr(1,path.length()-1).find('/')) >= 0 ){
	  jsonId_.setThisName(path.substr(0,slash+2));
		path      = path.substr(slash+2,path.length()-slash-2);
  	jsonId_.setText(jsonId_.getThisName().substr(0,jsonId_.getThisName().length()-1));
		jsonId_.setLeaf(false);
    //Look for childName
		if(path.length() != 0){
	    if( (slash=path.substr(1,path.length()-1).find('/')) >= 0 ){
	      childName = path.substr(0,slash+2);
	    }
	    else{
		    childName = path;//this is a file name and doesn't have any extra char at the end
	    }
		}
	}
	else{
//	  cout << mthn << "This is a file name!" << endl;
		if(path == "/root"){
		  jsonId_.setThisName(producer_+path);//this is a file name and doesn't have any extra char at the end
		}
		else{
		  jsonId_.setThisName(path);//this is a file name and doesn't have any extra char at the end
  	}
		jsonId_.setText(path);
		jsonId_.setLeaf(true);
		path = "" ;
	}
	jsonId_.setId(id + jsonId_.getThisName());
	jsonId_.setProducer(producer_);

/*
	cout << mthn 
	     << "Path: "      << path 
	     << " Id: "       << jsonId_.getId() 
	     << " Text: "     << jsonId_.getText() 
	     << " ThisName: " << jsonId_.getThisName() 
	     << " child: "    << childName
			 << " Prod: "     << jsonId_.getProducer()
			 << endl;
*/			 
	if(path.length() != 0 && childName.length() > 0 && childName[childName.length()-1] == '/'){
	  for(multimap<JsonIdentifiers,JsonList*>::iterator it=children_.begin(); it != children_.end(); ++it){
		  if(it->second != 0){
			  if(it->first.getThisName() == getThisName() && it->second->getThisName() == childName){
//			    cout << mthn << it->second->getThisName() << ":" << childName << endl;
			    it->second->add(path,jsonId_.getId());
				  return;
			  }
		  }
	  }
		JsonList *tmpJson = new JsonList(listener_,producer_);
//		cout << mthn << "Insert: " << jsonId_.getThisName() << endl;
    children_.insert(pair<JsonIdentifiers,JsonList*>(jsonId_,tmpJson));
		tmpJson->add(path,jsonId_.getId());
	}
	else if(childName != ""){
//		cout << mthn << "Insert file: " << jsonId_.getThisName() << producer_ << endl;
    children_.insert(pair<JsonIdentifiers,JsonList*>(jsonId_,new JsonList(childName,jsonId_.getId(),listener_,true,producer_)));
	}
	else if(childName == ""){
//		cout << mthn << "Last one" << endl;
//		cout << mthn << "Insert file: " << jsonId_.getThisName() << endl;
//    if(getThisName()[getThisName().length()-1] != '/'){
//			jsonId_.setLeaf(true);
//		}
    children_.insert(pair<JsonIdentifiers,JsonList*>(jsonId_,0));
	}
}

JsonRoot::~JsonRoot(){
  //it uses the distructor in JasonList class
}
void JsonRoot::setList(void){
	TObject * contentObj;
	TIter contentNext(list_);
	while ( (contentObj = contentNext())){
		string path = contentObj->GetName(); 
		unsigned int n=1;
		for(n=1; n<path.length();++n){
			if(path[n] != '/'){
			  break;
			}
		}
//		cout << "[JsonRoot::setList()]\tName: " << path << endl;
//		cout << "[JsonRoot::setList()]\tTitle: " << contentObj->GetTitle() << endl;
		unsigned int doubleSlash;
		if( (doubleSlash=path.find("//"))){
		  producer_ = path.substr(0,doubleSlash+1);
			path = path.substr(doubleSlash+1,path.length()-doubleSlash);
//			cout << "[JsonRoot::setList()]\t" << producer_ << " : " << path << endl;
		}
		add(path.substr(n-1,path.length()-n+1),"");
	}
}

ostream &operator <<(ostream &str,JsonRoot &j) {
	str << "[" << ((JsonList&)j) << "]";
	return str;
}
