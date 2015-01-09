// Silvia Taroni
// 9th January 2008
//
// This program read the data of the baseline correction from a log file
// As input needs file path:
//
//     > ./baseTempCorr /home/someone/log.XXXXXXXX
//
// The program needs a file, named temp.txt,
// containing the number of the temperature to read from
// the database, and the ID of the temperature.
// The format of the file must be the following:
//  
//  //--number of temperature to plot
//  2
//  //--temperature IDs
//  32
//  53
//
// The name of the input file (log.XXXXXXXX) is also used for 
// the output file.
// The name of the output file will be: XXXXXXXX.root
// It can be open with ROOT.
//
// Some summary graphs are plot running the program.
// The canvas are saved in the output file.
//
// The program works with ROOT_v5.17.06 and
// require Oracle to connect to the database
//
// OUTPUT FILE
// -----------
//
// The output file contain the following graphs:
//
// Graph_Fed#_Channel#
// contains the baseline correction as a function of time
//
// OPT#_HD#_#
// contains the baseline correction of different channel, grouped
// according to the HALF DISK (HD#) they come from, according to
// the optoreceiver(OPT#) and according to the Fed(_#) they are read.
// CANVAS1 contains all this graphs with the legends.
//
// OPTOR_FED#
// the average value of the correction to the baseline of all 
// the channel read by each of the two optoreceiver of a same FED (FED#)
// are plotted.
// CANVAS3 contains all this graphs with the legends.
//
// FED#_Disk#
// the average value of all the channel read by the same FED (FED#)
// coming from the two disk (Disk#) (black and read points) are plotted.
// CANVAS2 contains all this graphs with the legends.
//
// Temperature_ID_#
// contains the data of the temperature of a specific RTD (ID_#) as a
// function of time.
// CANVAS4 contains all this graphs with the legends.
//
// RTD_Graph_Fed#_Channel#
// correlations plots of the Baseline correction and the temperature.
// In canvas 5 some channls are reported as example. These are one channel
// for each optoreceiver and each disk. Data from disk 2 are plotted in red.
//======================================================================

#include <fstream>
#include <iostream>

#include <map>
#include <sstream>
#include <string>
#include "DBReader.h"

#include <TSQLResult.h>
#include <TSQLRow.h>


#include <time.h>
#include <qregexp.h>
#include <qtextcodec.h>

#include <TApplication.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TDatime.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1F.h>
#include <TH1.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TROOT.h>
#include <TStyle.h>


using namespace std;

double timeOffset;
double timeStop;


map <int   , vector<pair <string,string> > > RTDmap;
map <string, vector<pair <string,double> > > BaselineMap;
map <int,int> tempId;

int  readDB(string,string) ;
int  readFile(string,string &,string &) ;
void matchTempBaseline(void) ;
  stringstream fOut; 

int main(int argc, char**argv)
{
  char str[1024];

  string fileName;
  string sDate0;
  string sDate1;
  string sTimeAnal;

  ifstream infile;
 
  if(argc > 1){
    fileName = argv[1];
    ifstream inputfile(fileName.c_str(),ifstream::in);
    if (inputfile.good()){
    }else {
      cout << "BAD file name. Does the file exist?"<< endl;
      exit(1);
    }  }else {
    cout << "To Run: ./baseTempCorr  filename"<< endl;
    cout << "Enter file path and name: ";
    cin.get (str,1024);
    fileName=str;
    ifstream inputfile(fileName.c_str(),ifstream::in);
    if (inputfile.good()){
    }else {
      cout << "BAD file name. Does the file exist?"<< endl;
      exit(1);
    }
  }

  QRegExp rfile("log.(\\d+\\w+\\d+_\\d+)");
  QString qFile=fileName;
  if( rfile.search(qFile)>=0 ) {
    string outFile=rfile.cap(1);
    fOut << outFile<<".root";
    cout << "Output File " << fOut.str()<< endl;
  }
  TFile fOutput(fOut.str().c_str(),"RECREATE");
  TApplication app("App",&argc, argv);

  cout << "reading data from log file "<< endl;
  readFile(fileName,sDate0,sDate1);
  cout <<"reading database"<< endl;
  if( readDB(sDate0,sDate1) <0 ) {
    cout << "No connection to database"<< endl;
    //    return -1 ;
  }
  matchTempBaseline() ;
 
  fOutput.Close();
  cout << "Finis Africae...." << endl ;

  app.Run();

  return 0;
  
}

