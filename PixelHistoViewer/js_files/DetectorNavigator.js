var DetectorNavigator = {};

DetectorNavigator.isFirstLoad = true;

DetectorNavigator.originX;
DetectorNavigator.originY;
DetectorNavigator.mx; //{x,y} mouse coords corresponding to pixel in image. {0,0} is top left.
DetectorNavigator.my;
DetectorNavigator.winObj;
DetectorNavigator.winPosX;
DetectorNavigator.winPosY;
DetectorNavigator.canvasOffX;
DetectorNavigator.canvasOffY;

DetectorNavigator.discs = new Array();
DetectorNavigator.barrels = new Array();
DetectorNavigator.discCaptions = new Array();
DetectorNavigator.barrelCaptions = new Array();
DetectorNavigator.discFlags = new Array();
DetectorNavigator.barrelFlags = new Array();
DetectorNavigator.rocHighlights = new Array();
DetectorNavigator.fedCaptions = new Array();
DetectorNavigator.fedCaptionTops = new Array(); //store y val so it only has to be calculated once
DetectorNavigator.fedCaptionLefts = new Array();//store x val so it only has to be calculated once
DetectorNavigator.fedViewMode;
DetectorNavigator.zoomedIndex = [-1,-1,-1]; //0,1 for Fpix,Bpix, 0-4,3 for disc,layer, 0,1 for left,right

DetectorNavigator.clickMask = [[[],[],[],[],[],[],[],[]],[[],[],[],[],[],[]]]; //[2][8,6][x]
DetectorNavigator.currentSummaryNode; //used for filling summary fields
DetectorNavigator.currentSummary = "-";
DetectorNavigator.currentSummaryField = "-";
DetectorNavigator.currentSummaryCaption;
DetectorNavigator.rocColorKeyImage;
DetectorNavigator.colorCodeImages = new Array();
DetectorNavigator.rocColorKeyValues = new Array();
DetectorNavigator.rocColorKeyValuesNumber = 5;
DetectorNavigator.summaryOvfPic;
DetectorNavigator.summaryOvfText;
DetectorNavigator.summaryUnfPic;
DetectorNavigator.summaryUnfText;
DetectorNavigator.summarySigma;
DetectorNavigator.summaryForm = 0;
DetectorNavigator.summaryFormField;
DetectorNavigator.summaryAverage;
DetectorNavigator.summaryTotalSamples;
DetectorNavigator.summaryMinValue;
DetectorNavigator.summaryMaxValue;
DetectorNavigator.summaryParameters = new Array(); //from ajax response
																									 //0 - total samples
																									 //1 - min value
																									 //2 - max value
																									 //3 - avg
																									 //4 - sigma (-1 if booleanOnly)
																									 //5 - min limit
																									 //6 - max limit
	//fpix naming
DetectorNavigator.disc;
DetectorNavigator.blade;
DetectorNavigator.panel;
DetectorNavigator.plaquette;
	//bpix naming
DetectorNavigator.sector;
DetectorNavigator.layer;
DetectorNavigator.ladder;
DetectorNavigator.module;
	//common naming
DetectorNavigator.plusMinus;
DetectorNavigator.inOut;
DetectorNavigator.roc;  
DetectorNavigator.rocPathDiv; //div with full string
DetectorNavigator.rocPath;  	//full string
	
	//fpix ROC location members
DetectorNavigator.oldHighlightIndex = -1;
	//bpix ROC location members
DetectorNavigator.endSectorRows = [3,3,5];
DetectorNavigator.otherSectorRows = [2,4,6];
DetectorNavigator.safariSubstractOne = 0;
DetectorNavigator.barrelCol;
DetectorNavigator.barrelRow;

	//start c++ constants input here
DetectorNavigator.WEB_LARGE_WIDTH        = 600;
DetectorNavigator.WEB_LARGE_HEIGHT       = 600;
DetectorNavigator.WEB_MINI_WIDTH         = 150;
DetectorNavigator.WEB_MINI_HEIGHT        = 150;
DetectorNavigator.WEB_BLARGE_WIDTH			 = 600;
DetectorNavigator.WEB_BLARGE_HEIGHT 		 = 600;
DetectorNavigator.WEB_BMINI_WIDTH 			 = 100;
DetectorNavigator.WEB_BMINI_HEIGHT			 = 200;
DetectorNavigator.WEB_XOFFSET         	 = 0;
DetectorNavigator.WEB_YOFFSET         	 = 2;
DetectorNavigator.WEB_ROC_SIZE         	 = 9;
DetectorNavigator.WEB_DISC_X        		 = 300;
DetectorNavigator.WEB_DISC_Y        		 = 300;
DetectorNavigator.WEB_BLADE_HI       		 = 89401;
DetectorNavigator.WEB_BLADE_LO       		 = 45369;
DetectorNavigator.WEB_ROC_OFFSET       	 = 3;
DetectorNavigator.WEB_BLYR_XOFF 				 = 14;
DetectorNavigator.WEB_BLYR_YOFF 				 = 5;
DetectorNavigator.WEB_BROC_WIDTH				 = 14;
DetectorNavigator.WEB_BROC_HEIGHT 			 = 5;
DetectorNavigator.WEB_BROW_OFFSET 			 = 2;
DetectorNavigator.WEB_BCOL_OFFSET 			 = 4;
DetectorNavigator.BPIX_LYR_COLS					 = 32;
DetectorNavigator.BPIX_LYR_ROWS				   = 
	[36,60,84];

	//query Plaquette/ROC parameters from C++
	//		[PlaqHighUpBoundary,PlaqLowUpBoundary,PlaqMiddleUpBoundary,
	//					[[numOfRocs,rocOff,rocSign,upRocMult],
	//						[numOfRocs,rocOff,rocSign,upRocMult]],
	//							rocSizeMult,rocOffMult],
