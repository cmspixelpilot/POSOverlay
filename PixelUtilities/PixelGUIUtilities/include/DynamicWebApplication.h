#ifndef DYNAMICWEBAPPLICATION_H_
#define DYNAMICWEBAPPLICATION_H_

//core imports
#include <vector>
#include <map>
#include <string>

//Xdaq core
#include <xdaq/WebApplication.h>

//xgi
#include <xgi/Input.h>
#include <xgi/Output.h>
#include <xgi/Method.h>
#include <xgi/exception/Exception.h>//xgi::exception::Exception

//toolbox
#include <toolbox/lang/Class.h>

namespace pixel
{

/** \brief A web application class that is designed to allow for
 * easy factorization of GUI code.
 * 
 * This class inherits from xdaq::WebApplication, so you may use it
 * in place of xdaq::Application and xdaq::WebApplicaiton.  The idea
 * behind this class is that you create one or more other seperate classes
 * to hold your GUI code, in order to keep your application implementation
 * as clean as possible.  The callback functions that are part of these
 * classes are then dynamically bound.  
 * 
 * In order for this class to know what functions to bind to callbacks,
 * you must call xgi::bind() in your classes constructor for the functions
 * that you want bound.  For example, if you have a callback function that is
 * <pre>
 * void MyClass::myCallback(xgi::Input* in, xgi::Output* out);
 * </pre>
 * you will need this line in your constructor:
 * <pre>
 * xgi::bind(this, &MyClass::myCallback, "myCallback");//you can change "myCallback" to whatever you want
 * </pre>
 * 
 * This would allow you to access your callback via the following URL in your
 * browser: 
 * http://< your hostname >:< your port >/urn:xdaq-application:lid=< your app id >/myCallback
 * (of course, you replace "<your hostname>," "<your port>,", and "<your app id>" with the
 * appropriate information to make this work).
 * 
 * In order to get all of this to work, you need to do the following:
 * <br/>
 * 1) Inherit from pixel::DynamicWebApplication (do everything just like normal,
 * ie, just like you do with an xdaq::Application or xdaq::WebApplication... see 
 * http://xdaqwiki.cern.ch for more information).
 * <br/>
 * 2) In the constructor, do something similar to the following:
 * 
 * <pre>
 * //...other initialization stuff
 * this->addGuiClass(new MyClass);//where MyClass is a class that did the things described above.
 * this->bind();	
 * </pre>
 * 
 * This code would then allow all callbacks registered in your version of "MyClass"
 * to be accessed as described above.
 * 
 * Also of note: if you just go to the root of the URN, ie, if you go to
 * http://< your hostname >:< your port >/urn:xdaq-application:lid=< your app id >/
 * , the DynamicWebApplication tries to call a function that is bound as "Main" with
 * xgi::bind() (xgi::bind(this, &YourGuiClass::Main, "Main") ).  If it can't call it,
 * you'll get an exception in the webinterface, until the user types in a proper callback.
 * 
 * 
 */
class DynamicWebApplication : public xdaq::WebApplication
{
public:
	DynamicWebApplication(xdaq::ApplicationStub* stub);
	virtual ~DynamicWebApplication();
	
	void Default(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
	void bind();
	
	/** \brief Adds a class that has inherited from toolbox::lang::Class to the internal list of 
	 * gui classes
	 * 
	 * You must call this method once for each of your gui classes that you wish to be associated
	 * with this application.  See the DynamicWebApplication class description for
	 * an overview of how everything works together.
	 * 
	 */
	inline void addGuiClass(toolbox::lang::Class* guiClass) {this->guiClasses.push_back(guiClass);}
	
	/** \brief Returns true after bind() has been called.
	 * 
	 */
	inline bool isBound() {return bound;}
	
	/** \brief Returns the vector that contains all classes that have been registered via addGuiClass().
	 * 
	 */
	inline const std::vector<toolbox::lang::Class*>& getGuiClasses() {return guiClasses;}
	
private:
	std::vector<toolbox::lang::Class*> guiClasses;
	std::map<std::string, xgi::MethodSignature*> guiMethods;
	bool bound;
	bool foundMain;
	
};

}

#endif /*DYNAMICWEBAPPLICATION_H_*/
