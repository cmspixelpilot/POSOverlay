//
// This class is a first attempt at writing a configuration
// object that will perform a calibration loop.
//
//
//
#include "PixelCalib.h"
#include "PixelDACNames.h"
#include <fstream>
#include <iostream>
#include <sstream>
//#include <ios>
#include <assert.h>
#include <stdlib.h>


PixelCalib::PixelCalib(string calibname, string translationname):
  PixelCalibBase(), PixelConfigBase("","","") 
{

  if (translationname!="") {
    tr.open(translationname.c_str(),fstream::in);
    theFedAndChansMap = getAllFedAndChans();
    fedInfo = getPanelTypesAndFEDInfo();
    //cout << "fedInfo.size()=" << fedInfo.size()<<endl;
    tr.clear();
    tr.seekg(0, ios::beg);
  }

  ifstream in(calibname.c_str());

  if (!in.good()){
    cout << "Could not open:"<<calibname<<endl;
    assert(0);
  }
  else {
    cout << "Opened:"<<calibname<<endl;
  }

  string tmp;

  in >> tmp;

  if (tmp=="Mode:"){
    in >> mode_;
    cout << "PixelCalib mode="<<mode_
	      << endl;

    assert(mode_=="FEDBaselineWithTestDACs"||
	   mode_=="FEDAddressLevelWithTestDACs"||
	   mode_=="FEDBaselineWithPixels"||
	   mode_=="ROCUBEqualization"||
	   mode_=="FEDAddressLevelWithPixels"||
	   mode_=="GainCalibration"||
	   mode_=="PixelAlive"||
	   mode_=="SCurve"||
	   mode_=="Delay25"||
	   mode_=="ClockPhaseCalibration"||
	   mode_=="ThresholdCalDelay");
    in >>tmp;
  } else {
    mode_="FEDChannelOffsetPixel";
    cout << "PixelCalib mode not set"
	      << endl;
    assert(0);
  }

  singleROC_=false;
      
  if (tmp=="SingleROC") {
    singleROC_=true;
    in >> tmp;
  }

  assert(tmp=="Rows:");

  in >> tmp;

  vector <unsigned int> rows;
  while (tmp!="Cols:"){
    if (tmp=="|") {
      rows_.push_back(rows);
      rows.clear();
    }
    else{
      if (tmp!="*"){
	rows.push_back(atoi(tmp.c_str()));
      }
    }
    in >> tmp;
  }
  rows_.push_back(rows);
  rows.clear();
    
  in >> tmp;

  vector <unsigned int> cols;
  while ((tmp!="VcalLow:")&&(tmp!="VcalHigh:")&&
	 (tmp!="Vcal:")&&(tmp!="VcalHigh")&&(tmp!="VcalLow")){
    if (tmp=="|") {
      cols_.push_back(cols);
      cols.clear();
    }
    else{
      if (tmp!="*"){
	cols.push_back(atoi(tmp.c_str()));
      }
    }
    in >> tmp;
  }
  cols_.push_back(cols);
  cols.clear();

  if (tmp=="VcalLow") {
    highVCalRange_=false;
    in >> tmp;
  }

  if (tmp=="VcalHigh") {
    highVCalRange_=true;
    in >> tmp;
  }

  highVCalRange_=true;
  if (tmp=="VcalLow:") {
    highVCalRange_=false;
  }

  if ((tmp=="VcalLow:")||(tmp=="VcalHigh:")||(tmp=="Vcal:")){
    unsigned int  first,last,step;
    in >> first >> last >> step;
    unsigned int index=1;
    if (dacs_.size()>0) {
      index=dacs_.back().index()*dacs_.back().getNPoints();
    }
    PixelDACScanRange dacrange(pos::k_DACName_Vcal,first,last,step,index);
    dacs_.push_back(dacrange);
    in >> tmp;
  }
  else{

    //in >> tmp;
    while(tmp=="Scan:"){
      in >> tmp;
      unsigned int  first,last,step;
      in >> first >> last >> step;
      unsigned int index=1;
      if (dacs_.size()>0) {
	index=dacs_.back().index()*dacs_.back().getNPoints();
      }
      PixelDACScanRange dacrange(tmp,first,last,step,index);
      dacs_.push_back(dacrange);
      in >> tmp;
    }
      

    while (tmp=="Set:"){
      in >> tmp;
      unsigned int val;
      in >> val;
      unsigned int index=1;
      if (dacs_.size()>0) index=dacs_.back().index()*dacs_.back().getNPoints();
      PixelDACScanRange dacrange(tmp,val,val,1,index);
      dacs_.push_back(dacrange);
      in >> tmp;
    }
  }

  assert(tmp=="Repeat:");

  in >> ntrigger_;

  in >> tmp;

  if (in.eof()){
    roclistfromconfig_=false;
    in.close();
    return;
  }

  assert(tmp=="Rocs:");

  in >> tmp;

  if (tmp=="all"){
    cout << "testing all ROCS" << endl;
    if (!tr.is_open()) {
      cout << "transliation.dat file not open!!!!!!" << endl;
      return;
    }

    string nttmp = "";
    tr >> nttmp;
    while (!tr.eof()){
      if (nttmp.find("FPix_")!=string::npos){
	//cout << "reading: " << nttmp << endl;
	PixelROCName rocname(nttmp);
	PixelModuleName modulename(nttmp);
	rocs_.push_back(rocname);
	modules_.insert(modulename);
	countROC_[modulename]++;
      }
      tr >> nttmp;
    }
  } else {

    while (!in.eof()){
      PixelROCName rocname(tmp);
      PixelModuleName modulename(tmp);
      rocs_.push_back(rocname);
      modules_.insert(modulename);
      countROC_[modulename]++;
      in >> tmp;
    }
  }
    
  nROC_=1;
  
  //if (singleROC_){
    
    map<PixelModuleName,unsigned int>::iterator imodule=countROC_.begin();
    
    unsigned maxROCs=0;
    
    for (;imodule!=countROC_.end();++imodule){
      if (imodule->second>maxROCs) maxROCs=imodule->second;
    }
    
    nROC_=maxROCs;
    
    cout << "Max ROCs on a module="<<nROC_<<endl;
    
    //}
  
  in.close();

  for(unsigned int irocs=0;irocs<rocs_.size();irocs++){
    old_irows.push_back(-1);
    old_icols.push_back(-1);
  }

  if (translationname!="") {
    fedInfo = getPanelTypesAndFEDInfo();
    mapFEDChaToPanType = getMapFEDChaToPanType();
    //cout << "fedInfo.size()=" << fedInfo.size()<<endl;
  }
    
  return;

}

