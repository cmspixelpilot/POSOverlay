/*************************************************************
 * 
 *
 *************************************************************/

#include "SimpleCommand.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h> 
#include <iostream>
#include <fstream>
using namespace std;



//-----------------------------------------------------------

SimpleCommand::SimpleCommand()
{
	nOpen=0;
	inputFile=NULL;
	prompt=">";
}



//------------------------------------------------------------

const char* SimpleCommand::GetWord(const char *s, int* l)
{
	/*
	   helper for chopping a line into tokens, a variation of strtok,
	   with two differences:
	   1) no '\0' characters are inserted, instead the length
	      of the string is returned
	   2) the list of separators is hard-coded (sym/white)

	   return values:
	   * pointer to the 1st character of the Next word (or symbol)
	   * number of characters in that word,l 
	     l>0   : length of the word starting at the pointer returned
	     l==0  : end of line reached, no new symbol
	 */

	static int idx;
	static const char* buf;         // pointer to the input line (not a copy!!)
	const char* sym=",:();";   // single character symbols
	const char* white=" \t\n";  // whitespace characters
	const char* word;

	// first call with a new string,
	if (s!=NULL)
	{
		idx=0;
		buf=s;
	}
	// skip whitespace characters
	while((buf[idx]!='\0')&&(index(white,buf[idx])!=NULL)){idx++; }
	// set the pointer to the beginning of the Next word/symbol
	word=&buf[idx];
	if(buf[idx]=='\0')
	{
		word=NULL;
		*l=0; // the end
	}
	else if(index(sym,buf[idx])!=NULL)
	{
		// single letter symbols, return them as words
		*l=1; idx++;
	}
	else
	{
		// extract words delimited by symbols or blanks
		*l=0;
		while ( (buf[idx]!='\0') && (*l<20)
		        && (index(sym,buf[idx])==NULL)
		        && (index(white,buf[idx])==NULL) )
		{
			idx++; (*l)++;
		}
	}
	return word;
}


//------------------------------------------------------------

int SimpleCommand::StrToI(const char* word, const int len, int* v)
  /* convert strings to integers. Numbers are assumed to 
     be integers unless preceded by "$" or "0x", in which
     case they are taken to be hexadecimal,
     the return value is non-zero if an error occured
*/

{
  const char* digits="0123456789ABCDEF";
  int base;
  int i;
  const char* d0=index(digits,'0');
  if( word[0]=='$'){
    base=16;        
    i=1;
  }else if ((word[0]=='0')&&(word[1]=='x')){
    base=16;
    i=2;
  }else{
    base=10;
    i=0;
  }

  int a=0;
  const char* d;
  while( (i<len) && ( (d=index(digits,toupper(word[i])))!=NULL) ){
    a=base*a+d-d0;
    i++;
  }
 

  *v=a;
  if(i==len){return 0; }else{ return 1;}

}
//------------------------------------------------------------

int SimpleCommand::StrToF(const char* word, const int len, float* v)
  /* convert strings to floats. 
     the return value is non-zero if an error occured
*/

{
  const char* digits="0123456789";
  float sign=1.;
  int i=0;
  const char* d0=index(digits,'0');

  // look for a leading sign
  if( word[0]=='-'){
    sign=-1.; i=1;
  }else if( word[0]=='+'){
    sign=1.; i=1;
  }

  // parse the integral part
  int a=0;
  const char* d;
  while( (i<len) && ( (d=index(digits,word[i]))!=NULL) ){
    a=10*a+d-d0;  i++;
  }

  // return if this is all
  *v=a*sign;
  if(i==len){ return 0; };
  
  // parse the fractional part if it exists
  if (word[i]=='.'){
    i++;
    float f=0;
    float p=0.1;
    while( (i<len) && ( (d=index(digits,word[i]))!=NULL) ){
      f+=(d-d0)*p;  p*=0.1;   i++;
    }
    *v=(a+f)*sign;
  }

  if (i==len){ return 0; }else{ return 1;}
}

//------------------------------------------------------------
string SimpleCommand::CharToStr(const char* word, const int len){
  char * buf = new char[len+1];
  strncpy(buf,word,len);
  buf[len]=0;
  string s=buf;
  return s;
}
//

