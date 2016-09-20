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

  //------------------------------------------------------------------------------------------------------------------------
  //Getting main directory environment variable and building path to home 
  //------------------------------------------------------------------------------------------------------------------------
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

  int runNumber1 = -1, runNumber2 = -1;
  runNumber1 = atoi(argv[1]); 
  runNumber2 = atoi(argv[2]);
  if (runNumber1 > runNumber2)
  {
    cout << "For purposes to see if a ROC improved or deteriorated, please put the older run number first (the one with the smaller number). This is for consistency." << endl;
    exit(0);
  }//fi runNumber1 > runNumber2 

  //---------------------------------
  //Getting input and output File Path+Name
  //---------------------------------
  int groupDirectory1 = ((int)runNumber1/1000)*1000,  groupDirectory2 = ((int)runNumber2/1000)*1000;
  stringstream inFile1Dir, inFile2Dir, outFileNameTEMP, BadROCOutputLocationTEMP, BadMODandPNLLocationTEMP;

  //2 Runs have 3 .root files each. Names declared below
  inFile1Dir << "/pixel/data0/Run_" << groupDirectory1 <<  "/Run_" << argv[1] << "/";
  inFile2Dir << "/pixel/data0/Run_" << groupDirectory2 <<  "/Run_" << argv[2] << "/";
  string inTotalName11 = inFile1Dir.str() + "AddressLevels_1.root", inTotalName12 = inFile1Dir.str() + "AddressLevels_2.root", inTotalName13 = inFile1Dir.str() + "AddressLevels_3.root";
  string inTotalName21 = inFile2Dir.str() + "AddressLevels_1.root", inTotalName22 = inFile2Dir.str() + "AddressLevels_2.root", inTotalName23 = inFile2Dir.str() + "AddressLevels_3.root";

  //output Root file with plots
  outFileNameTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(outFileNameTEMP.str()[outFileNameTEMP.str().length()-1] != '/')
    outFileNameTEMP << "/";
  outFileNameTEMP << "AddLevelComparison_Run_" <<  argv[1] << "_Run_" << argv[2] << ".root"; 
  string outFileName = outFileNameTEMP.str();

  //output .out file with Bad Roc lists
  BadROCOutputLocationTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(BadROCOutputLocationTEMP.str()[BadROCOutputLocationTEMP.str().length()-1] != '/')
    BadROCOutputLocationTEMP << "/";
  BadROCOutputLocationTEMP << "AddLevelBadROCNames_Run_" << argv[1] << "_Run_" << argv[2] << ".out";
  string BadROCOutputLocation = BadROCOutputLocationTEMP.str();

  BadMODandPNLLocationTEMP << string(getenv("POS_OUTPUT_DIRS"));
  if(BadMODandPNLLocationTEMP.str()[BadMODandPNLLocationTEMP.str().length()-1] != '/')
    BadMODandPNLLocationTEMP << "/";
  BadMODandPNLLocationTEMP << "AddLevelBadMODandPNLNames_Run_" << argv[1] << "_Run_" << argv[2] << ".out";
  string BadMODandPNLLocation = BadMODandPNLLocationTEMP.str();
  

  cout << "Run #" << runNumber1 << " has path " << inFile1Dir.str() << "  inTotalName11= " << inTotalName11.c_str() << endl;
  cout << "Run #" << runNumber2 << " has path " << inFile2Dir.str() << "  inTotalName21= " << inTotalName21.c_str() << endl;
  cout << "Output Root file path+name= " << outFileNameTEMP.str() << endl;
  cout << "Bad ROC output file path+name= " << BadROCOutputLocationTEMP.str() << endl;
  cout << "Bad MOD or PNL output file path+name= " << BadMODandPNLLocationTEMP.str() << endl;


  //Input file declarations
  TFile* f11 = new TFile(inTotalName11.c_str() );
  TFile* f12 = new TFile(inTotalName12.c_str() );
  TFile* f13 = new TFile(inTotalName13.c_str() );

  TFile* f21 = new TFile(inTotalName21.c_str() );
  TFile* f22 = new TFile(inTotalName22.c_str() );
  TFile* f23 = new TFile(inTotalName23.c_str() );

  //output .out file declaration
  ofstream BadROCFile_;
  BadROCFile_.open(BadROCOutputLocation.c_str() );

  ofstream BadMODFile_;
  BadMODFile_.open(BadMODandPNLLocation.c_str() );


  //-----------------
  //Defining Trees, Histograms, and Canvases
  //-----------------
  TFile *out_ = new TFile(outFileName.c_str(), "RECREATE");

  //The TDirectories are made for the TBM histograms of that are within the range stated in the name. Sep = Separation
  TDirectory *topDir = gDirectory;
  TDirectory *DeltaRMSgt2Dec = out_->mkdir("DeltaRMS_gt_2_Decline");
  TDirectory *DeltaRMSgt2Imp = out_->mkdir("DeltaRMS_gt_2_Improved");
  TDirectory *DeltaSepgt5Imp = out_->mkdir("DeltaSep_gt_5_Improved");
  TDirectory *DeltaSepgt5Dec = out_->mkdir("DeltaSep_gt_5_Decline");
  TDirectory *BothRunsRMSgt7= out_->mkdir("BadROC_in_Both_Runs_RMS_gt_7");
  TDirectory *BothRunsSeplt10= out_->mkdir("BadROC_in_Both_Runs_Sep_lt_10");
  stringstream dPeakNameTemp1, dPeakNameTemp2, wPeakNameTemp1, wPeakNameTemp2;
  dPeakNameTemp1 << "DoublePeaks_Run_" << argv[1];
  dPeakNameTemp2 << "DoublePeaks_Run_" << argv[2];
  wPeakNameTemp1 << "WidePeaks_Run_" << argv[1];
  wPeakNameTemp2 << "WidePeaks_Run_" << argv[2];
  string dPeakName1 = dPeakNameTemp1.str(), dPeakName2 = dPeakNameTemp2.str(), wPeakName1 = wPeakNameTemp1.str(), wPeakName2 = wPeakNameTemp2.str();
  TDirectory *DoublePeaks1 = out_->mkdir(dPeakName1.c_str() );
  TDirectory *DoublePeaks2 = out_->mkdir(dPeakName2.c_str() );
  TDirectory *WidePeaks1 = out_->mkdir(wPeakName1.c_str() );
  TDirectory *WidePeaks2 = out_->mkdir(wPeakName2.c_str() );

  TCanvas DeltaMaxRMSCanvas_("DeltaMaxRMSCanvas", "", 600, 600);
  TCanvas DeltaMinSepCanvas_("DeltaMinSepCanvas", "", 600, 600);
  TCanvas DeltaMinSepVsDeltaMaxRMSCanvas_("DeltaMinSepVsDeltaMaxRMSCanvas", "", 600, 600);

  TCanvas MaxRMSCanvas_("MaxRMSCanvas", "", 600, 600);
  TCanvas MinSepCanvas_("MinSepCanvas", "", 600, 600);

  TH1F *DeltaMaxRMS_ = new TH1F("DeltaMaxRMS", ";#DeltaMaxRMS", 100, -15, 15);
  TH1F *DeltaMinSep_ = new TH1F("DeltaMinSep", ";#DeltaMinSep", 100, -15, 15);
  TH2F *DeltaMinSepVsDeltaMaxRMS_ = new TH2F("DeltaMinSepVsDeltaMaxRMS", ";#DeltaMinSep;#DeltaMaxRMS", 200, -15, 15, 100, -15, 15);

  TH1F *MaxRMS_ = new TH1F("MaxRMS", ";MaxRMS", 100, -15, 15);
  TH1F *MinSep_ = new TH1F("MinSep", ";MinSep", 100, -15, 15);

  TCanvas DeltaRMSFail2Canvas_("DeltaRMSFail2Combined", "", 600, 600);
  TH1F *DeltaRMSFail2_ = new TH1F("DeltaRMSFail2", ";#DeltaSep", 40, -15, 15);
  TH1F *DeltaRMSPass2_ = new TH1F("DeltaRMSPass2", ";#DeltaSep", 40, -15, 15);

  TCanvas DeltaSepFail5Canvas_("DeltaSepFail5Combined", "", 600, 600);
  TH1F *DeltaSepFail5_ = new TH1F("DeltaSepFail5", ";#DeltaRMS", 40, -15, 15);
  TH1F *DeltaSepPass5_ = new TH1F("DeltaSepPass5", ";#DeltaRMS", 40, -15, 15);

  //Declaring Trees, the Struct to read from the trees, and the address to read read from the trees 
  TTree *tree11 = (TTree*)f11->Get("SummaryTrees/LevelSummary");
  TTree *tree12 = (TTree*)f12->Get("SummaryTrees/LevelSummary");
  TTree *tree13 = (TTree*)f13->Get("SummaryTrees/LevelSummary");
  
  TTree *tree21 = (TTree*)f21->Get("SummaryTrees/LevelSummary");
  TTree *tree22 = (TTree*)f22->Get("SummaryTrees/LevelSummary");
  TTree *tree23 = (TTree*)f23->Get("SummaryTrees/LevelSummary");

  struct addressLevelBranch{
    float pass;
    float nPeaks;
    float maxrms;
    float minseparation;
    float blackrms;
    char  rocName[38];
  };

  addressLevelBranch branch11, branch12, branch13, branch21, branch22, branch23;

  tree11->SetBranchAddress("AllLevels", &branch11);
  tree12->SetBranchAddress("AllLevels", &branch12);
  tree13->SetBranchAddress("AllLevels", &branch13);
  tree21->SetBranchAddress("AllLevels", &branch21);
  tree22->SetBranchAddress("AllLevels", &branch22);
  tree23->SetBranchAddress("AllLevels", &branch23);