unsigned int PixelCalib::iScan(string dac) const{

  for (unsigned int i=0;i<dacs_.size();i++){
    if (dac==dacs_[i].name()) return i;
  }

  cout << "In PixelCalib::iScan could not find dac="
            << dac <<endl; 

  assert(0);

  return 0;

}

unsigned int PixelCalib::scanValue(unsigned int iscan,
				   unsigned int state) const{

  
  assert(state<nConfigurations());

  unsigned int i_scan=state%nScanPoints();

  for(unsigned int i=0;i<iscan;i++){
    i_scan/=nScanPoints(i);
  }

  unsigned int i_threshold=i_scan%nScanPoints(iscan);

  unsigned int threshold=dacs_[iscan].first()+
    i_threshold*dacs_[iscan].step();

  return threshold;

}


unsigned int PixelCalib::scanCounter(unsigned int iscan,
				     unsigned int state) const{


  assert(state<nConfigurations());

  unsigned int i_scan=state%nScanPoints();

  for(unsigned int i=0;i<iscan;i++){
    i_scan/=nScanPoints(i);
  }

  unsigned int i_threshold=i_scan%nScanPoints(iscan);

  return i_threshold;

}


vector<pair<vector<int>,string> > PixelCalib::getPanelTypes() 
{
  //cout << "PixelCalib::getPanelTypes" << endl;
  /** MR&GC
      Recognition of panel type:
      1. test how many times you read the same plaquette
      2. if we read 2  ROC on same plaquette ==> we are 4 ==> skip remaining 21-2-1  = 18 ROC (2 already read and 1 added by iter++)
      3. if we read 6  ROC on same plaquette ==> we are 3 ==> skip remaining 24-6-1  = 17 ROC (2 already read and 1 added by iter++)
      4. if next ROC is 0 it is L otherwise it is R
  */
  vector<PixelROCName>::iterator iter = rocs_.begin() ;
  int plaquette = 0 ;
  bool first = true ;
  int counter = 0 ;
  vector<pair<vector<int>,string> >  result ;
  for(;iter!=rocs_.end() ; iter++)
    {
      //cout << iter->panel() << " " << iter->plaquet() << " " << iter->roc() << endl ;
      if(first) {
	plaquette = iter->plaquet() ;
	first = false ;
	counter++ ;
	continue ;
      }
      if(plaquette == iter->plaquet()) {
	counter++ ;
      }
      else
	{
	  if(counter == 2) 
	    {
	      if (iter->roc()==0) {	       
		vector<int> tmp;
		tmp.push_back(iter->disk());
		tmp.push_back(iter->blade());
		tmp.push_back(iter->panel());
		result.push_back(make_pair<vector<int>,string>(tmp,"4L") ) ;
		//cout << "push disk=" <<  iter->disk() << " blade=" << iter->blade() << " panel=" << iter->panel() << " type=4L" << endl;
	      }
	      else {
		vector<int> tmp;
		tmp.push_back(iter->disk());
		tmp.push_back(iter->blade());
		tmp.push_back(iter->panel());
		result.push_back(make_pair<vector<int>,string>(tmp,"4R") ) ;
		//cout << "push disk=" <<  iter->disk() << " blade=" << iter->blade() << " panel=" << iter->panel() << " type=4R" << endl;
	      }
	      iter += 18 ;
	      //cout << "skipping 18 on 4 " << endl ;
	    }
	  else if(counter == 6) 
	    {
	      //cout << "iter->roc()=" << iter->roc() << endl;
	      if (iter->roc()==0) {
		vector<int> tmp;
		tmp.push_back(iter->disk());
		tmp.push_back(iter->blade());
		tmp.push_back(iter->panel());
		result.push_back(make_pair<vector<int>,string>(tmp,"3L") ) ;
		//cout << "push disk=" <<  iter->disk() << " blade=" << iter->blade() << " panel=" << iter->panel() << " type=3L" << endl;
	      }
	      else {
		vector<int> tmp;
		tmp.push_back(iter->disk());
		tmp.push_back(iter->blade());
		tmp.push_back(iter->panel());
		result.push_back(make_pair<vector<int>,string>(tmp,"3R") ) ;
		//cout << "push disk=" <<  iter->disk() << " blade=" << iter->blade() << " panel=" << iter->panel() << " type=3R" << endl;
	      }
	      iter += 17 ;
	      //cout << "skipping 17 on 3 " << endl ;
	    }
	  plaquette = iter->plaquet() ;
	  counter = 1 ;
	}
    }
  //   for(vector<string>::iterator ii = result.begin() ; ii != result.end() ; ii++)
  //     {
  //       cout << "Panels type: " << *ii << endl ;
  //     }
  return result ;
}