// //------------------------------------------------------------
// bool SimpleCommand::ParseNumberOrAlias(const char* word, const int &l, int *id){
//   // where number can be a number or a network id
//   //  group identifier. The return value is true in case
//   // of successful parsing, false otherwise
  

//   if(
//      // test for control network id
//      ((l==5) || (l==6)) &&
//      ((word[0]=='+') || (word[0]=='-')) &&
//      ( index("12345678",word[1])!=NULL ) &&
//      ((word[2]=='P') || (word[2]=='N')) &&
//      (word[3]=='L') &&
//      ( ((l==5) && (word[4]=='3')) || ((l==6) && (word[4]=='1') && (word[5]=='2')))
//      ){
//     *id=getCNidx(word);
//     return true;
//   }else if(
// 	   // test for sector id
// 	   (l==3) &&
// 	   ((word[0]=='+') || (word[0]=='-')) &&
// 	   ( index("12345678",word[1])!=NULL ) &&
// 	   ((word[2]=='P') || (word[2]=='N'))
//      ){
//     *id=getCNidx(((std::string)word)+"12"); //get the index of layer1&2
//     return true;
//   }else if( index("$0123456789",word[0])!=NULL ){
//     // must be a number
//     if (StrToI(word,l,id)){
//       return false;
//     }else{
//       return true;
//     }
//   }else{
//     return false;
//   }
// }
//------------------------------------------------------------


int SimpleCommand::Parse(const string & cmd)
{
  // Parse one line of instruction
  // the return value is non-zero when the result is an
  // executable instruction
  // when reading files, this method is called recursively
  // it must therefore keep its own copy of the tokens

  // avoid crashed for code that doesnt check for narg>0
  exitFlag=false;


  char line[256];
  strncpy(line, cmd.c_str(),255);
  buffer=cmd; // copy input to buffer for getBuffer()


  // skip comments and empty lines
  if (line[0] == '#')  return 0; /* filter out the comments */
  if (line[0] == '\n') return 0; /* filter out empty lines */
  if (line[0] == 0)    return 0; /* filter out empty lines */
  
  int l;
  const char* word;
  
  int i1;
  float f;
  
  //    Part I: chop the input into items
  // no lists at the moment
  vector<item> lTokens;  // local to Parse
  vector<vector<item> > lStack;

  word=GetWord(line,&l);
  if (word==NULL) return 0; // only whitespace
  //cout << "the word is [" << CharToStr(word,l) << "]" <<endl;

  while(l>0)
    {
      // new item
      item t; t.token=""; t.i=0; t.f=0; t.type=kNone;

      // sector identifier
      if( 
	 (l==3) &&
	 ((word[0]=='+') || (word[0]=='-')) &&
	 ( index("12345678",word[1])!=NULL ) &&
	 ((word[2]=='P') || (word[2]=='N'))){
	
	t.type=kSector;
	t.sector.phi=atoi(word);
	t.sector.z=word[2];
	t.token=CharToStr(word,l);

      //  group identifier
      }else if( 
		(l==6)&&
		((word[0]=='+') || (word[0]=='-')) &&
		( index("12345678",word[1])!=NULL ) &&
		((word[2]=='P') || (word[2]=='N')) &&
		(word[3]=='L') &&
		( ((word[4]=='1') &&(word[5]=='2')) || ((word[4]=='3') && (word[5]=='4')) ||((word[4]=='1') &&(word[5]=='4')) || ((word[4]=='2') && (word[5]=='3')) )
		){
	t.type=kGroup;
	t.group.phi=atoi(word);
	t.group.z=word[2];
	if((word[4]=='1')&&(word[5]=='2')){t.group.layer=12;} // grouping of the channels
	if((word[4]=='3')&&(word[5]=='4')){t.group.layer=34;} //
	if((word[4]=='1')&&(word[5]=='4')){t.group.layer=14;} // grouping for the power supply
	if((word[4]=='2')&&(word[5]=='3')){t.group.layer=23;} //
	t.token=CharToStr(word,l);

      // allow shell names +P,-P,N,-N
      }else if( 
	       (l==2) &&
	       ((word[0]=='+') || (word[0]=='-')) &&
	       ((word[1]=='P') || (word[1]=='N'))
	       ){
	t.type=kShell;
	t.shell.x=word[0];
	t.shell.z=word[1];
	t.token=CharToStr(word,l);
	   
      }else if( index("+-.$0123456789",word[0])!=NULL ){
	// number

	char temp[100];
	strncpy(temp,word,l);
	if ( StrToI(word,l,&i1) ){
	  if ( StrToF(word,l,&f) ){
	    cerr << "illegal number ["<<temp << "]"<<endl;
	    return 0;
	  }
	  // float
	  t.f=f;
	  t.type=kFloat;
	}else{ // integer 
	  t.i=i1;
	  t.type=kInt;
	}
      }else if( (l==1)&&(word[0]==';') ){
	lStack.push_back(lTokens);
	lTokens.clear();
	word=GetWord(NULL,&l);
	continue;
      }else{
	// string
	t.token=CharToStr(word,l);
	t.type=kString;
      }
      lTokens.push_back(t);
      word=GetWord(NULL,&l);
    }// done with the first pass

  // now put stuff on the global stack (in reverse order)
  cmdStack.push_back(lTokens);
  while(!(lStack.empty())){
    cmdStack.push_back(lStack.back());
    lStack.pop_back();
  }
  
  // prepare execution 
  return Pop();
}

