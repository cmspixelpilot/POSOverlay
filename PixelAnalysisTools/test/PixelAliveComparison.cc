//////////////////////////////////////////////////////////////////////////////////////////////
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
#include <dirent.h>

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

string MDLorPNLName(string rocNameSearch);

int main(int argc, char **argv)
{

  //----------------------------------------------------------------------
  //Getting main directory environment variable and building path to home 
  //----------------------------------------------------------------------
  if( getenv("BUILD_HOME") == 0 )
  {
    cout  << "BUILD_HOME environment variable undefined...You need to set this variable!" << endl;
    exit(0);
  }//if Build Home

  //checks correct number of command line inputs
  if(argc < 2 || argc > 3)
  {
    cout << "Command should look like: ./script.sh  Run1 Run2" << endl;
    exit(0);
  }//if argc < 2 || argc > 3

  int runNumber1 = atoi(argv[1]), runNumber2 = atoi(argv[2]);
  if (runNumber1 > runNumber2)
  {
    cout << "For purposes to see if a ROC improved or deteriorated, please put the older run number first (the one with the smaller number). This is for consistency." << endl;
    exit(0);
  }//fi runNumber1 > runNumber2 

  ////////////////////////////////
  //The input files' declarations
  ////////////////////////////////
  int groupDirectory1 = ((int)runNumber1/1000)*1000,  groupDirectory2 = ((int)runNumber2/1000)*1000;
  stringstream inFile1Dir, inFile2Dir, outFileNameTEMP, BadROCOutputLocationTEMP, BadMODandPNLLocationTEMP, skippedROCNamesTEMP, inFileDetconfig1, inFileDetconfig2, comparisonLimitsTEMP;
  stringstream inFileBegin1, inFileBegin2;

  // Tries to open the base Run directory in /pixel/data0/ and /pixelscratch/pixelscratch/data0/
  inFileBegin1 << "/pixel/data0/" ;
  inFile1Dir << inFileBegin1.str() << "Run_" << groupDirectory1;
  inFileBegin2 << "/pixel/data0/" ;
  inFile2Dir << inFileBegin2.str() << "Run_" << groupDirectory2;
  DIR *dirTest1, *dirTest2;
  string strTEST1 = inFile1Dir.str(), strTEST2 = inFile2Dir.str();
  dirTest1 = opendir(strTEST1.c_str() );
  dirTest2 = opendir(strTEST2.c_str() );
  if (dirTest1 == NULL)
  {
    inFileBegin1.str(std::string());
    inFileBegin1 << "/pixelscratch/pixelscratch/data0/";
    inFile1Dir.str(std::string());
    inFile1Dir << inFileBegin1.str() << "Run_" << groupDirectory1;
  }//if dirTest == NULL
  if (dirTest2 == NULL)
  { 
    inFileBegin2.str(std::string());
    inFileBegin2 << "/pixelscratch/pixelscratch/data0/";
    inFile2Dir.str(std::string());
    inFile2Dir << inFileBegin2.str() << "Run_" << groupDirectory2;
  }//if dirTest == NULL
  

  inFile1Dir <<  "/Run_" << argv[1] << "/" << "PixelAlive_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31-32-33-34-35-36-37-38-39_Run_" << argv[1] << ".root";
  inFile2Dir <<  "/Run_" << argv[2] << "/" << "PixelAlive_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31-32-33-34-35-36-37-38-39_Run_" << argv[2] << ".root";
  string inTotalName1 = inFile1Dir.str();
  string inTotalName2 = inFile2Dir.str();
  TFile* f1 = new TFile(inTotalName1.c_str() );
  TFile* f2 = new TFile(inTotalName2.c_str() );

  // Looks for the file with all FED's, just FPix, just BPix, or individual. NOTE: doesn't include all of the root files if there are multiple. Only the first one it finds
  std::cout << "f1->IsOpen()= " << f1->IsOpen() << "  " << inTotalName1.c_str() << std::endl;
  if (!f1->IsOpen() )
  {
    stringstream inFile1DirTEST1;
    inFile1DirTEST1 << inFileBegin1.str() << "/Run_" << groupDirectory1 <<  "/Run_" << argv[1] << "/" 
     	       << "PixelAlive_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_" << argv[1] << ".root";
    string inTotalName1TEST1 = inFile1DirTEST1.str();
    f1 = new TFile(inTotalName1TEST1.c_str() );
    std::cout << "\tf1->IsOpen()= " << f1->IsOpen() << "  " << inTotalName1TEST1.c_str() << std::endl;
    if (!f1->IsOpen() )
    {
      stringstream inFile1DirTEST2;
      inFile1DirTEST2 << inFileBegin1.str() << "/Run_" << groupDirectory1 <<  "/Run_" << argv[1] << "/" << "PixelAlive_Fed_32-33-34-35-36-37-38-39_Run_" << argv[1] << ".root";
      string inTotalName1TEST2 = inFile1DirTEST2.str();
      f1 = new TFile(inTotalName1TEST2.c_str() );
      std::cout << "\tf1->IsOpen()= " << f1->IsOpen() << "  " << inTotalName1TEST2.c_str() << std::endl;
      int indivFED = 1;
      while (!f1->IsOpen() && indivFED != 40)
      {
        stringstream inFile1DirTEST3;
	inFile1DirTEST3 << inFileBegin1.str() << "/Run_" << groupDirectory1 <<  "/Run_" << argv[1] << "/" << "PixelAlive_Fed_" << indivFED << "_Run_" << argv[1] << ".root";	
        string inTotalName1TEST3 = inFile1DirTEST3.str();
        f1 = new TFile(inTotalName1TEST3.c_str() );
        std::cout << "\tf1->IsOpen()= " << f1->IsOpen() << "  " << inTotalName1TEST3.c_str() << std::endl;
        indivFED++;
      }//while 
      if (!f1->IsOpen() )
      {
  	std::cout << "The first run number you gave doesn't include either all FEDs, just BPix, just FPix, or just 1 FED. Don't have this included yet." << std::endl;
	exit(0);
      }// if
    }//if 
  }//if

  std::cout << "f2->IsOpen()= " << f2->IsOpen() << "  " << inTotalName2.c_str() << std::endl;
  if (!f2->IsOpen() )
  {
    stringstream inFile2DirTEST1;
    inFile2DirTEST1 << inFileBegin2.str() << "/Run_" << groupDirectory2 <<  "/Run_" << argv[2] << "/"
               << "PixelAlive_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_" << argv[2] << ".root";
    string inTotalName2TEST1 = inFile2DirTEST1.str();
    f2 = new TFile(inTotalName2TEST1.c_str() );
    std::cout << "\tf2->IsOpen()= " << f2->IsOpen() << "  " << inTotalName2TEST1.c_str() << std::endl;
    if (!f2->IsOpen() )
    { 
      stringstream inFile2DirTEST2;
      inFile2DirTEST2 << inFileBegin2.str() << "/Run_" << groupDirectory2 <<  "/Run_" << argv[2] << "/" << "PixelAlive_Fed_32-33-34-35-36-37-38-39_Run_" << argv[2] << ".root";
      string inTotalName2TEST2 = inFile2DirTEST2.str();
      f2 = new TFile(inTotalName2TEST2.c_str() );
      std::cout << "\tf2->IsOpen()= " << f2->IsOpen() << "  " << inTotalName2TEST2.c_str() << std::endl;
      int indivFED = 1;
      while (!f2->IsOpen() && indivFED != 40)
      { 
        stringstream inFile2DirTEST3;
        inFile2DirTEST3 << inFileBegin2.str() << "/Run_" << groupDirectory2 <<  "/Run_" << argv[2] << "/" << "PixelAlive_Fed_" << indivFED << "_Run_" << argv[2] << ".root";
        string inTotalName2TEST3 = inFile2DirTEST3.str();
        f2 = new TFile(inTotalName2TEST3.c_str() );
        std::cout << "\tf2->IsOpen()= " << f2->IsOpen() << "  " << inTotalName2TEST3.c_str() << std::endl;
        indivFED++;
      }//while 
      if (!f2->IsOpen() )
      { 
        std::cout << "The second run number you gave doesn't include either all FEDs, just BPix, just FPix, or just 1 FED. Don't have this included yet." << std::endl;
        exit(0);
      }// if
    }//if 
  }//if

  //Opens the detectconfig.dat files to check if the ROC is a noAnalogSignal
  inFileDetconfig1 << inFileBegin1.str() << "Run_" << groupDirectory1 <<  "/Run_" << argv[1] << "/" << "detectconfig.dat";
  inFileDetconfig2 << inFileBegin2.str() << "Run_" << groupDirectory2 <<  "/Run_" << argv[2] << "/" << "detectconfig.dat";
  string inTotalNameDetConf1 = inFileDetconfig1.str();
  string inTotalNameDetConf2 = inFileDetconfig1.str();

  comparisonLimitsTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if (comparisonLimitsTEMP.str()[outFileNameTEMP.str().length()-1] != '/')
    comparisonLimitsTEMP << "/";
  comparisonLimitsTEMP << "CalibrationComparisons/ComparisonLimits.out";
  string comparisonLimits = comparisonLimitsTEMP.str();

  ifstream detConf1, detConf2, compLimits;
  detConf1.open(inTotalNameDetConf1.c_str() );
  detConf2.open(inTotalNameDetConf2.c_str() );
  compLimits.open(comparisonLimits.c_str() );

  if (!detConf1.is_open() )
  {
    cout << "\nThe detectconfig.dat could not be opened for Run_" << argv[1] << "." << endl;
    exit(0);
  }//if detConf1

  if (!detConf2.is_open() )
  {
    cout << "\nThe detectconfig.dat could not be opened for Run_" << argv[2] << "." << endl;
    exit(0);
  }//if detConf2 

  if (!compLimits.is_open() )
  {
    cout << "\nThe ComparisonLimits.out could not be opened. Make sure the file exists in your PixelRuns/Runs/CalibrationComparisons/ directory." << endl;
    cout << "If the file doesn't exist. Then look in the /nfshome0/ktos/TriDAS/pixel/PixelRun/Runs/CalibrationComparisons/ for a copy of it." << std::endl;
    exit(0);
  }//if compLimits.is_open

  //////////////////////////////////////////////////////
  // Getting limits from the ComparisonLimits.out file
  //////////////////////////////////////////////////////
  size_t posChangeROCLimit, posBadROCLimit;
  float deadPixelPercLimit, deltaDeadPixelPercLimit;
  string line, dump;
  while (getline(compLimits,line) )
  {
    posChangeROCLimit = line.find("DeltaDeadPixelPercentage="); 
    posBadROCLimit = line.find("DeadPixelPercentage=");
    if (posChangeROCLimit != string::npos) // string::npos is returned if string is not found
    {
      stringstream stringChangeROCLimit(line);
      stringChangeROCLimit >> dump >> deltaDeadPixelPercLimit;
    }//if posChangeROCLimit
    if (posBadROCLimit != string::npos) // string::npos is returned if string is not found
    {
      stringstream stringBadROCLimit(line);
      stringBadROCLimit >> dump >> deadPixelPercLimit;
    }//if posBadROCLimit
  }//while  
  
  std::cout << "Limits: deadPixelPercLimit= " << deadPixelPercLimit << "  deltaDeadPixelPercLimit= " << deltaDeadPixelPercLimit << std::endl;
  /////////////////////////////////////
  // output files declarations
  ////////////////////////////////////
  outFileNameTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(outFileNameTEMP.str()[outFileNameTEMP.str().length()-1] != '/')
    outFileNameTEMP << "/";
  outFileNameTEMP << "CalibrationComparisons/PiAliveComparison_Run_" <<  argv[1] << "_Run_" << argv[2] << ".root";
  string outFileName = outFileNameTEMP.str();

  //output .out file with Bad Roc lists
  BadROCOutputLocationTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(BadROCOutputLocationTEMP.str()[BadROCOutputLocationTEMP.str().length()-1] != '/')
    BadROCOutputLocationTEMP << "/";
  BadROCOutputLocationTEMP << "CalibrationComparisons/PiAlive_BadROCNames_Run_" << argv[1] << "_Run_" << argv[2] << ".out";
  string BadROCOutputLocation = BadROCOutputLocationTEMP.str();

  //output.out file with Bad PNL or MOD's with > 1 bad ROC
  BadMODandPNLLocationTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(BadMODandPNLLocationTEMP.str()[BadMODandPNLLocationTEMP.str().length()-1] != '/')
    BadMODandPNLLocationTEMP << "/";
  BadMODandPNLLocationTEMP << "CalibrationComparisons/PiAlive_BadMODandPNLNames_Run_" << argv[1] << "_Run_" << argv[2] << ".out";
  string BadMODandPNLLocation = BadMODandPNLLocationTEMP.str();
 

  ofstream BadROCFile_;
  BadROCFile_.open(BadROCOutputLocation.c_str() );

  ofstream BadMODFile_;
  BadMODFile_.open(BadMODandPNLLocation.c_str() );


  ////////////////////////////////////////////
  //Defining Trees, Histograms, and Canvases
  ////////////////////////////////////////////
  TFile *out_ = new TFile(outFileName.c_str(), "RECREATE");
  TDirectory *topDir = gDirectory;
  topDir->cd();
  TDirectory *ChangingROC_Improved = out_->mkdir("ChangingROC_Improved");
  TDirectory *ChangingROC_Degraded = out_->mkdir("ChangingROC_Degraded");
  TDirectory *BadInBothRuns_Histo2D = out_->mkdir("BadInBothRuns_Histo2D");

  TCanvas DeltaBadPixNumbCanvas_("DeltaBadPixNumbCanvas", "", 600, 600);
  TCanvas DeltaBadPixPercCanvas_("DeltaBadPixPercCanvas", "", 600, 600);
  TCanvas DeltaDeadPixNumbCanvas_("DeltaDeadPixNumbCanvas", "", 600, 600);
  TCanvas DeltaDeadPixPercCanvas_("DeltaDeadPixPercCanvas", "", 600, 600);

  TH1F *DeltaBadPixNumb_ = new TH1F("DeltaBadPixNumb", ";#DeltaBad Pixel Number", 5000, 0, 5000);
  TH1F *DeltaBadPixPerc_ = new TH1F("DeltaBadPixPerc", ";#DeltaBad Pixel Perctage", 500, 0, 100);
  TH1F *DeltaDeadPixNumb_ = new TH1F("DeltaDeadPixNumb", ";#DeltaDead Pixel Number", 5000, 0, 5000);
  TH1F *DeltaDeadPixPerc_ = new TH1F("DeltaDeadPixPerc", ";#DeltaDead Pixel Perctage", 500, 0, 100);

  TCanvas BadPixNumbCanvas_("BadPixNumbCanvas", "", 600, 600);
  TCanvas BadPixPercCanvas_("BadPixPercCanvas", "", 600, 600);
  TCanvas DeadPixNumbCanvas_("DeadPixNumbCanvas", "", 600, 600);
  TCanvas DeadPixPercCanvas_("DeadPixPercCanvas", "", 600, 600);

  TH1F *BadPixNumb_ = new TH1F("BadPixNumb", ";Number of Bad Pixels", 5000, 0, 5000);
  TH1F *BadPixPerc_ = new TH1F("BadPixPerc", ";Percentage of Bad Pixels", 500, 0, 100);
  TH1F *DeadPixNumb_ = new TH1F("DeadPixNumb", ";Number of Dead Pixels", 5000, 0, 5000);
  TH1F *DeadPixPerc_ = new TH1F("DeadPixPerc", ";Percentage of Dead Pixels", 500, 0, 100);

  //Declaring Trees, the Struct to read from the trees, and the address to read read from the trees. W = WronglyDecoded branch, P = Pixels branch
  TTree *tree1W = (TTree*)f1->Get("SummaryTrees/SummaryTree");
  TTree *tree2W = (TTree*)f2->Get("SummaryTrees/SummaryTree");
  TTree *tree1P = (TTree*)f1->Get("SummaryTrees/SummaryTree");
  TTree *tree2P = (TTree*)f2->Get("SummaryTrees/SummaryTree");

  struct BadDecodingBranch{      
        int numberOfBadPixelsGT0;
        float percentOfBadPixels;
        int numberOfBadPixels; 
        char rocName[40];        
  };

  struct PixelAliveBadPixelsBranch{
        int rocsWithNumberOfDeadPixelsGTN;
        float percentageOfDeadPixels;
        int numberOfDeadPixels;
        char rocName[40];
  };


  BadDecodingBranch branch1W, branch2W;
  PixelAliveBadPixelsBranch branch1P, branch2P;

  tree1W->SetBranchAddress("WronglyDecoded", &branch1W);
  tree1P->SetBranchAddress("Pixels", &branch1P);
  tree2W->SetBranchAddress("WronglyDecoded", &branch2W);
  tree2P->SetBranchAddress("Pixels", &branch2P);

std::cout << "<-------------Done Initializing--------------> " << std::endl;

  /////////////////////////////////////
  //  Looping over trees 
  /////////////////////////////////////
  Int_t nEntries1 = tree1W->GetEntries();
  Int_t entry1W = -10, entry2W = -10,  entry1P = -10, entry2P = -10;

  typedef std::map<string, unsigned int> NameMap;
  NameMap MapImpROCDeadPixPerc1Name, MapDegROCDeadPixPerc1Name, MapConstROCDeadPixPerc1Name;
  std::vector<double> ImpROCDeadPixPerc1_PercOld, ImpROCDeadPixPerc1_PercNew, DegROCDeadPixPerc1_PercOld, DegROCDeadPixPerc1_PercNew, ImpROCDeadPixPerc1_DeltaPerc, DegROCDeadPixPerc1_DeltaPerc;
  std::vector<double> ConstROCDeadPixPerc1_PercOld,  ConstROCDeadPixPerc1_PercNew, ConstROCDeadPixPerc1_DeltaPerc;
  std::vector<string> ImpROCDeadPixPerc1_Name, DegROCDeadPixPerc1_Name, ConstROCDeadPixPerc1_Name ;
  string MOD_PNL_Name;
  int entriesSkipped1 = 0, entriesSkipped2 = 0; //skipped are for if the noAnalogSignals appears in 1 detectconfig file and not the other. This is needed to sync the TTree entry to the same ROC

  for (Int_t i=0; i < nEntries1; i++)
  {
    topDir->cd();
    entry1W = tree1W->GetEntry(i + entriesSkipped1);
    entry2W = tree2W->GetEntry(i + entriesSkipped2);
    entry1P = tree1P->GetEntry(i + entriesSkipped1); 
    entry2P = tree2P->GetEntry(i + entriesSkipped2);
    if (entry1W > 0 && entry2W > 0 && entry1P > 0 && entry2P > 0)
    {
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Sometimes ROC isn't included in PA scan. These have the string "noAnalogSignal" next to the rocName in detConf.
      // The if statements below re sync the 2 trees by finding which scan skipped a ROC and how many more it skipped.
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      string name1 = branch1P.rocName, name2 = branch2P.rocName;
      if (name1 != name2)
      {
	string input;
	bool checkFile1MissingROC = false, checkFoundRoc1 = false, checkFile2MissingROC = false, checkFoundRoc2 = false;
	int nLineNoAnalogSignal1 = 0, nLineNoAnalogSignal2 = 0;

	//checking detconfig for run 1 for missing "noAnalygSignal" next to the missing ROC
	while (detConf1 && !checkFoundRoc1)
  	{
	  getline(detConf1, input);
	  string ROCName;
	  unsigned int found = 0;
	  found = input.find_first_of(" \t\n",0);
	  ROCName = input.substr(0, found);
	  if (ROCName == name2)
	  {
	    checkFoundRoc1 = true;
	    size_t check = input.find("noAnalogSignal");
	    if (check != std::string::npos)
	      checkFile1MissingROC = true;
	  }//if ROCName == branch1P.rocName
	}//while 
	if (checkFile1MissingROC)
	{
	  nLineNoAnalogSignal1++;
	  while(detConf1)
	  {
            getline(detConf1, input);
 	    size_t check = input.find("noAnalogSignal");
  	    if (check != std::string::npos)
  	      nLineNoAnalogSignal1++;
	    else
	      break;
	  }//while
  	}//if checkFile1MissingROC

        //checking detconfig for run 2 for missing "noAnalygSignal" next to the missing ROC
        while (detConf2 && !checkFoundRoc2 && !checkFile1MissingROC)
        { 
          getline(detConf2, input);
          string ROCName;
          unsigned int found = 0;
          found = input.find_first_of(" \t\n",0);
          ROCName = input.substr(0, found); 
          if (ROCName == name1)
          { 
            checkFoundRoc2 = true;
            size_t check = input.find("noAnalogSignal");
            if (check != std::string::npos)
              checkFile2MissingROC = true;
          }//if ROCName == branch1P.rocName
        }//while 
        if (checkFile2MissingROC)
        { 
	  nLineNoAnalogSignal2++;
          while(detConf2)
          { 
            getline(detConf2, input);
            size_t check = input.find("noAnalogSignal");
            if (check != std::string::npos)
              nLineNoAnalogSignal2++;
            else
              break;
          }//while
        }//if checkFile1MissingROC

	//check that only 1 file skipped the ROC. If not then something is terribly, terribly wrong
        if (!checkFile2MissingROC && !checkFile1MissingROC)
	  std::cout << "<-----WELL, You done messed up kid with the ROC names. Can't find same ROC in both TTrees. Probably with the searching of the files.----------------------------->" << std::endl;
	if (checkFile2MissingROC && checkFile1MissingROC)
	  std::cout << "<-----WELL, That's weird. Two skipped ROC's. Shouldn't affect the trees' sync. Skipped Lines: 1= " << nLineNoAnalogSignal1 << " 2= " << nLineNoAnalogSignal2 << std::endl;
	//Now reset the file to be looking at the first line again
	detConf1.clear();
	detConf2.clear();
	detConf1.seekg(0, detConf1.beg);
	detConf2.seekg(0, detConf2.beg);
	if (checkFile2MissingROC && !checkFile1MissingROC)
	{
	  entriesSkipped1 += nLineNoAnalogSignal2;
          entry1W = tree1W->GetEntry(i + entriesSkipped1);
          entry1P = tree1P->GetEntry(i + entriesSkipped2);
	}//File 2 has skipped ROCs so the current entry of file 1 needs to be modified
        if (!checkFile2MissingROC && checkFile1MissingROC)
	{
	  entriesSkipped2 += nLineNoAnalogSignal1;
          entry2W = tree2W->GetEntry(i + entriesSkipped1);
          entry2P = tree2P->GetEntry(i + entriesSkipped2);
	}//File1 has skipped ROCs so the current entry of file 2 needs to be modified
      }//if branch1P.rocName != branch2P.rocName

      double deltaBadPixNumb = branch1W.numberOfBadPixels - branch2W.numberOfBadPixels;
      double deltaBadPixPerc = branch1W.percentOfBadPixels - branch2W.percentOfBadPixels;
      double deltaDeadPixNumb = branch1P.numberOfDeadPixels - branch2P.numberOfDeadPixels;
      double deltaDeadPixPerc = branch1P.percentageOfDeadPixels - branch2P.percentageOfDeadPixels;
      BadPixNumb_->Fill(branch1W.numberOfBadPixels );
      BadPixNumb_->Fill(branch2W.numberOfBadPixels );
      BadPixPerc_->Fill(branch1W.percentOfBadPixels );
      BadPixPerc_->Fill(branch2W.percentOfBadPixels );
      DeltaBadPixNumb_->Fill(deltaBadPixNumb );
      DeltaBadPixPerc_->Fill(deltaBadPixPerc );
      DeadPixNumb_->Fill(branch1P.numberOfDeadPixels );
      DeadPixNumb_->Fill(branch2P.numberOfDeadPixels );
      DeadPixPerc_->Fill(branch1P.percentageOfDeadPixels );
      DeadPixPerc_->Fill(branch2P.percentageOfDeadPixels );
      DeltaDeadPixNumb_->Fill(deltaDeadPixNumb );
      DeltaDeadPixPerc_->Fill(deltaDeadPixPerc );

      ///////////////////////////////////////////////////////////////////////////
      // Finding Improving ROCs based upon DeltaDeadPixelPercentage 
      ///////////////////////////////////////////////////////////////////////////
      bool checkDeadPixPerc = false, checkImproveDeadPixPerc = false, checkConstBad = false, checkDegradeDeadPixPerc = false;
      MOD_PNL_Name = MDLorPNLName(branch2P.rocName);
      if ( deltaDeadPixPerc > deltaDeadPixelPercLimit)
      {
 	checkDeadPixPerc = true;
	checkImproveDeadPixPerc = true;
   	if ( MapImpROCDeadPixPerc1Name.find(MOD_PNL_Name) == MapImpROCDeadPixPerc1Name.end() )
	  MapImpROCDeadPixPerc1Name[MOD_PNL_Name ] = 1;
	else
	  MapImpROCDeadPixPerc1Name[MOD_PNL_Name ] = MapImpROCDeadPixPerc1Name[MOD_PNL_Name ] + 1;
	ImpROCDeadPixPerc1_Name.push_back(branch2P.rocName );
	ImpROCDeadPixPerc1_PercOld.push_back(branch1P.percentageOfDeadPixels );
	ImpROCDeadPixPerc1_PercNew.push_back(branch2P.percentageOfDeadPixels );
	ImpROCDeadPixPerc1_DeltaPerc.push_back(deltaDeadPixPerc );
      }//if delta > 2

      ///////////////////////////////////////////////////////////////////////////
      // Finding Degrading ROCs based upon DeltaDeadPixelPercentage 
      ///////////////////////////////////////////////////////////////////////////
      if ( deltaDeadPixPerc < deltaDeadPixelPercLimit * -1)
      {
	checkDegradeDeadPixPerc = true;
        checkDeadPixPerc = true;
        if ( MapDegROCDeadPixPerc1Name.find(MOD_PNL_Name) == MapDegROCDeadPixPerc1Name.end() )
          MapDegROCDeadPixPerc1Name[MOD_PNL_Name ] = 1;
        else
          MapDegROCDeadPixPerc1Name[MOD_PNL_Name ] = MapDegROCDeadPixPerc1Name[MOD_PNL_Name ] + 1;
        DegROCDeadPixPerc1_Name.push_back(branch2P.rocName );
        DegROCDeadPixPerc1_PercOld.push_back(branch1P.percentageOfDeadPixels );
        DegROCDeadPixPerc1_PercNew.push_back(branch2P.percentageOfDeadPixels );
        DegROCDeadPixPerc1_DeltaPerc.push_back(deltaDeadPixPerc );
      }//if Delta > 2
      
      //////////////////////////////////////////////////////////////////////////////
      // Finding Consistently bad ROC's dependingon individual DeadPixelPercentage 
      //////////////////////////////////////////////////////////////////////////////
      if ( branch2P.percentageOfDeadPixels > deadPixelPercLimit && branch1P.percentageOfDeadPixels > deadPixelPercLimit)
      {
	checkDeadPixPerc = true;
	checkConstBad = true;
        if ( MapConstROCDeadPixPerc1Name.find(MOD_PNL_Name) == MapConstROCDeadPixPerc1Name.end() )
          MapConstROCDeadPixPerc1Name[MOD_PNL_Name ] = 1;
        else
          MapConstROCDeadPixPerc1Name[MOD_PNL_Name ] = MapConstROCDeadPixPerc1Name[MOD_PNL_Name ] + 1;
        ConstROCDeadPixPerc1_Name.push_back(branch2P.rocName );
        ConstROCDeadPixPerc1_PercOld.push_back(branch1P.percentageOfDeadPixels );
        ConstROCDeadPixPerc1_PercNew.push_back(branch2P.percentageOfDeadPixels );
        ConstROCDeadPixPerc1_DeltaPerc.push_back(deltaDeadPixPerc );
      }//if Delta > 2


      //////////////////////////////////////////////////////////////////////////////////////////////////////
      // Now getting 2D plot of the changing ROC 
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      if (checkDeadPixPerc )
      {
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Getting the Path+Name of the histogram of the ROC and plotting the f12 and f1 together
	//////////////////////////////////////////////////////////////////////////////////////////////////////////// 
        stringstream Name_Path_TEMP1, Name_Path_TEMP2, CanvasNameTemp, NewTitleNameTemp, OldTitleNameTemp;
 	string ROCName = branch2P.rocName, OldTitleName, NewTitleName;
	unsigned int found = 0;
	Name_Path_TEMP1 << "/";
	Name_Path_TEMP2 << "/";
	for (int i = 0; i < 6; i++)
	{
	  found = ROCName.find('_',found+1);
 	  Name_Path_TEMP1 << ROCName.substr(0,found) << "/";
          Name_Path_TEMP2 << ROCName.substr(0,found) << "/";
	}//for i	  
        CanvasNameTemp << "Delta Dead Pixel Percentage > " << deltaDeadPixelPercLimit << ": " << ROCName;
        string CanvasName = CanvasNameTemp.str();
        TCanvas *ROCCanvas_ = new TCanvas(CanvasName.c_str(), "", 600, 600);
        ROCCanvas_->Divide(2,1);
	Name_Path_TEMP1 << ROCName;
	Name_Path_TEMP2 << ROCName;
	string Name_Path1 = Name_Path_TEMP1.str();
	string Name_Path2 = Name_Path_TEMP2.str();
        TH2F *hROCOld_ = (TH2F*)f1->Get(Name_Path1.c_str() );
        TH2F *hROCNew_ = (TH2F*)f2->Get(Name_Path2.c_str() );

        if (checkImproveDeadPixPerc)
        {
          OldTitleNameTemp << "Run_" << argv[1] << " Improving Dead Pixel Percentage";
          NewTitleNameTemp << "Run_" << argv[2] << " Improving Dead Pixel Percentage";
        }//if  checkImproveDeadPixPerc
        else if (checkDegradeDeadPixPerc)
        {
          OldTitleNameTemp << "Run_" << argv[1] << " Degrading Dead Pixel Percentage"; 
          NewTitleNameTemp << "Run_" << argv[2] << " Degrading Dead Pixel Percentage";
        }//else

	if (hROCOld_ == NULL)
	{
	  Name_Path_TEMP1 << " (inv)";
	  Name_Path1 = Name_Path_TEMP1.str();
	  hROCOld_ = (TH2F*)f1->Get(Name_Path1.c_str() );
	  OldTitleNameTemp << " (inv)";
        }//if hROCNew_ == NULL
        if (hROCNew_ == NULL)
        { 
          Name_Path_TEMP2 << " (inv)";
          Name_Path2 = Name_Path_TEMP2.str();
          hROCNew_ = (TH2F*)f2->Get(Name_Path2.c_str() );
	  NewTitleNameTemp << " (inv)"; 
        }//if hROCNew_ == NULL

	OldTitleName = OldTitleNameTemp.str();
 	NewTitleName = NewTitleNameTemp.str();
 
	hROCOld_->SetTitle(OldTitleName.c_str() );
	hROCNew_->SetTitle(NewTitleName.c_str() );
        ROCCanvas_->cd(1);
        hROCOld_->Draw();
	ROCCanvas_->cd(2);
        hROCNew_->Draw();
        if (checkImproveDeadPixPerc)
        {
          ChangingROC_Improved->cd();
          ROCCanvas_->Write();
        }//if checkImproveThreshold && checkThreshold
        else if (checkDegradeDeadPixPerc)
        {
	  ChangingROC_Degraded->cd();
          ROCCanvas_->Write();
        }//if checkImproveThreshold && checkThreshold
	if (checkConstBad)
	{
	  BadInBothRuns_Histo2D->cd();
	  stringstream ConstOldNameTemp, ConstNewNameTemp, CanvasConstBadTemp;
          CanvasConstBadTemp << "Dead Pixel Percentage > " << deadPixelPercLimit << ": " << ROCName;
          string CanvasConstBad = CanvasConstBadTemp.str();
          TCanvas *ROCCanvasConstBad_ = new TCanvas(CanvasConstBad.c_str(), "", 600, 600);
          ROCCanvasConstBad_->Divide(2,1);
          ConstOldNameTemp << "Run_" << argv[1] << " Constantly Bad Dead Pixel Percentage";
          ConstNewNameTemp << "Run_" << argv[2] << " Constantly Bad Dead Pixel Percentage";
	  string ConstOldName = ConstOldNameTemp.str(), ConstNewName = ConstNewNameTemp.str();
	  hROCOld_->SetTitle(ConstOldName.c_str() );
          hROCNew_->SetTitle(ConstNewName.c_str() );
          ROCCanvasConstBad_->cd(1);
          hROCOld_->Draw();
          ROCCanvasConstBad_->cd(2);
          hROCNew_->Draw();
	  ROCCanvasConstBad_->Write(); 
	  ROCCanvasConstBad_->Clear();
	}//if checkConstBad
        ROCCanvas_->Clear();
      }//if checkDeadPixPerc 
    }//if entry1 > 0 && entry2 > 0
  }//for nEntries1
std::cout << "<-------------Done Looping over TTrees--------------> " << std::endl;

//------------------------------------------------------------------------------------------
//Writing Histograms and Canvases
//------------------------------------------------------------------------------------------
  out_->cd();
  topDir->cd();
  DeltaBadPixNumbCanvas_.cd();
  DeltaBadPixNumb_->Draw();
  DeltaBadPixNumbCanvas_.Write();

  DeltaBadPixPercCanvas_.cd();
  DeltaBadPixPerc_->Draw();
  DeltaBadPixPercCanvas_.Write();

  BadPixNumbCanvas_.cd();
  BadPixNumb_->Draw();
  BadPixNumbCanvas_.Write();

  BadPixPercCanvas_.cd();
  BadPixPerc_->Draw();
  BadPixPercCanvas_.Write();

  DeltaDeadPixNumbCanvas_.cd();
  DeltaDeadPixNumb_->Draw();
  DeltaDeadPixNumbCanvas_.Write();

  DeltaDeadPixPercCanvas_.cd();
  DeltaDeadPixPerc_->Draw();
  DeltaDeadPixPercCanvas_.Write();

  DeadPixNumbCanvas_.cd();
  DeadPixNumb_->Draw();
  DeadPixNumbCanvas_.Write();

  DeadPixPercCanvas_.cd();
  DeadPixPerc_->Draw();
  DeadPixPercCanvas_.Write();

  out_->Write();

  gDirectory->Delete("DeadPixPerc;1");
  gDirectory->Delete("DeadPixNumb;1");
  gDirectory->Delete("DeltaDeadPixPerc;1");
  gDirectory->Delete("DeltaDeadPixNumb;1");
  gDirectory->Delete("BadPixPerc;1");
  gDirectory->Delete("BadPixNumb;1");
  gDirectory->Delete("DeltaBadPixPerc;1");
  gDirectory->Delete("DeltaBadPixNumb:1");

  out_->Close();

std::cout << "<-------------Done Writing Histograms--------------> " << std::endl;
////////////////////////////////////
// Writing out the Changing Modules
////////////////////////////////////
  BadMODFile_ << "##############################################################\n## Improving MODs or PNLs Failing |Delta DeadPixel Percentage| > "
	      << deltaDeadPixelPercLimit << "\n##############################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapImpROCDeadPixPerc1Name.begin(); iter != MapImpROCDeadPixPerc1Name.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << " has " << iter->second << " bad ROC's.\n";  
  }

  BadMODFile_ << "##############################################################\n## Degrading MODs or PNLs Failing |Delta DeadPixel Percentage| > "
              << deltaDeadPixelPercLimit << "\n##############################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapDegROCDeadPixPerc1Name.begin(); iter != MapDegROCDeadPixPerc1Name.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << " has " << iter->second << " bad ROC's.\n";
  }   

  BadMODFile_ << "##############################################################\n## Constantly Bad MODs or PNLs Failing DeadPixel Percentage > "
              << deadPixelPercLimit << "\n##############################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapConstROCDeadPixPerc1Name.begin(); iter != MapConstROCDeadPixPerc1Name.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << " has " << iter->second << " bad ROC's.\n";
  }