/*
  void PixelCalib::nextFECState(PixelFECConfigInterface* pixelFEC,
  PixelDetectorConfig* detconfig,
  PixelNameTranslation* trans,
  unsigned int state) const {

  if ((!roclistfromconfig_)&&rocs_.size()==0){

  //This code is not at all tested
  assert(0);
      
  int nmodule=detconfig->getNModules();
  for (int imodule=0;imodule<nmodule;imodule++){
  PixelModuleName module=detconfig->getModule(imodule);
  //This is ugly need to fix this somehow
  for (unsigned int iplq=1;iplq<5;iplq++){
  for (unsigned int iroc=1;iroc<5;iroc++){
  string name=module.modulename()+"_PLQ"+itoa(iplq)+"_ROC"+itoa(iroc);
  const PixelHdwAddress* hdwadd=0;
  PixelROCName rocname(name);
  hdwadd=trans->getHdwAddress(rocname);
  if (hdwadd!=0){
  rocs_.push_back(rocname);
  }
  }
  }
  }
     
  }

  assert(state<nConfigurations());

  unsigned int i_ROC=state/(cols_.size()*rows_.size()*
  nScanPoints());

  unsigned int i_row=(state-i_ROC*cols_.size()*rows_.size()*
  nScanPoints())/
  (cols_.size()*nScanPoints());

  unsigned int i_col=(state-i_ROC*cols_.size()*rows_.size()*
  nScanPoints()-
  i_row*cols_.size()*nScanPoints())/
  (nScanPoints());


  vector<unsigned int> dacvalues;

  unsigned int first_scan=true;

  for (unsigned int i=0;i<dacs_.size();i++){
  dacvalues.push_back(scanValue(i,state));
  if (scanCounter(i,state)!=0) first_scan=false;
  }

  assert(i_row<rows_.size());
  assert(i_col<cols_.size());

  if (first_scan){

  if (state!=0){

  unsigned int statetmp=state-1;
	
  unsigned int i_ROC=statetmp/(cols_.size()*rows_.size()*
  nScanPoints());

  unsigned int i_row=(statetmp-i_ROC*cols_.size()*rows_.size()*
  nScanPoints())/
  (cols_.size()*nScanPoints());

  unsigned int i_col=(statetmp-i_ROC*cols_.size()*rows_.size()*
  nScanPoints()-
  i_row*cols_.size()*nScanPoints())/
  (nScanPoints());


  assert(i_row<rows_.size());
  assert(i_col<cols_.size());

  for(unsigned int i=0;i<rocs_.size();i++){
  const PixelHdwAddress* hdwadd=trans->getHdwAddress(rocs_[i]);

  // cout << "Got Hdwadd" << endl;

  assert(hdwadd!=0);
  PixelHdwAddress theROC=*hdwadd;
          
  if (singleROC_&&theROC.fedrocnumber()!=i_ROC) continue;
          
  disablePixels(pixelFEC, i_row, i_col, theROC);

  }

  }
  }
    
  for(unsigned int i=0;i<rocs_.size();i++){

  //	cout << "Will configure roc:"<<rocs_[i] << endl;

  const PixelHdwAddress* hdwadd=trans->getHdwAddress(rocs_[i]);

  // cout << "Got Hdwadd" << endl;

  assert(hdwadd!=0);
  PixelHdwAddress theROC=*hdwadd;
        
  if (singleROC_&&theROC.fedrocnumber()!=i_ROC) continue;

  //	cout << "Will call progdac for vcal:"<< vcal << endl;

  for (unsigned int i=0;i<dacs_.size();i++){
  pixelFEC->progdac(theROC.mfec(),
  theROC.mfecchannel(),
  theROC.hubaddress(),
  theROC.portaddress(),
  theROC.rocid(),
  dacs_[i].dacchannel(),
  dacvalues[i]);
  //          cout << "Will set dac "<<dacchannel_[i]
  //          <<" to "<<dacvalues[i]<<endl;
  }

  //cout << "Will set Vcal="<<vcal_<<endl;
  //
  //pixelFEC->progdac(theROC.mfec(),
  //		  theROC.mfecchannel(),
  //		  theROC.hubaddress(),
  //		  theROC.portaddress(),
  //		  theROC.rocid(),
  //		  25,
  //		  vcal_);
  //

  //	cout << "Done with progdac" << endl;
  if (first_scan){

  //cout << "Will enable pixels!" <<endl;
  enablePixels(pixelFEC, i_row, i_col, theROC);
  //            cout << "Will do a clrcal on roc:"<<theROC.rocid()<<endl;

  //FIXME!!!!
  //TODO retrieve ROC control register from configuration
  //range is controlled here by one bit, but rest must match config
  //bit 0 on/off= 20/40 MHz speed; bit 1 on/off=disabled/enable; bit 3=Vcal range
  int range=0;  //MUST replace this line with desired control register setting
  if (highVCalRange_) range|=0x4;  //turn range bit on
  else range&=0x3;                 //turn range bit off

  pixelFEC->progdac(theROC.mfec(),
  theROC.mfecchannel(),
  theROC.hubaddress(),
  theROC.portaddress(),
  theROC.rocid(),
  0xfd,
  range);


  pixelFEC->clrcal(theROC.mfec(),
  theROC.mfecchannel(),
  theROC.hubaddress(),
  theROC.portaddress(),
  theROC.rocid());
  unsigned int nrow=rows_[i_row].size();
  unsigned int ncol=cols_[i_col].size();
  unsigned int nmax=max(nrow,ncol);
  if (nrow==0||ncol==0) nmax=0;
  for (unsigned int n=0;n<nmax;n++){
  unsigned int irow=n;
  unsigned int icol=n;
  if (irow>=nrow) irow=nrow-1;
  if (icol>=ncol) icol=ncol-1;
  unsigned int row=rows_[i_row][irow];
  unsigned int col=cols_[i_col][icol];
  cout << "Will do a calpix on roc, col, row:"
  <<theROC.rocid()<<" "<<col<<" "<<row<<endl;
		
  pixelFEC->calpix(theROC.mfec(),
  theROC.mfecchannel(),
  theROC.hubaddress(),
  theROC.portaddress(),
  theROC.rocid(),
  col,
  row,
  1);
  }
  }
  }
    
  return;

  } 
*/