//------------------------------------------------------------
int SimpleCommand::Pop(){
  //copy the the commands on the top of the stack into the visible/user area
  // returns 1 if there is something to be executed, 0 if not
  tokens.assign(cmdStack.back().begin(), cmdStack.back().end() );
  cmdStack.pop_back();
//   cout << "just popped: ";
//   for(int i=0; i<tokens.size(); i++){
//     cout << " [" <<tokens[i].token <<"]";
//   }
//   cout << endl;
  

  // look for commands for the Parser itself
    if(tokens[0].token=="verbose"){
      verbose=1;
      if(cmdStack.empty()){return 0;}else{return Pop();}
    }else if(tokens[0].token=="quiet"){
      verbose=0;
      if(cmdStack.empty()){return 0;}else{return Pop();}
    }else if(tokens[0].token=="exec"){
      if((tokens.size())==2){
	// this leads to a recursive call of Parse
	if(Read(tokens[1].token)==0){ // file is still open
	  return Next();  // avoid returning empty commands
	}else{
	  return 0;
	}
      }else {
	cerr << "exec filename??" << endl;
	return 0;
      }
    }else if(tokens[0].token=="echo"){
      for(unsigned int i=1; i<tokens.size(); i++){
	cout << tokens[i].token<< " ";
      }
      cout << endl;
      if(cmdStack.empty()){return 0;}else{return Pop();}
    }else if(tokens[0].token=="exit"){
      // stop reading
      exitFlag=true;
      return 0;
    }
  return 1;  // do something
}

//------------------------------------------------------------

int SimpleCommand::Next()
  /* load the next simple command into the user accessible area.
     for single lines this implements iterations over lists
     when reading commands from a file it will also
     get the next line until the end of file is encountered

     returns 1 if successful, executable command has been loaded
     returns 0 if no more commands/iterations are available

   */
{
  /* list iteration not implemented 
        if( ( narg>0 ) && (iHub<target.nModule)
	    && ( (iHub<target.nModule-1)||(iRoc<target.nRoc-1)) )
	{

		// incrementmodule/roc id
		iRoc++;
		if(iRoc>=target.nRoc)
		{
			iRoc=0; iHub++;
		}

		// update visible addresses
		module=target.hub[iHub];
		roc   =target.id[iRoc];

		// advance pointers for list arguments
		for(int j=0; j<narg; j++)
		{
		  if(isIterated[j])
		    {
		      iarg[j]++;
		      *(iarg[j])=temp[j];
		      temp[j]=*(iarg[j]+1);  // no overrun, -1 always follows
		      *(iarg[j]+1)=-1;
		    }
		}
		return 1;  // i.e. execute this

	}
	else
	{  
  */

  // execute things on the stack 
  if(!cmdStack.empty()){
    return Pop();
  }

  // no more targets from previously parsed line
  // are we reading from a file ?

  if(nOpen>0){
    char line[100];
    // skip lines not meant to be executed
    while((Getline(line,99)==0)&&(Parse(line)==0));
    return nOpen>0;
  }else{  // no file, nothing else to do
    return 0;
  }

}


//------------------------------------------------------------

