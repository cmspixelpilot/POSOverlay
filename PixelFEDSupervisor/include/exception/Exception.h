#ifndef _PixelFEDSupervisor_exception_PixelException_h_
#define _PixelFEDSupervisor_exception_PixelException_h_

#include "xcept/Exception.h"
#include <iostream>

namespace pixel {

	class PixelFEDSupervisorException : public xcept::Exception
	{
		public : 

			PixelFEDSupervisorException(std::string name, std::string message, std::string module, int line, std::string function):
			   xcept::Exception(name, message, module, line,function) {}


			PixelFEDSupervisorException(std::string name, std::string message, std::string module, int line, std::string function, xcept::Exception & e):
				xcept::Exception(name, message, module, line, function, e) {}


	};
}

#endif	