// This code breaks if it is called more than once with different crate numbers!
/*
  vector<pair<unsigned int, vector<unsigned int> > >& PixelCalib::fedCardsAndChannels(unsigned int crate,
  PixelNameTranslation* translation,
  PixelFEDConfig* fedconfig) const{

  assert(rocs_.size()!=0);

  for(unsigned int i=0;i<rocs_.size();i++){
  const PixelHdwAddress* hdw=translation->getHdwAddress(rocs_[i]);
  assert(hdw!=0);
  //cout << "ROC, fednumber:"<<rocs_[i]<<" "<<hdw->fednumber()
  //	  << endl;
  //first check if fed associated with the roc is in the right crate
  if (fedconfig->crateFromFEDNumber(hdw->fednumber())!=crate) continue;
  //next look if we have already found fed number
  unsigned int index=fedCardsAndChannels_.size();
  for(unsigned int j=0;j<fedCardsAndChannels_.size();j++){
  if (fedCardsAndChannels_[j].first==hdw->fednumber()){
  index=j;
  break;
  }
  }
  //If we didn't find the fedcard we will create it
  if (index==fedCardsAndChannels_.size()){
  vector<unsigned int> tmp;
  tmp.push_back(hdw->fedchannel());
  pair<unsigned int, vector<unsigned int> > tmp2(hdw->fednumber(),tmp);
  fedCardsAndChannels_.push_back(tmp2);
  continue;
  }
  //Now look and see if the channel has been added
  vector<unsigned int>& channels=fedCardsAndChannels_[index].second;
  bool found=false;
  for(unsigned int k=0;k<channels.size();k++){
  if (channels[k]==hdw->fedchannel()) {
  found=true;
  break;
  }
  }
  if (found) continue;
  channels.push_back(hdw->fedchannel());

  }


  return fedCardsAndChannels_;

  }

*/