int SimpleCommand::Getline(char* line, int n)
{
  while( !(inputFile->good()) && (nOpen>0) ){
    delete inputFile;
    nOpen--;
    if(nOpen>0){
      inputFile=fileStack[nOpen-1];
    }else{
      //inputFile==NULL; has no effect, says the compiler
      return 1;
    }
  }
  if(nOpen>0){
    inputFile->getline(line,n);
    return 0;
  }else{
    return 1;
  }
    
}
//------------------------------------------------------------

int SimpleCommand::Read(const string& fileName)
{

  // open a new file
  if(nOpen<nFileMax){
    inputFile=new ifstream(fileName.c_str());
	if (inputFile->is_open() )
	{
	        fileStack[nOpen++]=inputFile;
		char line[100];
		inputFile->getline(line,99);
		Parse(line);  // this may be a recursive call
		return 0;  // 0 means file is open
	}
	else
	{
		cerr << " Unable to open file " << endl;
		delete inputFile;
		inputFile=nOpen>0 ? fileStack[nOpen-1] : NULL;
		return 1;  // 1 means no file is open
	}
  }else{
    cerr << "sorry, maximum number of open files exceeded " << endl;
    return 1;
  }
}

//------------------------------------------------------------

bool SimpleCommand::Exit() {
	return exitFlag;
}


//------------------------------------------------------------

int SimpleCommand::PrintList(char* buf, int n, int v[]){
  int a,b;
  int i=0;
  int l=0;

  while((i<n)&&(l<90)){
    a=b=v[i++];
    while((i<n)&&(v[i]==(b+1))){b++; i++;}
    if(a==b){
      l+=sprintf(&buf[l],"%d",a);
    }else{
      l+=sprintf(&buf[l],"%d:%d",a,b);
    }
    if(i<n) l+=sprintf(&buf[l],",");
  }
  return l;
}

// //-----------------------------------------------------------
// // control network name to index mapping:
// int SimpleCommand::getCNidx(const std::string& s){
//   int idx=atoi(s.substr(1,1).c_str())-1;
//   if(s.substr(0,1)=="-") idx+=16;
//   if(s.substr(2,1)=="N") idx+=8;
//   if(s.substr(4,1)=="3") idx+=32;
//   return idx;
// }


// std::string SimpleCommand::getCNname(const int idx){
//   std::ostringstream o;
//   int phi=(idx%8)+1;
//   if (idx % 32 <16){  o<<"+"<<phi;  }else{  o<<"-"<<phi;}
//   if (idx % 16 < 8){  o<<"P" ;      }else{  o<<"N";     }
//   if (idx <32     ){  o<<"L12";     }else{  o<<"L3" ;   }
//   return o.str();
// }



//-----------------------------------------------------------


  /* helper functions for analyzing commands of the type
          keyword value1 value2 ...
     in Execute methods.
     Keyword(...) returns true iff the keyword and the number of
     integer arguments match.
     On return the integer pointers point to the values in 
     the commands internal buffer.
  */

bool SimpleCommand::Keyword(const string& keyword){
  if( (tokens.size()==1) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword))
  {
    return true;
  }else{
    return false;
  }
}

bool SimpleCommand::Keyword(const string& keyword, int& value){
  if( (tokens.size()==2) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword) &&
      (tokens[1].type==kInt   )
      )
  {
    value=tokens[1].i;
    return true;
  }else{
    return false;
  }
}

bool SimpleCommand::Keyword(const string& keyword, float & value){
  if( (tokens.size()==2) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword) &&
      ((tokens[1].type==kFloat) || (tokens[1].type==kInt))
      )
  {
    if (tokens[1].type==kFloat){
      value=tokens[1].f;
    }else{
      value=float(tokens[1].i);
    }
    return true;
  }else{
    return false;
  }
}

bool SimpleCommand::Keyword(const string& keyword1, const string& keyword2){
  if( (tokens.size()==2) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kString) && (tokens[1].token==keyword2) 
      )
  {
    return true;
  }else{
    return false;
  }
}

bool SimpleCommand::Keyword(const string& keyword1, const string& keyword2, int& value){
  if( (tokens.size()==3) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kString) && (tokens[1].token==keyword2) &&
      (tokens[2].type==kInt)
      )
  {
    value=tokens[2].i;
    return true;
  }else{
    return false;
  }
}





