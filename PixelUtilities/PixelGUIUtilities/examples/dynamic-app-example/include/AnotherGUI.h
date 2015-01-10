#ifndef ANOTHERGUI_H_
#define ANOTHERGUI_H_

//Superclass
#include <toolbox/lang/Class.h>//toolbox::lang::Class

//xgi
#include <xgi/Input.h>
#include <xgi/Output.h>

namespace tests {

class AnotherGUI : public toolbox::lang::Class
{
public:
	AnotherGUI();
	virtual ~AnotherGUI();
	
	void anotherCallback(xgi::Input* in, xgi::Output* out);
};
}
#endif /*ANOTHERGUI_H_*/