/*
  map <unsigned int, set<unsigned int> > PixelCalib::getFEDsAndChannels (PixelNameTranslation *translation) {

  map <unsigned int, set<unsigned int> > fedsChannels;
  assert(rocs_.size()!=0);
  vector<PixelROCName>::iterator iroc=rocs_.begin();

  for (;iroc!=rocs_.end();++iroc){
  const PixelHdwAddress *roc_hdwaddress=translation->getHdwAddress(*iroc);
  unsigned int fednumber=roc_hdwaddress->fednumber();
  unsigned int fedchannel=roc_hdwaddress->fedchannel();
  if (fedsChannels.find(fednumber)!=fedsChannels.end()){
  set<unsigned int> channels;
  channels.insert(fedchannel);
  fedsChannels.insert(make_pair(fednumber, channels));
  } else {
  fedsChannels[fednumber].insert(fedchannel);
  }
  }

  return fedsChannels;
  }

  set <unsigned int> PixelCalib::getFEDCrates(PixelNameTranslation* translation, PixelFEDConfig* fedconfig) const{

  set<unsigned int> fedcrates;
  assert(modules_.size()!=0);
  set<PixelModuleName>::iterator imodule=modules_.begin();

  for (;imodule!=modules_.end();++imodule)
  {
  const PixelHdwAddress *module_hdwaddress=translation->getHdwAddress(*imodule);
  unsigned int fednumber=module_hdwaddress->fednumber();
  fedcrates.insert(fedconfig->crateFromFEDNumber(fednumber));
  }

  return fedcrates;
  }

  set <unsigned int> PixelCalib::getFECCrates(PixelNameTranslation* translation, PixelFECConfig* fecconfig) const{

  set<unsigned int> feccrates;
  assert(modules_.size()!=0);
  set<PixelModuleName>::iterator imodule=modules_.begin();

  for (;imodule!=modules_.end();++imodule)
  {
  const PixelHdwAddress *module_hdwaddress=translation->getHdwAddress(*imodule);
  unsigned int fecnumber=module_hdwaddress->fecnumber();
  feccrates.insert(fecconfig->crateFromFECNumber(fecnumber));
  }

  return feccrates;
  }

*/