int SimpleCommand::Keyword(const string& keyword1, const vector<string>& keyword2){
  // note: this one does not return a bool, but an int >=0 in case of a match
  //       it returns -1 in case of no match
  // use it like  if ((index=command.Keyword("keyword1",vectorOfKeywords)>0){ ....
  if( (tokens.size()==2) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kString) 
      )
  {
    for (unsigned int i=0; i<keyword2.size(); i++){
      if ( keyword2[i]==tokens[1].token ){ return (int) i; }
    }
    return -1;
  }else{
    return -1;
  }
}

int SimpleCommand::Keyword(const string& keyword1, const vector<string>& keyword2, int& value){
  // note: this one does not return a bool, but an int >=0 in case of a match
  //       it return -1 in case of no match
  // use it like  if ((index=command.Keyword("keyword1",vectorOfKeywords)>0){ ....
  if( (tokens.size()==3) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kString) &&
      (tokens[2].type==kInt)
      )
  {
    for (unsigned int i=0; i<keyword2.size(); i++){
      if ( keyword2[i]==tokens[1].token ){ value=tokens[2].i; return (int) i; }
    }
    return -1;
  }else{
    return -1;
  }
}


bool SimpleCommand::Keyword(const string& keyword, int& value1, int& value2){
  if( (tokens.size()==3) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword) &&
      (tokens[1].type==kInt   ) &&
      (tokens[2].type==kInt   )  
      )
  {
    value1=tokens[1].i;
    value2=tokens[2].i;
    return true;
  }else{
    return false;
  }
}

bool SimpleCommand::Keyword(const string& keyword, float& value1, float& value2){
  if( (tokens.size()==3) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword) &&
      (tokens[1].type==kFloat   ) &&
      (tokens[2].type==kFloat   )  
      )
  {
    if (tokens[1].type==kFloat){
      value1=tokens[1].f;
    }else{
      value1=tokens[1].i;
    }

    if (tokens[2].type==kFloat){
      value2=tokens[2].f;
    }else{
      value2=tokens[2].i;
    }

    return true;
  }else{
    return false;
  }
}

bool SimpleCommand::Keyword(const string& keyword, int& value1, int& value2, int& value3){
  if( (tokens.size()==4) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword) &&
      (tokens[1].type==kInt   ) &&
      (tokens[2].type==kInt   ) &&
      (tokens[3].type==kInt   ) 
      )
  {
    value1=tokens[1].i;
    value2=tokens[2].i;
    value3=tokens[3].i;
    return true;
  }else{
    return false;
  }
}

 
bool SimpleCommand::Keyword(const string& keyword,
			    int& value1, int& value2, int& value3, int& value4, int& value5, int& value6){
  if( (tokens.size()==7) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword) &&
      (tokens[1].type==kInt   ) &&
      (tokens[2].type==kInt   ) &&
      (tokens[3].type==kInt   ) &&
      (tokens[4].type==kInt   ) &&
      (tokens[5].type==kInt   ) &&
      (tokens[6].type==kInt   ) 
      )
  {
    value1=tokens[1].i;
    value2=tokens[2].i;
    value3=tokens[3].i;
    value4=tokens[4].i;
    value5=tokens[5].i;
    value6=tokens[6].i;
    return true;
  }else{
    return false;
  }
}

 
bool SimpleCommand::Keyword(const string& keyword1, int& value1, int& value2, const string& keyword2){
  if( (tokens.size()==4) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kInt   ) &&
      (tokens[2].type==kInt   ) &&
      (tokens[3].type==kString) && (tokens[3].token==keyword2)  
      )
  {
    value1=tokens[1].i;
    value2=tokens[2].i;
    return true;
  }else{
    return false;
  }
}

 
bool SimpleCommand::Keyword(const string& keyword1, sector_t& sect){
  if( (tokens.size()==2) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kSector) 
      )
  {
    sect=tokens[1].sector;
    return true;
  }else{
    return false;
  }
}


bool SimpleCommand::Keyword(const string& keyword1, sector_t& sect, const string& keyword2, int& lay){
  if( (tokens.size()==4) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kSector) &&
      (tokens[2].type==kString) &&(tokens[0].token==keyword2) &&
      (tokens[3].type==kInt   ) 
      )
  {
    sect=tokens[1].sector;
    lay =tokens[3].i;
    return true;
  }else{
    return false;
  }
}