//=============================================================
int readDB(string sDate0, string sDate1)
{
  double time;
  double timebegin=0;
  double timeend=0;
  string mthn = "main()";
  
  int inTempId;
  int nnTemp;
  int thePad=0;
  string allDateBegin;
  string allDateEnd;
  
  int maxChars=256;
  char lineList[maxChars];
  ifstream inList;

  map <int,int> nLines;
  map <int, TGraph *> tempGraphs;

  stringstream allDateBefBeg;
  stringstream allDateAftEnd;
  stringstream sTemp;
  stringstream tempTitle;
  stringstream tempName;

  inList.open("temp.txt",ifstream::in);
  inList.seekg(ios::beg) ;
  inList.getline(lineList,maxChars);
  inList.getline(lineList,maxChars);
  nnTemp=atoi(lineList);
  cout<< "NUMBER OF TEMPERATURES TO READ: "<< nnTemp << endl;
  inList.getline(lineList,maxChars);
  QRegExp rTemp("(\\d+)\\s+//\\s+\\w+");
  for (int iTemp=0;iTemp<nnTemp; iTemp++){
    inList.getline(lineList,maxChars);
    QString qinList=lineList;
    if (rTemp.search(lineList)>=0){
      int inTempId=rTemp.cap(1).toInt();
      //    inTempId=atoi(lineList);
      tempId[iTemp]=inTempId;
      cout << iTemp << " Temperature ID: "<< tempId[iTemp] << endl;
    }
  }
  allDateBegin=sDate0;
  allDateEnd=sDate1;
  inList.close();
  inList.clear() ;
  
  for (int iTemp=0; iTemp<nnTemp;iTemp++){
     inTempId=tempId[iTemp];
     sTemp.str("");
     sTemp<<"TemperatureID_"<<tempId[iTemp];
     cout << sTemp.str()<<endl;
     tempGraphs[iTemp] = new TGraph();
     nLines[iTemp] = 1;
   }
  
  int  nTemp=(int)round(nnTemp/2);
  TCanvas * canvas4 = new TCanvas("canvas4", "c4",  0,  0, 800, 800 ) ;
  canvas4->Divide(nTemp,2); 

  for (int iTemp=0;iTemp<nnTemp;iTemp++){
    if (thePad>nnTemp){
      thePad=0;
    }
    thePad++;
    inTempId=tempId[iTemp];
    DBReader * dbR = new DBReader();

    QRegExp rStringDate("(\\w\\w\\w)\\s+(\\d+)\\s+(\\d+):(\\d+):(\\d+)\\s+(\\d+)");
    QString qTime1=allDateBegin;
    if (rStringDate.search(qTime1)>=0){
      double hour=rStringDate.cap(3).toDouble();
      double min =rStringDate.cap(4).toDouble();
      double sec =rStringDate.cap(5).toDouble();

      string month=rStringDate.cap(1);
      int    day  =rStringDate.cap(2).toInt();
      int    year =rStringDate.cap(6).toInt();

      timebegin=hour*3600+min*60+sec;

      hour=hour-1;
      if (hour<0){
	hour=hour+1;
	min=0;
      }
      allDateBefBeg.str("");
      allDateBefBeg<<month<<" "<<day<<" "<<hour<<":"<<min<<":"<<sec<<" "<<year;
    }

    QString qTime2=allDateEnd;
    if (rStringDate.search(qTime2)>=0){
      double hour=rStringDate.cap(3).toDouble();
      double min =rStringDate.cap(4).toDouble();
      double sec =rStringDate.cap(5).toDouble();

      string month=rStringDate.cap(1);
      int    day  =rStringDate.cap(2).toInt();
      int    year =rStringDate.cap(6).toInt();
      
      timeend=hour*3600+min*60+sec;

      hour=hour+1;
      if (hour>=24){
	hour=hour-1;
	min=59;
      }
      allDateAftEnd.str("");
      allDateAftEnd<<month<<" "<<day<<" "<<hour<<":"<<min<<":"<<sec<<" "<<year;
    }


    //======= Searching point before the first datum
    dbR->makeSelection(inTempId,allDateBefBeg.str(),allDateAftEnd.str());
    if( dbR->connectToDB() == 0 ) {
      return -1;
    }
     if( dbR->queryDB()     == 0 ) {return -2;}
    string mthn = "[DBReader::getSelectionResult()]\t" ;
    TSQLResult * selectionResult = dbR->getSelectionResult();
    int nRows = selectionResult->GetRowCount();
    cout << mthn << "Found " << nRows << " elements" << endl ;
    cout << nRows<< endl;
    pair<string, string> timeTemp ;
    int controlEnd=0;
     if (nRows > 0){
      TSQLRow* row = NULL;
       while ((row = selectionResult->Next()) ){
	QString qTime=row->GetField(0);
	if (rStringDate.search(qTime)>=0){
	  double hour=rStringDate.cap(3).toDouble();
	  double min =rStringDate.cap(4).toDouble();
	  double sec =rStringDate.cap(5).toDouble();
	  time=hour*3600+min*60+sec;
	  timeTemp.first   =  row->GetField(0);
	  timeTemp.second  =  row->GetField(1);
	  RTDmap[iTemp].push_back(timeTemp) ;
	  double temperature=strtod(row->GetField(1),NULL);
	  
	  if (time>=timebegin && time<=timeend){
	    tempGraphs[iTemp]->SetPoint(nLines[iTemp],time,temperature);
	    nLines[iTemp]++;
	  }else if(time<timebegin){
	      tempGraphs[iTemp]->SetPoint(0,time,temperature);
	  }else if(time>timeend && controlEnd==0){
	    tempGraphs[iTemp]->SetPoint(nLines[iTemp],time,temperature);
	    controlEnd=1;
	  }
	  
	  if (tempGraphs[iTemp]!=NULL ){
	    if( tempGraphs[iTemp]->GetXaxis() != NULL ){
	      tempGraphs[iTemp]->GetXaxis()->SetLabelSize(0.03);
	      tempGraphs[iTemp]->GetXaxis()->SetTimeDisplay(1);
	      tempGraphs[iTemp]->GetXaxis()->SetTimeFormat("%H:%M:%S");
	      tempGraphs[iTemp]->SetMarkerStyle(20);
	      tempGraphs[iTemp]->SetMarkerSize(0.3);
	    }
	  }
	  canvas4->cd(thePad);
	  tempGraphs[iTemp]->Draw("AP");
	  tempTitle.str("");
	  tempName.str("");
	  tempTitle<<"Temperature ID "<<tempId[iTemp];
	  tempName<<"Temperature_ID_"<<tempId[iTemp];
	}
      }
    } else {
      cout << "There are no entries for your selection!!!" << endl;
    }
  
    tempGraphs[iTemp]->SetTitle(tempTitle.str().c_str());
    tempGraphs[iTemp]->SetName(tempName.str().c_str());
    tempGraphs[iTemp]->Write();
    delete dbR;
  }
  canvas4->Modified();
  canvas4->Update();
  canvas4->Write();
  return 0;
}

