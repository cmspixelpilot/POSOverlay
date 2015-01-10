#include "PixelUtilities/PixelGUIUtilities/include/DynamicWebApplication.h"

//core
#include <vector>

//xdaq
#include <xdaq/WebApplication.h>//xdaq::WebApplication

//xgi
#include <xgi/Input.h>//xgi::Input
#include <xgi/Output.h>//xgi::Output
#include <xgi/Method.h>//xgi::bind()
#include <xgi/exception/Exception.h>//xgi::exception::Exception

namespace pixel {

/**
 * Just calls the constructor for the xdaq::WebApplication, so that
 * the user can act like this is just a normal xdaq::Application.
 * 
 */
DynamicWebApplication::DynamicWebApplication(xdaq::ApplicationStub* stub) :
	xdaq::WebApplication(stub) {
	
	this->foundMain = false;
	this->bound = false;

}

/**
 * No explict destruction is necessary
 */
DynamicWebApplication::~DynamicWebApplication() {
}

/** \brief The main callback for your application (it is suggested
 *  that you do not override this function).
 * 
 * This function deals with routing the dynamic bindings.  Whatever
 * names you gave when you called xgi::bind() in your constructor for
 * your GUI class is the name that shall be used to access your callback.
 * for example, if you called it "myCallback", by calling
 * xgi::bind(this, &YourClass::YourFunction, "myCallback"), you go to 
 * http://<your hostname>:<your portname>/urn:xdaq-application:lid=<your app id>/myCallback
 * to access your callback.
 * 
 */
void DynamicWebApplication::Default(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception){

	//Get the name of the callback
	std::string name = in->getenv("PATH_INFO");

	//If it's blank, call the "Main" method of the GUI class
	//If a blank callback is called with no other function bound, for some reason it seg faults...?
	if (name.compare("")==0|| name.compare("Main")==0) {
		if(this->foundMain) {
		guiMethods["Main"]->invoke(in, out);
		} else {
			XCEPT_RAISE( xgi::exception::Exception, "Main callback was not bound properly!" );		
		}
	} else {
		guiMethods[name]->invoke(in, out);
	}

}

/** \brief Binds all the methods for all the registered GUI classes.
 * 
 * This method takes all the classes that have been registered using 
 * the addGuiClass() method and binds all of the callbacks.
 * <p/>
 * You should call this method after you have called addGuiClass()
 * for all of your gui classes.  Also, you may check to see if
 * it has been bound by calling isBound(), if you wish.
 * 
 */
void DynamicWebApplication::bind() {
	xgi::bind(this, &DynamicWebApplication::Default, "Default");

	//Get all the methods registered with the GUIs

	std::vector<toolbox::lang::Class*>::iterator classes;

	for (classes = guiClasses.begin(); classes != guiClasses.end(); classes++) {
		std::vector<toolbox::lang::Method*> v = (*classes)->getMethods();
		std::vector<toolbox::lang::Method*>::iterator i;

		//Bind all of them to the routing callback
		for (i = v.begin(); i != v.end(); ++i) {
			if ((*i)->type() == "cgi") {
				std::string name = static_cast<xgi::MethodSignature*>( *i )->name();
				
				//Set the flag that "Main" was found, if it was
				if(name.compare("Main")==0) {
					this->foundMain = true;
				}
				xgi::bind(this, &DynamicWebApplication::Default, name);
				this->guiMethods[name] = static_cast<xgi::MethodSignature*>((*classes)->getMethod(name));
			}
		}
	}
	this->bound = true;
}

}