bool SimpleCommand::Keyword(const string& keyword1, int& value1, int& value2, sector_t& sect){
  if( (tokens.size()==4) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kInt) &&
      (tokens[2].type==kInt) &&
      (tokens[3].type==kSector) 
      )
  {
    sect=tokens[3].sector;
    value1 = tokens[1].i;
    value2 = tokens[2].i;
    return true;
  }else{
    return false;
  }
}

 
bool SimpleCommand::Keyword(const string& keyword1, group_t& sect){
  if( (tokens.size()==2) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kGroup) 
      )
  {
    sect=tokens[1].group;
    return true;
  }else{
    return false;
  }
}

bool SimpleCommand::Keyword(const string& keyword1, group_t& group, int& value1, int& value2, int& value3, int& value4){
  if( (tokens.size()==6) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kGroup)  &&
      (tokens[2].type==kInt)  &&
      (tokens[3].type==kInt)  &&
      (tokens[4].type==kInt)  &&
      (tokens[5].type==kInt) 
      )
  {
    group=tokens[1].group;
    value1=tokens[2].i;
    value2=tokens[3].i;
    value3=tokens[4].i;
    value4=tokens[5].i;
    return true;
  }else{
    return false;
  }
}


bool SimpleCommand::Keyword(const string& keyword1, group_t& group, int& value1, int& value2, int& value3, int& value4, int& value5){
  if( (tokens.size()==7) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kGroup)  &&
      (tokens[2].type==kInt)  &&
      (tokens[3].type==kInt)  &&
      (tokens[4].type==kInt)  &&
      (tokens[5].type==kInt)  &&
      (tokens[6].type==kInt) 
      )
  {
    group=tokens[1].group;
    value1=tokens[2].i;
    value2=tokens[3].i;
    value3=tokens[4].i;
    value4=tokens[5].i;
    value5=tokens[6].i;
    return true;
  }else{
    return false;
  }
}



bool SimpleCommand::Keyword(const string& keyword1, const string& keyword2, int& value1, int& value2, int& value3, int& value4){
  if( (tokens.size()==6) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kString) && (tokens[1].token==keyword2) &&
      (tokens[2].type==kInt)  &&
      (tokens[3].type==kInt)  &&
      (tokens[4].type==kInt)  &&
      (tokens[5].type==kInt) 
      )
  {
    value1=tokens[2].i;
    value2=tokens[3].i;
    value3=tokens[4].i;
    value4=tokens[5].i;
    return true;
  }else{
    return false;
  }
}


bool SimpleCommand::Keyword(const string& keyword1, const string& keyword2, int& value1, int& value2, int& value3, int& value4, int& value5){
  if( (tokens.size()==7) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kString) && (tokens[1].token==keyword2) &&
      (tokens[2].type==kInt)  &&
      (tokens[3].type==kInt)  &&
      (tokens[4].type==kInt)  &&
      (tokens[5].type==kInt)  &&
      (tokens[6].type==kInt) 
      )
  {
    value1=tokens[2].i;
    value2=tokens[3].i;
    value3=tokens[4].i;
    value4=tokens[5].i;
    value5=tokens[6].i;
    return true;
  }else{
    return false;
  }
}



bool SimpleCommand::Keyword(const string& keyword1, shell_t& shell, int& value){
//   if (tokens.size()==3){
//     cout << "kw string shell int " << endl;
//     for(unsigned int i=0; i<tokens.size(); i++){
//       cout << i<< ") " <<  tokens[i].token << " " << tokens[i].type << endl;
//     }
//   }
  if( (tokens.size()==3) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kShell)  &&
      (tokens[2].type==kInt) 
      )
  {
    shell=tokens[1].shell;
    value=tokens[2].i;
    return true;
  }else{
    return false;
  }
}



bool SimpleCommand::Keyword(const string& keyword1, shell_t& shell, const string& keyword2){
  if( (tokens.size()==3) &&
      (tokens[0].type==kString) && (tokens[0].token==keyword1) &&
      (tokens[1].type==kShell)  &&
      (tokens[2].type==kString) && (tokens[2].token==keyword2) 
      )
  {
    shell=tokens[1].shell;
    return true;
  }else{
    return false;
  }
}


// this is a non-member function
std::ostream& operator<< (std::ostream &os, const SimpleCommand& c){
  for(unsigned int i=0; i<c.tokens.size(); i++){
    os << c.tokens[i].token << " ";
  }
  return os;
}