//////////////////////////////////////////////////////////////////////////////////////////////////////
//Writing out hte bad ROC's in a list with headers for the various cuts
//////////////////////////////////////////////////////////////////////////////////////////////////////
  BadROCFile_ << "##############################################################\n## Improving ROCs Failing |Delta Dead Pixel Percentage| > "
              << deltaDeadPixelPercLimit << "\n##############################################################\n";
  for (std::vector<int>::size_type i = 0; i != ImpROCDeadPixPerc1_PercNew.size(); i++)
  {
    BadROCFile_ << ImpROCDeadPixPerc1_Name[i] << "   \tOld Run Dead Pixel Percentage= " <<  ImpROCDeadPixPerc1_PercOld[i];
    BadROCFile_ << "   \tNew Dead Pixel Percentage= " <<  ImpROCDeadPixPerc1_PercNew[i] << "   \tDeltaDead Pixel Percentage= " <<  ImpROCDeadPixPerc1_DeltaPerc[i] << "\n";
  }//for i

  BadROCFile_ << "##############################################################\n## Degrading ROCs Failing |Delta Dead Pixel Percentage| > "
              <<deltaDeadPixelPercLimit << "\n##############################################################\n";
  for (std::vector<int>::size_type i = 0; i != DegROCDeadPixPerc1_PercNew.size(); i++)
  {
    BadROCFile_ << DegROCDeadPixPerc1_Name[i] << "   \tOld Run Dead Pixel Percentage= " <<  DegROCDeadPixPerc1_PercOld[i];
    BadROCFile_ << "   \tNew Dead Pixel Percentage= " <<  DegROCDeadPixPerc1_PercNew[i] << "   \tDeltaDead Pixel Percentage= " <<  DegROCDeadPixPerc1_DeltaPerc[i] << "\n";
  }//for i

  BadROCFile_ << "##############################################################\n## Constantly ROCs Failing Dead Pixel Percentage > "
              << deadPixelPercLimit << "\n##############################################################\n";
  for (std::vector<int>::size_type i = 0; i != ConstROCDeadPixPerc1_PercNew.size(); i++)
  {
    BadROCFile_ << ConstROCDeadPixPerc1_Name[i] << "   \tOld Run Dead Pixel Percentage= " <<  ConstROCDeadPixPerc1_PercOld[i];
    BadROCFile_ << "   \tNew Dead Pixel Percentage= " <<  ConstROCDeadPixPerc1_PercNew[i] << "   \tDeltaDead Pixel Percentage= " <<  ConstROCDeadPixPerc1_DeltaPerc[i] << "\n";
  }//for i

  BadROCFile_.close();
  BadMODFile_.close();
  return 0;
}//end main



////////////////////////////////////////////////////////////////////////////////
// Gets MOD or PNL name from ROC name
////////////////////////////////////////////////////////////////////////////////
string MDLorPNLName(string rocNameSearch)
{
  unsigned int found = 0;
  string name;
  if (rocNameSearch[0] == 'B')
  {
    found = rocNameSearch.find("MOD");
    if (rocNameSearch[found+2] != '_')
      name = rocNameSearch.substr(0,found+4);
    else 
      name = rocNameSearch.substr(0,found+3);
  }//if BPix
  if (rocNameSearch[0] == 'F')
  {
    found = rocNameSearch.find("PNL");
    if (rocNameSearch[found+2] != '_')
      name = rocNameSearch.substr(0,found+4);
    else 
      name = rocNameSearch.substr(0,found+3);
  }//if FPix

  return name;
}//MDLorPNLName

