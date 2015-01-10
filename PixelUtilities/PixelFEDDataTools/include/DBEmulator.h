#ifndef _PixelFEDDataTools_DB_Emulator_
#define _PixelFEDDataTools_DB_Emulator_
#include <iostream>
#include <string.h>
#include <fstream>
#include <stdlib.h>

// This could effectively be turned into a class, but what me worry?

std::string ReadField(std::string filename, std::string field)
{
	ifstream fin(filename.c_str());
	std::string line, entry="";
	unsigned long int npos=std::string::npos;

	while (!fin.eof())
	{
		getline (fin, line);
		if (line.find(field)!=npos)
			entry=line.substr(line.find(":")+1, npos);
	}
	fin.close();
	return entry;
}


int FillField(std::string filename, std::string field, std::string entry)
{
	ifstream fin(filename.c_str());
	ofstream fout("buffer.tmp", ios::trunc);
	unsigned long int npos=std::string::npos;
	//std::string::size_type loc;
	std::string line;
	int pass=0;

	while (!fin.eof())
	{
		getline(fin, line);
		if (line.find(field)!=npos)
		{
			fout<<field<<entry<<endl;
			pass++;
		}
		else
		{
			fout<<line<<endl;
		}
	}

	fin.close();
	fout.close();

	line="cp buffer.tmp "+filename;
	system(line.c_str());
	system("rm buffer.tmp");
	return pass;
}
#endif
