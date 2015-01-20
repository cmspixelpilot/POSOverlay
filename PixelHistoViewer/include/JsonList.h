#include <iostream>
#include <map>

class TList;

class JsonIdentifiers{
	public:
	JsonIdentifiers(){id_="";text_="";thisName_="";leaf_=false;producer_="";}
	JsonIdentifiers(const JsonIdentifiers& j){id_=j.id_;text_=j.text_;thisName_=j.thisName_;leaf_=j.leaf_;producer_=j.producer_;}
	~JsonIdentifiers(){;}
	std::string getId      (void) const {return id_;}
	std::string getText    (void) const {return text_;}
	std::string getThisName(void) const {return thisName_;}
	std::string getProducer(void) const {return producer_;}
	bool        isLeaf     (void) const {return leaf_;}
		
	void   set        (std::string id,std::string text,std::string thisName,bool leaf=false,std::string producer=""){id_=id;text_=text;thisName_=thisName;leaf_=leaf;producer_=producer;}
	void   setId      (std::string id)      {id_       = id;}
	void   setText    (std::string text)    {text_     = text;}
	void   setThisName(std::string thisName){thisName_ = thisName;}	
	void   setProducer(std::string producer){producer_ = producer;}	
	void   setLeaf    (bool leaf)           {leaf_     = leaf;}	
	
	//Oerators overloading needed for multimap
	bool operator<(const JsonIdentifiers& j) const{
		if(thisName_ < j.thisName_){
      return true;
		}
		return false;
	} 
	
	private:
	std::string id_;
	std::string text_;
	std::string thisName_;	
	std::string producer_;	
	bool   leaf_;	
};


class JsonList{
  public:
	JsonList(std::string listener,std::string producer);
	JsonList(std::string thisName, std::string parentId,std::string listener,bool leaf=false,std::string producer="");
	virtual ~JsonList();

	std::string getThisName(void){return jsonId_.getThisName();}
  friend std::ostream& operator <<(std::ostream& str,JsonList& j);

	protected:
	void add(std::string path,std::string id);
	void setPar(std::string id,std::string text, std::string thisName);
	std::multimap<JsonIdentifiers,JsonList*> children_;
	JsonIdentifiers jsonId_;
	std::string listener_;
	std::string producer_;
};

class JsonRoot : public JsonList{
  public:
	JsonRoot(TList * list,std::string listener,std::string producer): JsonList(listener,producer){list_=list;}
	~JsonRoot();
	void setList(void);
  friend std::ostream& operator <<(std::ostream& str,JsonRoot& j);
	private:
	TList * list_;
};

