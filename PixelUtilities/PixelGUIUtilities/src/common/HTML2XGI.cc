#include "HTML2XGI.h"

#include <string>
#include <fstream>

void HTML2XGI(xgi::Output *out, std::string filename)
{
	std::string line;
	std::ifstream fin(filename.c_str());
	while (getline(fin,line))
	{
		*out<<line;
		*out<<std::endl;
	}
}