//=============================================================
int readFile(string fileName, string &sDate0, string &sDate1)
{
  double  Channel   = 0;
  double  Fed       = 0;
  double  meanD     = 0;
  double  newTime2     ;
  double  timeEnd      ;
  double  x            ;
  double  y            ;
  double  yhigh        ;
  double  yhighest     ;
  double  ylow         ;
  double  ylowest      ;
  double  ymean        ; 
  double  ynew         ;
  double  yold         ;
  double  yvalue       ;

  ifstream infile;

  int anHour     =0;
  int anMin      =0;
  int anSec      =0;
  int chanOffset   ;
  int day        =0;
  int hour       =0;
  int min        =0;
  int nchan        ;
  int nPoint     =0;
  int sec        =0;
  int thePad       ;
  int thePad1      ;
  int year       =0;

  map <string, TMultiGraph *> aohGraph;
  map <string, TLegend *>     aohLegend;
  map <string, TMultiGraph *> fedgraphs;
  map <string, TMultiGraph *> fedOpto;
  map <string, TH1F   *>      histograms;
  map <string, TGraph *>      graphs;
  map <string, int>           counter;
  map <string, double>        mYLow;
  map <string, double>        mYHigh;
  map <string, int>           nChOpto;
  map <string, double>        normy;
  map <string, TLegend *>     optoLegend;
  map <string, TGraph *>      optoRs;
  map <string, TGraph *>      subfedgraphs;
  map <string, TLegend *>     sLegend;
  map <int, TGraph *>         temperature;
  map <int   , TLegend *>     tempLegend;
  map <string,double>         xOpto;
  map <string,double>         yOptoRlow;
  map <string,double>         yOptoR1;
  map <string,double>         yOptoRhigh;
  map <string,double>         yOptoRMean;

  pair<string, double> timeBasel ;

  string sHour;
  string line;
  string sMonth;
  string sDate;
  
  stringstream channelIndex;
  stringstream graphIndex;
  stringstream pointOpto;
  stringstream sOptoRs;
  stringstream ssAoh;
  stringstream ssFedOpto;

  
  cout << "FileName: " << fileName << endl ;

  TCanvas * canvas1 = new TCanvas("canvas1", "c1",  0,  0, 800, 800 ) ;
  canvas1->Divide(2,4);
  TCanvas * canvas2 = new TCanvas("canvas2", "c2",200,200, 800, 400 ) ;
  canvas2->Divide(2,2);
  TCanvas * canvas3 = new TCanvas("canvas3", "c3",800,400, 400, 400 ) ;
  canvas3->Divide(1,2);
 
  for( int fed=0; fed<2; fed++)
    {
      for( int chan=1; chan<=24; chan++)
	{
	  channelIndex.str("") ;
	  channelIndex << "Fed" << fed + 11 << "_" << "Channel" << chan + 12 - fed*(12) ;
	  graphIndex.str("") ;
	  graphIndex << "Graph_Fed" << fed + 11 << "_" << "Channel" << chan + 12 - fed*(12) ;
	  graphs[graphIndex.str()] = new TGraph();
	  counter[channelIndex.str()]=-1;
	  mYLow[graphIndex.str()]=0;
	  mYHigh[graphIndex.str()]=0;
	}
    }

  infile.open(fileName.c_str(),ifstream::in);
  
  QRegExp rline("\\s+:\\s*Baseline\\s+Correction\\s+for\\s+FED");
  QRegExp rStringDate("(\\w\\w\\w)\\s+(\\d+)\\s+(\\d+):(\\d+):(\\d+)\\s+(\\d+)");
  QRegExp rMean("\\s*Mean=(.*\\d+.\\d+)");

  cout << "------------------> file: " << infile.good() << endl ;

  int bin = 0;
  while (infile.good()){
    getline (infile,line);	
    QString linea = line ;
    if( rline.search(linea)>=0 ) {
      if( rStringDate.search(linea)>=0 ) {
	bin++;
	string	stringDate=rStringDate.cap(0);
	sDate=stringDate;
	string 	month=rStringDate.cap(1);
	day= rStringDate.cap(2).toInt();
	hour =rStringDate.cap(3).toInt();
	min  =rStringDate.cap(4).toInt();
	sec  =rStringDate.cap(5).toInt();
	year =rStringDate.cap(6).toInt();
	
	if (rMean.search(linea)>=0){
	  meanD=rMean.cap(1).toDouble();
	}else {
	  cout << "ERROR baseline correction value not found!" << endl;
	}
	if (bin == 1 ){
	  anHour=hour;
	  anMin=min;
	  anSec=sec;
	  sDate0=sDate;
	  cout << "Starting point "<< anHour << " " << anMin << " " << anSec << endl;
	}
	timeOffset = 0;
	timeOffset = anHour*3600+anMin*60+anSec;
	newTime2   = hour*3600+min*60+sec;
	timeStop   = newTime2;

	if (newTime2>timeOffset){
	  QRegExp rChannel("Channel\\s+(\\d+)");
	  QRegExp rFED("FED\\s+0x(\\d+)000000");
	  
	  if (rFED.search(linea)>=0){
	    Fed=rFED.cap(1).toDouble();
	  } else{
	    cout << "ERROR FED # NOT FOUND!"<< endl;
	  }
	  
	  if (rChannel.search(linea)>=0){
	    Channel=rChannel.cap(1).toDouble();
	  }else{
	    cout << "ERROR CHANNEL # NOT FOUND!"<<endl;
	  }

	  channelIndex.str("") ;
	  channelIndex << "Fed" << Fed << "_" << "Channel" << Channel ;
	  counter[channelIndex.str()]++;
	  graphIndex.str("") ;
	  graphIndex << "Graph_Fed" << Fed << "_" << "Channel" << Channel ;
	  
	  if (counter[channelIndex.str()]==0){
	    normy[channelIndex.str()]=meanD;
	  }
	  
	  yvalue=meanD-normy[channelIndex.str()];
	  if (yvalue<mYLow[graphIndex.str()]){
	    mYLow[graphIndex.str()]=yvalue;
	  }
	  if (yvalue>mYHigh[graphIndex.str()]){
	    mYHigh[graphIndex.str()]=yvalue;
	  }

	  timeBasel.first   = sDate;
	  timeBasel.second  = yvalue;
	  BaselineMap[graphIndex.str()].push_back(timeBasel) ;

	  
	  graphs[graphIndex.str()]->SetPoint(counter[channelIndex.str()],newTime2,yvalue);
	  if (graphs[graphIndex.str()]!=NULL ){
	    if( graphs[graphIndex.str()]->GetYaxis() != NULL ){
	      graphs[graphIndex.str()]->GetYaxis()->SetRangeUser(mYLow[graphIndex.str()]-0.5,mYHigh[graphIndex.str()]+0.5);
	    }
	    if( graphs[graphIndex.str()]->GetXaxis() != NULL ){
	      graphs[graphIndex.str()]->GetXaxis()->SetLabelSize(0.03);
	      graphs[graphIndex.str()]->GetXaxis()->SetTimeDisplay(1);
	      graphs[graphIndex.str()]->GetXaxis()->SetTimeFormat("%H:%M:%S");
	      graphs[graphIndex.str()]->SetMarkerStyle(20);
	      graphs[graphIndex.str()]->SetMarkerSize(0.2);
	    }
	  }
	}
      }
    }
  }

  for(map<string, TGraph*>::iterator it=graphs.begin(); it!=graphs.end(); it++){
    stringstream title;
    title.str("");
    title<<it->first ;
    it->second->SetTitle(title.str().c_str());
    it->second->SetName(title.str().c_str());
    it->second->SetDrawOption("AP");
    it->second->GetXaxis()->SetLabelSize(0.03);
    it->second->GetXaxis()->SetTimeDisplay(1);
    it->second->GetXaxis()->SetTimeFormat("%H:%M:%S");
    it->second->SetMarkerStyle(20);
    it->second->SetMarkerSize(0.2);
    it->second->Write();
  }
  timeEnd=newTime2;
  sDate1=sDate;
  cout << sDate0 << " "<< sDate1<< endl;
  infile.close(); 

  //----------------------------------------
  cout << "Starting point "<< anHour << " " << anMin << " " << anSec << endl;

  /// channel divided in 8 groups:   ///////////////////////////////////////////////
  ///   FED 11
  ///         form 13 to 18 -- HD 1
  ///         form 25 to 30 -- HD 1
  ///         form 19 to 24 -- HD 2
  ///         form 25 to 36 -- HD 2
  ///   FED 12
  ///         form  1 to 6 -- HD 1
  ///         form 13 to 18 -- HD 1
  ///         form  7 to 12 -- HD 2
  ///         form 19 to 24 -- HD 2

  for (int disk=0; disk<2;disk++){
    for (int fed=0; fed<2; fed++){
      for (int subfed=0;subfed<2;subfed++){
	ssAoh.str("");
	ssAoh<<"OPT"<<subfed+1<<"_HD"<<disk+1<<"_"<<fed+11;
	aohGraph[ssAoh.str()]=new TMultiGraph();
	aohGraph[ssAoh.str()]->SetName(ssAoh.str().c_str());
	aohLegend[ssAoh.str()]=new TLegend(0.6,0.15,0.89,0.5);
      }
    }
  }

  thePad1=0;
  for (int disk=0; disk<2;disk++){
    for (int fed=0; fed<2; fed++){
      for (int subfed=0;subfed<2;subfed++){
	double yhigh=0;
	double ylow=0;
	thePad1++;
	ssAoh.str("");
	ssAoh<<"OPT"<<subfed+1<<"_HD"<<disk+1<<"_"<<fed+11;
	int color=0;
	color=0;
	chanOffset=0;
	chanOffset=12-12*fed+6*disk+12*subfed;
	for( int chan=1+chanOffset; chan<=6+chanOffset; chan++){
	  double y=0;
	  double x=0;
	  graphIndex.str("") ;
	  graphIndex << "Graph_Fed" << fed + 11 << "_" << "Channel" << chan ;
	  color++;
	  stringstream ssAohLegend;
	  ssAohLegend << "FED"<< fed+11<<", Disk "<< disk+1<<", Channel "<<chan;
	  if( graphs[graphIndex.str()] != NULL ) {
	    nPoint=graphs[graphIndex.str()]->GetN();
	    if (graphs[graphIndex.str()]->GetN() != 0 ){
	      for (int ipoint=0;ipoint< nPoint-1;ipoint ++){
		graphs[graphIndex.str()]->GetPoint(ipoint,x,y);
		if (y < ylow){
		  ylow=y;
		}
		if (y>yhigh){
		  yhigh=y;
		}
	      }
	      graphs[graphIndex.str()]->GetXaxis()->SetTimeDisplay(1);
	      graphs[graphIndex.str()]->GetXaxis()->SetLabelSize(0.03);
	      graphs[graphIndex.str()]->GetXaxis()->SetTimeFormat("%H:%M:%S");
	      aohLegend[ssAoh.str()]->AddEntry(graphs[graphIndex.str()], ssAohLegend.str().c_str(),"LP");
	      graphs[graphIndex.str()]->SetMarkerColor(color);
	      graphs[graphIndex.str()]->SetLineColor(color);
	      aohGraph[ssAoh.str()]->Add(graphs[graphIndex.str()],"p1");
	    }
	  }
	}
	stringstream aohTitle;
	aohTitle.str("");
	aohTitle<<"FED "<< fed+11 << ", Channel "<< chanOffset+1 << "-" << chanOffset+7;
	aohGraph[ssAoh.str()]->SetTitle(aohTitle.str().c_str());
	canvas1->cd(thePad1);
	aohGraph[ssAoh.str()]->Draw("ap");
	if ( aohGraph[ssAoh.str()]!=NULL ){
	  if(aohGraph[ssAoh.str()]->GetYaxis() != NULL ){
	    aohGraph[ssAoh.str()]->GetYaxis()->SetRangeUser(ylow-0.5,yhigh+0.5);
	  }
	}
	if ( aohGraph[ssAoh.str()]!=NULL ){
	  if(aohGraph[ssAoh.str()]->GetXaxis() != NULL ){
	    aohGraph[ssAoh.str()]->GetXaxis()->SetTimeDisplay(1);
	    aohGraph[ssAoh.str()]->GetXaxis()->SetTimeFormat("%H:%M:%S");
	    aohGraph[ssAoh.str()]->GetXaxis()->SetLabelSize(0.03);
	  } 
	}

	aohGraph[ssAoh.str()]->Write();
	aohLegend[ssAoh.str()]->Draw();
      }
    }
  }  

  cout << "average"<< endl;

//---average value on each subfed-------------------------------------

  thePad=0;

  for (int fed=0; fed<2; fed++){
    ssFedOpto.str("");
    ssFedOpto<< "OPTOR_FED" << fed+11;
    fedOpto[ssFedOpto.str()]= new TMultiGraph();
    fedOpto[ssFedOpto.str()]->SetName(ssFedOpto.str().c_str());
    fedOpto[ssFedOpto.str()]->SetTitle(ssFedOpto.str().c_str());

    optoLegend[ssFedOpto.str()]=new TLegend(0.6,0.13,0.89,0.3);
    for (int subFed=0; subFed<2;subFed++){
      sOptoRs.str("");
      sOptoRs << "FED"<< 11+fed<<"_OPTOR"<<subFed+1;
      optoRs[sOptoRs.str()]= new TGraph();
      optoRs[sOptoRs.str()]->SetName(sOptoRs.str().c_str());
      optoRs[sOptoRs.str()]->SetTitle(sOptoRs.str().c_str());
    }
    for (int disk=0; disk<2;disk++){
      stringstream shisto;
      shisto << "FED"<< 11+fed << "_Disk"<<1+disk ;
      fedgraphs[shisto.str()] = new TMultiGraph();
      fedgraphs[shisto.str()]->SetName(shisto.str().c_str());
      fedgraphs[shisto.str()]->SetTitle(shisto.str().c_str());
      sLegend[shisto.str()]=new TLegend(0.5,0.15,0.89,0.4);
      int color=0;
      yhigh=0;
      ylow=0;
      for (int subfed=0; subfed<2; subfed++){
	color++;
	stringstream ssubhisto;
	ssubhisto<<"HC"<<disk+1<<"_Fed"<<fed<<"_SubF"<<subfed;
	subfedgraphs[ssubhisto.str()] = new TGraph();
	int ichanoffset=12-fed*12+subfed*12+disk*6;
	for (int ipoint=0;ipoint<nPoint;ipoint++){
	  pointOpto.str("");
	  pointOpto << "FED"<<fed+11<<"_OPTOR"<<subfed+1<<"_point"<<ipoint;
	  if (disk == 0) {
	    yOptoR1[pointOpto.str()]=0;
	    nChOpto[pointOpto.str()]=0;
	    yOptoRhigh[ssFedOpto.str()]=0;
	    yOptoRlow[ssFedOpto.str()]=0;
	    yOptoRMean[pointOpto.str()]=0;
	  }
	  yold=0;
	  ynew=0;
	  ymean=0;
	  ylowest=999;
	  yhighest=-999;
	  nchan=0;
	  for( int chan=1+ichanoffset; chan<7+ichanoffset; chan++){
	    graphIndex.str("");
	    graphIndex << "Graph_Fed" << fed + 11 << "_" << "Channel" << chan  ;
	    if( graphs[graphIndex.str()] != NULL ) {
	      if ( graphs[graphIndex.str()] -> GetN()!=0){
		nchan++;
		nChOpto[pointOpto.str()]++;
		graphs[graphIndex.str()]->GetPoint(ipoint,x,y);
		xOpto[pointOpto.str()]=x;
		ynew=yold+y;
		yold=ynew;
		yOptoR1[pointOpto.str()]=yOptoR1[pointOpto.str()]+y;
		if (y<ylowest){
		  ylowest=y;
		}
		if (y>yhighest){
		  yhighest=y;
		}
	      }
	    }
	  }
	  ymean=ynew/nchan;
	  if (nchan !=1){
	    if (ymean < ylowest || ymean > yhighest){
	      cout << nchan <<" mean error! " << graphIndex.str()
		   << "lowest value "<< ylowest<< " higher value "
		   << yhighest<< " mean "<< ymean << endl; 
	    }
	  }
	  if (yhigh<=ymean){
	    yhigh=ymean;
	  }
	  if (ylow>=ymean){
	    ylow=ymean;
	  }
	  yOptoRMean[pointOpto.str()]=yOptoR1[pointOpto.str()]/nChOpto[pointOpto.str()];
	  if (  yOptoRMean[pointOpto.str()]> yOptoRhigh[ssFedOpto.str()]){
	    yOptoRhigh[ssFedOpto.str()]=yOptoRMean[pointOpto.str()];
	  }
	  if (  yOptoRMean[pointOpto.str()]< yOptoRlow[ssFedOpto.str()]){
	    yOptoRlow[ssFedOpto.str()]=yOptoRMean[pointOpto.str()];
	  }
	    
	  subfedgraphs[ssubhisto.str()]->SetPoint(ipoint,x,ymean);
	  subfedgraphs[ssubhisto.str()]->GetXaxis()->SetLabelSize(0.03);
	  subfedgraphs[ssubhisto.str()]->GetXaxis()->SetTimeDisplay(1);
	  subfedgraphs[ssubhisto.str()]->GetXaxis()->SetTimeFormat("%H:%M:%S");
	  subfedgraphs[ssubhisto.str()]->GetXaxis()->SetLabelSize(0.03);
	}
	subfedgraphs[ssubhisto.str()]->SetName(ssubhisto.str().c_str());
	subfedgraphs[ssubhisto.str()]->Write();
	subfedgraphs[ssubhisto.str()]->SetMarkerColor(color);
	subfedgraphs[ssubhisto.str()]->SetLineColor(color);
	if (subfedgraphs[ssubhisto.str()] != NULL ){
	  if (subfedgraphs[ssubhisto.str()]->GetN()!=0){
	    fedgraphs[shisto.str()]->Add(subfedgraphs[ssubhisto.str()],"lp1");
	    stringstream sstringLegend;
	    sstringLegend << "FED"<< fed+11<<", Disk "<< disk+1<<", Channel "<<ichanoffset+1<<"-"<<ichanoffset+6;
	    sLegend[shisto.str()]->AddEntry(subfedgraphs[ssubhisto.str()],sstringLegend.str().c_str(),"LP");
	  }
	}
      }
      thePad++;
      canvas2->cd(thePad);
      fedgraphs[shisto.str()]->Write();
      fedgraphs[shisto.str()]->Draw("ap");
      sLegend[shisto.str()]->Draw(); 
      if (fedgraphs[shisto.str()]!=NULL ){
	if( fedgraphs[shisto.str()]->GetYaxis() != NULL ){
	  fedgraphs[shisto.str()]->GetYaxis()->SetRangeUser(ylow-0.5,yhigh+0.5);
	}
      }
      
      
      
      if (fedgraphs[shisto.str()]!=NULL ){
	if( fedgraphs[shisto.str()]->GetXaxis() != NULL ){
	  fedgraphs[shisto.str()]->GetXaxis()->SetLabelSize(0.03);
	  fedgraphs[shisto.str()]->GetXaxis()->SetTimeDisplay(1);
	  fedgraphs[shisto.str()]->GetXaxis()->SetTimeFormat("%H:%M:%S");
	  fedgraphs[shisto.str()]->GetXaxis()->SetLabelSize(0.03);
	}
      }
    }
    int color=0;
    for (int subFed=0; subFed<2;subFed++){
      color++;
      for (int ipoint=0; ipoint<nPoint;ipoint++){
	pointOpto.str("");
	pointOpto << "FED"<<fed+11<<"_OPTOR"<<subFed+1<<"_point"<<ipoint;
	sOptoRs.str("");
	sOptoRs << "FED"<< 11+fed<<"_OPTOR"<<subFed+1;
	optoRs[sOptoRs.str()]->SetPoint(ipoint,xOpto[pointOpto.str()], yOptoRMean[pointOpto.str()]);
      }
      if ( optoRs[sOptoRs.str()] !=NULL){
	if (optoRs[sOptoRs.str()]->GetXaxis() != NULL){
	  optoRs[sOptoRs.str()]->GetXaxis()->SetLabelSize(0.03);
	  optoRs[sOptoRs.str()]->GetXaxis()->SetTimeDisplay(1);
	  optoRs[sOptoRs.str()]->GetXaxis()->SetTimeFormat("%H:%M:%S");
	  optoRs[sOptoRs.str()]->GetXaxis()->SetLabelSize(0.03);
	}
      }
      optoRs[sOptoRs.str()]->Write();
      optoRs[sOptoRs.str()]->SetMarkerColor(color);
      optoRs[sOptoRs.str()]->SetLineColor(color);
      fedOpto[ssFedOpto.str()]->Add(optoRs[sOptoRs.str()],"lp1");
      optoLegend[ssFedOpto.str()]->AddEntry(optoRs[sOptoRs.str()],sOptoRs.str().c_str(),"LP");
      
    }

    canvas3->cd(fed+1);
    fedOpto[ssFedOpto.str()]->Draw("ap");
    if (  fedOpto[ssFedOpto.str()] !=NULL ){
      if (  fedOpto[ssFedOpto.str()]->GetXaxis() !=NULL ){
	fedOpto[ssFedOpto.str()]->GetXaxis()->SetLabelSize(0.03);
	fedOpto[ssFedOpto.str()]->GetXaxis()->SetTimeDisplay(1);
	fedOpto[ssFedOpto.str()]->GetXaxis()->SetTimeFormat("%H:%M:%S");
	fedOpto[ssFedOpto.str()]->GetXaxis()->SetLabelSize(0.03);
      }
      if (  fedOpto[ssFedOpto.str()]->GetYaxis() !=NULL ){
	 fedOpto[ssFedOpto.str()]->GetYaxis()->SetRangeUser( yOptoRlow[ssFedOpto.str()]-0.5,yOptoRhigh[ssFedOpto.str()]+0.5);
      }
    }
    optoLegend[ssFedOpto.str()]->Draw();
    fedOpto[ssFedOpto.str()]->Write();
  }

  canvas1->Modified() ;
  canvas1->Update();
  canvas1->Write();
  canvas2->Modified();
  canvas2->Update();
  canvas2->Write();
  canvas3->Modified();
  canvas3->Update();
  canvas3->Write();

 
  return 0;
  
}
//=============================================================
void matchTempBaseline(void)
{
  
  map <int,TGraph *> tempGraphs;
  map <int, int > nPoint;
  map <string , map <int , int> > mapPoint;
  map <int, map <string, TGraph*> > baseTemp;
  double m          =-999;
  double newTemp    =   0;
  double q          =-999;
  double tBefore    =   0;
  double temp       =   0;
  double tempBefore =   0;
  double time       =   0;
  double timeTemp   =   0;
  int nTemp     =0;
  stringstream namehisto;

  for (map<int,vector<pair<string,string> > >::iterator it=RTDmap.begin(); it!=RTDmap.end(); it++){
    tempGraphs[it->first] = new TGraph();
    nTemp++;
  }
  //   int nnTemp=(int)round(nTemp/2);
  TCanvas * canvas5 = new TCanvas("canvas5", "c5",  200,  100, 1200, 600 ) ;
  canvas5->Divide(2*nTemp,4); 
  cout << endl << endl << endl ;
  cout << "----- Baseline Temperature Correlation -----" <<endl;
  QRegExp rStringDate("(\\w\\w\\w)\\s+(\\d+)\\s+(\\d+):(\\d+):(\\d+)\\s+(\\d+)");
  cout << "TimeOffset "<< timeOffset << " TimeStop "<< timeStop << endl;
  for (map<int,vector<pair<string,string> > >::iterator it=RTDmap.begin(); it!=RTDmap.end(); it++){
    nPoint[it->first]=0;
  }
  for (map<int,vector<pair<string,string> > >::iterator it=RTDmap.begin(); it!=RTDmap.end(); it++){
    map <string, TGraph *> dummymap;
    baseTemp[it->first]=dummymap;
    for (map <string,vector<pair<string,double> > >::iterator itCh=BaselineMap.begin();itCh!=BaselineMap.end();itCh++){
      TGraph * dummy =new TGraph();
      ( baseTemp[it->first])[itCh->first]=dummy;
    }
  }
  cout << " Temperature analysis "<<endl;
  for (map<int,vector<pair<string,string> > >::iterator it=RTDmap.begin(); it!=RTDmap.end(); it++){
    cout << endl<<endl;
    cout << " Temperature: "<< tempId[it->first]; 
    tBefore=0;
    for (vector<pair<string,string> >::iterator jt=it->second.begin(); jt!=it->second.end(); jt++){
      QString qTime1=jt->first;
      if (rStringDate.search(qTime1)>=0){
      double hour=rStringDate.cap(3).toDouble();
      double min =rStringDate.cap(4).toDouble();
      double sec =rStringDate.cap(5).toDouble();
      string month=rStringDate.cap(1);
//       int    day  =rStringDate.cap(2).toInt();
//       int    year =rStringDate.cap(6).toInt();
      timeTemp=hour*3600+min*60+sec;
      temp=strtod(jt->second.c_str(),NULL);
      m=(temp-tempBefore)/(timeTemp-tBefore);
      q=temp-m*timeTemp;
      for (map <string,vector<pair<string,double> > >::iterator itCh=BaselineMap.begin();itCh!=BaselineMap.end();itCh++){
	for (vector <pair<string,double> >::iterator jtCh=itCh->second.begin();jtCh!=itCh->second.end();jtCh++){
	  QString qTime2=jtCh->first;
	  if (rStringDate.search(qTime2)>=0){
	    double hour=rStringDate.cap(3).toDouble();
	    double min =rStringDate.cap(4).toDouble();
	    double sec =rStringDate.cap(5).toDouble();
	    string month=rStringDate.cap(1);
// 	    int    day  =rStringDate.cap(2).toInt();
// 	    int    year =rStringDate.cap(6).toInt();
	    time=hour*3600+min*60+sec;
	    if (time<=timeTemp && time>=tBefore && time!=0){
	      newTemp=m*time+q;
	      TGraph * grafico = (baseTemp[it->first])[itCh->first];
	      grafico->SetPoint(grafico->GetN(),jtCh->second,newTemp);
	    }
	  }
	}
      }
      tempBefore=temp;
      tBefore=timeTemp;
      }
    }
    cout << endl;
  }
  cout<< "------ CANVAS 5 -------"<<endl;
  int thePad=0;
  QRegExp rStringGraph("Graph_Fed(\\d+)_Channel(\\d+)");
  for (map<int,map <string, TGraph*> >::iterator it=baseTemp.begin(); it!=baseTemp.end(); it++){
    for(map <string, TGraph*>::iterator itCh=it->second.begin(); itCh!=it->second.end(); itCh++){
      if ((baseTemp[it->first])[itCh->first]!=NULL){
	namehisto.str("");
	  namehisto<<"RTD"<<tempId[it->first]<<"_"<<itCh->first;
	  (baseTemp[it->first])[itCh->first]->SetTitle(namehisto.str().c_str());
	  (baseTemp[it->first])[itCh->first]->SetName(namehisto.str().c_str());
      }
      (baseTemp[it->first])[itCh->first]->SetMarkerStyle(20);
      (baseTemp[it->first])[itCh->first]->SetMarkerSize(0.3);
      if ((baseTemp[it->first])[itCh->first]!=NULL){
	if  ((baseTemp[it->first])[itCh->first]->GetXaxis()!=NULL){
	  (baseTemp[it->first])[itCh->first]->GetXaxis()->SetTitle("Baseline correction");
	  (baseTemp[it->first])[itCh->first]->GetXaxis()->SetLabelSize(0.03);
	}
	if  ((baseTemp[it->first])[itCh->first]->GetYaxis()!=NULL){
	  (baseTemp[it->first])[itCh->first]->GetYaxis()->SetTitle("Celsius degrees");
	  (baseTemp[it->first])[itCh->first]->GetYaxis()->SetLabelSize(0.03);
	}
      }
      QString nameChannel=itCh->first;
      if (rStringGraph.search(nameChannel)>=0){
	int Fed =rStringGraph.cap(1).toInt();
	int Chan=rStringGraph.cap(2).toInt();
	if ((Chan+12*(Fed-12))==1 ||(Chan+12*(Fed-12))==7 || (Chan+12*(Fed-12))==13 ||  (Chan+12*(Fed-12))==19){
	  thePad++;
	  if (thePad>8*nTemp){
	    thePad=1;
	  }
	  canvas5->cd(thePad);
	  if ((Chan+12*(Fed-12))==7 || (Chan+12*(Fed-12))==19){
	    (baseTemp[it->first])[itCh->first]->SetMarkerColor(2);
	  }else {
	    (baseTemp[it->first])[itCh->first]->SetMarkerColor(1);
	  }
	  (baseTemp[it->first])[itCh->first]->Draw("ALP");	    
	  cout << "FED = "<< Fed << "; Chan = "<< Chan<< endl;
	  canvas5->Modified();
	  canvas5->Update();
	}
      }
    //      (baseTemp[it->first])[itCh->first]->Draw("AP");
      (baseTemp[it->first])[itCh->first]->Write();
    }
  }

  canvas5->Write();
 

}
