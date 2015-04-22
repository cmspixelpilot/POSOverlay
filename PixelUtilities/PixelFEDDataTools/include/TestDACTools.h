#ifndef _TestDACTools_h_
#define _TestDACTools_h_

#include <iostream>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include "Converter.h"
#include "LevelEncoderDecoder.h"

std::vector<unsigned int> createPulseTrain(std::string filename)
{
	const unsigned int UB=100;
	const unsigned int B=332;
	const unsigned int offset=0;
        vector <unsigned int> pulseTrain(256), pixelDCol(1), pixelPxl(2), pixelTBMHeader(3), pixelTBMTrailer(3);
        unsigned int DCol, LorR, start=5;
	std::string line;
	std::string::size_type loc1, loc2, loc3, loc4;
	//unsigned int npos=std::string::npos;
	int i;

	// Initialise the pulseTrain to offset+black
        for (unsigned int i=0;i<pulseTrain.size();++i)
        {
                pulseTrain[i]=offset+B;
        }

	ifstream fin(filename.c_str());

	i=start;
	
	while (!fin.eof())
	{
		getline(fin, line);
		
		if (line.find("TBMHeader")!=std::string::npos)
		{
			loc1=line.find("("); if (loc1==std::string::npos) {cout<<"'(' not found after TBMHeader.\n"; break;}
			loc2=line.find(")", loc1+1); if (loc2==std::string::npos) {cout<<"')' not found after TBMHeader.\n"; break;}
			int TBMHeader=atoi(line.substr(loc1+1,loc2-loc1-1).c_str());
			
			pulseTrain[i]=UB;++i;
			pulseTrain[i]=UB;++i;
			pulseTrain[i]=UB;++i;
			pulseTrain[i]=B;++i;

			pixelTBMHeader=DecimalToBaseX(TBMHeader, 6, 4);
			pulseTrain[i]=levelEncoder(pixelTBMHeader[3]);++i;
			pulseTrain[i]=levelEncoder(pixelTBMHeader[2]);++i;
			pulseTrain[i]=levelEncoder(pixelTBMHeader[1]);++i;
			pulseTrain[i]=levelEncoder(pixelTBMHeader[0]);++i;
		}
		else if (line.find("ROCHeader")!=std::string::npos)
		{
			loc1=line.find("("); if (loc1==std::string::npos) {cout<<"'(' not found after ROCHeader.\n"; break;}
                        loc2=line.find(")", loc1+1); if (loc2==std::string::npos) {cout<<"')' not found after ROCHeader.\n"; break;}
                        int LastDAC=atoi(line.substr(loc1+1,loc2-loc1-1).c_str());
			
			pulseTrain[i]=UB;++i;
			pulseTrain[i]=B;++i;
			pulseTrain[i]=levelEncoder(LastDAC); ++i;
		}
		else if (line.find("PixelHit")!=std::string::npos)
		{
			loc1=line.find("("); if (loc1==std::string::npos) {cout<<"'(' not found after PixelHit.\n"; break;}
                        loc2=line.find(",", loc1+1); if (loc2==std::string::npos) {cout<<"',' not found after the first argument of PixelHit.\n"; break;}
			loc3=line.find(",", loc2+1); if (loc3==std::string::npos) {cout<<"'.' not found after the second argument of PixelHit.\n"; break;}
			loc4=line.find(")", loc3+1); if (loc4==std::string::npos) {cout<<"')' not found after the third argument of PixelHit.\n"; break;}
			int column=atoi(line.substr(loc1+1, loc2-loc1-1).c_str());
			int row=atoi(line.substr(loc2+1, loc3-loc2-1).c_str());
			int charge=atoi(line.substr(loc3+1, loc4-loc3-1).c_str());
			
			DCol=int(column/2);
			LorR=int(column-DCol*2);
			pixelDCol=DecimalToBaseX(DCol, 6, 2);
			pixelPxl=DecimalToBaseX((80-row)*2+LorR, 6, 3);
			
			pulseTrain[i]=levelEncoder(pixelDCol[0]);++i;
			pulseTrain[i]=levelEncoder(pixelDCol[1]);++i;
			pulseTrain[i]=levelEncoder(pixelPxl[0]);++i;
			pulseTrain[i]=levelEncoder(pixelPxl[1]);++i;
			pulseTrain[i]=levelEncoder(pixelPxl[2]);++i;
			pulseTrain[i]=charge;++i;

		}
		else if (line.find("TBMTrailer")!=std::string::npos)
		{
			loc1=line.find("("); if (loc1==std::string::npos) {cout<<"'(' not found after TBMTrailer.\n"; break;}
                        loc2=line.find(")", loc1+1); if (loc2==std::string::npos) {cout<<"')' not found after TBMTrailer.\n"; break;}
                        int TBMTrailer=atoi(line.substr(loc1+1,loc2-loc1-1).c_str());

			pulseTrain[i]=UB;++i;
			pulseTrain[i]=UB;++i;
			pulseTrain[i]=B; ++i;
			pulseTrain[i]=B; ++i;
			
			pixelTBMTrailer=DecimalToBaseX(TBMTrailer, 6, 4);
                        pulseTrain[i]=levelEncoder(pixelTBMTrailer[3]);++i;
                        pulseTrain[i]=levelEncoder(pixelTBMTrailer[2]);++i;
                        pulseTrain[i]=levelEncoder(pixelTBMTrailer[1]);++i;
                        pulseTrain[i]=levelEncoder(pixelTBMTrailer[0]);++i;
		}
	}
	fin.close();
	return pulseTrain;
}

/*
std::vector<unsigned int> blurPulseTrain(std::vector<unsigned int> inputPulseTrain, float stddev)
{
	vector<unsigned int> outputPulseTrain(inputPulseTrain.size());

	for (int i=0;i<=inputPulseTrain.size();++i)
	{
		outputPulseTrain[i]=inputPulseTrain[i]+int(((rand()/float(RAND_MAX))*2.0-1.0)*stddev); //Change to Gaussian when you feel like it.
	}
	
	return outputPulseTrain;
}

std::vector<float> BoxMuller (float stddev)
{
	float r,phi;
	vector<float> z(2);
	float pi=3.14159;

	r=randf();
	phi=randf();

	z[0]=cos(2*pi*phi)*pow(-log(r*r/stddev),0.5);
	z[1]=sin(2*pi*phi)*pow(-log(r*r/stddev),0.5);

	return z;
}	

*/

#endif




	
	
