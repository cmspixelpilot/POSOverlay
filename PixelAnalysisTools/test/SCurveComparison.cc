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

  ////////////////////////////////////////////
  // Getting input and output File Path+Name
  ////////////////////////////////////////////
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
    
  inFile1Dir << "/Run_" << argv[1] << "/" << "SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31-32-33-34-35-36-37-38-39_Run_" << argv[1] << ".root";
  inFile2Dir << "/Run_" << argv[2] <<"/" << "SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31-32-33-34-35-36-37-38-39_Run_" << argv[2] << ".root";
  string inTotalName1 = inFile1Dir.str();
  string inTotalName2 = inFile2Dir.str();
  TFile* f1 = new TFile(inTotalName1.c_str() );
  TFile* f2 = new TFile(inTotalName2.c_str() );

  inFileDetconfig1 << inFileBegin1.str() << "Run_" << groupDirectory1 <<  "/Run_" << argv[1] << "/" << "detectconfig.dat";
  inFileDetconfig2 << inFileBegin2.str() << "Run_" << groupDirectory2 <<  "/Run_" << argv[2] << "/" << "detectconfig.dat";
  string inTotalNameDetConf1 = inFileDetconfig1.str();
  string inTotalNameDetConf2 = inFileDetconfig2.str();

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
  size_t posDeltaThresholdLimit, posDeltaThresholdRMSLimit, posThresholdMinLimit, posThresholdMaxLimit, posThresholdRMSLimit;
  float deltaThresholdLimit, deltaThresholdRMSLimit, thresholdMinLimit, thresholdMaxLimit, thresholdRMSLimit;
  string line, dump;
  while (getline(compLimits,line) )
  {
    posDeltaThresholdRMSLimit = line.find("DeltaThresholdRMS=");
    posDeltaThresholdLimit = line.find("DeltaThreshold=");
    posThresholdMinLimit = line.find("ThresholdMin");
    posThresholdMaxLimit = line.find("ThresholdMax=");
    posThresholdRMSLimit = line.find("ThresholdRMS=");
    if (posDeltaThresholdRMSLimit != string::npos) // string::npos is returned if string is not found
    {
      stringstream ss(line);
      ss >> dump >> deltaThresholdRMSLimit;
    }//if posChangeROCLimit
    if (posDeltaThresholdLimit != string::npos) 
    {
      stringstream ss(line);
      ss >> dump >> deltaThresholdLimit;
    }//if posBadROCLimit
    if (posThresholdMinLimit != string::npos) 
    { 
      stringstream ss(line);
      ss >> dump >> thresholdMinLimit;
    }//if posChangeROCLimit
    if (posThresholdMaxLimit != string::npos)
    { 
      stringstream ss(line);
      ss >> dump >> thresholdMaxLimit;
    }//if posChangeROCLimit
    if (posThresholdRMSLimit != string::npos) 
    { 
      stringstream ss(line);
      ss >> dump >> thresholdRMSLimit;
    }//if posChangeROCLimit
  }//while  