std::cout << "<-------------Done Initializing--------------> " << std::endl;
  //------------------------------------------------------
  //  Looping over trees 
  //------------------------------------------------------
  Int_t nEntries11 = tree11->GetEntries(), nEntries12 = tree12->GetEntries(), nEntries13 = tree13->GetEntries();
  Int_t entry11= -10, entry12 = -10, entry13 = -10, entry21 = -10, entry22 = -10, entry23 = -10; 
  // The bool double array is for checking if the Fed#_Chan# TBM plot has already been saved in the TDirectory. When it is saved for the first time, the element will be set to true
  
  //The lists of the names of the BadRocs that fail within the DeltaRMS or DeltaSeparation defined in the names of the TDirectories
  std::vector<string> BadROCListRMS2_Improve, BadROCListRMS2_Decline, BadROCListSep5_Improve, BadROCListSep5_Decline;
  std::vector<double> ImproveRMS2_RMSOld, ImproveRMS2_RMSNew, DeclineRMS2_RMSOld, DeclineRMS2_RMSNew, ImproveSep5_SepOld, ImproveSep5_SepNew, DeclineSep5_SepOld, DeclineSep5_SepNew;
  std::vector<double> ImproveRMS2_DeltaRMS, DeclineRMS2_DeltaRMS, ImproveSep5_DeltaSep, DeclineSep5_DeltaSep, DoublePeakSep1, DoublePeakSep2, WidePeakSep1, WidePeakSep2, DoublePeakRMS1, DoublePeakRMS2, WidePeakRMS1, WidePeakRMS2;
  std::vector<double> BadRMS_Old, BadRMS_New, BadRMS_Delta, BadSep_Old, BadSep_New, BadSep_Delta;
  std::vector<string> BadRMSNames, BadSepNames, DoublePeakNames1, DoublePeakNames2, WidePeakNames1, WidePeakNames2;
  typedef std::map<string, unsigned int> NameMap;
  NameMap MapBadMODListRMS2_Improve, MapBadMODListRMS2_Decline, MapBadMODListSep5_Improve, MapBadMODListSep5_Decline,  MapConstBadRMS, MapConstBadSep;
  string MOD_PNL_Name;

  //-----------------------------------------------
  // Looping over the Address_1.root for both runs
  //-----------------------------------------------
  for (Int_t i=0; i < nEntries11; i++)
  {
    entry11 = tree11->GetEntry(i);
    entry21 = tree21->GetEntry(i);
    if (entry11 > 0 && entry21 > 0)// Before comparing them, the if statements ensures that both entries have been read. So if one tree is empty and the other isn't, the loop will not compare values
    {
      double DeltaRMS = branch11.maxrms - branch21.maxrms;
      double DeltaSep = branch11.minseparation - branch21.minseparation;
      MaxRMS_->Fill(branch11.maxrms );
      MaxRMS_->Fill(branch21.maxrms );
      MinSep_->Fill(branch11.minseparation );
      MinSep_->Fill(branch21.minseparation );
      if (DeltaSep > -200000 && DeltaSep < 200000 && DeltaRMS > -100000 && DeltaRMS < 1000000)
      {
        DeltaMaxRMS_->Fill(DeltaRMS);
        DeltaMinSep_->Fill(DeltaSep);
        DeltaMinSepVsDeltaMaxRMS_->Fill(DeltaSep, DeltaRMS);
      }//if DeltaSep > -200000 && DeltaSep < 200000 && DeltaRMS > -100000 && DeltaRMS < 1000000      

      /////////////////////////
      // Filling the RMS
      /////////////////////////
      topDir->cd();    
      if ( (DeltaRMS < 2 && DeltaRMS > 1.5) || (DeltaRMS > -2 && DeltaRMS < -1.5 ) )//Pass
        DeltaRMSPass2_->Fill(DeltaSep );
      if ( (DeltaRMS > 2 && DeltaRMS < 2.5) || (DeltaRMS < -2 && DeltaRMS > -2.5 ) )//Fail
        DeltaRMSFail2_->Fill(DeltaSep );

      /////////////////////////
      // Filling the Separation
      /////////////////////////
      topDir->cd();
      if ( (DeltaSep < 5 && DeltaSep > 4.5) || (DeltaSep > -5 && DeltaSep < -4.5 ) )//Pass
        DeltaSepPass5_->Fill(DeltaRMS );
      if ( (DeltaSep > 5 && DeltaSep < 5.5) || (DeltaSep < -5 && DeltaSep > -5.5 ) )//Fail
        DeltaSepFail5_->Fill(DeltaRMS );

      ///////////////////////////////////////////////////////////////////////////
      // Finding Finding MOD or PNL name of the failing ROC's for various RMS cuts
      ///////////////////////////////////////////////////////////////////////////
      bool checkRMS = false, checkSep = false, improveRMS = false, improveSep = false, checkBadRMS = false, checkBadSep = false;
      MOD_PNL_Name = MDLorPNLName(branch21.rocName);
      if ( DeltaRMS > 2.0 && branch21.maxrms < 50.0 && branch11.maxrms < 50.0)
      {
	checkRMS = true;
	improveRMS = true;
   	if ( MapBadMODListRMS2_Improve.find(MOD_PNL_Name) == MapBadMODListRMS2_Improve.end() )
	  MapBadMODListRMS2_Improve[MOD_PNL_Name ] = 1;
	else
	  MapBadMODListRMS2_Improve[MOD_PNL_Name ] = MapBadMODListRMS2_Improve[MOD_PNL_Name ] + 1;
	BadROCListRMS2_Improve.push_back(branch21.rocName );
	ImproveRMS2_RMSOld.push_back(branch11.maxrms );
	ImproveRMS2_RMSNew.push_back(branch21.maxrms );
	ImproveRMS2_DeltaRMS.push_back(DeltaRMS );
      }//if Delta > 2

      if ( DeltaRMS < -2.0 && branch21.maxrms < 50.0 && branch11.maxrms < 50.0)
      {
        checkRMS = true;
        if ( MapBadMODListRMS2_Decline.find(MOD_PNL_Name ) == MapBadMODListRMS2_Decline.end() )
          MapBadMODListRMS2_Decline[MOD_PNL_Name ] = 1;
        else
          MapBadMODListRMS2_Decline[MOD_PNL_Name ] = MapBadMODListRMS2_Decline[MOD_PNL_Name ] + 1;
        BadROCListRMS2_Decline.push_back(branch21.rocName );
        DeclineRMS2_RMSOld.push_back(branch11.maxrms );
        DeclineRMS2_RMSNew.push_back(branch21.maxrms );
        DeclineRMS2_DeltaRMS.push_back(DeltaRMS );
      }//if Delta > 2

      ///////////////////////////////////////////////////////////////////////////
      // Finding Finding MOD or PNL name of the failing ROC's for various Separation cuts
      ///////////////////////////////////////////////////////////////////////////
      if ( DeltaSep > 5.0 && branch21.minseparation > 1.0 && branch11.minseparation > 1.0)
      {
	checkSep = true;
        if ( MapBadMODListSep5_Improve.find(MOD_PNL_Name ) == MapBadMODListSep5_Improve.end() )
          MapBadMODListSep5_Improve[MOD_PNL_Name ] = 1;
        else
          MapBadMODListSep5_Improve[MOD_PNL_Name ] = MapBadMODListSep5_Improve[MOD_PNL_Name ] + 1;
        BadROCListSep5_Improve.push_back(branch21.rocName );
        ImproveSep5_SepOld.push_back(branch11.minseparation );
        ImproveSep5_SepNew.push_back(branch21.minseparation );
        ImproveSep5_DeltaSep.push_back(DeltaSep );
      }//if Delta > 5

      if ( DeltaSep < -5.0 && branch21.minseparation > 1.0 && branch11.minseparation > 1.0)
      {
        checkSep = true;
	improveSep = true;
        if ( MapBadMODListSep5_Decline.find(MOD_PNL_Name ) == MapBadMODListSep5_Decline.end() )
          MapBadMODListSep5_Decline[MOD_PNL_Name ] = 1;
        else
          MapBadMODListSep5_Decline[MOD_PNL_Name ] = MapBadMODListSep5_Decline[MOD_PNL_Name ] + 1;
        BadROCListSep5_Decline.push_back(branch21.rocName );
        DeclineSep5_SepOld.push_back(branch11.minseparation );
        DeclineSep5_SepNew.push_back(branch21.minseparation );
        DeclineSep5_DeltaSep.push_back(DeltaSep );
      }//if Delta > 5

      ///////////////////////////////////////////////////////////////////////////
      // Finding the consistently bad rocs 
      ///////////////////////////////////////////////////////////////////////////
      if (branch11.maxrms > 7 && branch21.maxrms > 7 && branch21.maxrms < 50.0 && branch11.maxrms < 50.0)
      {
	checkBadRMS = true;
        if ( MapConstBadRMS.find(MOD_PNL_Name ) == MapConstBadRMS.end() )
          MapConstBadRMS[MOD_PNL_Name ] = 1;
        else
          MapConstBadRMS[MOD_PNL_Name ] = MapConstBadRMS[MOD_PNL_Name ] + 1;
	BadRMSNames.push_back(branch11.rocName );
	BadRMS_Old.push_back(branch11.maxrms );
	BadRMS_New.push_back(branch21.maxrms );
	BadRMS_Delta.push_back(DeltaRMS );
      }//if maxRMS > 7
      else if (branch11.minseparation < 10 && branch21.minseparation < 10 && branch21.minseparation > 1.0 && branch11.minseparation > 1.0)
      {
        checkBadSep = true;
        if ( MapConstBadSep.find(MOD_PNL_Name ) == MapConstBadSep.end() )
          MapConstBadSep[MOD_PNL_Name ] = 1;
        else
          MapConstBadSep[MOD_PNL_Name ] = MapConstBadSep[MOD_PNL_Name ] + 1;
        BadSepNames.push_back(branch11.rocName );
	BadSep_Old.push_back(branch11.minseparation );
	BadSep_New.push_back(branch21.minseparation );
	BadSep_Delta.push_back(DeltaSep );
      }//if maxSep > 7

      //////////////////////////////////////////////////////////////////////////////////////////////////////
      // Finding Double Peaks and Wide Peaks
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      bool doublePeak1 = false, doublePeak2 = false, widePeak1 = false, widePeak2 = false;
      if (branch11.minseparation < 5.75 && branch11.maxrms > 10 && branch11.minseparation > 1.1) //The greater than 1 is for empty plots
      { 
        doublePeak1 = true;
        DoublePeakNames1.push_back(branch11.rocName );
	DoublePeakSep1.push_back(branch11.minseparation );
        DoublePeakRMS1.push_back(branch11.maxrms );
      }//if branch21      
      if (branch21.minseparation < 5.75 && branch21.maxrms > 10 && branch21.minseparation > 1.1)
      {
	doublePeak2 = true;
        DoublePeakNames2.push_back(branch21.rocName );
        DoublePeakSep2.push_back(branch21.minseparation );
        DoublePeakRMS2.push_back(branch21.maxrms );
      }//if branch21      
      if (branch11.minseparation > 5.75 && branch11.minseparation < 6.5)
      {
        widePeak1 = true;
        WidePeakNames1.push_back(branch11.rocName );
        WidePeakSep1.push_back(branch11.minseparation );
        WidePeakRMS1.push_back(branch11.maxrms );
      }//if branch21      
      if (branch21.minseparation > 5.75 && branch21.minseparation < 6.5)
      {
        widePeak2 = true;
        WidePeakNames2.push_back(branch21.rocName );
        WidePeakSep2.push_back(branch21.minseparation );
        WidePeakRMS2.push_back(branch21.maxrms );
      }//if branch21      


      //////////////////////////////////////////////////////////////////////////////////////////////////////
      // Checking if the TBM plot has already been plotted in the TDirectory 
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      if (checkRMS)
      {
	if (DeltaRMS < 0)
	  DeltaRMSgt2Dec->cd();
	else
          DeltaRMSgt2Imp->cd();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Getting the Path+Name of the histogram of the ROC and plotting the f12 and f11 together
	//////////////////////////////////////////////////////////////////////////////////////////////////////////// 
        stringstream RMS_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
 	string ROCName = branch21.rocName, TitleName;
	unsigned int found = 0;
	for (int i = 0; i < 6; i++)
	{
	  found = ROCName.find('_',found+1);
 	  RMS_Name_Path_TEMP << ROCName.substr(0,found) << "/";
	}//for i	  
	RMS_Name_Path_TEMP << ROCName;
	string RMS_Name_Path = RMS_Name_Path_TEMP.str();
	CanvasNameTemp << "Failed RMS < 2.0: " << ROCName;
	string CanvasName = CanvasNameTemp.str();

	TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_RMS11_ = (TH1F*)f11->Get(RMS_Name_Path.c_str() );
        TH1F *hROC_RMS21_ = (TH1F*)f21->Get(RMS_Name_Path.c_str() );
	if (improveRMS)
	  TitleNameTemp << "Improved RMS: " << ROCName;
	else
	  TitleNameTemp << "Declined RMS: " << ROCName;
	TitleName = TitleNameTemp.str();
	hROC_RMS11_->SetTitle(TitleName.c_str() );
	hROC_RMS21_->SetTitle(TitleName.c_str() );
        hROC_RMS11_->SetLineColor(kRed);
        hROC_RMS21_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_RMS11_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_RMS21_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_RMS11_->DrawNormalized();
        hROC_RMS21_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkRMS 

      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Checking if the ROC plot has already been plotted in the correct TDirectory 
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      if (checkSep)
      {
	if (DeltaSep < 0)
	  DeltaSepgt5Imp->cd();
	else
          DeltaSepgt5Dec->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f11 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream Sep_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch21.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Sep_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Sep_Name_Path_TEMP << ROCName;
        string Sep_Name_Path = Sep_Name_Path_TEMP.str();
        CanvasNameTemp << "Failed Sep < 5.0: " << ROCName;
        string CanvasName = CanvasNameTemp.str();
        
        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_Sep11_ = (TH1F*)f11->Get(Sep_Name_Path.c_str() );
        TH1F *hROC_Sep21_ = (TH1F*)f21->Get(Sep_Name_Path.c_str() );
        if (improveSep)
          TitleNameTemp << "Improved Sep: " << ROCName;
        else
          TitleNameTemp << "Declined Sep: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_Sep11_->SetTitle(TitleName.c_str() );
        hROC_Sep21_->SetTitle(TitleName.c_str() );
        hROC_Sep11_->SetLineColor(kRed);
        hROC_Sep21_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_Sep11_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_Sep21_,Run2.c_str(),"l");
        
        ROCCanvas_.cd();
        hROC_Sep11_->DrawNormalized();
        hROC_Sep21_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkSep  

      if (checkBadRMS)
      {
        BothRunsRMSgt7->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f11 together for the ROCs that fail maxrms
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream RMS_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch21.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          RMS_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        RMS_Name_Path_TEMP << ROCName;
        string RMS_Name_Path = RMS_Name_Path_TEMP.str();
        CanvasNameTemp << "Both Runs RMS > 7: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_RMS11_ = (TH1F*)f11->Get(RMS_Name_Path.c_str() );
        TH1F *hROC_RMS21_ = (TH1F*)f21->Get(RMS_Name_Path.c_str() );
        TitleNameTemp << "Both Runs RMS > 7: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_RMS11_->SetTitle(TitleName.c_str() );
        hROC_RMS21_->SetTitle(TitleName.c_str() );
        hROC_RMS11_->SetLineColor(kRed);
        hROC_RMS21_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_RMS11_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_RMS21_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_RMS11_->DrawNormalized();
        hROC_RMS21_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkBadRMS 

      if (checkBadSep)
      {
        BothRunsSeplt10->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f11 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream Sep_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch21.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Sep_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Sep_Name_Path_TEMP << ROCName;
        string Sep_Name_Path = Sep_Name_Path_TEMP.str();
        CanvasNameTemp << "Both Runs Sep < 10: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_Sep11_ = (TH1F*)f11->Get(Sep_Name_Path.c_str() );
        TH1F *hROC_Sep21_ = (TH1F*)f21->Get(Sep_Name_Path.c_str() );
        TitleNameTemp << "Both Runs Sep < 10: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_Sep11_->SetTitle(TitleName.c_str() );
        hROC_Sep21_->SetTitle(TitleName.c_str() );
        hROC_Sep11_->SetLineColor(kRed);
        hROC_Sep21_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_Sep11_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_Sep21_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_Sep11_->DrawNormalized();
        hROC_Sep21_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkBadSep 

      if (doublePeak1 || doublePeak2 || widePeak1 || widePeak2)
      {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f11 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	stringstream Name_Path_TEMP;
        string ROCName = branch21.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Name_Path_TEMP << ROCName;
        string Name_Path = Name_Path_TEMP.str();
        TH1F *hROC_Sep11_ = (TH1F*)f11->Get(Name_Path.c_str() );
        TH1F *hROC_Sep21_ = (TH1F*)f21->Get(Name_Path.c_str() );
        hROC_Sep11_->SetLineColor(kRed);
        hROC_Sep21_->SetLineColor(kBlue);

	if (doublePeak1)
	{
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
	  DoublePeaks1->cd();
	  CanvasNameTemp << "DoublePeak Run #" << argv[1] << ": " << ROCName;
	  TitleNameTemp << "DoublePeak Run #" << argv[1] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep11_->SetTitle(TitleName.c_str() );
          ROCCanvas_.cd();
          hROC_Sep11_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
	}//if doublePeak1
        if (doublePeak2)
        {
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          DoublePeaks2->cd();
          CanvasNameTemp << "DoublePeak Run #" << argv[2] << ": " << ROCName;
          TitleNameTemp << "DoublePeak Run #" << argv[2] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep21_->SetTitle(TitleName.c_str() ); 
          ROCCanvas_.cd();
          hROC_Sep21_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if doublePeak1
        if (widePeak1)
        {
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          WidePeaks1->cd();
          CanvasNameTemp << "WidePeak Run #" << argv[1] << ": " << ROCName;
          TitleNameTemp << "WidePeak Run #" << argv[1] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep11_->SetTitle(TitleName.c_str() ); 
          ROCCanvas_.cd();
          hROC_Sep11_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if widePeak1
        if (widePeak2)
        { 
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          WidePeaks2->cd();
          CanvasNameTemp << "WidePeak Run #" << argv[2] << ": " << ROCName;
          TitleNameTemp << "WidePeak Run #" << argv[2] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep21_->SetTitle(TitleName.c_str() );
          ROCCanvas_.cd();
          hROC_Sep21_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if widePeak1
      }//if doublePeak
    }//if entry11 > 0 && entry21 > 0
  }//for nEntries11
std::cout << "<-------------Done Looping over AddressLevels_1.root--------------> " << std::endl;

  //-----------------------------------------------
  // Looping over the Address_2.root for both runs
  //-----------------------------------------------
  for (Int_t i=0; i < nEntries12; i++)
  { 
    entry12 = tree12->GetEntry(i);
    entry22 = tree22->GetEntry(i);
    if (entry12 > 0 && entry22 > 0)//Makes sure both entries exists
    {
      double DeltaRMS = branch12.maxrms - branch22.maxrms;
      double DeltaSep = branch12.minseparation - branch22.minseparation;
      MaxRMS_->Fill(branch12.maxrms );
      MaxRMS_->Fill(branch22.maxrms );
      MinSep_->Fill(branch12.minseparation );
      MinSep_->Fill(branch22.minseparation );
      if (DeltaSep > -200000 && DeltaSep < 200000 && DeltaRMS > -100000 && DeltaRMS < 1000000)
      {
        DeltaMaxRMS_->Fill(DeltaRMS);
        DeltaMinSep_->Fill(DeltaSep);
        DeltaMinSepVsDeltaMaxRMS_->Fill(DeltaSep, DeltaRMS);
      }//if DeltaSep > -200000 && DeltaSep < 200000 && DeltaRMS > -100000 && DeltaRMS < 1000000      
     
      /////////////////////////
      // Filling the RMS
      /////////////////////////
      topDir->cd();
      if ( (DeltaRMS < 2 && DeltaRMS > 1.5) || (DeltaRMS > -2 && DeltaRMS < -1.5 ) )//Pass
        DeltaRMSPass2_->Fill(DeltaSep );
      if ( (DeltaRMS > 2 && DeltaRMS < 2.5) || (DeltaRMS < -2 && DeltaRMS > -2.5 ) )//Fail
        DeltaRMSFail2_->Fill(DeltaSep );
      
      /////////////////////////
      // Filling the Separation
      /////////////////////////
      topDir->cd();
      if ( (DeltaSep < 5 && DeltaSep > 4.5) || (DeltaSep > -5 && DeltaSep < -4.5 ) )//Pass
        DeltaSepPass5_->Fill(DeltaRMS );
      if ( (DeltaSep > 5 && DeltaSep < 5.5) || (DeltaSep < -5 && DeltaSep > -5.5 ) )//Fail
        DeltaSepFail5_->Fill(DeltaRMS );
      
      ///////////////////////////////////////////////////////////////////////////
      // Finding Finding MOD or PNL name of the failing ROC's for various RMS cuts
      ///////////////////////////////////////////////////////////////////////////
      bool checkRMS = false, checkSep = false, improveRMS = false, improveSep = false, checkBadRMS = false, checkBadSep = false;
      MOD_PNL_Name = MDLorPNLName(branch22.rocName);
      if ( DeltaRMS > 2.0 && branch22.maxrms < 50.0 && branch12.maxrms < 50.0)
      {
        checkRMS = true;
        improveRMS = true;
        if ( MapBadMODListRMS2_Improve.find(MOD_PNL_Name) == MapBadMODListRMS2_Improve.end() )
          MapBadMODListRMS2_Improve[MOD_PNL_Name ] = 1;
        else
          MapBadMODListRMS2_Improve[MOD_PNL_Name ] = MapBadMODListRMS2_Improve[MOD_PNL_Name ] + 1;
        BadROCListRMS2_Improve.push_back(branch22.rocName );
        ImproveRMS2_RMSOld.push_back(branch12.maxrms );
        ImproveRMS2_RMSNew.push_back(branch22.maxrms );
        ImproveRMS2_DeltaRMS.push_back(DeltaRMS );
      }//if Delta > 2

      if ( DeltaRMS < -2.0 && branch22.maxrms < 50.0 && branch12.maxrms < 50.0)
      {
        checkRMS = true;
        if ( MapBadMODListRMS2_Decline.find(MOD_PNL_Name ) == MapBadMODListRMS2_Decline.end() )
          MapBadMODListRMS2_Decline[MOD_PNL_Name ] = 1;
        else
          MapBadMODListRMS2_Decline[MOD_PNL_Name ] = MapBadMODListRMS2_Decline[MOD_PNL_Name ] + 1;
        BadROCListRMS2_Decline.push_back(branch22.rocName );
        DeclineRMS2_RMSOld.push_back(branch12.maxrms );
        DeclineRMS2_RMSNew.push_back(branch22.maxrms );
        DeclineRMS2_DeltaRMS.push_back(DeltaRMS );
      }//if Delta > 2

      ///////////////////////////////////////////////////////////////////////////
      // Finding Finding MOD or PNL name of the failing ROC's for various Separation cuts
      ///////////////////////////////////////////////////////////////////////////
      if ( DeltaSep > 5.0 && branch22.minseparation > 1.0 && branch12.minseparation > 1.0)
      {
        checkSep = true;
        if ( MapBadMODListSep5_Improve.find(MOD_PNL_Name ) == MapBadMODListSep5_Improve.end() )
          MapBadMODListSep5_Improve[MOD_PNL_Name ] = 1;
        else
          MapBadMODListSep5_Improve[MOD_PNL_Name ] = MapBadMODListSep5_Improve[MOD_PNL_Name ] + 1;
        BadROCListSep5_Improve.push_back(branch22.rocName );
        ImproveSep5_SepOld.push_back(branch12.minseparation );
        ImproveSep5_SepNew.push_back(branch22.minseparation );
        ImproveSep5_DeltaSep.push_back(DeltaSep );
      }//if Delta > 5

      if ( DeltaSep < -5.0 && branch22.minseparation > 1.0 && branch12.minseparation > 1.0)
      {
        checkSep = true;
        improveSep = true;
        if ( MapBadMODListSep5_Decline.find(MOD_PNL_Name ) == MapBadMODListSep5_Decline.end() )
          MapBadMODListSep5_Decline[MOD_PNL_Name ] = 1;
        else
          MapBadMODListSep5_Decline[MOD_PNL_Name ] = MapBadMODListSep5_Decline[MOD_PNL_Name ] + 1;
        BadROCListSep5_Decline.push_back(branch22.rocName );
        DeclineSep5_SepOld.push_back(branch12.minseparation );
        DeclineSep5_SepNew.push_back(branch22.minseparation );
        DeclineSep5_DeltaSep.push_back(DeltaSep );
      }//if Delta > 5

      ///////////////////////////////////////////////////////////////////////////
      // Finding the consistently bad rocs 
      ///////////////////////////////////////////////////////////////////////////
      if (branch12.maxrms > 7 && branch22.maxrms > 7 && branch22.maxrms < 50.0 && branch12.maxrms < 50.0)
      {
        checkBadRMS = true;
        if ( MapConstBadRMS.find(MOD_PNL_Name ) == MapConstBadRMS.end() )
          MapConstBadRMS[MOD_PNL_Name ] = 1;
        else
          MapConstBadRMS[MOD_PNL_Name ] = MapConstBadRMS[MOD_PNL_Name ] + 1;
        BadRMSNames.push_back(branch12.rocName );
        BadRMS_Old.push_back(branch12.maxrms );
        BadRMS_New.push_back(branch22.maxrms );
	BadRMS_Delta.push_back(DeltaRMS );
      }//if maxRMS > 7
      else if (branch12.minseparation < 10 && branch22.minseparation < 10 && branch22.minseparation > 1.0 && branch12.minseparation > 1.0)
      {
        checkBadSep = true;
        if ( MapConstBadSep.find(MOD_PNL_Name ) == MapConstBadSep.end() )
          MapConstBadSep[MOD_PNL_Name ] = 1;
        else
          MapConstBadSep[MOD_PNL_Name ] = MapConstBadSep[MOD_PNL_Name ] + 1;
        BadSepNames.push_back(branch12.rocName );
        BadSep_Old.push_back(branch12.minseparation );
        BadSep_New.push_back(branch22.minseparation );
	BadSep_Delta.push_back(DeltaSep );
      }//if maxSep > 7

      //////////////////////////////////////////////////////////////////////////////////////////////////////
      // Finding Double Peaks and Wide Peaks
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      bool doublePeak1 = false, doublePeak2 = false, widePeak1 = false, widePeak2 = false;
      if (branch12.minseparation < 5.75 && branch12.maxrms > 10 && branch12.minseparation > 1.1) //The greater than 1 is for empty plots
      { 
        doublePeak1 = true;
        DoublePeakNames1.push_back(branch12.rocName );
        DoublePeakSep1.push_back(branch12.minseparation );
        DoublePeakRMS1.push_back(branch12.maxrms );
      }//if branch22      
      if (branch22.minseparation < 5.75 && branch22.maxrms > 10 && branch22.minseparation > 1.1)
      { 
        doublePeak2 = true;
        DoublePeakNames2.push_back(branch22.rocName );
        DoublePeakSep2.push_back(branch22.minseparation );
        DoublePeakRMS2.push_back(branch22.maxrms );
      }//if branch22      
      if (branch12.minseparation > 5.75 && branch12.minseparation < 6.5) //The greater than 1 is for empty plots
      { 
        widePeak1 = true;
        WidePeakNames1.push_back(branch12.rocName );
        WidePeakSep1.push_back(branch12.minseparation );
        WidePeakRMS1.push_back(branch12.maxrms );
      }//if branch22      
      if (branch22.minseparation > 5.75 && branch22.minseparation < 6.5)
      { 
        widePeak2 = true;
        WidePeakNames2.push_back(branch22.rocName );
        WidePeakSep2.push_back(branch22.minseparation );
        WidePeakRMS2.push_back(branch22.maxrms );
      }//if branch22      

      //////////////////////////////////////////////////////////////////////////////////////////////////////
      // Checking to see if the TBM plot has already been plotted for that FED Chan in the TDirectory 
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      if (checkRMS)
      {
	if (DeltaRMS < 0)
	  DeltaRMSgt2Dec->cd();
	else
          DeltaRMSgt2Imp->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f12 together
        //////////////////////////////////////////////////////////////////////////////////////////////////////////// 
        stringstream RMS_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch22.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          RMS_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i          
        RMS_Name_Path_TEMP << ROCName;
        string RMS_Name_Path = RMS_Name_Path_TEMP.str();
        CanvasNameTemp << "Failed RMS < 2.0: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_RMS12_ = (TH1F*)f12->Get(RMS_Name_Path.c_str() );
        TH1F *hROC_RMS22_ = (TH1F*)f22->Get(RMS_Name_Path.c_str() );
        if (improveRMS)
          TitleNameTemp << "Improved RMS: " << ROCName;
        else
          TitleNameTemp << "Declined RMS: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_RMS12_->SetTitle(TitleName.c_str() );
        hROC_RMS22_->SetTitle(TitleName.c_str() );
        hROC_RMS12_->SetLineColor(kRed);
        hROC_RMS22_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_RMS12_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_RMS22_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_RMS12_->DrawNormalized();
        hROC_RMS22_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkRMS 

      //////////////////////////////////////////////////////////////////////////////////////////////////////
      // Checking to see if the TBM plot has already been plotted for that FED Chan in the TDirectory 
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      if (checkSep)
      {
	if (DeltaSep < 0)
	  DeltaSepgt5Imp->cd();
	else
          DeltaSepgt5Dec->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f12 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream Sep_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch22.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Sep_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Sep_Name_Path_TEMP << ROCName;
        string Sep_Name_Path = Sep_Name_Path_TEMP.str();
        CanvasNameTemp << "Failed Sep < 5.0: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_Sep12_ = (TH1F*)f12->Get(Sep_Name_Path.c_str() );
        TH1F *hROC_Sep22_ = (TH1F*)f22->Get(Sep_Name_Path.c_str() );
        if (improveSep)
          TitleNameTemp << "Improved Sep: " << ROCName;
        else
          TitleNameTemp << "Declined Sep: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_Sep12_->SetTitle(TitleName.c_str() );
        hROC_Sep22_->SetTitle(TitleName.c_str() );
        hROC_Sep12_->SetLineColor(kRed);
        hROC_Sep22_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_Sep12_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_Sep22_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_Sep12_->DrawNormalized();
        hROC_Sep22_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkSep  

      if (checkBadRMS)
      {
        BothRunsRMSgt7->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f12 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream RMS_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch22.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          RMS_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        RMS_Name_Path_TEMP << ROCName;
        string RMS_Name_Path = RMS_Name_Path_TEMP.str();
        CanvasNameTemp << "Both Runs RMS > 7: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_RMS12_ = (TH1F*)f12->Get(RMS_Name_Path.c_str() );
        TH1F *hROC_RMS22_ = (TH1F*)f22->Get(RMS_Name_Path.c_str() );
        TitleNameTemp << "Both Runs RMS > 7: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_RMS12_->SetTitle(TitleName.c_str() );
        hROC_RMS22_->SetTitle(TitleName.c_str() );
        hROC_RMS12_->SetLineColor(kRed);
        hROC_RMS22_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_RMS12_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_RMS22_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_RMS12_->DrawNormalized();
        hROC_RMS22_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkBadRMS 

      if (checkBadSep)
      {
        BothRunsSeplt10->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f12 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream Sep_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch22.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Sep_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Sep_Name_Path_TEMP << ROCName;
        string Sep_Name_Path = Sep_Name_Path_TEMP.str();
        CanvasNameTemp << "Both Runs Sep < 10: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_Sep12_ = (TH1F*)f12->Get(Sep_Name_Path.c_str() );
        TH1F *hROC_Sep22_ = (TH1F*)f22->Get(Sep_Name_Path.c_str() );
        TitleNameTemp << "Both Runs Sep < 10: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_Sep12_->SetTitle(TitleName.c_str() );
        hROC_Sep22_->SetTitle(TitleName.c_str() );
        hROC_Sep12_->SetLineColor(kRed);
        hROC_Sep22_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_Sep12_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_Sep22_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_Sep12_->DrawNormalized();
        hROC_Sep22_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkBadSep 

      if (doublePeak1 || doublePeak2 || widePeak1 || widePeak2)
      {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f12 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	stringstream Name_Path_TEMP;
        string ROCName = branch22.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Name_Path_TEMP << ROCName;
        string Name_Path = Name_Path_TEMP.str();
        TH1F *hROC_Sep12_ = (TH1F*)f12->Get(Name_Path.c_str() );
        TH1F *hROC_Sep22_ = (TH1F*)f22->Get(Name_Path.c_str() );
        hROC_Sep12_->SetLineColor(kRed);
        hROC_Sep22_->SetLineColor(kBlue);

        if (doublePeak1)
        {
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          DoublePeaks1->cd();
          CanvasNameTemp << "DoublePeak Run #" << argv[1] << ": " << ROCName;
          TitleNameTemp << "DoublePeak Run #" << argv[1] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep12_->SetTitle(TitleName.c_str() ); 
          ROCCanvas_.cd();
          hROC_Sep12_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if doublePeak1
        if (doublePeak2)
        { 
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          DoublePeaks2->cd();
          CanvasNameTemp << "DoublePeak Run #" << argv[2] << ": " << ROCName;
          TitleNameTemp << "DoublePeak Run #" << argv[2] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep22_->SetTitle(TitleName.c_str() );
          ROCCanvas_.cd();
          hROC_Sep22_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if doublePeak1
        if (widePeak1)
        {
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          WidePeaks1->cd();
          CanvasNameTemp << "WidePeak Run #" << argv[1] << ": " << ROCName;
          TitleNameTemp << "WidePeak Run #" << argv[1] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep12_->SetTitle(TitleName.c_str() );
          ROCCanvas_.cd();
          hROC_Sep12_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if widePeak1
        if (widePeak2)
        { 
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          WidePeaks2->cd();
          CanvasNameTemp << "WidePeak Run #" << argv[2] << ": " << ROCName;
          TitleNameTemp << "WidePeak Run #" << argv[2] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep22_->SetTitle(TitleName.c_str() );
          ROCCanvas_.cd();
          hROC_Sep22_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if widePeak1
      }//if doublePeak
    }//if entry12 > 0 && entry22 > 0
  }//for nEntries12
std::cout << "<-------------Done Looping over AddressLevels_2.root--------------> " << std::endl;


  //-----------------------------------------------
  // Looping over the Address_3.root for both runs
  //-----------------------------------------------
  for (Int_t i=0; i < nEntries13; i++)
  { 
    entry13 = tree13->GetEntry(i);
    entry23 = tree23->GetEntry(i);
    if (entry13 > 0 && entry23 > 0)//Makes sure both entries exist
    {
      double DeltaRMS = branch13.maxrms - branch23.maxrms;
      double DeltaSep = branch13.minseparation - branch23.minseparation;
      MaxRMS_->Fill(branch13.maxrms );
      MaxRMS_->Fill(branch23.maxrms );
      MinSep_->Fill(branch13.minseparation );
      MinSep_->Fill(branch23.minseparation );
      if (DeltaSep > -200000 && DeltaSep < 200000 && DeltaRMS > -100000 && DeltaRMS < 1000000) //Makes sure the Delta values aren't simply absurd
      {
        DeltaMaxRMS_->Fill(DeltaRMS);
        DeltaMinSep_->Fill(DeltaSep);
        DeltaMinSepVsDeltaMaxRMS_->Fill(DeltaSep, DeltaRMS);
      }//if DeltaSep > -200000 && DeltaSep < 200000 && DeltaRMS > -100000 && DeltaRMS < 1000000      

      /////////////////////////
      // Filling the RMS
      /////////////////////////
      topDir->cd();
      if ( (DeltaRMS < 2 && DeltaRMS > 1.5) || (DeltaRMS > -2 && DeltaRMS < -1.5 ) )//Pass
        DeltaRMSPass2_->Fill(DeltaSep );
      if ( (DeltaRMS > 2 && DeltaRMS < 2.5) || (DeltaRMS < -2 && DeltaRMS > -2.5 ) )//Fail
        DeltaRMSFail2_->Fill(DeltaSep );
      
      /////////////////////////
      // Filling the Separation
      /////////////////////////
      topDir->cd();
      if ( (DeltaSep < 5 && DeltaSep > 4.5) || (DeltaSep > -5 && DeltaSep < -4.5 ) )//Pass
        DeltaSepPass5_->Fill(DeltaRMS );
      if ( (DeltaSep > 5 && DeltaSep < 5.5) || (DeltaSep < -5 && DeltaSep > -5.5 ) )//Fail
        DeltaSepFail5_->Fill(DeltaRMS );
      
      ///////////////////////////////////////////////////////////////////////////
      // Finding Finding MOD or PNL name of the failing ROC's for various RMS cuts
      ///////////////////////////////////////////////////////////////////////////
      bool checkRMS = false, checkSep = false, improveRMS = false, improveSep = false, checkBadRMS = false, checkBadSep = false;
      MOD_PNL_Name = MDLorPNLName(branch23.rocName);
      if ( DeltaRMS > 2.0 && branch23.maxrms < 50.0 && branch13.maxrms < 50.0)
      {
        checkRMS = true;
        improveRMS = true;
        if ( MapBadMODListRMS2_Improve.find(MOD_PNL_Name) == MapBadMODListRMS2_Improve.end() )
          MapBadMODListRMS2_Improve[MOD_PNL_Name ] = 1;
        else
          MapBadMODListRMS2_Improve[MOD_PNL_Name ] = MapBadMODListRMS2_Improve[MOD_PNL_Name ] + 1;
        BadROCListRMS2_Improve.push_back(branch23.rocName );
        ImproveRMS2_RMSOld.push_back(branch13.maxrms );
        ImproveRMS2_RMSNew.push_back(branch23.maxrms );
        ImproveRMS2_DeltaRMS.push_back(DeltaRMS );
      }//if Delta > 2

      if ( DeltaRMS < -2.0 && branch23.maxrms < 50.0 && branch13.maxrms < 50.0)
      {
        checkRMS = true;
        if ( MapBadMODListRMS2_Decline.find(MOD_PNL_Name ) == MapBadMODListRMS2_Decline.end() )
          MapBadMODListRMS2_Decline[MOD_PNL_Name ] = 1;
        else
          MapBadMODListRMS2_Decline[MOD_PNL_Name ] = MapBadMODListRMS2_Decline[MOD_PNL_Name ] + 1;
        BadROCListRMS2_Decline.push_back(branch23.rocName );
        DeclineRMS2_RMSOld.push_back(branch13.maxrms );
        DeclineRMS2_RMSNew.push_back(branch23.maxrms );
        DeclineRMS2_DeltaRMS.push_back(DeltaRMS );
      }//if Delta > 2

      ///////////////////////////////////////////////////////////////////////////
      // Finding Finding MOD or PNL name of the failing ROC's for various Separation cuts
      ///////////////////////////////////////////////////////////////////////////
      if ( DeltaSep > 5.0 && branch23.minseparation > 1.0 && branch13.minseparation > 1.0)
      {
        checkSep = true;
        if ( MapBadMODListSep5_Improve.find(MOD_PNL_Name ) == MapBadMODListSep5_Improve.end() )
          MapBadMODListSep5_Improve[MOD_PNL_Name ] = 1;
        else
          MapBadMODListSep5_Improve[MOD_PNL_Name ] = MapBadMODListSep5_Improve[MOD_PNL_Name ] + 1;
        BadROCListSep5_Improve.push_back(branch23.rocName );
        ImproveSep5_SepOld.push_back(branch13.minseparation );
        ImproveSep5_SepNew.push_back(branch23.minseparation );
        ImproveSep5_DeltaSep.push_back(DeltaSep );
      }//if Delta > 5

      if ( DeltaSep < -5.0 && branch23.minseparation > 1.0 && branch13.minseparation > 1.0)
      {
        checkSep = true;
        improveSep = true;
        if ( MapBadMODListSep5_Decline.find(MOD_PNL_Name ) == MapBadMODListSep5_Decline.end() )
          MapBadMODListSep5_Decline[MOD_PNL_Name ] = 1;
        else
          MapBadMODListSep5_Decline[MOD_PNL_Name ] = MapBadMODListSep5_Decline[MOD_PNL_Name ] + 1;
        BadROCListSep5_Decline.push_back(branch23.rocName );
        DeclineSep5_SepOld.push_back(branch13.minseparation );
        DeclineSep5_SepNew.push_back(branch23.minseparation );
        DeclineSep5_DeltaSep.push_back(DeltaSep );
      }//if Delta > 5

      ///////////////////////////////////////////////////////////////////////////
      // Finding the consistently bad rocs 
      ///////////////////////////////////////////////////////////////////////////
      if (branch13.maxrms > 7 && branch23.maxrms > 7 && branch23.maxrms < 50.0 && branch13.maxrms < 50.0)
      {
        checkBadRMS = true;
        if ( MapConstBadRMS.find(MOD_PNL_Name ) == MapConstBadRMS.end() )
          MapConstBadRMS[MOD_PNL_Name ] = 1;
        else
          MapConstBadRMS[MOD_PNL_Name ] = MapConstBadRMS[MOD_PNL_Name ] + 1;
        BadRMSNames.push_back(branch13.rocName );
        BadRMS_Old.push_back(branch13.maxrms );
        BadRMS_New.push_back(branch23.maxrms );
	BadRMS_Delta.push_back(DeltaRMS );
      }//if maxRMS > 7
      else if (branch13.minseparation < 10 && branch23.minseparation < 10 && branch23.minseparation > 1.0 && branch13.minseparation > 1.0)
      {
        checkBadSep = true;
        if ( MapConstBadSep.find(MOD_PNL_Name ) == MapConstBadSep.end() )
          MapConstBadSep[MOD_PNL_Name ] = 1;
        else
          MapConstBadSep[MOD_PNL_Name ] = MapConstBadSep[MOD_PNL_Name ] + 1;
        BadSepNames.push_back(branch13.rocName );
        BadSep_Old.push_back(branch13.minseparation );
        BadSep_New.push_back(branch23.minseparation );
	BadSep_Delta.push_back(DeltaSep );
      }//if maxSep > 7

      //////////////////////////////////////////////////////////////////////////////////////////////////////
      // Finding Double Peaks and Wide Peaks
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      bool doublePeak1 = false, doublePeak2 = false, widePeak1 = false, widePeak2 = false;
      if (branch13.minseparation < 5.75 && branch13.maxrms > 10 && branch13.minseparation > 1.1) //The greater than 1 is for empty plots
      { 
        doublePeak1 = true;
        DoublePeakNames1.push_back(branch13.rocName );
        DoublePeakSep1.push_back(branch13.minseparation );
        DoublePeakRMS1.push_back(branch13.maxrms );
      }//if branch23      
      else if (branch23.minseparation < 5.75 && branch23.maxrms > 10 && branch23.minseparation > 1.1)
      { 
        doublePeak2 = true;
        DoublePeakNames2.push_back(branch23.rocName );
        DoublePeakSep2.push_back(branch23.minseparation );
        DoublePeakRMS2.push_back(branch23.maxrms );
      }//if branch23      
      if (branch13.minseparation > 5.75 && branch13.minseparation < 6.5) //The greater than 1 is for empty plots
      { 
        widePeak1 = true;
        WidePeakNames1.push_back(branch13.rocName );
        WidePeakSep1.push_back(branch13.minseparation );
        WidePeakRMS1.push_back(branch13.maxrms );
      }//if branch23      
      else if (branch23.minseparation > 5.75 && branch23.minseparation < 6.5)
      { 
        widePeak2 = true;
        WidePeakNames2.push_back(branch23.rocName );
        WidePeakSep2.push_back(branch23.minseparation );
        WidePeakRMS2.push_back(branch23.maxrms );
      }//if branch23      

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Making sure that the TBM plot for the particulare Fed Chan hasn't already been plotted in that TDirectory 
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if (checkRMS)
      {
	if (DeltaRMS < 0)
	  DeltaRMSgt2Dec->cd();
	else
          DeltaRMSgt2Imp->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f13 together
        //////////////////////////////////////////////////////////////////////////////////////////////////////////// 
        stringstream RMS_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch23.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          RMS_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i          
        RMS_Name_Path_TEMP << ROCName;
        string RMS_Name_Path = RMS_Name_Path_TEMP.str();
        CanvasNameTemp << "Failed RMS < 2.0: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_RMS13_ = (TH1F*)f13->Get(RMS_Name_Path.c_str() );
        TH1F *hROC_RMS23_ = (TH1F*)f23->Get(RMS_Name_Path.c_str() );
        if (improveRMS)
          TitleNameTemp << "Improved RMS: " << ROCName;
        else
          TitleNameTemp << "Declined RMS: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_RMS13_->SetTitle(TitleName.c_str() );
        hROC_RMS23_->SetTitle(TitleName.c_str() );
        hROC_RMS13_->SetLineColor(kRed);
        hROC_RMS23_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_RMS13_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_RMS23_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_RMS13_->DrawNormalized();
        hROC_RMS23_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkRMS

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Making sure that the TBM plot for the particulare Fed Chan hasn't already been plotted in that TDirectory 
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if (checkSep)
      {
	if (DeltaSep < 0)
	  DeltaSepgt5Imp->cd();
	else
          DeltaSepgt5Dec->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f13 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream Sep_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch23.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Sep_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Sep_Name_Path_TEMP << ROCName;
        string Sep_Name_Path = Sep_Name_Path_TEMP.str();
        CanvasNameTemp << "Failed Sep < 5.0: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_Sep13_ = (TH1F*)f13->Get(Sep_Name_Path.c_str() );
        TH1F *hROC_Sep23_ = (TH1F*)f23->Get(Sep_Name_Path.c_str() );
        if (improveSep)
          TitleNameTemp << "Improved Sep: " << ROCName;
        else
          TitleNameTemp << "Declined Sep: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_Sep13_->SetTitle(TitleName.c_str() );
        hROC_Sep23_->SetTitle(TitleName.c_str() );
        hROC_Sep13_->SetLineColor(kRed);
        hROC_Sep23_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_Sep13_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_Sep23_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_Sep13_->DrawNormalized();
        hROC_Sep23_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkSep 

      if (checkBadRMS)
      {
        BothRunsRMSgt7->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f13 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream RMS_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch23.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          RMS_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        RMS_Name_Path_TEMP << ROCName;
        string RMS_Name_Path = RMS_Name_Path_TEMP.str();
        CanvasNameTemp << "Both Runs RMS > 7: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_RMS13_ = (TH1F*)f13->Get(RMS_Name_Path.c_str() );
        TH1F *hROC_RMS23_ = (TH1F*)f23->Get(RMS_Name_Path.c_str() );
        TitleNameTemp << "Both Runs RMS > 7: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_RMS13_->SetTitle(TitleName.c_str() );
        hROC_RMS23_->SetTitle(TitleName.c_str() );
        hROC_RMS13_->SetLineColor(kRed);
        hROC_RMS23_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_RMS13_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_RMS23_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_RMS13_->DrawNormalized();
        hROC_RMS23_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkBadRMS 

      if (checkBadSep)
      {
        BothRunsSeplt10->cd();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f12 and f13 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream Sep_Name_Path_TEMP, Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
        string ROCName = branch23.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Sep_Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Sep_Name_Path_TEMP << ROCName;
        string Sep_Name_Path = Sep_Name_Path_TEMP.str();
        CanvasNameTemp << "Both Runs Sep < 10: " << ROCName;
        string CanvasName = CanvasNameTemp.str();

        TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
        TH1F *hROC_Sep13_ = (TH1F*)f13->Get(Sep_Name_Path.c_str() );
        TH1F *hROC_Sep23_ = (TH1F*)f23->Get(Sep_Name_Path.c_str() );
        TitleNameTemp << "Both Runs Sep < 10: " << ROCName;
        TitleName = TitleNameTemp.str();
        hROC_Sep13_->SetTitle(TitleName.c_str() );
        hROC_Sep23_->SetTitle(TitleName.c_str() );
        hROC_Sep13_->SetLineColor(kRed);
        hROC_Sep23_->SetLineColor(kBlue);
        Run1_TEMP << "Run_" << argv[1];
        Run2_TEMP << "Run_" << argv[2];
        string Run1 = Run1_TEMP.str();
        string Run2 = Run2_TEMP.str();
        TLegend *legROC = new TLegend(0.8, 0.67, 0.99, 0.75);
        legROC->AddEntry(hROC_Sep13_,Run1.c_str(),"l");
        legROC->AddEntry(hROC_Sep23_,Run2.c_str(),"l");

        ROCCanvas_.cd();
        hROC_Sep13_->DrawNormalized();
        hROC_Sep23_->DrawNormalized("SAME");
        legROC->Draw();
        ROCCanvas_.Write();
        ROCCanvas_.Clear();
      }//if checkBadSep 

      if (doublePeak1 || doublePeak2 || widePeak1 || widePeak2)
      {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Getting the Path+Name of the histogram of the ROC and plotting the f13 and f13 together for the ROCs that fail minSeparation
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        stringstream Name_Path_TEMP;
        string ROCName = branch23.rocName, TitleName;
        unsigned int found = 0;
        for (int i = 0; i < 6; i++)
        {
          found = ROCName.find('_',found+1);
          Name_Path_TEMP << ROCName.substr(0,found) << "/";
        }//for i           
        Name_Path_TEMP << ROCName;
        string Name_Path = Name_Path_TEMP.str();
        TH1F *hROC_Sep13_ = (TH1F*)f13->Get(Name_Path.c_str() );
        TH1F *hROC_Sep23_ = (TH1F*)f23->Get(Name_Path.c_str() );
        hROC_Sep13_->SetLineColor(kRed);
        hROC_Sep23_->SetLineColor(kBlue);

        if (doublePeak1)
        {
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          DoublePeaks1->cd();
          CanvasNameTemp << "DoublePeak Run #" << argv[1] << ": " << ROCName;
          TitleNameTemp << "DoublePeak Run #" << argv[1] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep13_->SetTitle(TitleName.c_str() ); 
          ROCCanvas_.cd();
          hROC_Sep13_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if doublePeak1
        if (doublePeak2)
        { 
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          DoublePeaks2->cd();
          CanvasNameTemp << "DoublePeak Run #" << argv[2] << ": " << ROCName;
          TitleNameTemp << "DoublePeak Run #" << argv[2] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep23_->SetTitle(TitleName.c_str() );
          ROCCanvas_.cd();
          hROC_Sep23_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if doublePeak1
        if (widePeak1)
        {
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          WidePeaks1->cd();
          CanvasNameTemp << "WidePeak Run #" << argv[1] << ": " << ROCName;
          TitleNameTemp << "WidePeak Run #" << argv[1] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep13_->SetTitle(TitleName.c_str() );
          ROCCanvas_.cd();
          hROC_Sep13_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if widePeak1
        if (widePeak2)
        { 
          stringstream Run1_TEMP, Run2_TEMP, CanvasNameTemp, TitleNameTemp;
          WidePeaks2->cd();
          CanvasNameTemp << "WidePeak Run #" << argv[2] << ": " << ROCName;
          TitleNameTemp << "WidePeak Run #" << argv[2] << ": " << ROCName;
          string CanvasName = CanvasNameTemp.str();
          TitleName = TitleNameTemp.str();
          TCanvas ROCCanvas_(CanvasName.c_str(), "", 600, 600);
          hROC_Sep23_->SetTitle(TitleName.c_str() );
          ROCCanvas_.cd();
          hROC_Sep23_->Draw();
          ROCCanvas_.Write();
          ROCCanvas_.Clear();
        }//if widePeak1
      }//if doublePeak
    }//if entry13 > 0 && entry23 > 0
  }//for nEntries13
std::cout << "<-------------Done Looping over AddressLevels_3.root--------------> " << std::endl;
//------------------------------------------------------------------------------------------
//Writing Histograms and Canvases
//------------------------------------------------------------------------------------------
  out_->cd();
  DeltaMaxRMSCanvas_.cd();
  DeltaMaxRMS_->Draw();
  DeltaMaxRMSCanvas_.Write();

  DeltaMinSepCanvas_.cd();
  DeltaMinSep_->Draw();
  DeltaMinSepCanvas_.Write();

  MaxRMSCanvas_.cd();
  MaxRMS_->Draw();
  MaxRMSCanvas_.Write();

  MinSepCanvas_.cd();
  MinSep_->Draw();
  MinSepCanvas_.Write();

  DeltaMinSepVsDeltaMaxRMSCanvas_.cd();
  DeltaMinSepVsDeltaMaxRMS_->Draw();
  //The lines draw the lowest and highest cuts on the Separation and RMS
  TLine *line11 = new TLine(-5,2,5,2);
  line11->SetLineColor(kRed);
  line11->Draw();
  TLine *line21 = new TLine(-5,-2,5,-2);
  line21->SetLineColor(kRed);
  line21->Draw();
  TLine *line31 = new TLine(5,-2,5,2);
  line31->SetLineColor(kRed);
  line31->Draw();
  TLine *line41 = new TLine(-5,-2,-5,2);
  line41->SetLineColor(kRed);
  line41->Draw();

  TLegend *legLimits = new TLegend(0.79, 0.6, 0.98, 0.65);
  legLimits->AddEntry(line11,"Bad ROC Limit","l");
  legLimits->Draw();

  DeltaMinSepVsDeltaMaxRMSCanvas_.Write();

//////////////////////////////////
// Legends Definition
//////////////////////////////////
  TLegend *legPassFail = new TLegend(0.75, 0.67, 0.99, 0.75);
  legPassFail->AddEntry(DeltaRMSFail2_,"2 < |DeltaRMS| < 2.5","l");
  legPassFail->AddEntry(DeltaRMSPass2_,"1.5 < |DeltaRMS| < 2","l");

//////////////////////////////////
// RMS Plotting and Writing
//////////////////////////////////
  topDir->cd();
  DeltaRMSFail2Canvas_.cd();
  DeltaRMSFail2_->SetLineColor(kRed);
  DeltaRMSPass2_->SetLineColor(kBlue);
  DeltaRMSPass2_->DrawNormalized();
  DeltaRMSFail2_->DrawNormalized("SAME");
  legPassFail->Draw();
  DeltaRMSFail2Canvas_.Write();
 
//////////////////////////////////
// Separation Plotting and Writing
//////////////////////////////////
  topDir->cd();
  DeltaSepFail5Canvas_.cd();
  DeltaSepFail5_->SetLineColor(kRed);
  DeltaSepPass5_->SetLineColor(kBlue);
  DeltaSepPass5_->Draw();
  DeltaSepFail5_->Draw("SAME");
  legPassFail->Draw();
  DeltaSepPass5_->GetXaxis()->SetTitle("#DeltaRMS");
  DeltaSepFail5Canvas_.Write();
  
  out_->Write();
  gDirectory->Delete("DeltaSepFail5;1");
  gDirectory->Delete("DeltaSepPass5;1");
  gDirectory->Delete("DeltaRMSPass2;1");
  gDirectory->Delete("DeltaRMSFail2;1");
  gDirectory->Delete("DeltaMaxRMS;1");
  gDirectory->Delete("DeltaMinSep;1");
  gDirectory->Delete("DeltaMinSepVsDeltaMaxRMS;1");
  gDirectory->Delete("MinSep;1");
  gDirectory->Delete("MaxRMS:1");
  out_->Close();

std::cout << "<-------------Done Writing Histograms--------------> " << std::endl;
////////////////////////////////////
// Writing out the Changing Modules
////////////////////////////////////
  BadMODFile_ << "#########################################################\n## Improving MODs or PNLs Failing |Delta RMS| > 2\n#########################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapBadMODListRMS2_Improve.begin(); iter != MapBadMODListRMS2_Improve.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << "\n";  
  }

  BadMODFile_ << "#########################################################\n## Declining MODs or PNLs Failing |Delta RMS| > 2\n#########################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapBadMODListRMS2_Decline.begin(); iter != MapBadMODListRMS2_Decline.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << "\n";
  }

  BadMODFile_ << "#########################################################\n## Improving MODs or PNLs Failing |Delta Sep| > 5\n#########################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapBadMODListSep5_Improve.begin(); iter != MapBadMODListSep5_Improve.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << "\n";
  }

  BadMODFile_ << "#########################################################\n## Declining MODs or PNLs Failing |Delta Sep| > 5\n#########################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapBadMODListSep5_Decline.begin(); iter != MapBadMODListSep5_Decline.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << "\n";
  }

//////////////////////////////////////////////
// Writing out the consistently Bad Modules
//////////////////////////////////////////////
  BadMODFile_ << "#########################################################\n## Consistently Bad ROC's with maxRMS > 7\n#########################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapConstBadRMS.begin(); iter != MapConstBadRMS.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << "\n";
  }

  BadMODFile_ << "#########################################################\n## Consistently Bad ROC's with minSeparation > 10\n#########################################################\n";
  for(std::map<string, unsigned int>::iterator iter = MapBadMODListSep5_Improve.begin(); iter != MapBadMODListSep5_Improve.end(); ++iter)
  {
    if(iter->second > 1)
      BadMODFile_ << iter->first << "\n";
  }


//////////////////////////////////////////////////////////////////////////////////////////////////////
//Writing out hte bad ROC's in a list with headers for the various cuts
//////////////////////////////////////////////////////////////////////////////////////////////////////
  BadROCFile_ << "#########################################################\n## Improving ROCs Failing |Delta RMS| > 2\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != BadROCListRMS2_Improve.size(); i++)
    BadROCFile_ << BadROCListRMS2_Improve[i] << "   \tOld RMS= " <<  ImproveRMS2_RMSOld[i] << "   \tNew RMS= " <<  ImproveRMS2_RMSNew[i] << "   \tDeltaRMS= " <<  ImproveRMS2_DeltaRMS[i] << "\n";

  BadROCFile_ << "#########################################################\n## Declining ROCs Failing |Delta RMS| > 2\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != BadROCListRMS2_Decline.size(); i++)
    BadROCFile_ << BadROCListRMS2_Decline[i] << "   \tOld RMS= " <<  DeclineRMS2_RMSOld[i] << "   \tNew RMS= " <<  DeclineRMS2_RMSNew[i] << "   \tDeltaRMS= " <<  DeclineRMS2_DeltaRMS[i] << "\n";

  BadROCFile_ << "#########################################################\n## Improving ROCs Failing |Delta Sep| > 5\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != BadROCListSep5_Improve.size(); i++)
    BadROCFile_ << BadROCListSep5_Improve[i] << "   \tOld Sep= " <<  ImproveSep5_SepOld[i] << "   \tNew Sep= " <<  ImproveSep5_SepNew[i] << "   \tDeltaSep= " <<  ImproveSep5_DeltaSep[i] << "\n";

  BadROCFile_ << "#########################################################\n## Declining ROCs Failing |Delta Sep| > 5\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != BadROCListSep5_Decline.size(); i++)
    BadROCFile_ << BadROCListSep5_Decline[i] << "   \tOld Sep= " <<  DeclineSep5_SepOld[i] << "   \tNew Sep= " <<  DeclineSep5_SepNew[i] << "   \tDeltaSep= " <<  DeclineSep5_DeltaSep[i] << "\n";

  BadROCFile_ << "#########################################################\n## Bad ROC's in Both Runs maxRMS > 7\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != BadRMSNames.size(); i++)
    BadROCFile_ << BadRMSNames[i] << "   \tOld RMS= " << BadRMS_Old[i] << "   \tNew RMS= " << BadRMS_New[i] << "   \tDeltaRMS= " << BadRMS_Delta[i] << "\n";
  
  BadROCFile_ << "#########################################################\n## Bad ROC's in Both Runs minSep < 10\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != BadSepNames.size(); i++) 
    BadROCFile_ << BadSepNames[i] << "   \tOld Sep= " << BadSep_Old[i] << "   \tNew Sep= " << BadSep_New[i] << "   \tDeltaSep= " << BadSep_Delta[i] << "\n";

  BadROCFile_ << "#########################################################\n## Double Peaks in Run #" << argv[1] << "\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != DoublePeakNames1.size(); i++)
    BadROCFile_ << DoublePeakNames1[i] << "   \tSep= " << DoublePeakSep1[i] << "   \tRMS= " << DoublePeakRMS1[i] << "\n";

  BadROCFile_ << "#########################################################\n## Wide Peaks in Run #" << argv[1] << "\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != WidePeakNames1.size(); i++)
    BadROCFile_ << WidePeakNames1[i] << "   \tSep= " << WidePeakSep1[i] << "   \tRMS= " << WidePeakRMS1[i] << "\n";

  BadROCFile_ << "#########################################################\n## Double Peaks in Run #" << argv[2] << "\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != DoublePeakNames2.size(); i++)
    BadROCFile_ << DoublePeakNames2[i] << "   \tSep= " << DoublePeakSep2[i] << "   \tRMS= " << DoublePeakRMS2[i] << "\n";

  BadROCFile_ << "#########################################################\n## Wide Peaks in Run #" << argv[2] << "\n#########################################################\n";
  for (std::vector<int>::size_type i = 0; i != WidePeakNames2.size(); i++)
    BadROCFile_ << WidePeakNames2[i] << "   \tSep= " << WidePeakSep2[i] << "   \tRMS= " << WidePeakRMS2[i] << "\n";

  BadROCFile_.close();
  BadMODFile_.close();

  return 0;

}//end main



////////////////////////////////////////////////////////////////////////////////
// This Gets the MOD or PNL name from ROC name
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





