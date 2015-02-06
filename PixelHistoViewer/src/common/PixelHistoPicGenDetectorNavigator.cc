#include "PixelHistoPicGen.h"
#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;

////////////////////////////////////////////////////////////////////////
void PixelHistoPicGen::PrepareDetectorNavigatorHtml(){
	string mthn = "[PicGen::PrepareDetectorNavigatorHtml()]\t";
	
	char rFile[] = "html_files/_DetectorNavigator.html";
	char wFile[] = "html_files/DetectorNavigator.html";	
	
	//Keyword:  		<!-- c++ generated code here -->
	string keyWord = "<!-- c++ generated code here -->";
	char	 findWord[keyWord.length()+1];
	
		//input java stream
	ifstream infile(rFile);
	if(!infile.is_open())
	  {cout << mthn << "IN File not found." << endl; return;}
	ofstream outfile(wFile);
	if(!outfile.is_open())
	  {cout << mthn << "OUT File not found." << endl; return;}

	infile.seekg(0,ifstream::end);
	unsigned long size = infile.tellg();
	infile.seekg(0);
	
	char c;
	
	cout << mthn << ""<<endl;
	unsigned long i;
	for(unsigned long p=0;p<size;++p){ //locate each insert and handle
	  infile.get(c);
	  if(c == '<'){ //found one
		
			hasCarrot:
			for(i=0;i<keyWord.length() && i<size;++i,++p){
				outfile.put(c);
				findWord[i] = c;
				infile.get(c);
				if(c == '<'){
					goto hasCarrot;
				}
			}
			outfile.put(c);
			findWord[i] = '\0';

			if(findWord == keyWord){
				//output code
				outfile << endl;
				
				for(int i=0;i<24;++i) //draw FED:CHANNEL Divs (Clock-wise from 12)
					outfile << "<div class='fedCaptions' id='fedCaption" << i << "'" \
									<< "onmodown='event.button==2?DetectorNavigator.zoom(false,this):DetectorNavigator.expandFedPath(this.id.slice(10));' " \
									<< "onmouseover='document.body.style.cursor=\"pointer\";' " \
									<< "onmouseout='document.body.style.cursor=\"auto\";' " \
									<< ">FD:CH</div>" << endl;
								
				for(int i=0;i<6;++i){ // draw ROC Highlights 0-5
				  outfile << "<img id='rocHighlight" << i << "' src='/pixel/PixelHistoViewer/images/generated/rocHighlight" << i \
					  			<< ".png' style='position:absolute;z-index:2;top:-1000px' " \
									<< "onmousedown='event.button==2?DetectorNavigator.zoom(false,this):DetectorNavigator.clickRoc();' " \
									<< "onmouseover='document.body.style.cursor=\"pointer\";' " \
									<< "onmouseout='document.body.style.cursor=\"auto\";' " \
									<< "/>" << endl;
				}
					//draw barrel ROC Highlight
				outfile << "<img id='rocHighlight' src='/pixel/PixelHistoViewer/images/generated/rocHighlight.png' " \
					  << "style='position:absolute;z-index:2;top:-1000px' " \
									<< "onmousedown='event.button==2?DetectorNavigator.zoom(false,this):DetectorNavigator.clickRoc();' " \
									<< "onmouseover='document.body.style.cursor=\"pointer\";' " \
									<< "onmouseout='document.body.style.cursor=\"auto\";' " \
									<< "/>" << endl;
				
					//use table to force window to be as wide as desired display			
				outfile << "<table border='0' cellpadding='0' cellspacing='0' width='" \
								<< WEB_LARGE_WIDTH + WEB_XOFFSET << "px' height='" << WEB_LARGE_HEIGHT << "px' >" \
								<< "<td border='0' valign='middle' height='100%' width='" \
								<< WEB_LARGE_WIDTH + WEB_XOFFSET << "px' height='" << WEB_LARGE_HEIGHT << "px' >"; 
								
				for(int i=0;i<8;++i){ //discs
					outfile << "<img id='disc" << i << "' src='##" << i << "##'" \
									<< (i==0?"onLoad='DetectorNavigator.initObjects()' ":" ") \
									<< "onmousedown='event.button==2?DetectorNavigator.zoom(false,this):DetectorNavigator.zoom(true,this);' " \
									<< "style='position:relative;z-index:0;left:" << WEB_XOFFSET << "px;" \
						    	<< "top:" << WEB_YOFFSET << "px;width:" << WEB_MINI_WIDTH << "px;" \
						    	<< "height:" << WEB_MINI_HEIGHT << "px' />";
					outfile << "<div align='center' class='captions' id='discCaption" << i << "'" \
									<< "onmousedown='DetectorNavigator.zoom(true,this);' " \
									<< "onmouseover='document.body.style.cursor=\"pointer\";' " \
									<< "onmouseout='document.body.style.cursor=\"auto\";' " \
									<< ">FPix<br/>B" << (i<4?'m':'p') << "_D" << (i<4?2-i/2:i/2-1) << "_PNL" << i%2 + 1 << "</div>";
					outfile << "<img class='goodBad' id='discFlag" << i << "'" \
									<< "onmousedown='DetectorNavigator.zoom(true,this);' " \
									<< "onmouseover='document.body.style.cursor=\"pointer\";' " \
									<< "onmouseout='document.body.style.cursor=\"auto\";' " \
									<< "src='##" << 20+i << "##' />";
				}

				for(int i=0;i<6;++i){ //barrels
					outfile << "<img id='barrel" << i << "' src='##" << 8+i << "##'" \
									<< "onmousedown='event.button==2?DetectorNavigator.zoom(false,this):DetectorNavigator.zoom(true,this);' " \
									<< "style='position:relative;z-index:0;left:" << WEB_XOFFSET << "px;" \
									<< "top:" << WEB_YOFFSET << "px;width:" << WEB_BMINI_WIDTH << "px;" \
									<< "height:" << WEB_BMINI_HEIGHT << "px' />";
					outfile << "<div align='center' class='captions' id='barrelCaption" << i << "'" \
									<< "onmousedown='DetectorNavigator.zoom(true,this);' " \
									<< "onmouseover='document.body.style.cursor=\"pointer\";' " \
									<< "onmouseout='document.body.style.cursor=\"auto\";' " \
									<< ">BPix<br/>B" << (i%2==0?"m":"p") << "_LYR" << i/2+1 << "</div>";
					outfile << "<img class='goodBad' id='barrelFlag" << i << "'" \
									<< "onmousedown='DetectorNavigator.zoom(true,this);' " \
									<< "onmouseover='document.body.style.cursor=\"pointer\";' " \
									<< "onmouseout='document.body.style.cursor=\"auto\";' " \
									<< "src='##" << 28+i << "##' />";
				}
				
				outfile << "</td></table>";
	
			}	    
		}
		else{
      outfile.put(c);
	  }			
	}
	
	infile.close();
	outfile.close();
}

