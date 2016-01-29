#ifndef _PixelTKFECSupervisor_exception_PixelException_h_
#define _PixelTKFECSupervisor_exception_PixelException_h_

#include "xcept/Exception.h"
#include <iostream>

namespace pixel {

	class PixelTKFECSupervisorException : public xcept::Exception
	{
		public : 

			PixelTKFECSupervisorException(std::string name, std::string message, std::string module, int line, std::string function):
			   xcept::Exception(name, message, module, line,function) {}


			PixelTKFECSupervisorException(std::string name, std::string message, std::string module, int line, std::string function, xcept::Exception & e):
				xcept::Exception(name, message, module, line, function, e) {}


	};
}

#endif	