std::cout << "Limits: deltaThresholdRMSLimit= " << deltaThresholdRMSLimit << "  deltaThresholdLimit= " << deltaThresholdLimit << "  thresholdMinLimit= " << thresholdMinLimit 
          << "  thresholdMaxLimit= " << thresholdMaxLimit << "  thresholdRMSLimit= " << thresholdRMSLimit << std::endl;
  
  ///////////////////////////////////////////
  // output files' declarations
  ///////////////////////////////////////////
  outFileNameTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(outFileNameTEMP.str()[outFileNameTEMP.str().length()-1] != '/')
    outFileNameTEMP << "/";
  outFileNameTEMP << "CalibrationComparisons/SCurveComparison_Run_" <<  argv[1] << "_Run_" << argv[2] << ".root";
  string outFileName = outFileNameTEMP.str();
  
  BadROCOutputLocationTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(BadROCOutputLocationTEMP.str()[BadROCOutputLocationTEMP.str().length()-1] != '/')
    BadROCOutputLocationTEMP << "/";
  BadROCOutputLocationTEMP << "CalibrationComparisons/SCurve_BadROCNames_Run_" << argv[1] << "_Run_" << argv[2] << ".out";
  string BadROCOutputLocation = BadROCOutputLocationTEMP.str();
  
  BadMODandPNLLocationTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(BadMODandPNLLocationTEMP.str()[BadMODandPNLLocationTEMP.str().length()-1] != '/')
    BadMODandPNLLocationTEMP << "/";
  BadMODandPNLLocationTEMP << "CalibrationComparisons/SCurve_BadMODandPNLNames_Run_" << argv[1] << "_Run_" << argv[2] << ".out";
  string BadMODandPNLLocation = BadMODandPNLLocationTEMP.str();
  
  ofstream BadROCFile_, BadMODFile_;
  BadROCFile_.open(BadROCOutputLocation.c_str() );
  BadMODFile_.open(BadMODandPNLLocation.c_str() );

  //////////////////////////////////////////////
  //Defining Trees, Histograms, and Canvases
  //////////////////////////////////////////////
  TFile *out_ = new TFile(outFileName.c_str(), "RECREATE");

  //The TDirectories are made for the TBM histograms of that are within the range stated in the name. Sep = Separation
  TDirectory *topDir = gDirectory;
  topDir->cd();
  TDirectory *ChangingROCs_Improved = out_->mkdir("ChangingROCs_Improved");
  TDirectory *ChangingROCs_Degraded = out_->mkdir("ChangingROCs_Degraded");
  TDirectory *BadROCsInBothRuns = out_->mkdir("BadROCsInBothRuns");

  TCanvas DeltaBadPixNumbCanvas_("DeltaBadPixNumbCanvas", "", 600, 600);
  TCanvas DeltaBadPixPercCanvas_("DeltaBadPixPercCanvas", "", 600, 600);
  TCanvas DeltaThresholdCanvas_("DeltaThresholdCanvas", "", 600, 600);
  TCanvas DeltaThresholdRMSCanvas_("DeltaThresholdRMSCanvas", "", 600, 600);

  TH1F *DeltaBadPixNumb_ = new TH1F("DeltaBadPixNumb", ";#DeltaBad Pixel Number", 5000, 0, 5000);
  TH1F *DeltaBadPixPerc_ = new TH1F("DeltaBadPixPerc", ";#DeltaBad Pixel Perctage", 100, 0, 100);
  TH1F *DeltaThreshold_ = new TH1F("DeltaThreshold", ";#DeltaThreshold", 50, 0, 50);
  TH1F *DeltaThresholdRMS_ = new TH1F("DeltaThresholdRMS", ";#DeltaThresholdRMS", 40, 0, 40);

  TCanvas BadPixNumbCanvas_("BadPixNumbCanvas", "", 600, 600);
  TCanvas BadPixPercCanvas_("BadPixPercCanvas", "", 600, 600);
  TCanvas ThresholdCanvas_("ThresholdCanvas", "", 600, 600);
  TCanvas ThresholdRMSCanvas_("ThresholdRMSCanvas", "", 600, 600);

  TH1F *BadPixNumb_ = new TH1F("BadPixNumb", ";Number of Bad Pixels", 5000, 0, 5000);
  TH1F *BadPixPerc_ = new TH1F("BadPixPerc", ";Percentage of Bad Pixels", 100, 0, 100);
  TH1F *Threshold_ = new TH1F("Threshold", ";Threshold", 100, 0, 100);
  TH1F *ThresholdRMS_ = new TH1F("ThresholdRMS", ";Threshold RMS", 40, 0, 50);

  //Declaring Trees, the Struct to read from the trees, and the address to read read from the trees. W = WronglyDecoded branch, P = Pixels branch    
  TTree *tree1W = (TTree*)f1->Get("SummaryTrees/SummaryTree");
  TTree *tree2W = (TTree*)f2->Get("SummaryTrees/SummaryTree");
  TTree *tree1P = (TTree*)f1->Get("SummaryTrees/SummaryTree");
  TTree *tree2P = (TTree*)f2->Get("SummaryTrees/SummaryTree");

   struct BadDecodingBranch{                    
        int numberOfBadPixelsGT0;              
        float percentOfBadPixels;              
        unsigned int numberOfBadPixels;        
        char rocName[40];                      
  };

  struct PixelSCurveBranch{
        unsigned int rocsWithThresholdGTN;
        unsigned int rocsWithNoiseGTN;
        unsigned int rocsWithChisquareGTN;
        unsigned int rocsWithProbabilityGTN;
        float threshold;
        float noise;
        float chisquare;
        float probability;
        float thresholdRMS;
        float noiseRMS;
        char  rocName[38];
};


  BadDecodingBranch branch1W, branch2W;
  PixelSCurveBranch branch1P, branch2P;

  tree1W->SetBranchAddress("WronglyDecoded", &branch1W);
  tree1P->SetBranchAddress("Pixels", &branch1P);
  tree2W->SetBranchAddress("WronglyDecoded", &branch2W);
  tree2P->SetBranchAddress("Pixels", &branch2P);