ostream& operator<<(ostream& s, const PixelCalib& calib){

  s<< "Rows:"<<endl;
  for (unsigned int i=0;i<calib.rows_.size();i++){
    for (unsigned int j=0;j<calib.rows_[i].size();j++){
      s<<calib.rows_[i][j]<<" "<<endl;
    }
    s<< "|"<<endl;
  }

  s<< "Cols:"<<endl;
  for (unsigned int i=0;i<calib.cols_.size();i++){
    for (unsigned int j=0;j<calib.cols_[i].size();j++){
      s<<calib.cols_[i][j]<<" "<<endl;
    }
    s<< "|"<<endl;
  }

  s << "Vcal:"<<endl;

  //s << calib.vcal_<<endl;

  s << "Vcthr:"<<endl;

  s << calib.dacs_[0].first() << " " << calib.dacs_[0].last() 
    << " "<< calib.dacs_[0].step()<<endl;

  s << "CalDel:"<<endl;

  s << calib.dacs_[1].first() << " " << calib.dacs_[0].last() 
    << " "<< calib.dacs_[1].step()<<endl;

  s << "Repeat:"<<endl;
    
  s << calib.ntrigger_<<endl;

  return s;

}

/*
  void PixelCalib::enablePixels(PixelFECConfigInterface* pixelFEC,
  unsigned int irows, unsigned int icols,
  PixelHdwAddress theROC) const{

  //cout << "irows, icols:"<<irows<<" "<<icols<<endl;

  for (unsigned int irow=0;irow<rows_[irows].size();irow++){
  for (unsigned int icol=0;icol<cols_[icols].size();icol++){
  //	    cout << "Will turn on pixel col="
  //      <<cols_[icols][icol]
  //    <<" row="<<rows_[irows][irow]<<endl;
  //
  pixelFEC->progpix(theROC.mfec(),
  theROC.mfecchannel(),
  theROC.hubaddress(),
  theROC.portaddress(),
  theROC.rocid(),
  cols_[icols][icol],
  rows_[irows][irow],
  0x80);
		
  }
  }


  //cout << "Done"<<endl;

  }

  void PixelCalib::disablePixels(PixelFECConfigInterface* pixelFEC,
  unsigned int irows, unsigned int icols,
  PixelHdwAddress theROC) const{


  for (unsigned int irow=0;irow<rows_[irows].size();irow++){
  for (unsigned int icol=0;icol<cols_[icols].size();icol++){
  //		cout << "Will turn off pixel col="
  //	  <<cols_[old_icols][icol]
  //  <<" row="<<rows_[old_irows][irow]<<endl;
  //
  pixelFEC->progpix(theROC.mfec(),
  theROC.mfecchannel(),
  theROC.hubaddress(),
  theROC.portaddress(),
  theROC.rocid(),
  cols_[icols][icol],
  rows_[irows][irow],
  0x0);
		
  }
  }
  }

*/

