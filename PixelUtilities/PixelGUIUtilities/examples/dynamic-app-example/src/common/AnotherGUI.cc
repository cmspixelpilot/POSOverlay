#include "AnotherGUI.h"

//xgi
#include <xgi/Input.h>//xgi::Input
#include <xgi/Output.h>//xgi::Output
#include <xgi/Method.h>//xgi::bind()
//cgicc
#include <cgicc/HTMLClasses.h>
#include <cgicc/Cgicc.h>

using namespace tests;

AnotherGUI::AnotherGUI() {
	xgi::bind(this, &AnotherGUI::anotherCallback, "Callback");
}

AnotherGUI::~AnotherGUI() {
}

void AnotherGUI::anotherCallback(xgi::Input* in, xgi::Output* out) {

	*out << cgicc::html() << std::endl;

	*out << cgicc::p() << std::endl;

	*out << cgicc::h1() << std::endl;
	*out << "Another Test Callback"<< std::endl;
	*out << cgicc::h1() << std::endl;

	*out << cgicc::p() << std::endl;

	*out << cgicc::html() << std::endl;
}
