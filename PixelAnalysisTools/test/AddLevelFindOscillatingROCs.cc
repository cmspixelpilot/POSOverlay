/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
// author Kyle Tos									    //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
#include "PixelAnalysisTools/include/PixelAnalyzer.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include "PixelAnalysisTools/include/PixelHistoManager.h"
#include "PixelAnalysisTools/include/PixelSCurveHistoManager.h"
#include "PixelAnalysisTools/include/PixelGainHistoManager.h"
#include "PixelAnalysisTools/include/PixelAliveHistoManager.h"
#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
#include "PixelAnalysisTools/include/PixelConfigurationsManager.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoServer.h"

#include <fstream>
#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>

#include <TH2F.h>
#include <TROOT.h>
#include <TDirectory.h>
#include <TTree.h>
#include <TBranch.h>
#include <TFile.h>
#include "TCanvas.h"
#include "TLine.h"
#include "TLegend.h"

using namespace std;
using namespace pos;

int main(int argc, char **argv)
{

  //------------------------------------------------------------------------------------------------------------------------
  //Getting main directory environment variable and building path to home as well as making sure that translation.dat exists
  //------------------------------------------------------------------------------------------------------------------------
  if( getenv("BUILD_HOME") == 0 )
  {
    cout  << "\nBUILD_HOME environment variable undefined...You need to set this variable!" << endl;
    exit(0);
  }//if Build Home

  //checks correct number of command line inputs
  if(argc < 3 || argc > 4)
  {
    cout << "\nCommand should look like: ./script.sh  File1.out File2.out outputName" << endl;
    exit(0);
  }//if argc < 2 || argc > 3

//////////////////////////////////////////
//Getting input and output File Path+Name
//////////////////////////////////////////
  stringstream outFileNameTEMP, BadROCOutputLocationTEMP1, BadROCOutputLocationTEMP2;
  BadROCOutputLocationTEMP1 << string(getenv("POS_OUTPUT_DIRS")) << "/" << argv[1];
  BadROCOutputLocationTEMP2 << string(getenv("POS_OUTPUT_DIRS")) << "/" << argv[2];
  outFileNameTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(outFileNameTEMP.str()[outFileNameTEMP.str().length()-1] != '/')
    outFileNameTEMP << "/";
  outFileNameTEMP << "AddLevelOscillatingROCs_" <<  argv[3] << ".out";
  string outFileName = outFileNameTEMP.str(), inTotalName1 = BadROCOutputLocationTEMP1.str(), inTotalName2 = BadROCOutputLocationTEMP2.str();


  cout << "BadROC1 FileName= " << inTotalName1.c_str() << endl;
  cout << "BadROC2 FileName= " << inTotalName2.c_str() << endl;
  cout << "Output .out file path+name= " << outFileName.c_str() << endl;

  //Input file declarations
  ifstream inFile1, inFile2;
  inFile1.open(inTotalName1.c_str() );
  inFile2.open(inTotalName2.c_str() );

  if (!inFile1.is_open() )
  { 
    cout << "\nFile1 could not be opened. Make sure you have the file extension included in the name (i.e. File1.txt, not File1)." << endl;
    exit(0);
  }//if translation_dat 

  if (!inFile2.is_open() )
  {
    cout << "\nFile2 could not be opened. Make sure you have the file extension included in the name (i.e. File2.txt, not File2)." << endl;
    exit(0);
  }//if translation_dat 
  

  //output .out file declaration
  ofstream OscROCFile_;
  OscROCFile_.open(outFileName.c_str() );

////////////////////////////////////////////////
// Finding common lines 
////////////////////////////////////////////////
  string input;
  int skippedLines = 0;
  std::vector<string> RMSImproveFound1, RMSDeclineFound1, SepImproveFound1, SepDeclineFound1, RMSImproveFound2, RMSDeclineFound2, SepImproveFound2, SepDeclineFound2;
  std::cout << "<-------FILE 1 READIN NAME= " << inTotalName1.c_str() << " ------->" << std::endl;
  while(inFile1)
  {
    getline(inFile1, input);
    string ROCName;
    unsigned int found = 0;
    if (input.find("Peaks") != std::string::npos)
      break;      
    if (input[0] != '#')
    {
      found = input.find_first_of(" \t\n",0);
      ROCName = input.substr(0, found);
      if (skippedLines/3 == 1)
        RMSImproveFound1.push_back(ROCName);
      if (skippedLines/3 == 2)
        RMSDeclineFound1.push_back(ROCName);
      if (skippedLines/3 == 3)
        SepImproveFound1.push_back(ROCName);
      if (skippedLines/3 == 4)
        SepDeclineFound1.push_back(ROCName);
   }//if
    else
      skippedLines++;
  }//while

  skippedLines = 0;
  std::cout << "<-------FILE 2 READIN NAME= " << inTotalName2.c_str() << " ------->" << std::endl;
  while(inFile2)
  { 
    getline(inFile2, input);
    string ROCName;
    unsigned int found = 0;
    if (input.find("Peaks") != std::string::npos)
      break;
    if (input[0] != '#')
    { 
      found = input.find_first_of(" \t\n",0);
      ROCName = input.substr(0, found); 
      if (skippedLines/3 == 1)
        RMSImproveFound2.push_back(ROCName);
      if (skippedLines/3 == 2)
        RMSDeclineFound2.push_back(ROCName);
      if (skippedLines/3 == 3)
        SepImproveFound2.push_back(ROCName);
      if (skippedLines/3 == 4)
        SepDeclineFound2.push_back(ROCName);
   }//if
    else
      skippedLines++;
  }//while

  std::vector<string> RMSOscROCNames, SepOscROCNames;
  
  //if ROC improved and then declined, then it is oscilatory
  for (std::vector<int>::size_type iter = 0; iter != RMSDeclineFound1.size(); iter++)
  {
    if ( std::find(RMSImproveFound2.begin(), RMSImproveFound2.end(), RMSDeclineFound1[iter]) != RMSImproveFound2.end() ) 
      RMSOscROCNames.push_back(RMSDeclineFound1[iter] );
  }//for 

  //if ROC declined and then improved, then it is oscilatory
  for (std::vector<int>::size_type iter = 0; iter != RMSImproveFound1.size(); iter++)
  { 
    if ( std::find(RMSDeclineFound2.begin(), RMSDeclineFound2.end(), RMSImproveFound1[iter]) != RMSDeclineFound2.end() )
      RMSOscROCNames.push_back(RMSImproveFound1[iter] );
  }//for 

   //if ROC improved and then declined, then it is oscilatory
   for (std::vector<int>::size_type iter = 0; iter != SepDeclineFound1.size(); iter++)
   { 
    if ( std::find(SepImproveFound2.begin(), SepImproveFound2.end(), SepDeclineFound1[iter]) != SepImproveFound2.end() )
      SepOscROCNames.push_back(SepDeclineFound1[iter] );
   }//for 
   
   //if ROC declined and then improved, then it is oscilatory
   for (std::vector<int>::size_type iter = 0; iter != SepImproveFound1.size(); iter++)
   { 
    if ( std::find(SepDeclineFound2.begin(), SepDeclineFound2.end(), SepImproveFound1[iter]) != SepDeclineFound2.end() )
      SepOscROCNames.push_back(SepImproveFound1[iter] );
   }//for 

std::cout << "<------------Done Comparing------------->" << std::endl;

  OscROCFile_ << "#############################\n## Oscillating ROC based on RMS\n#############################\n";
  for (std::vector<int>::size_type i = 0; i != RMSOscROCNames.size(); i++)
    OscROCFile_ << RMSOscROCNames[i] << std::endl;

  OscROCFile_ << "#############################\n## Oscillating ROC based on Sep\n#############################\n";
  for (std::vector<int>::size_type i = 0; i != SepOscROCNames.size(); i++)
  {
    if (std::find(RMSOscROCNames.begin(), RMSOscROCNames.end(), SepOscROCNames[i]) == RMSOscROCNames.end() ) //This makes sure the ROC isn't already printed
      OscROCFile_ << SepOscROCNames[i] << std::endl; 
  }//for
  OscROCFile_.close();
  return 0;
}//int main