////////////////////////////////////////////////////////////////////////
void PixelHistoPicGen::PrepareDetectorNavigatorJava(){
	string mthn = "[PicGen::PrepareDetectorNavigatorJava()]\t";
	
	char rFile[] = "js_files/_DetectorNavigator.js";
	char wFile[] = "js_files/DetectorNavigator.js";	
	
	//============ Keyword:  		##constant-name##

		//input java stream
	ifstream infile(rFile);
	if(!infile.is_open())
	  {cout << mthn << "IN File not found." << endl; return;}
	ofstream outfile(wFile);
	if(!outfile.is_open())
	  {cout << mthn << "OUT File not found." << endl; return;}

	infile.seekg(0,ifstream::end);
	unsigned long size = infile.tellg();
	infile.seekg(0);
	
	char c;
	unsigned long maxSize = 100;
	char constantName[maxSize];
	
	int dist;
	
	unsigned long i;
	for(unsigned long p=0;p<size;++p){ //locate each insert and handle
	  infile.get(c);
		
	  if(c == '#'){ //found one
			dist = -1;
			++p;
 			infile.get(c);
 			
 			if(c == '#'){ //found two
 				
				++p;
				infile.get(c);
				
				i=0;
 				while(c != '#' && i<maxSize && p<size){
					
 					constantName[i] = c;
					++p; ++i;
 					infile.get(c);
 				}				
				constantName[i] = '\0';
				
				if(i == maxSize){
					cout << mthn << "Improper C++ Constant Formation!!! --> " << constantName << endl;
					infile.close();
					outfile.close();
					return;
				}
				++p;
 				infile.get(c); //read out last '#' 				
				
					//------------ have constantName
				string constantStr(constantName);
				if		 (constantStr == "WEB_LARGE_WIDTH")
					outfile <<															  WEB_LARGE_WIDTH;
				else if(constantStr == "WEB_LARGE_HEIGHT")
					outfile <<  															WEB_LARGE_HEIGHT;
				else if(constantStr == "WEB_MINI_WIDTH")
					outfile <<  															WEB_MINI_WIDTH;
				else if(constantStr == "WEB_MINI_HEIGHT")
					outfile <<  															WEB_MINI_HEIGHT;
				else if(constantStr == "WEB_BLARGE_WIDTH")
					outfile <<															  WEB_BLARGE_WIDTH;
				else if(constantStr == "WEB_BLARGE_HEIGHT")
					outfile <<  															WEB_BLARGE_HEIGHT;
				else if(constantStr == "WEB_BMINI_WIDTH")
					outfile <<  															WEB_BMINI_WIDTH;
				else if(constantStr == "WEB_BMINI_HEIGHT")
					outfile <<  															WEB_BMINI_HEIGHT;
				else if(constantStr == "WEB_XOFFSET")
					outfile <<  															WEB_XOFFSET;
				else if(constantStr == "WEB_YOFFSET")
					outfile <<  															WEB_YOFFSET;
				else if(constantStr == "WEB_ROC_SIZE")
					outfile <<  															WEB_ROC_SIZE;
				else if(constantStr == "WEB_DISC_X")
					outfile <<  															WEB_DISC_X;
				else if(constantStr == "WEB_DISC_Y")
					outfile <<  															WEB_DISC_Y;
				else if(constantStr == "WEB_BLADE_HI"){
					int sqDist = WEB_DISC_RADIUS + 4;
					sqDist *= sqDist;	 
					outfile <<  															sqDist;
				}
				else if(constantStr == "WEB_BLADE_LO"){
					int sqDist = WEB_DISC_RADIUS-WEB_ROC_SIZE*6-WEB_ROC_OFFSET*6-WEB_PLAQ_OFFSET*2-2;
					sqDist *= sqDist;	 
					outfile <<  															sqDist;
				}
				else if(constantStr == "WEB_ROC_OFFSET")
					outfile <<  															WEB_ROC_OFFSET;
				else if(constantStr == "WEB_BLYR_XOFF")
					outfile <<  															WEB_BLYR_XOFF;
				else if(constantStr == "WEB_BLYR_YOFF")
					outfile <<  															WEB_BLYR_YOFF;
				else if(constantStr == "WEB_BROC_WIDTH")
					outfile <<  															WEB_BROC_WIDTH;
				else if(constantStr == "WEB_BROC_HEIGHT")
					outfile <<  															WEB_BROC_HEIGHT;
				else if(constantStr == "WEB_BROW_OFFSET")
					outfile <<  															WEB_BROW_OFFSET;
				else if(constantStr == "WEB_BCOL_OFFSET")
					outfile <<  															WEB_BCOL_OFFSET;
				else if(constantStr == "BPIX_LYR1_ROWS")
					outfile <<  															BPIX_LYR1_ROWS;
				else if(constantStr == "BPIX_LYR2_ROWS")
					outfile <<  															BPIX_LYR2_ROWS;
				else if(constantStr == "BPIX_LYR3_ROWS")
					outfile <<  															BPIX_LYR3_ROWS;
				else if(constantStr == "BPIX_LYR_COLS")
					outfile <<  															BPIX_LYR_COLS;
				else if(constantStr == "PNL1_PLAQ1_H")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*5 - WEB_ROC_OFFSET*5 - WEB_PLAQ_OFFSET*3;
				else if(constantStr == "PNL1_PLAQ1_L")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*6 - WEB_ROC_OFFSET*5 - WEB_PLAQ_OFFSET*3;
				else if(constantStr == "PNL1_PLAQ2_H")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*3 - WEB_ROC_OFFSET*3 - WEB_PLAQ_OFFSET*2;
				else if(constantStr == "PNL1_PLAQ2_L")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*5 - WEB_ROC_OFFSET*4 - WEB_PLAQ_OFFSET*2;
				else if(constantStr == "PNL1_PLAQ2_M")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*4 - WEB_ROC_OFFSET*3 - WEB_PLAQ_OFFSET*2;
				else if(constantStr == "PNL1_PLAQ3_H")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE 	- WEB_ROC_OFFSET   - WEB_PLAQ_OFFSET;
				else if(constantStr == "PNL1_PLAQ3_L")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*3 - WEB_ROC_OFFSET*2 - WEB_PLAQ_OFFSET;
				else if(constantStr == "PNL1_PLAQ3_M")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*2 - WEB_ROC_OFFSET   - WEB_PLAQ_OFFSET;
				else if(constantStr == "PNL1_PLAQ4_H")
					dist = WEB_DISC_RADIUS;
				else if(constantStr == "PNL1_PLAQ4_L")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE;
				else if(constantStr == "PNL2_PLAQ1_H")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*4 - WEB_ROC_OFFSET*4 - WEB_PLAQ_OFFSET*2;
				else if(constantStr == "PNL2_PLAQ1_L")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*6 - WEB_ROC_OFFSET*5 - WEB_PLAQ_OFFSET*2;
				else if(constantStr == "PNL2_PLAQ1_M")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*5 - WEB_ROC_OFFSET*4 - WEB_PLAQ_OFFSET*2;
				else if(constantStr == "PNL2_PLAQ2_H")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*2 - WEB_ROC_OFFSET*2 - WEB_PLAQ_OFFSET;
				else if(constantStr == "PNL2_PLAQ2_L")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*4 - WEB_ROC_OFFSET*3 - WEB_PLAQ_OFFSET;
				else if(constantStr == "PNL2_PLAQ2_M")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*3 - WEB_ROC_OFFSET*2 - WEB_PLAQ_OFFSET;
				else if(constantStr == "PNL2_PLAQ3_H")
					dist = WEB_DISC_RADIUS;
				else if(constantStr == "PNL2_PLAQ3_L")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE*2 - WEB_ROC_OFFSET;
				else if(constantStr == "PNL2_PLAQ3_M")
					dist = WEB_DISC_RADIUS - WEB_ROC_SIZE;
				else
					cout << mthn << "C++ Constant not found: " << constantStr << endl;
				
				if(dist != -1) //for location query map
					outfile << (int)sqrt((float)dist*dist);
				
			}
			else{ //not keyword
				outfile.put('#');
			}   
		}
		else{ //normal character
      outfile.put(c);
	  }			
	}
	
	infile.close();
	outfile.close();
}
 
 
 
 
 
 
 
 
 
 
 
 