void PixelCalib::getRowsAndCols(unsigned int state,
				const vector<unsigned int> *&rows,
				const vector<unsigned int> *&cols) const{

  //cout << "PixelCalib::getRowsAndCols" << endl;

  assert(state<nConfigurations());
  
  unsigned int i_ROC=state/(cols_.size()*rows_.size()*
			    nScanPoints());

  unsigned int i_row=(state-i_ROC*cols_.size()*rows_.size()*
		      nScanPoints())/
    (cols_.size()*nScanPoints());

  unsigned int i_col=(state-i_ROC*cols_.size()*rows_.size()*
		      nScanPoints()-
		      i_row*cols_.size()*nScanPoints())/
    (nScanPoints());
  
  assert(i_row<rows_.size());
  assert(i_col<cols_.size());

  rows= &(rows_[i_row]);
  cols= &(cols_[i_col]);
}


//get fed and channel given disk, blade and panel numbers
pair<unsigned int, unsigned int> PixelCalib::getFedAndChan(unsigned int disk,
							   unsigned int blade, 
							   unsigned int panel) {
  stringstream dbp("");
  dbp << "D" << disk <<"_BLD"<<blade<<"_PNL"<<panel;
  //cout << dbp.str() << " " << theFedAndChansMap[dbp.str()].first << " " << theFedAndChansMap[dbp.str()].second << endl;
  return theFedAndChansMap[dbp.str()];

}

//get the map of panel name to fed and channel
map<string, pair<unsigned int, unsigned int> > PixelCalib::getAllFedAndChans(){
  //cout << "PixelCalib::getAllFedAndChans" << endl;

  map<string, pair<unsigned int, unsigned int> > result;
  unsigned int myfed = 0;
  unsigned int mycha = 0;
  string dbp ="";
  if (!tr.is_open()) cout << "translation.dat file not open!!!!!!" << endl;

  tr.clear();
  tr.seekg(0, ios::beg);

  stringstream calibroc("");
  calibroc << "FPix_BpO_";
  //cout << "calibroc= " << calibroc.str() <<endl;
  string tmpstr;
  while(!tr.eof()) {
    tr >> tmpstr;
    if (tmpstr.find(calibroc.str())!=string::npos){
      dbp = tmpstr;
      dbp.erase(0,(dbp.find("_D")+1));
      dbp.erase(dbp.find("_PLQ"),dbp.size());
      for (int u=0;u<6;u++){
	tr >> tmpstr;
      }
      tr >> myfed;
      tr >> mycha ;
      result[dbp]=make_pair<unsigned int, unsigned int>(myfed,mycha);
    }
  }
  return result;
}

//get disk blade panel numbers given the fed and the channel
vector<int> PixelCalib::getDiskBladePanel(unsigned int fed,unsigned int channel){
  //cout <<"PixelCalib::getDiskBladePanel"<<endl;
  vector<int> result;
  pair<unsigned int,unsigned int> tmp2 = make_pair<unsigned int,unsigned int>(fed-32,channel);
  pair<vector<int>,string> tmp = mapFEDChaToPanType[tmp2];
  result = tmp.first;
  return result;
}

//get the map of fed and channel to disk balde, panel numbers and panel type
map<pair<unsigned int,unsigned int>,pair<vector<int>,string> > PixelCalib::getMapFEDChaToPanType() {
  //cout <<"PixelCalib::getMapFEDChaToPanType()"<<endl;
  map<pair<unsigned int,unsigned int>,pair<vector<int>,string> > result;

  vector<pair<vector<int>,string> > panelTypes = getPanelTypes() ;

  for (unsigned int it=0;it<panelTypes.size();it++) {

    int disk  = panelTypes[it].first[0];
    int blade = panelTypes[it].first[1];
    int panel = panelTypes[it].first[2];

    pair<unsigned int, unsigned int> fedAndChan = getFedAndChan(disk,blade,panel);

    vector<int> resultvec;
    resultvec.push_back(disk);
    resultvec.push_back(blade);
    resultvec.push_back(panel);

    result[fedAndChan] = make_pair<vector<int>,string>(resultvec,panelTypes[it].second);   

    //cout << "f=" << fedAndChan.first << " c=" << fedAndChan.second << "d=" << disk << " b=" << blade << " p=" << panel << endl;    
  }
  //cout <<"end PixelCalib::getPanelTypesAndFEDInfo"<<endl;
  return result;

}

