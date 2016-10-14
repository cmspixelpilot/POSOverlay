#ifndef _PixelFECSupervisor_exception_PixelException_h_
#define _PixelFECSupervisor_exception_PixelException_h_

#include "xcept/Exception.h"
#include <iostream>

namespace pixel {

	class PixelFECSupervisorException : public xcept::Exception
	{
		public : 

			PixelFECSupervisorException(std::string name, std::string message, std::string module, int line, std::string function):
			   xcept::Exception(name, message, module, line,function) {}


			PixelFECSupervisorException(std::string name, std::string message, std::string module, int line, std::string function, xcept::Exception & e):
				xcept::Exception(name, message, module, line, function, e) {}


	};
}

#endif	