var q 						       	 = 
	[ 
		[	//4-pplaquette panel type 1
			[223,214,0,
				[ [2,1,-1,(223 + 214)/2]  ,
					[]]	,
				0.0, 0.5],
			[251,230,242,
				[ [3,2,-1,(230 + 242 - 3)/2]	,
					[3,3, 1,(251 + 242)/2]] ,
				0.5, 1.0],
			[279,258,270,
				[ [4,3,-1,(258 + 270 - 3)/2]	,
					[4,4, 1,(279 + 270)/2]] ,
				1.0, 1.5],
			[295,286,0,
				[ [5,4,-1,(295 + 286)/2]	,
					[]] ,
				1.5, 2.0]
		],
		[ //3-pplaquette panel type 2
			[239,218,230,
				[ [3,2,-1,(218 + 230 - 3)/2]	,
					[3,3, 1,(239 + 230)/2]] ,
				0.5, 1.0],
			[267,246,258,
				[ [4,3,-1,(246 + 258 - 3)/2]	,
					[4,4, 1,(267 + 258)/2]] ,
				1.0, 1.5],
			[295,274,286,
				[ [5,4,-1,(274 + 286 - 3)/2]	,
					[5,5, 1,(295 + 286)/2]] ,
				1.5, 2.0]
		]	
	];
	//end c++ constants section
	
	
			//saved code snippets
	//document.body.style.cursor='pointer';//hand
	//document.body.style.cursor='auto';   //normal
	//document.body.style.cursor='arrow';   //arrow		
	//document.forms['detNavForm'].mouseXDisplay.value = "-";
	//document.forms['detNavForm'].mouseYDisplay.value = 	"-";
	//document.forms['detNavForm'].mouseXDisplay.value = mx;
	//document.forms['detNavForm'].mouseYDisplay.value = my;
	//var time = (new Date()).getTime();
	//document.forms['detNavForm'].mouseXDisplay.value = (new Date()).getTime() - time;
	//document.forms['detNavForm'].mouseYDisplay.value = (new Date()).getTime() - time;
		//end saved code snippets
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.init = function(dWin) {
	dWin.win.addListener('move',DetectorNavigator.updateWinPos);
	DetectorNavigator.winObj = dWin;	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.updateWinPos = function(w,x,y){
	DetectorNavigator.winPosX = DetectorNavigator.winObj.win.getPosition()[0];
	DetectorNavigator.winPosY = DetectorNavigator.winObj.win.body.getY();    
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.initObjects = function(){

	if(!DetectorNavigator.isFirstLoad) //prevent re-initialization when images are updated
		return;
	DetectorNavigator.isFirstLoad = false;
	
	DetectorNavigator.winPosX = DetectorNavigator.winObj.win.getPosition()[0];
	DetectorNavigator.winPosY = DetectorNavigator.winObj.win.body.getY();    

	DetectorNavigator.rocPathDiv = document.getElementById('currentPath');
	
	for(i=0;i<8;++i){ //acquire highlight objects
    DetectorNavigator.discs[i] = document.getElementById('disc'+i);
    DetectorNavigator.discCaptions[i] = document.getElementById('discCaption'+i);
    DetectorNavigator.discFlags[i] = document.getElementById('discFlag'+i);
  }
	for(i=0;i<6;++i){ //acquire highlight objects
    DetectorNavigator.barrels[i] = document.getElementById('barrel'+i);
    DetectorNavigator.barrelCaptions[i] = document.getElementById('barrelCaption'+i);
    DetectorNavigator.barrelFlags[i] = document.getElementById('barrelFlag'+i);
  }
  for(i=0;i<6;++i){ //acquire highlight objects
    DetectorNavigator.rocHighlights[i] = document.getElementById('rocHighlight' + i);
		DetectorNavigator.rocHighlights[i].style.width = DetectorNavigator.WEB_ROC_SIZE*2+"px";
    DetectorNavigator.rocHighlights[i].style.height = DetectorNavigator.WEB_ROC_SIZE*2+"px";
  }
	DetectorNavigator.barrelHighlight = document.getElementById('rocHighlight');
	DetectorNavigator.barrelHighlight.style.width = DetectorNavigator.WEB_BROC_WIDTH+2+"px";
  DetectorNavigator.barrelHighlight.style.height = DetectorNavigator.WEB_BROC_HEIGHT+4+"px";

		//setup summary objects
	DetectorNavigator.colorCodeImages[0] = document.getElementById('rocColorKey');
	DetectorNavigator.colorCodeImages[0].style.top = '-100px';
	DetectorNavigator.colorCodeImages[1] = document.getElementById('rocColorKeyBool');
	DetectorNavigator.colorCodeImages[1].style.top = '-100px';
	DetectorNavigator.colorCodeImages[0].style.width = '300px';
	DetectorNavigator.colorCodeImages[0].style.height = '25px';
	DetectorNavigator.colorCodeImages[1].style.width = '300px';
	DetectorNavigator.colorCodeImages[1].style.height = '25px';
	DetectorNavigator.rocColorKeyImage = DetectorNavigator.colorCodeImages[0];
		
	DetectorNavigator.currentSummaryCaption = document.getElementById('summaryName'); 
	var tmpsummary = DetectorNavigator.currentSummary;
	while((i = tmpsummary.indexOf("/")) != -1){
		tmpsummary = tmpsummary.slice(i+1);	
  }
	DetectorNavigator.currentSummaryCaption.innerHTML = tmpsummary + " - " + DetectorNavigator.currentSummaryField;
	DetectorNavigator.currentSummaryCaption.style.left = 7 +  		//only top is changed
		DetectorNavigator.WEB_LARGE_WIDTH/2 - DetectorNavigator.currentSummaryCaption.offsetWidth/2 + "px";
		
	var tmpxoff = 10;
	DetectorNavigator.summarySigma = document.getElementById('summarySigma'); 
	DetectorNavigator.summarySigma.innerHTML = "&sigma;: " + DetectorNavigator.summaryParameters[4];
	DetectorNavigator.summarySigma.style.left = tmpxoff + "px";
		
	DetectorNavigator.summaryAverage = document.getElementById('summaryAvg'); 
	DetectorNavigator.summaryAverage.innerHTML = "Avg: " + DetectorNavigator.summaryParameters[3];
	DetectorNavigator.summaryAverage.style.left = tmpxoff + "px";
	
	DetectorNavigator.summaryTotalSamples = document.getElementById('summarySamples'); 
	DetectorNavigator.summaryTotalSamples.innerHTML = "Samples: " + DetectorNavigator.summaryParameters[0];
	DetectorNavigator.summaryTotalSamples.style.left = tmpxoff + "px";
	
	DetectorNavigator.summaryMinValue = document.getElementById('sumamryMin'); 
	DetectorNavigator.summaryMinValue.innerHTML = "Min: " + DetectorNavigator.summaryParameters[1];
	DetectorNavigator.summaryMinValue.style.left = tmpxoff + "px";
	
	DetectorNavigator.summaryMaxValue = document.getElementById('summaryMax'); 
	DetectorNavigator.summaryMaxValue.innerHTML = "Max: " + DetectorNavigator.summaryParameters[2];
	DetectorNavigator.summaryMaxValue.style.left = tmpxoff + "px";

	for(i=0;i<DetectorNavigator.rocColorKeyValuesNumber;++i)
		DetectorNavigator.rocColorKeyValues[i] = document.getElementById('rocColorKeyValues'+i);

	if(DetectorNavigator.summaryParameters[4] == -1){ //boolean only summary			
		DetectorNavigator.rocColorKeyValues[0].innerHTML = "Fail";
		DetectorNavigator.rocColorKeyValues[1].innerHTML = "Pass";
		DetectorNavigator.rocColorKeyImage = DetectorNavigator.colorCodeImages[1];
		DetectorNavigator.summarySigma.style.left = '-1000px';
		//DetectorNavigator.summaryAverage.style.left = '-1000px';
		//DetectorNavigator.summaryMinValue.style.left = '-1000px';
		//DetectorNavigator.summaryMaxValue.style.left = '-1000px';
	}
	else{ 																						//normal summary
		var p1 = parseFloat(DetectorNavigator.summaryParameters[5]);
		p1 *= 100;
		p1 = Math.round(p1);
		p1 /= 100;
		var p2 = parseFloat(DetectorNavigator.summaryParameters[6]);
		p2 *= 100;
		p2 = Math.round(p2);
		p2 /= 100;
		var p3 = (p1 + p2)/2;
		p3 *= 100;
		p3 = Math.round(p3);
		p3 /= 100;
		var p4 = (p1 + p3)/2;
		p4 *= 100;
		p4 = Math.round(p4);
		p4 /= 100;
		var p5 = (p3 + p2)/2;
		p5 *= 100;
		p5 = Math.round(p5);
		p5 /= 100;
		
		DetectorNavigator.rocColorKeyValues[0].innerHTML = p1;
		DetectorNavigator.rocColorKeyValues[4].innerHTML = p2;
		DetectorNavigator.rocColorKeyValues[2].innerHTML = p3;
		DetectorNavigator.rocColorKeyValues[1].innerHTML = p4;
		DetectorNavigator.rocColorKeyValues[3].innerHTML = p5;
	}
	
	DetectorNavigator.rocColorKeyImage.style.left = 7 +  		//only top is changed
		DetectorNavigator.WEB_LARGE_WIDTH/2 - DetectorNavigator.rocColorKeyImage.offsetWidth/2 + "px";
	
	if(DetectorNavigator.summaryParameters[4] == -1){		
		DetectorNavigator.rocColorKeyValues[0].style.left = DetectorNavigator.rocColorKeyImage.offsetLeft
			+ 1*DetectorNavigator.rocColorKeyImage.offsetWidth/(DetectorNavigator.rocColorKeyValuesNumber-1)
			- DetectorNavigator.rocColorKeyValues[0].offsetWidth/2 + "px";
		DetectorNavigator.rocColorKeyValues[1].style.left = DetectorNavigator.rocColorKeyImage.offsetLeft
			+ 3*DetectorNavigator.rocColorKeyImage.offsetWidth/(DetectorNavigator.rocColorKeyValuesNumber-1)
			- DetectorNavigator.rocColorKeyValues[0].offsetWidth/2 + "px";
		for(i=2;i<DetectorNavigator.rocColorKeyValuesNumber;++i){
			DetectorNavigator.rocColorKeyValues[i].style.left = "-1000px";
		}
	}
	else{
		for(i=0;i<DetectorNavigator.rocColorKeyValuesNumber;++i){
			DetectorNavigator.rocColorKeyValues[i].style.left = DetectorNavigator.rocColorKeyImage.offsetLeft
				+ i*DetectorNavigator.rocColorKeyImage.offsetWidth/(DetectorNavigator.rocColorKeyValuesNumber-1)
				- DetectorNavigator.rocColorKeyValues[i].offsetWidth/2 + "px";
		}
  }
	
	DetectorNavigator.summaryOvfPic = document.getElementById('rocColorKeyOver');
	DetectorNavigator.summaryOvfPic.style.width = '10px';
	DetectorNavigator.summaryOvfPic.style.height = '10px';
	DetectorNavigator.summaryOvfText = document.getElementById('rocColorKeyOverText'); 
	DetectorNavigator.summaryOvfText.innerHTML = "Overflow";
	
	DetectorNavigator.summaryOvfText.style.left = 7 + 
		DetectorNavigator.WEB_LARGE_WIDTH/2 + 60 - 
		(DetectorNavigator.summaryOvfPic.offsetWidth + 
		DetectorNavigator.summaryOvfText.offsetWidth + 5)/2 + "px";
	DetectorNavigator.summaryOvfPic.style.left = DetectorNavigator.summaryOvfText.offsetLeft + 
		DetectorNavigator.summaryOvfText.offsetWidth + 5 + "px";
		
	DetectorNavigator.summaryUnfPic = document.getElementById('rocColorKeyUnder');
	DetectorNavigator.summaryUnfPic.style.width = '10px';
	DetectorNavigator.summaryUnfPic.style.height = '10px';
	DetectorNavigator.summaryUnfText = document.getElementById('rocColorKeyUnderText'); 
	DetectorNavigator.summaryUnfText.innerHTML = "Underflow";
	
	DetectorNavigator.summaryUnfText.style.left = 7 + 
		DetectorNavigator.WEB_LARGE_WIDTH/2 - 60 - 
		(DetectorNavigator.summaryUnfPic.offsetWidth +
		DetectorNavigator.summaryUnfText.offsetWidth + 5)/2 + "px";
	DetectorNavigator.summaryUnfPic.style.left = DetectorNavigator.summaryUnfText.offsetLeft + 
		DetectorNavigator.summaryUnfText.offsetWidth + 5 + "px";	
	
		//capture mouse coordinates
	document.captureEvents(Event.MOUSEMOVE);
	document.onmousemove = DetectorNavigator.getMouseXY;
	
	if("Apple Computer, Inc." == navigator.vendor){ // fix for safari right click issue
		for(i=0;i<8;++i){
			DetectorNavigator.discs[i].oncontextmenu = function(){DetectorNavigator.zoom(false);};
		}	
		for(i=0;i<6;++i){
			DetectorNavigator.barrels[i].oncontextmenu = function(){DetectorNavigator.zoom(false);};
		}
		for(i=0;i<6;++i){
    	DetectorNavigator.rocHighlights[i].oncontextmenu = function(){DetectorNavigator.zoom(false);};
  	}
		DetectorNavigator.barrelHighlight.oncontextmenu = function(){DetectorNavigator.zoom(false);};

			//fix for weird pixel off problem on bpix highlight
		DetectorNavigator.safariSubstractOne = -1;
	}
	
		//sigma form for summary
	if(!DetectorNavigator.summaryForm){
		DetectorNavigator.summaryForm = new Ext.form.FormPanel({
	      baseCls: 'x-plain',
				autoScroll:   false,
				buttonAlign:  'right',
				labelAlign: 	'top',
				style: "position:absolute;z-index:2",
				x: 490,
				y: 37-DetectorNavigator.safariSubstractOne,
				renderTo: "Detector Navigator",
				
	      items: [{
					xtype: 'combo', 																											
					name: 'sigmas', 																											
					hideLabel: true,																											
					style: 'width:101px', 																								
					fieldLabel: '', 																											
					tabIndex: 0,																													
					mode: 'local',																												
					triggerAction: 'all', 																								
					typeAhead: true,																											
					emptyText: '<enter command>', 																				
					selectOnFocus: true,																									
					displayField: 'commands', 																						
					store: new Ext.data.SimpleStore({ 																		
					  fields: ['commands'], 			  															       
					  data: [ 										  															       
										['S p1'], 																					       
					          ['M > p1'], 				  															       
					          ['M < p1'], 				  															       
					          ['M > p1 < p2'],		  															       
					          ['B <  p1'],				  															       
					          ['B <= p1'],				  															       
					          ['B >  p1'],				  															       
					          ['B >= p1'],				  															       
					          ['B >  p1 & <  p2'],  															       
					          ['B >  p1 | <  p2'],  															       
					        ] 										  															       
					})																		
				}],
				buttons: [{
	        text: 'Set',																					    			
				  listeners: {click: DetectorNavigator.SetSigmaThreshold} 				
	      }]
	  });
	}
	DetectorNavigator.summaryFormField = document.getElementById('summaryFormField'); 
	DetectorNavigator.summaryFormField.innerHTML = "Filter String:";
	DetectorNavigator.summaryFormField.style.left = '500px';
		
	DetectorNavigator.summaryForm.setVisible(false);
	
	DetectorNavigator.initFedCaptions();
	
		//init view
	if(DetectorNavigator.zoomedIndex[0] < 0){
		DetectorNavigator.zoomedIndex[0] = -1;
	  DetectorNavigator.zoomedIndex[1] = -1;
	  DetectorNavigator.zoomedIndex[2] = -1;
	  DetectorNavigator.displayMiniView();
		DetectorNavigator.canvasOffX = DetectorNavigator.discs[0].offsetLeft+1;
	}
	else{
		DetectorNavigator.displayLargeView();
		DetectorNavigator.canvasOffX = 8;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//needs to be as fast as possible -- this is run all the time!!
DetectorNavigator.getMouseXY = function(e) {	
	
	DetectorNavigator.originX = DetectorNavigator.winPosX + DetectorNavigator.canvasOffX;
	DetectorNavigator.originY = DetectorNavigator.winPosY + DetectorNavigator.canvasOffY;

		//have correct origin---------------------------------------------------------------------------------
	
	var mx = DetectorNavigator.mx = e.pageX-DetectorNavigator.originX;
	var my = DetectorNavigator.my = e.pageY-DetectorNavigator.originY;
		
	DetectorNavigator.inOut = "-";
	DetectorNavigator.blade = "-";
	DetectorNavigator.plaquette = "-";
	DetectorNavigator.roc = "-";

	var rocNotFound = true;
	var highlightIndex = DetectorNavigator.oldHighlightIndex;

	if( mx >= 0 &&
			my >= 0 &&
			mx < DetectorNavigator.WEB_LARGE_WIDTH &&
			my < DetectorNavigator.WEB_LARGE_HEIGHT){		
		
		if(DetectorNavigator.zoomedIndex[0] < 0){ //mini view
			return;
		}

		if(DetectorNavigator.zoomedIndex[0] == 1){ //bpix
			DetectorNavigator.searchBarrelForCoords(mx,my);
			DetectorNavigator.createCurrentPath();
			return;
		}
		
			//fpix	
		var sqDist;
		var angle;
		var angleMod;
		var up;
		var rt;
		var tmp;
		var invertCondUpDn;
		var invertCondLtRt;
		var slope;
		var vslope;
		var rel;
		var diff;
		var pnl;
		var numOfPlaqs;
		var plaqNotFound;
		var rowIndex;
		var plaq;
		var rocNotFound;
		var i;
		var rowParams;
		var rtRocMult;
		var rocPos;
			
		if(mx < DetectorNavigator.WEB_DISC_X){ 
			DetectorNavigator.inOut = 'O';
		}
		else{
			DetectorNavigator.inOut = 'I';
		}
		
		up = [-1,-1];
		rt = [-1,-1];
		diff = [mx - DetectorNavigator.WEB_DISC_X,my - DetectorNavigator.WEB_DISC_Y];
		
		sqDist = diff[0]*diff[0] + diff[1]*diff[1];
		angle = Math.atan2(diff[1],diff[0]);
		if(angle < 0) //correct negative angles
		  angle += 2*Math.PI;
		angle = parseInt((angle*180/Math.PI + 90)%360); //calc angle in degrees in Z[0,359]

		angleMod = angle%15;
		if(angleMod >= 1 && angleMod < 14){ //eliminate dead space between blades
			if(sqDist >= DetectorNavigator.WEB_BLADE_LO && sqDist <= DetectorNavigator.WEB_BLADE_HI){
					//get up and rt vectors for blade
				if(angle > 180){ //half disc on left
					DetectorNavigator.blade = parseInt((360 - angle)/15) + 1;
					up = [-1,-Math.tan(13*Math.PI/24 + (12-DetectorNavigator.blade)*Math.PI/12)];
				}
				else{ //half-disc on right
					DetectorNavigator.blade = parseInt(angle/15) + 1;
					up = [1,Math.tan(-13*Math.PI/24 + DetectorNavigator.blade*Math.PI/12)];
				}
				
					//normalize up vector
				tmp = Math.sqrt(up[0]*up[0] + up[1]*up[1]);
  			if(tmp != 0){
  				up[0] /= tmp;
  				up[1] /= tmp;
				}
				rt = [-up[1], up[0]];
			}
		}
		
			//at this point, have normalized up and rt vector if blade is valid
		
		if(DetectorNavigator.blade != "-"){ //locate plaquettes
			invertCondUpDn = 1;  //comparisons switch on bottom half because looking at above and below lines
      if(DetectorNavigator.blade > 6)
				invertCondUpDn *= -1;
      invertCondLtRt = 1;  //comparisons switch on left half because looking at left and right of lines
      if(angle > 180)
				invertCondLtRt *= -1;

      slope = rt[1]/rt[0];
      rel = [diff[0],diff[1]];
      
      vslope = up[1]/up[0]; 
			
			pnl = DetectorNavigator.panel-1;
			numOfPlaqs = 4-pnl;			
			plaqNotFound = true;
			rowIndex = -1;
			plaq = 0;
			
				 //locate plaquette and row within plaquette
			while(plaqNotFound && plaq < numOfPlaqs){
	
				if(
						0 >= invertCondUpDn*((rel[1] - up[1]*q[pnl][plaq][1]) -
								 slope*(rel[0] - up[0]*q[pnl][plaq][1])) &&
  					0 <= invertCondUpDn*((rel[1] - up[1]*(q[pnl][plaq][0] + 4)) -
								 slope*(rel[0] - up[0]*(q[pnl][plaq][0] + 4)))
					){
					plaqNotFound = false;	
					
					if(pnl == 0 && (plaq == 0 || plaq == 3)){ //only one row
						rowIndex = 0;
					}
					else if(0 >= invertCondUpDn*((rel[1] - up[1]*q[pnl][plaq][2]) -
							  	slope*(rel[0] - up[0]*q[pnl][plaq][2])) ){ //top ROC row
					  rowIndex = 1;
					}
					else if(0 <= invertCondUpDn*((rel[1] - up[1]*(q[pnl][plaq][2] + 4 - DetectorNavigator.WEB_ROC_OFFSET)) -
									slope*(rel[0] - up[0]*(q[pnl][plaq][2] + 4 - DetectorNavigator.WEB_ROC_OFFSET))) ){ //bottom ROC row
					  rowIndex = 0;
					}
					else
						rowIndex = -1;
				}
	
				++plaq;
			}
	
			if(!plaqNotFound){
				DetectorNavigator.plaquette = (--plaq)+1;
			}
				//find ROC
			if(rowIndex != -1){				
				for(i=0;rocNotFound && i<q[pnl][plaq][3][rowIndex][0];++i){
				  if(
							0 <= invertCondLtRt*((rel[1] - rt[1]*(DetectorNavigator.WEB_ROC_SIZE*(q[pnl][plaq][4] - i) +
									 DetectorNavigator.WEB_ROC_OFFSET*(q[pnl][plaq][5] - i))) -
									 vslope*(rel[0] - rt[0]*(DetectorNavigator.WEB_ROC_SIZE*(q[pnl][plaq][4] - i) +
									 DetectorNavigator.WEB_ROC_OFFSET*(q[pnl][plaq][5] - i)))) &&
							0 >= invertCondLtRt*((rel[1] - rt[1]*(DetectorNavigator.WEB_ROC_SIZE*(q[pnl][plaq][4] + 1 - i) +
									 DetectorNavigator.WEB_ROC_OFFSET*(q[pnl][plaq][5] - i))) -
									 vslope*(rel[0] - rt[0]*(DetectorNavigator.WEB_ROC_SIZE*(q[pnl][plaq][4] + 1 - i) +
									 DetectorNavigator.WEB_ROC_OFFSET*(q[pnl][plaq][5] - i))))
				  	){
						
				    DetectorNavigator.roc = q[pnl][plaq][3][rowIndex][1]+q[pnl][plaq][3][rowIndex][2]*i;
						
				    	//display highlight
				    rtRocMult = ((DetectorNavigator.WEB_ROC_SIZE*(q[pnl][plaq][4] - i) + 
														 DetectorNavigator.WEB_ROC_OFFSET*(q[pnl][plaq][5] - i)) +
														(DetectorNavigator.WEB_ROC_SIZE*(q[pnl][plaq][4] + 1 - i) +
														 DetectorNavigator.WEB_ROC_OFFSET*(q[pnl][plaq][5] - i)))/2;
													
				    rocPos = [DetectorNavigator.WEB_DISC_X + up[0] * q[pnl][plaq][3][rowIndex][3] + rt[0] * rtRocMult,
				    						 DetectorNavigator.WEB_DISC_Y + up[1] * q[pnl][plaq][3][rowIndex][3] + rt[1] * rtRocMult];
		
				    highlightIndex = (DetectorNavigator.blade-1)%6;
				    if(angle > 180)
				      highlightIndex = 5 - highlightIndex;

				    DetectorNavigator.rocHighlights[highlightIndex].style.left = (DetectorNavigator.canvasOffX + rocPos[0] - DetectorNavigator.WEB_ROC_SIZE - 1 +
							DetectorNavigator.safariSubstractOne) + "px";
				    DetectorNavigator.rocHighlights[highlightIndex].style.top = (DetectorNavigator.canvasOffY + rocPos[1] - DetectorNavigator.WEB_ROC_SIZE - 1) + "px";
						
						document.body.style.cursor='pointer';
						rocNotFound = false;
				  }
			  }
			}
		}
		DetectorNavigator.createCurrentPath();
	}
	
		//hide previous highlight
	if(rocNotFound || DetectorNavigator.oldHighlightIndex != highlightIndex){	
		if(DetectorNavigator.oldHighlightIndex >= 0)
			DetectorNavigator.rocHighlights[DetectorNavigator.oldHighlightIndex].style.left = "-100px";
		DetectorNavigator.oldHighlightIndex = highlightIndex;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.zoom = function(isIn,obj) {

		//---------- Get zoomOne and zoomTwo index for display positioning
	if(!isIn){ //zoom out request
		if(DetectorNavigator.zoomedIndex[0] == -1){ //currently mini view
			return;//Already fully zoomed out
		}
		else{ //currently large view	
			DetectorNavigator.zoomedIndex[0] = -1;
			DetectorNavigator.zoomedIndex[1] = -1;
			DetectorNavigator.zoomedIndex[2] = -1;
		}
	}
	else{ //zoom in request
		if(DetectorNavigator.zoomedIndex[0] == -1){ //currently mini view
		
			var idNum = parseInt(obj.id.charAt(obj.id.length-1));
			DetectorNavigator.zoomedIndex[1] = (idNum/2)|0;
			DetectorNavigator.zoomedIndex[2] = idNum%2;
			if(obj.id[0] == 'd'){
				DetectorNavigator.zoomedIndex[0] = 0; //FPix was clicked				
			}
			else{
				DetectorNavigator.zoomedIndex[0] = 1; //BPix was clicked
			}
			
		}
		else{ //currently large view
			return;//Already fully zoomed in
		}	
	}

		//---------- display positioning	
  if(DetectorNavigator.zoomedIndex[0] == -1){  		//---------------- display mini
		DetectorNavigator.displayMiniView();
	}
  else{			      													//---------------- display large
		DetectorNavigator.displayLargeView();
  }
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.displayLargeView = function(){
	
	var captionOff = [DetectorNavigator.WEB_LARGE_WIDTH-2,-23];
	var imgIndex = DetectorNavigator.zoomedIndex[1]*2 + DetectorNavigator.zoomedIndex[2];
	
	if(DetectorNavigator.zoomedIndex[0] == 0){ //zoom on a disc
		  //zoom on disc
		DetectorNavigator.discs[imgIndex].onmouseover		 = function(){document.body.style.cursor='auto';};
		DetectorNavigator.discs[imgIndex].onmousemove		 = function(){document.body.style.cursor='auto';};
		DetectorNavigator.discs[imgIndex].style.position = 'relative';
		DetectorNavigator.discs[imgIndex].style.width		 = DetectorNavigator.WEB_LARGE_WIDTH + 'px';
		DetectorNavigator.discs[imgIndex].style.height 	 = DetectorNavigator.WEB_LARGE_HEIGHT + 'px';
		DetectorNavigator.discs[imgIndex].style.left 		 = DetectorNavigator.WEB_XOFFSET +'px';
	
		for(i=0;i<8;++i){ //hide other discs
		  if(i != imgIndex){
		    DetectorNavigator.discs[i].style.position 	 = 'absolute';
		    DetectorNavigator.discs[i].style.left 			 = '-5000px';
				DetectorNavigator.discCaptions[i].style.top  = '-100px';
				DetectorNavigator.discFlags[i].style.top		 = '-100px';
		  }
		}
		for(i=0;i<6;++i){ //hide barrels
			DetectorNavigator.barrels[i].style.position 	 = 'absolute';
			DetectorNavigator.barrels[i].style.left 			 = '-5000px';
			DetectorNavigator.barrelCaptions[i].style.top  = '-100px';
	    DetectorNavigator.barrelFlags[i].style.top     = '-100px';
		}
			//hide caption and flag for this disc
		DetectorNavigator.discCaptions[imgIndex].style.left = captionOff[0]-DetectorNavigator.discCaptions[imgIndex].offsetWidth;
		DetectorNavigator.discCaptions[imgIndex].style.top  = captionOff[1];
		DetectorNavigator.discFlags[imgIndex].style.top 		= '-100px';
		
		DetectorNavigator.canvasOffY = 
			DetectorNavigator.discs[imgIndex].offsetTop + 1;//29;	
			
		if(DetectorNavigator.currentSummary != "-"){
			DetectorNavigator.rocColorKeyImage.style.top = DetectorNavigator.WEB_LARGE_HEIGHT/2 - 
				DetectorNavigator.rocColorKeyImage.offsetHeight/2 + "px";
			for(i=0;i<DetectorNavigator.rocColorKeyValuesNumber;++i)
				DetectorNavigator.rocColorKeyValues[i].style.top = DetectorNavigator.WEB_LARGE_HEIGHT/2 + 
					DetectorNavigator.rocColorKeyImage.offsetHeight/2 + "px";

			DetectorNavigator.currentSummaryCaption.style.top = DetectorNavigator.rocColorKeyImage.offsetTop - 30 + "px";
			if(DetectorNavigator.summaryParameters[4] != -1){
				DetectorNavigator.summaryOvfPic.style.top = DetectorNavigator.rocColorKeyImage.offsetTop - 12 + "px";
				DetectorNavigator.summaryOvfText.style.top = DetectorNavigator.rocColorKeyImage.offsetTop - 12 + "px";
				DetectorNavigator.summaryUnfPic.style.top = DetectorNavigator.rocColorKeyImage.offsetTop - 12 + "px";
				DetectorNavigator.summaryUnfText.style.top = DetectorNavigator.rocColorKeyImage.offsetTop - 12 + "px";
			}
		}
		
		if(DetectorNavigator.fedViewMode)
			DetectorNavigator.displayFedCaptions();
		
	}
	else{ //zoom on a barrel
				
		  //zoom on barrel
		DetectorNavigator.barrels[imgIndex].onmouseover		 = function(){document.body.style.cursor='auto';};
		DetectorNavigator.barrels[imgIndex].onmousemove		 = function(){document.body.style.cursor='auto';};
		DetectorNavigator.barrels[imgIndex].style.position = 'relative';
		DetectorNavigator.barrels[imgIndex].style.width  	 = DetectorNavigator.WEB_BLARGE_WIDTH + 'px';
		DetectorNavigator.barrels[imgIndex].style.height 	 = DetectorNavigator.WEB_BLARGE_HEIGHT + 'px';
		DetectorNavigator.barrels[imgIndex].style.left 		 = DetectorNavigator.WEB_XOFFSET +'px';

		for(i=0;i<6;++i){ //hide other barrels
		  if(i != imgIndex){
		    DetectorNavigator.barrels[i].style.position 	 = 'absolute';
		    DetectorNavigator.barrels[i].style.left 			 = '-5000px';
				DetectorNavigator.barrelCaptions[i].style.top  = '-100px';
				DetectorNavigator.barrelFlags[i].style.top		 = '-100px';
		  }
		}
		for(i=0;i<8;++i){ //hide discs
			DetectorNavigator.discs[i].style.position 			 = 'absolute';
			DetectorNavigator.discs[i].style.left 					 = '-5000px';
			DetectorNavigator.discCaptions[i].style.top 		 = '-100px';
			DetectorNavigator.discFlags[i].style.top				 = '-100px';
		}
			//hide caption and flag for this barrel
		DetectorNavigator.barrelCaptions[imgIndex].style.left = captionOff[0]-DetectorNavigator.barrelCaptions[imgIndex].offsetWidth;
		DetectorNavigator.barrelCaptions[imgIndex].style.top  = captionOff[1];
		DetectorNavigator.barrelFlags[imgIndex].style.top 		= '-100px';
	
		DetectorNavigator.canvasOffY = 
			DetectorNavigator.barrels[imgIndex].offsetTop + 1;	
			
		DetectorNavigator.rocColorKeyImage.style.top = '-100px';
		for(i=0;i<DetectorNavigator.rocColorKeyValuesNumber;++i)
				DetectorNavigator.rocColorKeyValues[i].style.top = '-100px';			
		DetectorNavigator.currentSummaryCaption.style.top = '-100px';
		
		DetectorNavigator.summaryOvfPic.style.top = '-100px';
		DetectorNavigator.summaryOvfText.style.top = '-100px';
		DetectorNavigator.summaryUnfPic.style.top = '-100px';
		DetectorNavigator.summaryUnfText.style.top = '-100px';
		
		DetectorNavigator.hideFedCaptions();
	}
	
		//hide summary details
	DetectorNavigator.summarySigma.style.top = '-100px';
	DetectorNavigator.summaryAverage.style.top = '-100px';
	DetectorNavigator.summaryTotalSamples.style.top = '-100px';
	DetectorNavigator.summaryMinValue.style.top = '-100px';
	DetectorNavigator.summaryMaxValue.style.top = '-100px';
	DetectorNavigator.summaryForm.setVisible(false);
	DetectorNavigator.summaryFormField.style.top = '-100px';
	
		//========= Handle Known Naming Fields ========//
	
	if(DetectorNavigator.zoomedIndex[0] == 0){ //fpix
			//set plus/minus and disc
		if(DetectorNavigator.zoomedIndex[1] < 2){
			DetectorNavigator.plusMinus = 'm';
			DetectorNavigator.disc = 2 - DetectorNavigator.zoomedIndex[1];
		}
		else{
			DetectorNavigator.plusMinus = 'p';
			DetectorNavigator.disc = DetectorNavigator.zoomedIndex[1] - 1;
		}
			//set panel type
		if(DetectorNavigator.zoomedIndex[2] == 0){
			DetectorNavigator.panel = 1;
		}
		else{
			DetectorNavigator.panel = 2;
		}
	}
	else{ //bpix
			//set plus/minus and layer
		if(DetectorNavigator.zoomedIndex[2] == 0){
			DetectorNavigator.plusMinus = 'm';
		}
		else{
			DetectorNavigator.plusMinus = 'p';
		}
		
		DetectorNavigator.layer = DetectorNavigator.zoomedIndex[1] + 1;	
	}
	
	DetectorNavigator.showCurrentPath();	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.displayMiniView = function(){
	DetectorNavigator.hideCurrentPath();
	DetectorNavigator.barrelHighlight.style.top = '-100px'; //hide bpix highlight
	if(DetectorNavigator.oldHighlightIndex >= 0)
		DetectorNavigator.rocHighlights[DetectorNavigator.oldHighlightIndex].style.left = "-100px";
	
	var discCaptionOffset = [7 + DetectorNavigator.WEB_MINI_WIDTH/2,40 + DetectorNavigator.WEB_MINI_HEIGHT/2];
	var barrelCaptionOffset = [7 + DetectorNavigator.WEB_BMINI_WIDTH/2,
		30 + 2*DetectorNavigator.WEB_MINI_HEIGHT + DetectorNavigator.WEB_BMINI_HEIGHT/2];
	
	for(i=0;i<8;++i){ // discs
		DetectorNavigator.discs[i].onmouseover														= function(){document.body.style.cursor='pointer';};//hand
		DetectorNavigator.discs[i].onmousemove														= function(){document.body.style.cursor='pointer';};//hand
		DetectorNavigator.discs[i].onmouseout															= function(){document.body.style.cursor='auto';};
  	DetectorNavigator.discs[i].style.position 												= 'relative';
		DetectorNavigator.discs[i].style.width														= DetectorNavigator.WEB_MINI_WIDTH + 'px';
		DetectorNavigator.discs[i].style.height 													=	DetectorNavigator.WEB_MINI_HEIGHT + 'px';
		DetectorNavigator.discs[i].style.left 														= DetectorNavigator.WEB_XOFFSET + 'px';
		DetectorNavigator.discCaptions[i].style.left
			= discCaptionOffset[0] + DetectorNavigator.WEB_MINI_WIDTH *(i%4) - 
			parseInt(DetectorNavigator.discCaptions[i].offsetWidth)/2 + 'px';
		DetectorNavigator.discCaptions[i].style.top
			= discCaptionOffset[1] + DetectorNavigator.WEB_MINI_HEIGHT *((i/4)|0) + 'px';
		DetectorNavigator.discFlags[i].style.left
			= discCaptionOffset[0] + DetectorNavigator.WEB_MINI_WIDTH *(i%4) - 
			parseInt(DetectorNavigator.discFlags[i].offsetWidth)/2 + 'px';
		DetectorNavigator.discFlags[i].style.top
			= discCaptionOffset[1] + DetectorNavigator.WEB_MINI_HEIGHT *((i/4)|0) + 50 + 'px';
  }
	for(i=0;i<6;++i){ // barrels
		DetectorNavigator.barrels[i].onmouseover													= function(){document.body.style.cursor='pointer';};//hand
		DetectorNavigator.barrels[i].onmousemove													= function(){document.body.style.cursor='pointer';};//hand
		DetectorNavigator.barrels[i].onmouseout														= function(){document.body.style.cursor='auto';};
  	DetectorNavigator.barrels[i].style.position 											= 'relative';
		DetectorNavigator.barrels[i].style.width													= DetectorNavigator.WEB_BMINI_WIDTH + 'px';
		DetectorNavigator.barrels[i].style.height 												= DetectorNavigator.WEB_BMINI_HEIGHT + 'px';
		DetectorNavigator.barrels[i].style.left 													= DetectorNavigator.WEB_XOFFSET + 'px';
		DetectorNavigator.barrelCaptions[i].style.left
			= barrelCaptionOffset[0] + DetectorNavigator.WEB_BMINI_WIDTH *i - 
			parseInt(DetectorNavigator.barrelCaptions[i].offsetWidth)/2 + 'px';
		DetectorNavigator.barrelCaptions[i].style.top	= barrelCaptionOffset[1] + 'px';
		DetectorNavigator.barrelFlags[i].style.left
			= barrelCaptionOffset[0] + DetectorNavigator.WEB_BMINI_WIDTH *i - 
			parseInt(DetectorNavigator.barrelFlags[i].offsetWidth)/2 + 'px';
		DetectorNavigator.barrelFlags[i].style.top = barrelCaptionOffset[1] + 50 + 'px';
  }
	
	DetectorNavigator.canvasOffY = DetectorNavigator.discs[0].offsetTop;//79;
	
	if(DetectorNavigator.currentSummary != "-"){
		DetectorNavigator.rocColorKeyImage.style.top = '33px';
		for(i=0;i<DetectorNavigator.rocColorKeyValuesNumber;++i)
		 DetectorNavigator.rocColorKeyValues[i].style.top = 
		  DetectorNavigator.rocColorKeyImage.offsetTop + 25 + "px";
		 
		var tmpseparation = 14;
		var tmpyoff = 1;
		DetectorNavigator.currentSummaryCaption.style.top = tmpyoff + "px";
		DetectorNavigator.summarySigma.style.top = tmpyoff + "px";
		DetectorNavigator.summaryAverage.style.top = 1*tmpseparation + tmpyoff + "px";
		DetectorNavigator.summaryTotalSamples.style.top = 2*tmpseparation + tmpyoff + "px";
		DetectorNavigator.summaryMinValue.style.top = 3*tmpseparation + tmpyoff + "px";
		DetectorNavigator.summaryMaxValue.style.top = 4*tmpseparation + tmpyoff + "px";
		
		DetectorNavigator.summaryForm.setVisible(true);			
		DetectorNavigator.summaryFormField.style.top = '1px';
			
		if(DetectorNavigator.summaryParameters[4] != -1){	
			DetectorNavigator.summaryOvfPic.style.top = "20px";
			DetectorNavigator.summaryOvfText.style.top = "20px";
			DetectorNavigator.summaryUnfPic.style.top = "20px";
			DetectorNavigator.summaryUnfText.style.top = "20px";
		}		
	}
	
	DetectorNavigator.hideFedCaptions();
} 

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.createCurrentPath = function() {
	
	if(DetectorNavigator.zoomedIndex[0] < 0 || DetectorNavigator.inOut == undefined){ //nothing
		DetectorNavigator.rocPath = "";
	}
	else if(DetectorNavigator.zoomedIndex[0] == 0){ //fpix
			//plus/minus, disc, and panel are assumed available
	
		DetectorNavigator.rocPath = "FPix_B" + DetectorNavigator.plusMinus;
	
		if(DetectorNavigator.inOut != "-")
			DetectorNavigator.rocPath += DetectorNavigator.inOut;
		DetectorNavigator.rocPath += "_D" + DetectorNavigator.disc;
		if(DetectorNavigator.blade != "-")
			DetectorNavigator.rocPath += "_BLD" + DetectorNavigator.blade;
		DetectorNavigator.rocPath += "_PNL" + DetectorNavigator.panel;
		if(DetectorNavigator.plaquette != "-")
			DetectorNavigator.rocPath += "_PLQ" + DetectorNavigator.plaquette;
		if(DetectorNavigator.roc != "-")
			DetectorNavigator.rocPath += "_ROC" + DetectorNavigator.roc;
	}
	else{ //bpix
				//plus/minus and layer is assumed available
	
		DetectorNavigator.rocPath = "BPix_B" + DetectorNavigator.plusMinus;
	
		if(DetectorNavigator.inOut != "-")
			DetectorNavigator.rocPath += DetectorNavigator.inOut;
		if(DetectorNavigator.sector != "-")
			DetectorNavigator.rocPath += "_SEC" + DetectorNavigator.sector;
		DetectorNavigator.rocPath += "_LYR" + DetectorNavigator.layer;
		if(DetectorNavigator.ladder != "-")
			DetectorNavigator.rocPath += "_LDR" + DetectorNavigator.ladder;
		if(DetectorNavigator.module != "-")
			DetectorNavigator.rocPath += "_MOD" + DetectorNavigator.module;
		if(DetectorNavigator.roc != "-")
			DetectorNavigator.rocPath += "_ROC" + DetectorNavigator.roc;
	
	}
	
	DetectorNavigator.rocPathDiv.innerHTML = DetectorNavigator.rocPath;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.showCurrentPath = function() {
	DetectorNavigator.createCurrentPath();
	DetectorNavigator.rocPathDiv.style.left = '10px';
	DetectorNavigator.rocPathDiv.style.top = '0px';
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.hideCurrentPath = function() {
	DetectorNavigator.rocPathDiv.style.top = '-100px';
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.searchBarrelForCoords = function(x,y) {

	DetectorNavigator.inOut = "-";
	DetectorNavigator.sector = "-";
	DetectorNavigator.ladder = "-";
	DetectorNavigator.module = "-";
	DetectorNavigator.roc = "-";
	
	x -= DetectorNavigator.WEB_BLYR_XOFF; 
	y -= DetectorNavigator.WEB_BLYR_YOFF;

	if(x < 0 || y < 0){
		DetectorNavigator.barrelHighlight.style.top = '-100px'; //hide highlight
		return;
	}
	
	var p = [x%(DetectorNavigator.WEB_BROC_WIDTH  + DetectorNavigator.WEB_BCOL_OFFSET),
					 y%(DetectorNavigator.WEB_BROC_HEIGHT + DetectorNavigator.WEB_BROW_OFFSET)];
	
	var col = (x/ (DetectorNavigator.WEB_BROC_WIDTH  + DetectorNavigator.WEB_BCOL_OFFSET)) | 0;
	var row = (y/ (DetectorNavigator.WEB_BROC_HEIGHT + DetectorNavigator.WEB_BROW_OFFSET)) | 0;
	
	
  DetectorNavigator.barrelCol = col;
  DetectorNavigator.barrelRow = row;
	
	var li = DetectorNavigator.zoomedIndex[1];
	var numOfRows = DetectorNavigator.BPIX_LYR_ROWS[li];
	
	if(col >= DetectorNavigator.BPIX_LYR_COLS || 
		 row >= numOfRows){
		DetectorNavigator.barrelHighlight.style.top = '-100px'; //hide highlight
		return;
	}
	
	var moduloCmp = 0;

	if(DetectorNavigator.zoomedIndex[2] == 0){
		col = DetectorNavigator.BPIX_LYR_COLS - 1 - col;
		moduloCmp = 1;
	}

	//coordinate in valid region
					
		//find inner/outer
	if(row < numOfRows/2){
		DetectorNavigator.inOut = 'I';
	}
	else{
		DetectorNavigator.inOut = 'O';	
		row = numOfRows - 1 - row;
	}

		//find sector
	if(row < DetectorNavigator.endSectorRows[li]){ //first sector
		DetectorNavigator.sector = 1; 
	}
	else{
		row -= DetectorNavigator.endSectorRows[li];
		DetectorNavigator.sector = 2 + ((row/DetectorNavigator.otherSectorRows[li])|0);
		if(DetectorNavigator.sector > 8)
			DetectorNavigator.sector = 8;
			
		if(li == 2 && DetectorNavigator.sector >= 4){
			row -= DetectorNavigator.otherSectorRows[li]*2;
			if(row < 8){ //in sector 4 or 5
				DetectorNavigator.sector = 4 + ((row/4)|0);
				row = row%4;
			}
			else{
				DetectorNavigator.sector = 6 + (((row-8)/DetectorNavigator.otherSectorRows[li])|0);
				row = (row-8)%DetectorNavigator.otherSectorRows[li];			
			}
		}
		else{
			row -= DetectorNavigator.otherSectorRows[li]*(DetectorNavigator.sector-2);		
		}
	}

	//row now is the row withing the sector

	var doubleRow = ((row/2)|0);
	
		//find ladder
	if(DetectorNavigator.sector == 1){
		if(row == 0){
			DetectorNavigator.ladder = '1H'
		}
		else{
			DetectorNavigator.ladder = (2 + (((row-1)/2)|0)) + 'F';		
		}
	}
	else if(li == 2 && DetectorNavigator.sector >= 4){
		if(DetectorNavigator.sector <= 5){
			var startLadder = 10;
			DetectorNavigator.ladder = (startLadder + (DetectorNavigator.sector-4)*2 + doubleRow) + 'F';
		}
		else{
			var startLadder = 14;
			DetectorNavigator.ladder = (startLadder +
				(DetectorNavigator.sector-6)*DetectorNavigator.otherSectorRows[li]/2 + doubleRow) + 'F';
			if(DetectorNavigator.ladder == "22F"){
				DetectorNavigator.ladder = "22H";
			}
		}
	}
	else if(DetectorNavigator.sector == 8){

		DetectorNavigator.ladder = ((DetectorNavigator.endSectorRows[li]/2)|0) + 1 +
			(((numOfRows/2 - DetectorNavigator.endSectorRows[li])/2)|0) + doubleRow;
	
		if(row == DetectorNavigator.endSectorRows[li] - 1){
			DetectorNavigator.ladder += 'H';
		}
		else{
			DetectorNavigator.ladder += 'F';
		}
	}	
	else{
		DetectorNavigator.ladder = (((DetectorNavigator.endSectorRows[li]/2)|0) + 2 + 
			(DetectorNavigator.sector-2)*DetectorNavigator.otherSectorRows[li]/2 + doubleRow) + 'F';
	}
	
		//find module
	DetectorNavigator.module = ((col/8)|0) + 1;
	
		//find ROC	
	if(p[0] < DetectorNavigator.WEB_BROC_WIDTH && 
		 p[1] < DetectorNavigator.WEB_BROC_HEIGHT){
		
		if( (DetectorNavigator.inOut == 'I' && DetectorNavigator.sector == 1 && row != 0 && row%2 == moduloCmp) || 
				(DetectorNavigator.inOut == 'I' && DetectorNavigator.sector == 8 && 
					row != DetectorNavigator.endSectorRows[li] - 1 && row%2 == 1 - moduloCmp) || 
				(DetectorNavigator.inOut == 'I' && DetectorNavigator.sector != 8 &&
					DetectorNavigator.sector != 1 && row%2 == 1 - moduloCmp) || 
				(DetectorNavigator.inOut == 'O' && DetectorNavigator.sector == 1 && row != 0 && row%2 == 1 - moduloCmp) || 
				(DetectorNavigator.inOut == 'O' && DetectorNavigator.sector == 8 &&
					row != DetectorNavigator.endSectorRows[li] - 1 && row%2 == moduloCmp) || 
				(DetectorNavigator.inOut == 'O' && DetectorNavigator.sector != 8 &&
					DetectorNavigator.sector != 1 && row%2 == moduloCmp)){
			DetectorNavigator.roc = 8+(col%8);
		}
		else{
			DetectorNavigator.roc = 7-(col%8);
		}		

			//place highlight
		DetectorNavigator.barrelHighlight.style.left = DetectorNavigator.canvasOffX + DetectorNavigator.WEB_BLYR_XOFF - 1 +
			DetectorNavigator.safariSubstractOne + 
			DetectorNavigator.barrelCol*(DetectorNavigator.WEB_BROC_WIDTH  + DetectorNavigator.WEB_BCOL_OFFSET) + 'px';
		DetectorNavigator.barrelHighlight.style.top = DetectorNavigator.canvasOffY + DetectorNavigator.WEB_BLYR_YOFF - 2 +
			DetectorNavigator.barrelRow*(DetectorNavigator.WEB_BROC_HEIGHT + DetectorNavigator.WEB_BROW_OFFSET) + 'px';
		document.body.style.cursor='pointer';
	}
	else{
		DetectorNavigator.barrelHighlight.style.top = '-100px'; //hide highlight
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.clickRoc = function() {
	if(DetectorNavigator.roc == "-"){
		alert("Illegal Roc Click Call.");
		return;
	}
		
	var detIndex = DetectorNavigator.zoomedIndex[1]*2 + DetectorNavigator.zoomedIndex[2];
	var pushVal; //create an integer value that represents the ROC
	if(DetectorNavigator.zoomedIndex[0] == 0){//fpix
		DetectorNavigator.imageUpdateIndex = DetectorNavigator.zoomedIndex[1]*2 + DetectorNavigator.zoomedIndex[2];
		
		var bladeSize = DetectorNavigator.panel == 1 ? 21:24;
		var blade = DetectorNavigator.inOut == 'O' ? 13+(12-DetectorNavigator.blade):DetectorNavigator.blade;
		--blade;
		
		var plaqOff;
		var rowSize;
		if(DetectorNavigator.panel == 1){
			plaqOff = [0,2,8,16];
			rowSize = [2,3,4,5];
		}
		else{
			plaqOff = [0,6,14];	
			rowSize = [3,4,5];	
		}
		
		var roc = DetectorNavigator.roc < rowSize[DetectorNavigator.plaquette-1] ? 
			rowSize[DetectorNavigator.plaquette-1]-1-DetectorNavigator.roc : DetectorNavigator.roc;

		pushVal = blade*bladeSize + (bladeSize-1) - (plaqOff[DetectorNavigator.plaquette-1] + roc);
	}
	else{//bpix
		DetectorNavigator.imageUpdateIndex = -(DetectorNavigator.zoomedIndex[1]*2 + DetectorNavigator.zoomedIndex[2]) - 1;
		
		pushVal = DetectorNavigator.barrelRow*DetectorNavigator.BPIX_LYR_COLS + DetectorNavigator.barrelCol;
	}
	
	if(DetectorNavigator.clickMask[DetectorNavigator.zoomedIndex[0]][detIndex].indexOf(pushVal) == -1){ //add if necessary
		DetectorNavigator.clickMask[DetectorNavigator.zoomedIndex[0]][detIndex].push(pushVal);
		DetectorNavigator.clickMask[DetectorNavigator.zoomedIndex[0]][detIndex].sort(function(a,b){return a - b});
	}
	
	Ext.Ajax.request({
		url:     'XGI_DetectorRocRequest',
		success: DetectorNavigator.ajaxRocResponse,
		failure: ajaxFailure,
		params:  {
						   rocpath:   DetectorNavigator.rocPath,
							 clickmask: DetectorNavigator.clickMask[DetectorNavigator.zoomedIndex[0]][detIndex].toString(),
							 clicksize: DetectorNavigator.clickMask[DetectorNavigator.zoomedIndex[0]][detIndex].length,
							 summary:   DetectorNavigator.currentSummary,
							 field:     DetectorNavigator.currentSummaryField
						 }
	});
	
	//open up directory
	var expandStr = DetectorNavigator.createExpandString();
	FileTreeWindow.contentTree.expandPath(expandStr,"text",DetectorNavigator.expandCallBack);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.expandCallBack = function(success,rocFolder) {	

	if(success){
		
		var objects = "";
		var count = 0;
	  var rocName = DetectorNavigator.rocPath;

		rocFolder.eachChild(function(child){
		  if(child.id.indexOf(rocName) != -1){
			  var regExp=new RegExp(rocName+"[0-9]");
        if(child.id.match(regExp) == null){
		      objects += child.id + "--"; 
				  ++count;
        }
		  }
		});
//		rocFolder.eachChild(function(child){objects += child.id; ++count;});
		
		Canvas.add("Multi-Canvas",objects);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.createExpandString = function(){
	var retStr = "/contentRoot/";
	var recursiveStr;
	var pathStr = DetectorNavigator.rocPath;
	var i;
	
	recursiveStr = pathStr.slice(0,4);
	retStr += recursiveStr + "/";
	pathStr = pathStr.slice(5);
	
	while((i = pathStr.indexOf("_")) != -1){
		//if(pathStr[i-1] == "I" || pathStr[i-1] == "O") --i;
		
		recursiveStr += "_" + pathStr.slice(0,i);
//		recursiveStr = pathStr.slice(0,i);
		retStr += recursiveStr + "/";
		
		//if(pathStr[i] == "I" || pathStr[i] == "O"){ 
		//	++i; recursiveStr += pathStr[i-1];
		//	retStr += recursiveStr + "/";
		//}
		
		pathStr = pathStr.slice(i+1);
	
	}
//	recursiveStr += "_" + pathStr;
//	recursiveStr = pathStr;
//	retStr += recursiveStr;
	retStr = retStr.slice(0,retStr.length-1);
	return retStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.clearClickMask = function() {	
		//remove all ROC's from clickMask
	for(i=0;i<8;++i){
		if(DetectorNavigator.clickMask[0][i].length != 0)
			DetectorNavigator.clickMask[0][i].splice(0,DetectorNavigator.clickMask[0][i].length);
	}
	for(i=0;i<6;++i){
		if(DetectorNavigator.clickMask[1][i].length != 0)
			DetectorNavigator.clickMask[1][i].splice(0,DetectorNavigator.clickMask[1][i].length);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.ajaxRocResponse = function(ajaxObj) {	
	
	//DetectorNavigator.imageUpdateIndex is the positive index of the disc for FPix
	//DetectorNavigator.imageUpdateIndex is the negative index minus 1 of the layer for BPix
	if(ajaxObj.responseText == "[]"){
		alert("Requested Producer not found. The producer's name may have changed - try refreshing the file list.");
		return;
	}
	
	if(DetectorNavigator.imageUpdateIndex >= 0){ // fpix
		DetectorNavigator.discs[DetectorNavigator.imageUpdateIndex].src = ajaxObj.responseText;
	}
	else{ // bpix
		++DetectorNavigator.imageUpdateIndex;
		DetectorNavigator.imageUpdateIndex *= -1;
		DetectorNavigator.barrels[DetectorNavigator.imageUpdateIndex].src = ajaxObj.responseText;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.paintWithTree = function(summaryNode){

	if(summaryNode.text.indexOf("Field - ") < 0){ //is the summary root
		
		//remove current children of summary node
		var numOfChildren = summaryNode.childNodes.length;
		for(i=numOfChildren-1;i>=0;--i)
			summaryNode.removeChild(summaryNode.childNodes[i]);
		
		DetectorNavigator.currentSummaryNode = summaryNode;
		Ext.Ajax.request({
			url:     'XGI_RequestHistogram',
			success: DetectorNavigator.populateFieldNodes,
			failure: ajaxFailure,
			params:  {
									histoname: summaryNode.id,
									field: ""
								}
		});	
	}
	else{ //field is requested
		DetectorNavigator.currentSummary = summaryNode.id.slice(0,summaryNode.id.lastIndexOf("/"));
		DetectorNavigator.currentSummaryField = summaryNode.id.slice(summaryNode.id.lastIndexOf("/")+1);

		Ext.Ajax.request({
			url:     'XGI_RequestHistogram',
			success: DetectorNavigator.reload,
			failure: ajaxFailure,
			params:  {
									histoname: DetectorNavigator.currentSummary,
									field: DetectorNavigator.currentSummaryField,
									requestId: escape(HistoViewer.globalRequestId)
								}
		});		
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.populateFieldNodes = function(ajaxObj){

	var fieldStr = ajaxObj.responseText.slice(1);
	var node;
	var i;
	var field;
	while(((i = fieldStr.indexOf(",")) != -1 || (i = fieldStr.indexOf("]")) != -1) && i != 0){
		field = fieldStr.slice(0,i);
		if(i < fieldStr.length)
			fieldStr = fieldStr.slice(i+1);	

		DetectorNavigator.currentSummaryNode.appendChild(node = new Ext.tree.TreeNode({
  		text: 			'Field - ' + field,
  		id: 				DetectorNavigator.currentSummaryNode.id + "/" + field,
  		leaf: 			true,
  		cls:  			'file',
		}));
	
		node.addListener("click",FileTreeWindow.onContentClick);	
	}
	
	DetectorNavigator.currentSummaryNode.expand();	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.reload = function(ajaxObj){
		
	DetectorWindow.showWindow();
	if(!DetectorNavigator.isFirstLoad){ //window already loaded so need to reload.
		DetectorNavigator.isFirstLoad = true;
		DetectorWindow.win.load({
		  	url: "XGI_DetectorNavigator?requestId=" + 
					escape(HistoViewer.globalRequestId),
			});
  }	
		
	DetectorNavigator.clearClickMask();	
	
	if(ajaxObj){	//get stats for summary
		var fieldStr = ajaxObj.responseText; 
		var i;
		var field;
		var fieldNum = 0;
		while((i = fieldStr.indexOf(",")) != -1){
			field = fieldStr.slice(0,i);
			fieldStr = fieldStr.slice(i+1);
			DetectorNavigator.summaryParameters[fieldNum++] = field;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.SetSigmaThreshold = function(){
	Ext.Ajax.request({
		url:     'XGI_RequestHistogram',
		success: DetectorNavigator.reload,
		failure: ajaxFailure,
		params:  {
								histoname: DetectorNavigator.currentSummary,
								field: DetectorNavigator.currentSummaryField,
								requestId: escape(HistoViewer.globalRequestId),
								filter: DetectorNavigator.summaryForm.items.items[0].getValue()
							}
	});
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.initFedCaptions = function(){

	if(!DetectorNavigator.fedCaptionTops[0]){ //only initialize values once
		var dir;
		var radius = 195;
		var ang = Math.PI/24 - Math.PI/2;
		var tmp;			
		var xoffset = -10;
		var yoffset = 18;
		
		DetectorNavigator.fedViewMode = true;
		
		for(i=0;i<12;++i){
			dir = [1,Math.tan(ang)];
			tmp = Math.sqrt(dir[0]*dir[0] + dir[1]*dir[1]);
			dir[0] /= tmp;
			dir[1] /= tmp;
			
			DetectorNavigator.fedCaptionLefts[i] = DetectorNavigator.WEB_DISC_X + 
				dir[0]*radius + xoffset - 1000 + "px";
			DetectorNavigator.fedCaptionTops[i] = DetectorNavigator.WEB_DISC_Y + 
				dir[1]*radius + yoffset + "px";
			DetectorNavigator.fedCaptionLefts[i+12] = DetectorNavigator.WEB_DISC_X - 
				dir[0]*radius + xoffset - 1000 + "px";
			DetectorNavigator.fedCaptionTops[i+12] = DetectorNavigator.WEB_DISC_Y - 
				dir[1]*radius + yoffset + "px";
			ang += Math.PI/12;
		}
	}
	
	for(i=0;i<24;++i){
		DetectorNavigator.fedCaptions[i] = document.getElementById('fedCaption'+i);		
		DetectorNavigator.fedCaptions[i].style.top = DetectorNavigator.fedCaptionTops[i];
		DetectorNavigator.fedCaptions[i].style.left = DetectorNavigator.fedCaptionLefts[i];
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.displayFedCaptions = function(){

	if(DetectorNavigator.fedCaptions[0].offsetLeft > 0 || DetectorNavigator.zoomedIndex[0] != 0) //only show if hidden
		return;
		
	//zoomedIndex: 0,1 for Fpix,Bpix, 0-4,3 for disc,layer, 0,1 for left,right

	Ext.Ajax.request({
		url:     'XGI_RequestFedAssignment',
		success: DetectorNavigator.displayFedCallback,
		failure: ajaxFailure,
		params:  {
								disc: DetectorNavigator.zoomedIndex[1],
								inner: DetectorNavigator.zoomedIndex[2]
							}
	});
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.displayFedCallback = function(ajaxObj){

	var ps = new Array();
	if(ajaxObj){	//get stats for summary
		var fieldStr = ajaxObj.responseText; 
		var i;
		var field;
		var fieldNum = 0;
		while((i = fieldStr.indexOf(",")) != -1){
			field = fieldStr.slice(0,i);
			fieldStr = fieldStr.slice(i+1);
			ps[fieldNum++] = field;
		}
	}
	
	if(fieldNum != 24){
		alert("DetectorNavigator.displayFedCallback: invalid field amount");return;}		
	
	for(i=0;i<24;++i){
		DetectorNavigator.fedCaptions[i].innerHTML = ps[i];
		DetectorNavigator.fedCaptions[i].style.left =
			DetectorNavigator.fedCaptions[i].offsetLeft + 1000 + "px";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.hideFedCaptions = function(){
	if(DetectorNavigator.fedCaptions[0].offsetLeft > 0) //only hide if shown
		for(i=0;i<24;++i){
			DetectorNavigator.fedCaptions[i].style.left = 
				DetectorNavigator.fedCaptions[i].offsetLeft - 1000 + "px";
		}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
DetectorNavigator.expandFedPath = function(i){
	i = DetectorNavigator.fedCaptions[i].innerHTML;
	var c = i.indexOf(":");
	var fed = i.slice(0,c);
	var ch = i.slice(c+1);
	var expandStr = "/contentRoot/" + "FED" + fed + "_Channel" + ch;

	FileTreeWindow.contentTree.expandPath(expandStr,"text",DetectorNavigator.expandCallBack);
}
	




