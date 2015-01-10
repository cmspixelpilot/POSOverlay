#ifndef AGUI_H_
#define AGUI_H_

//Core imports
#include <string>

//xdaq core imports
#include <xdaq/WebApplication.h>
#include <xdaq/exception/Exception.h>

//xgi
#include <xgi/Input.h>
#include <xgi/Output.h>
#include <xgi/exception/Exception.h>

//toolbawx
#include <toolbox/lang/Class.h>


namespace tests {

class MyDynamicApp;


class AGUI : public toolbox::lang::Class 
{
public:
	AGUI(tests::MyDynamicApp* app);
	void Main(xgi::Input* in, xgi::Output* out);
	void fsmControl(xgi::Input* in, xgi::Output* out);
	virtual ~AGUI();
	
private:
	tests::MyDynamicApp* app_;
};

}

#endif /*AGUI_H_*/