std::cout << "<-------------Done Initializing--------------> " << std::endl;
  //------------------------------------------------------
  //  Looping over trees 
  //------------------------------------------------------

  Int_t nEntries1 = tree1W->GetEntries();
  Int_t entry1W = -10, entry2W = -10,  entry1P = -10, entry2P = -10;
  typedef std::map<string, unsigned int> NameMap;
  NameMap MapImpROCThreshold2Name, MapDegROCThreshold2Name, MapConstROCThreshold2Name;
  std::vector<double> ImpROCThreshold2_ThrOld, ImpROCThreshold2_ThrNew, DegROCThreshold2_ThrOld, DegROCThreshold2_ThrNew, ImpROCThreshold2_ThrRMSOld, ImpROCThreshold2_ThrRMSNew;
  std::vector<double> DegROCThreshold2_ThrRMSOld, DegROCThreshold2_ThrRMSNew, ConstROCThreshold2_ThrOld,  ConstROCThreshold2_ThrNew, ConstROCThreshold2_ThrRMSOld, ConstROCThreshold2_ThrRMSNew;
  std::vector<string> ImpROCThreshold2_Name, DegROCThreshold2_Name, SkippedROC_Names, ConstROCThreshold2_Name ;
  string MOD_PNL_Name;
  int entriesSkipped1 = 0, entriesSkipped2 = 0; //skipped are for if the noAnalogSignals appears in 1 detectconfig file and not the other. This is needed to sync the TTree entry to the same ROC

  for (Int_t i=0; i < nEntries1; i++)
  {
    topDir->cd();
    entry1W = tree1W->GetEntry(i);
    entry2W = tree2W->GetEntry(i);
    entry1P = tree1P->GetEntry(i); 
    entry2P = tree2P->GetEntry(i);
    if (entry1W > 0 && entry2W > 0 && entry1P > 0 && entry2P > 0)
    {
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Sometimes ROC isn't included in SC scan. These have the string "noAnalogSignal" next to the rocName in detConf.
      // The if statements below re sync the 2 trees by finding which scan skipped a ROC and how many more it skipped.
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      string name1 = branch1P.rocName, name2 = branch2P.rocName;
      if (name1 != name2)
      {
        std::cout << "\tThe two branch.rocName are not the same! branch1P.rocName= " << branch1P.rocName << "\tbranch2P.rocName= " << branch2P.rocName << std::endl;
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
          std::cout << "<-----WELL, You done messed up kid. Probably with the searching of the files.------------------------------------->" << std::endl;
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


      double DeltaBadPixNumb = branch1W.numberOfBadPixels - branch2W.numberOfBadPixels, DeltaThreshold = branch1P.threshold - branch2P.threshold;
      double DeltaBadPixPerc = branch1W.percentOfBadPixels - branch2W.percentOfBadPixels, DeltaThresholdRMS = branch1P.thresholdRMS - branch2P.thresholdRMS;
      BadPixNumb_->Fill(branch1W.numberOfBadPixels );
      BadPixNumb_->Fill(branch2W.numberOfBadPixels );
      BadPixPerc_->Fill(branch1W.percentOfBadPixels );
      BadPixPerc_->Fill(branch2W.percentOfBadPixels );
      DeltaBadPixNumb_->Fill(DeltaBadPixNumb );
      DeltaBadPixPerc_->Fill(DeltaBadPixPerc );
      Threshold_->Fill(branch1P.threshold );
      Threshold_->Fill(branch2P.threshold );
      ThresholdRMS_->Fill(branch1P.thresholdRMS );
      ThresholdRMS_->Fill(branch2P.thresholdRMS );
      DeltaThreshold_->Fill(DeltaThreshold );
      DeltaThresholdRMS_->Fill(DeltaThresholdRMS );


std::cout << "\nbranch1P: name= " << branch1P.rocName << "  threshold= " << branch1P.threshold << "  thresholdRMS= " << branch1P.thresholdRMS << std::endl;
std::cout << "branch2P: name= " << branch2P.rocName << "  threshold= " << branch2P.threshold << "  thresholdRMS= " << branch2P.thresholdRMS << std::endl;
std::cout << "\tDeltaThresh= " << DeltaThreshold << "\tDeltaThresholdRMS= " << DeltaThresholdRMS << std::endl;
      ///////////////////////////////////////////////////////////////////////////
      // Finding Improving ROCs based upon DeltaDeadPixelPercentage 
      ///////////////////////////////////////////////////////////////////////////
      bool checkThreshold = false, checkImproveThreshold = false, checkConstBad = false, checkDegradeThreshold = false;
      MOD_PNL_Name = MDLorPNLName(branch2P.rocName);
      if ( DeltaThresholdRMS > deltaThresholdRMSLimit || DeltaThreshold > deltaThresholdLimit)
      {
        checkThreshold = true;
        checkImproveThreshold = true;
        if ( MapImpROCThreshold2Name.find(MOD_PNL_Name) == MapImpROCThreshold2Name.end() )
          MapImpROCThreshold2Name[MOD_PNL_Name ] = 1;
        else
          MapImpROCThreshold2Name[MOD_PNL_Name ] = MapImpROCThreshold2Name[MOD_PNL_Name ] + 1;
        ImpROCThreshold2_Name.push_back(branch2P.rocName );
        ImpROCThreshold2_ThrOld.push_back(branch1P.threshold );
        ImpROCThreshold2_ThrNew.push_back(branch2P.threshold );
        ImpROCThreshold2_ThrRMSNew.push_back(branch2P.thresholdRMS );
        ImpROCThreshold2_ThrRMSOld.push_back(branch1P.thresholdRMS );
      }//if delta > 2
      
      ///////////////////////////////////////////////////////////////////////////
      // Finding Degrading ROCs based upon DeltaDeadPixelPercentage 
      ///////////////////////////////////////////////////////////////////////////
      if ( DeltaThresholdRMS < (deltaThresholdRMSLimit * -1) || DeltaThreshold < (deltaThresholdLimit * -1) )
      {
	checkDegradeThreshold = true;
        checkThreshold = true;
        if ( MapDegROCThreshold2Name.find(MOD_PNL_Name) == MapDegROCThreshold2Name.end() )
          MapDegROCThreshold2Name[MOD_PNL_Name ] = 1;
        else
          MapDegROCThreshold2Name[MOD_PNL_Name ] = MapDegROCThreshold2Name[MOD_PNL_Name ] + 1;
        DegROCThreshold2_Name.push_back(branch2P.rocName );
        DegROCThreshold2_ThrOld.push_back(branch1P.threshold );
        DegROCThreshold2_ThrNew.push_back(branch2P.threshold );
        DegROCThreshold2_ThrRMSNew.push_back(branch2P.thresholdRMS );
        DegROCThreshold2_ThrRMSOld.push_back(branch1P.thresholdRMS );
      }//if Delta > 2
      
      //////////////////////////////////////////////////////////////////////////////
      // Finding Consistently bad ROC's dependingon individual DeadPixelPercentage 
      //////////////////////////////////////////////////////////////////////////////
      if ( (branch2P.threshold > thresholdMaxLimit || branch2P.thresholdRMS > thresholdRMSLimit || branch2P.threshold < thresholdRMSLimit) || 
           (branch1P.threshold > thresholdMaxLimit || branch1P.thresholdRMS > thresholdRMSLimit || branch1P.threshold < thresholdMinLimit) )
      {
        checkConstBad = true;
 	checkThreshold = true;
        if ( MapConstROCThreshold2Name.find(MOD_PNL_Name) == MapConstROCThreshold2Name.end() )
          MapConstROCThreshold2Name[MOD_PNL_Name ] = 1;
        else
          MapConstROCThreshold2Name[MOD_PNL_Name ] = MapConstROCThreshold2Name[MOD_PNL_Name ] + 1;
        ConstROCThreshold2_Name.push_back(branch2P.rocName );
        ConstROCThreshold2_ThrOld.push_back(branch1P.threshold );
        ConstROCThreshold2_ThrNew.push_back(branch2P.threshold );
        ConstROCThreshold2_ThrRMSNew.push_back(branch2P.thresholdRMS );
        ConstROCThreshold2_ThrRMSOld.push_back(branch1P.thresholdRMS );
      }//if Delta > 2

      //////////////////////////////////////////////////////////////////////////////////////////////////////
      // Now getting 2D plot of the changing ROC 
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      if (checkThreshold )
      {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f1 together
        //////////////////////////////////////////////////////////////////////////////////////////////////////////// 
        stringstream Name_Path_TEMP11, Name_Path_TEMP21, Name_Path_TEMP12, Name_Path_TEMP22, CanvasNameTemp, NewTitleNameTemp1, OldTitleNameTemp1, NewTitleNameTemp2, OldTitleNameTemp2;
        string ROCName = branch2P.rocName, OldTitleName1, NewTitleName1, OldTitleName2, NewTitleName2;
        unsigned int found = 0;
        Name_Path_TEMP11 << "/";
        Name_Path_TEMP21 << "/";
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Name_Path_TEMP11 << ROCName.substr(0,found) << "/";
          Name_Path_TEMP21 << ROCName.substr(0,found) << "/";
          Name_Path_TEMP12 << ROCName.substr(0,found) << "/";
          Name_Path_TEMP22 << ROCName.substr(0,found) << "/";
        }//for i         
        CanvasNameTemp << "DeltaThresh > " << deltaThresholdLimit << " or DeltaThreshRMS > " << deltaThresholdRMSLimit << ": " << ROCName;
        string CanvasName = CanvasNameTemp.str();
        TCanvas *ROCCanvas_ = new TCanvas(CanvasName.c_str(), "", 600, 600);
        ROCCanvas_->Divide(2,2);
        Name_Path_TEMP11 << ROCName;
        Name_Path_TEMP21 << ROCName;
        Name_Path_TEMP11 << "_Threshold1D";
        Name_Path_TEMP21 << "_Threshold1D";
        Name_Path_TEMP12 << ROCName;
        Name_Path_TEMP22 << ROCName;
        Name_Path_TEMP12 << "_Threshold2D";
        Name_Path_TEMP22 << "_Threshold2D";
        string Name_Path11 = Name_Path_TEMP11.str();
        string Name_Path21 = Name_Path_TEMP21.str();
        string Name_Path12 = Name_Path_TEMP12.str();
        string Name_Path22 = Name_Path_TEMP22.str();
        TH1F *hROCOld1D_ = (TH1F*)f1->Get(Name_Path11.c_str() );
        TH1F *hROCNew1D_ = (TH1F*)f2->Get(Name_Path21.c_str() );
        TH2F *hROCOld2D_ = (TH2F*)f1->Get(Name_Path12.c_str() );
        TH2F *hROCNew2D_ = (TH2F*)f2->Get(Name_Path22.c_str() );
        if (checkImproveThreshold)
        {
          OldTitleNameTemp1 << "Run_" << argv[1] << " Improving Threshold1D";
          NewTitleNameTemp1 << "Run_" << argv[2] << " Improving Threshold1D";
          OldTitleNameTemp2 << "Run_" << argv[1] << " Improving Threshold2D";
          NewTitleNameTemp2 << "Run_" << argv[2] << " Improving Threshold2D";
        }//if  checkImproveThreshold
        else if (checkDegradeThreshold)
        {
          OldTitleNameTemp1 << "Run_" << argv[1] << " Degrading Threshold1D";
          NewTitleNameTemp1 << "Run_" << argv[2] << " Degrading Threshold1D";
          OldTitleNameTemp2 << "Run_" << argv[1] << " Degrading Threshold2D";
          NewTitleNameTemp2 << "Run_" << argv[2] << " Degrading Threshold2D";
        }//else

        if (hROCOld1D_ == NULL)
        {
          Name_Path_TEMP11 << " (inv)";
          Name_Path11 = Name_Path_TEMP11.str();
          hROCOld1D_ = (TH1F*)f1->Get(Name_Path11.c_str() );
          OldTitleNameTemp1 << " (inv)";
	}//if hROCOld1D_ == NULL
	if (hROCOld2D_ == NULL)
	{
          Name_Path_TEMP12 << " (inv)";
          Name_Path12 = Name_Path_TEMP12.str();
          hROCOld2D_ = (TH2F*)f1->Get(Name_Path12.c_str() );
          OldTitleNameTemp2 << " (inv)";
        }//if hROCOld2D_ == NULL
        if (hROCNew1D_ == NULL)
        {
          Name_Path_TEMP21 << " (inv)";
          Name_Path21 = Name_Path_TEMP21.str();
          hROCNew1D_ = (TH1F*)f2->Get(Name_Path21.c_str() );
          NewTitleNameTemp1 << " (inv)";
	}//if hROCNew1D_
	if (hROCNew2D_ == NULL)
	{
          Name_Path_TEMP22 << " (inv)";
          Name_Path22 = Name_Path_TEMP22.str();
          hROCNew2D_ = (TH2F*)f2->Get(Name_Path22.c_str() );
          NewTitleNameTemp2 << " (inv)";
        }//if hROCNew2D_ == NULL
        if (hROCOld1D_ == NULL)
        {
          SkippedROC_Names.push_back(branch1P.rocName );
          std::cout << Name_Path11.c_str() << "Cannot find Histo with this path with or without the \"(inv)\": " << std::endl;
          continue;
        }//if hROCOld1D_ and hROCNew1D_ still are NULL
        if (hROCNew1D_ == NULL)
        {
          SkippedROC_Names.push_back(branch2P.rocName );
          std::cout << Name_Path21.c_str() << "Cannot find Histo with this path with or without the \"(inv)\": " << std::endl;
          continue;
        }//if hROCOld1D_ and hROCNew1D_ still are NULL
        if (hROCOld2D_ == NULL)
        {
          SkippedROC_Names.push_back(branch1P.rocName );
          std::cout << Name_Path12.c_str() << "Cannot find Histo with this path with or without the \"(inv)\": " << std::endl;
          continue;
        }//if hROCOld1D_ and hROCNew1D_ still are NULL
        if (hROCNew1D_ == NULL)
        {
          SkippedROC_Names.push_back(branch2P.rocName );
          std::cout << Name_Path21.c_str() << "Cannot find Histo with this path with or without the \"(inv)\": " << std::endl;
          continue;
        }//if hROCOld1D_ and hROCNew1D_ still are NULL

        OldTitleName1 = OldTitleNameTemp1.str();
        NewTitleName1 = NewTitleNameTemp1.str();
        OldTitleName2 = OldTitleNameTemp2.str();
        NewTitleName2 = NewTitleNameTemp2.str();
        hROCOld1D_->SetTitle(OldTitleName1.c_str() );
        hROCNew1D_->SetTitle(NewTitleName1.c_str() );
        hROCOld2D_->SetTitle(OldTitleName2.c_str() );
        hROCNew2D_->SetTitle(NewTitleName2.c_str() );

        ROCCanvas_->cd(1);
        hROCOld1D_->Draw();
        ROCCanvas_->cd(2);
        hROCNew1D_->Draw();
        ROCCanvas_->cd(3);
        hROCOld2D_->Draw();
        ROCCanvas_->cd(4);
        hROCNew2D_->Draw();
	if (checkImproveThreshold)
	{
	  ChangingROCs_Improved->cd();
	  ROCCanvas_->Write();
	}//if checkImproveThreshold && checkThreshold
	else if (checkDegradeThreshold)
        {  
          ChangingROCs_Degraded->cd();
          ROCCanvas_->Write();
        }//if checkImproveThreshold && checkThreshold

	if (checkConstBad)
        {
	  BadROCsInBothRuns->cd();
	  stringstream ConstOldTemp1, ConstOldTemp2, ConstNewTemp1, ConstNewTemp2, CanvasConstBadTemp;
          CanvasConstBadTemp << thresholdMinLimit << " < Threshold < " << thresholdMaxLimit << " or ThresholdRMS > " << thresholdRMSLimit << ": " << ROCName;
          string CanvasConstBad = CanvasConstBadTemp.str();
          TCanvas *ROCCanvasConstBad_ = new TCanvas(CanvasConstBad.c_str(), "", 600, 600);
          ROCCanvasConstBad_->Divide(2,2);
          ConstOldTemp1 << "Run_" << argv[1] << " Constantly Bad Threshold1D";
          ConstOldTemp2 << "Run_" << argv[2] << " Constantly Bad Threshold1D";
          ConstNewTemp1 << "Run_" << argv[1] << " Constantly Bad Threshold2D";
          ConstNewTemp2 << "Run_" << argv[2] << " Constantly Bad Threshold2D";
          string ConstOld1 = ConstOldTemp1.str(), ConstOld2 = ConstOldTemp2.str(), ConstNew1 = ConstNewTemp1.str(), ConstNew2 = ConstNewTemp2.str();
	  hROCOld1D_->SetTitle(ConstOld1.c_str() );
          hROCNew1D_->SetTitle(ConstNew1.c_str() );
          hROCOld2D_->SetTitle(ConstOld2.c_str() );
          hROCNew2D_->SetTitle(ConstNew2.c_str() );
          ROCCanvasConstBad_->cd(1);
          hROCOld1D_->Draw();
          ROCCanvasConstBad_->cd(2);
          hROCNew1D_->Draw();
          ROCCanvasConstBad_->cd(3);
          hROCOld2D_->Draw();
          ROCCanvasConstBad_->cd(4);
          hROCNew2D_->Draw();
          ROCCanvasConstBad_->Write();
	  ROCCanvasConstBad_->Clear();
        }//else
        ROCCanvas_->Clear();
      }//if checkThreshold 
    }//if entry1 > 0 && entry2 > 0
  }//for nEntries1
