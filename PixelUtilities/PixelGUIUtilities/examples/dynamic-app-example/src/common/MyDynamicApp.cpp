#include "MyDynamicApp.h"
#include "AnotherGUI.h"
#include "AGUI.h"

//Core Imports
#include <string>
#include <iostream>

//xdaq core imports
#include <xdaq/Application.h>
#include <xdaq/WebApplication.h>
#include <xdaq/ApplicationStub.h>
#include <xdaq/ApplicationStubImpl.h>
#include <xdaq/NamespaceURI.h>
#include <xdaq/exception/Exception.h>

//xgi
#include <xgi/Input.h>
#include <xgi/Output.h>
#include <xgi/exception/Exception.h>
#include <xgi/Method.h>

//cgicc
#include <cgicc/Cgicc.h>
#include <cgicc/HTMLDoctype.h>
#include <cgicc/HTMLClasses.h>

//i2o


using namespace tests;

XDAQ_INSTANTIATOR_IMPL(tests::MyDynamicApp);

MyDynamicApp::MyDynamicApp(xdaq::ApplicationStub* stub) throw(xdaq::exception::Exception) : pixel::DynamicWebApplication(stub)
{

	LOG4CPLUS_INFO(this->getApplicationLogger(), "Hello World!");

	this->gui = new AGUI(this);
	this->addGuiClass(gui);
	this->addGuiClass(new AnotherGUI);
	this->bind();
	
}

MyDynamicApp::~MyDynamicApp()
{

}
