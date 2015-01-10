#ifndef MYDYNAMICAPP_H_
#define MYDYNAMICAPP_H_

//Core imports
#include <string>

//xdaq core imports
#include <xdaq/Application.h>
#include <xdaq/WebApplication.h>
#include <xdaq/ApplicationStub.h>
#include <xdaq/ApplicationStubImpl.h>
#include <xdaq/exception/Exception.h>

//xgi
#include <xgi/Input.h>
#include <xgi/Output.h>
#include <xgi/exception/Exception.h>

//Util import
#include <pixel/DynamicWebApplication.h>

//Local Import
#include "AGUI.h"

namespace tests{

class MyDynamicApp : public pixel::DynamicWebApplication
{

public:
	XDAQ_INSTANTIATOR();
	MyDynamicApp(xdaq::ApplicationStub* stub) throw(xdaq::exception::Exception);
	//void Default(xgi::Input* in, xgi::Output* out) throw(xgi::exception::Exception);
	virtual ~MyDynamicApp();
	
private:
	tests::AGUI* gui;
};
}
#endif /*MYDYNAMICAPP_H_*/