std::cout << "<-------------Done Looping over TTrees--------------> " << std::endl;

//------------------------------------------------------------------------------------------
//Writing Histograms and Canvases
//------------------------------------------------------------------------------------------
  out_->cd();
  DeltaBadPixNumbCanvas_.cd();
  DeltaBadPixNumb_->Draw();
  DeltaBadPixNumbCanvas_.Write();

  DeltaBadPixPercCanvas_.cd();
  DeltaBadPixPerc_->Draw();
  DeltaBadPixPercCanvas_.Write();

  DeltaThresholdCanvas_.cd();
  DeltaThreshold_->Draw();
  DeltaThresholdCanvas_.Write();

  DeltaThresholdRMSCanvas_.cd();
  DeltaThresholdRMS_->Draw();
  DeltaThresholdRMSCanvas_.Write();
  
  BadPixNumbCanvas_.cd();
  BadPixNumb_->Draw();
  BadPixNumbCanvas_.Write();

  BadPixPercCanvas_.cd();
  BadPixPerc_->Draw();
  BadPixPercCanvas_.Write();

  ThresholdCanvas_.cd();
  Threshold_->Draw();
  ThresholdCanvas_.Write();

  ThresholdRMSCanvas_.cd();
  ThresholdRMS_->Draw();
  ThresholdRMSCanvas_.Write();

  out_->Write();
  gDirectory->Delete("BadPixPerc;1");
  gDirectory->Delete("Threshold;1");
  gDirectory->Delete("ThresholdRMS;1");
  gDirectory->Delete("BadPixNumb;1");
  gDirectory->Delete("DeltaBadPixPerc;1");
  gDirectory->Delete("DeltaBadPixNumb;1");
  gDirectory->Delete("DeltaThresholdRMS;1");
  gDirectory->Delete("DeltaThreshold;1");
  out_->Close();

