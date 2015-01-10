#include "AGUI.h"

//xdaq core

//xgi
#include <xgi/Method.h>
#include <xgi/Input.h>
#include <xgi/Output.h>

//cgicc
#include <cgicc/HTMLClasses.h>
#include <cgicc/Cgicc.h>

namespace tests {

AGUI::AGUI(tests::MyDynamicApp* app) :
	app_(app) {
	xgi::bind(this, &AGUI::Main, "Main");
	xgi::bind(this, &AGUI::fsmControl, "fsmControl");
}

AGUI::~AGUI() {
}

/**
 * The main entry point for the GUI.
 * 
 * 
 */
void AGUI::Main(xgi::Input* in, xgi::Output* out) {
	*out << cgicc::html() << std::endl;

	*out << cgicc::p() << std::endl;

	*out << "Main callback."<< std::endl;

	*out << cgicc::p() << std::endl;

	*out << cgicc::html() << std::endl;
}

//Controls the finite state machine
void AGUI::fsmControl(xgi::Input* in, xgi::Output* out) {
	*out << cgicc::html() << std::endl;

	*out << cgicc::p() << std::endl;

	*out << "Finite State Machine Control"<< std::endl;

	*out << cgicc::p() << std::endl;

	*out << cgicc::html() << std::endl;
}

}