//get the list of panel types associated to the disk, blade, panel numbers and the fed and the channel (according to translation.dat)
vector<pair<vector<int>,string> > PixelCalib::getPanelTypesAndFEDInfo() {

  //cout <<"PixelCalib::getPanelTypesAndFEDInfo"<<endl;
  vector<pair<vector<int>,string> > result;

  vector<pair<vector<int>,string> > panelTypes = getPanelTypes() ;

  for (unsigned int it=0;it<panelTypes.size();it++) {

    int disk  = panelTypes[it].first[0];
    int blade = panelTypes[it].first[1];
    int panel = panelTypes[it].first[2];

    //cout << "d=" << disk << " b=" << blade << " p=" << panel << endl;

    pair<unsigned int, unsigned int> fedAndChan = getFedAndChan(disk,blade,panel);

    unsigned int myfed = fedAndChan.first;
    unsigned int mycha = fedAndChan.second;

    //cout << "f=" << myfed << " c=" << mycha << endl;
    if (find(theFeds.begin(),theFeds.end(),myfed)==theFeds.end()) theFeds.push_back(myfed);

    vector<int> resultvec;
    resultvec.push_back(disk);
    resultvec.push_back(blade);
    resultvec.push_back(panel);
    resultvec.push_back(myfed);
    resultvec.push_back(mycha);

    result.push_back(make_pair<vector<int>,string>(resultvec,panelTypes[it].second));   
  }
  //cout <<"end PixelCalib::getPanelTypesAndFEDInfo"<<endl;
  return result;
}

//this method looks into the translation.dat file for the plaquette and roc number (in the plaquette) 
//given the disk, blade, panel and roc number (in the panel)
pair<unsigned int, unsigned int> PixelCalib::getPlaquetteAndRoc(unsigned int disk,unsigned int blade,
								unsigned int panel,unsigned int roc) {
  //cout <<"PixelCalib::getPlaquetteAndRoc"<<endl;

  if (!tr.is_open()) cout << "translation.dat file not open!!!!!!" << endl;

  tr.clear();
  tr.seekg(0, ios::beg);

  stringstream calibroc("");
  calibroc << "FPix_BpO_D"<<disk<<"_BLD"<<blade<<"_PNL"<<panel;
  //cout << "calibroc= " << calibroc.str() <<endl;
  string name ="";
  string name2 ="";
  string tmpstr ="";
  while(!tr.eof()) {
    tr >> name;
    if (name.find(calibroc.str())!=string::npos){
      for (int u=0;u<8;u++){
	tr >> tmpstr;
      }
      tr >> tmpstr;
      if (atoi(tmpstr.c_str())==(int) roc) break;
    }
  }
  name2=name;
  name.replace(0,name.size()-6,"");
  name.replace(name.find("_"),5,"");
  name2.replace(0,name2.size()-1,"");
  return make_pair<unsigned int,unsigned int>(atoi(name.c_str()),atoi(name2.c_str()));

}

//use PixelCalib::getPlaquetteAndRoc to fill a map with all plaquette and roc numbers
map<unsigned int,map<unsigned int,map<unsigned int,map<unsigned int,pair<unsigned int, unsigned int> > > > > PixelCalib::getPlaquetteAndRocMap(){
  //cout <<"PixelCalib::getPlaquetteAndRocMap"<<endl;
  map<unsigned int,map<unsigned int,map<unsigned int,map<unsigned int,pair<unsigned int, unsigned int> > > > > result;
  for (vector<pair<vector<int>,string> >::iterator it=fedInfo.begin();it!=fedInfo.end();++it){
    int disk  = it->first[0];
    int blade = it->first[1];
    int panel = it->first[2];
    for (unsigned int roc=0;roc<nROC_;roc++){
      if (roc>20&&(it->second=="4L"||it->second=="4R")) break;
      pair<unsigned int, unsigned int> tmp = getPlaquetteAndRoc(disk,blade,panel,roc);
      //cout <<"d=" << disk <<" b="<< blade <<" p="<< panel <<" r="<< roc <<" plq="<< tmp.first <<" roc="<< tmp.second << endl;
      result[disk][blade][panel][roc] = tmp;
    }  
  }
  return result;
}