std::cout << "<-------------Done Writing Histograms--------------> " << std::endl;
////////////////////////////////////
// Writing out the Changing Modules
////////////////////////////////////
  BadMODFile_ << "###################################\n## Improving MODs or PNLs Failing |Delta Threshold| > 5 or |Delta ThresholdRMS| > .5\n###################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapImpROCThreshold2Name.begin(); iter != MapImpROCThreshold2Name.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << " has " << iter->second << " bad ROC's.\n";
  }

  BadMODFile_ << "###################################\n## Degrading MODs or PNLs Failing |Delta Threshold| > 5 or |Delta ThresholdRMS| > .5\n###################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapDegROCThreshold2Name.begin(); iter != MapDegROCThreshold2Name.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << " has " << iter->second << " bad ROC's.\n";
  }

  BadMODFile_ << "#######################################\n## Constantly Bad MODs or PNLs Failing Threshold > 45 or ThresholdRMS > 4\n#######################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapConstROCThreshold2Name.begin(); iter != MapConstROCThreshold2Name.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << " has " << iter->second << " bad ROC's.\n";
  }

//////////////////////////////////////////////////////////////////////////////////////////////////////
//Writing out hte bad ROC's in a list with headers for the various cuts
//////////////////////////////////////////////////////////////////////////////////////////////////////
  BadROCFile_ << "#######################################\n## Improving ROCs Failing |Delta Threshold| > 5 or |Delta ThresholdRMS| > .5\n#######################################\n";
  for (std::vector<int>::size_type i = 0; i != ImpROCThreshold2_ThrNew.size(); i++)
  {
    BadROCFile_ << ImpROCThreshold2_Name[i] << "   \tOld Run Threshold= " <<  ImpROCThreshold2_ThrOld[i] << "   \t New Run Threshold= " <<  ImpROCThreshold2_ThrNew[i];
    BadROCFile_ << "   \tOld Run ThresholdRMS= " <<  ImpROCThreshold2_ThrRMSOld[i] << "\tNew Run ThresholdRMS= " <<  ImpROCThreshold2_ThrRMSNew[i] << "\n";
  }//for i

  BadROCFile_ << "#######################################\n## Degrading ROCs Failing |Delta Threshold| > 5  or |Delta ThresholdRMS| > .5\n#######################################\n";
  for (std::vector<int>::size_type i = 0; i != DegROCThreshold2_ThrNew.size(); i++)
  {
    BadROCFile_ << DegROCThreshold2_Name[i] << "   \tOld Run Threshold= " <<  DegROCThreshold2_ThrOld[i] << "   \t New Run Threshold= " <<  DegROCThreshold2_ThrNew[i]; 
    BadROCFile_ << "   \tOld Run ThresholdRMS= " <<  DegROCThreshold2_ThrRMSOld[i] << "\tNew Run ThresholdRMS= " <<  DegROCThreshold2_ThrRMSNew[i] << "\n";
  }//for i

  BadROCFile_ << "#####################################\n## Constantly ROCs Failing Threshold > 45 or ThresholdRMS > 4\n###########################################\n";
  for (std::vector<int>::size_type i = 0; i != ConstROCThreshold2_ThrNew.size(); i++)
  {
    BadROCFile_ << ConstROCThreshold2_Name[i] << "   \tOld Run Threshold= " <<  ConstROCThreshold2_ThrOld[i] << "   \t New Run Threshold= " <<  ConstROCThreshold2_ThrNew[i]; 
    BadROCFile_ << "   \tOld Run ThresholdRMS= " <<  ConstROCThreshold2_ThrRMSOld[i] << "\tNew Run ThresholdRMS= " <<  ConstROCThreshold2_ThrRMSNew[i] << "\n";
  }//for i

  BadROCFile_.close();
  BadMODFile_.close();
  return 0;
}//end main



////////////////////////////////////////////////////////////////////////////////
// Gets MOD of PNL name from ROC name 
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





