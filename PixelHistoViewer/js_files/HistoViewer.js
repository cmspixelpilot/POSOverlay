var viewDoc;
var HistoViewer = {};
HistoViewer.globalRequestId;

document.oncontextmenu = function(){return false;}; //prevent right click menus everywhere on page

var keyPressOffset = 0;

HistoViewer.turtle;

////////////////////////////////////
Ext.onReady(function(){

	HistoViewer.globalRequestId = escape((new Date()).getTime());
	
	viewDoc = Ext.getDoc(); //get global handle to document
	viewDoc.addListener("scroll",onScroll);
	
	if("Apple Computer, Inc." == navigator.vendor){ // fix for safari getKey issue
		keyPressOffset = 64;
	}
	viewDoc.addListener("keypress",onKeyPress);
	
	Console.init();
	
	FileTreeWindow.createWindow();
	Console.add(FileTreeWindow);
	DetectorWindow.createWindow();
	Console.add(DetectorWindow); //add uncreated(for loading time) detector window	
	Canvas.createControlPanel();
	Console.add(Canvas);	
	
	HistoViewer.loadCookies();		
	
	HistoViewer.turtle = document.getElementById('turtle');
	HistoViewer.turtle.motion = false;
	HistoViewer.turtle.x = 0; 
	HistoViewer.turtle.y = 0;
	HistoViewer.turtleRedraw();	
});

////////////////////////////////////
Ext.EventManager.onWindowResize(function(){
	Console.redraw();
	HistoViewer.turtleRedraw();
});

////////////////////////////////////
function onScroll(){
	Console.redraw();
	HistoViewer.turtleRedraw();	
}

////////////////////////////////////
function onKeyPress(obj){
	if(obj.shiftKey && obj.ctrlKey){ //to access special keys hold SHIFT and CTRL
		var qk = String.fromCharCode(obj.getKey()+keyPressOffset);
		switch(qk){
		
			case 'H': //help window toggle
				HelpWindow.createWindow();
				break;
			case 'Z': //autoarrange style 1
				Console.autoArrangeWindows(1);
				break;
			case 'S': //autoarrange style 2
				Console.autoArrangeWindows(2);
				break;
			case 'M': //minimize all windows
				Console.minimizeAllWindows();
				break;
			case 'N': //show all windows
				Console.showAllWindows();
				break;
			case 'U': //close all histograms
				Console.closeAllOthers();
				break;
			case 'O': //scroll focus left
				Console.scrollWindowFocus(-1);
				break;
			case 'P': //scroll focus right
				Console.scrollWindowFocus(1);
				break;
			case 'C': //save cookies
				HistoViewer.saveCookies();
				break;			
			case 'D': //delete cookies
				HistoViewer.saveCookies(1);
				break;
			default:
				alert("Not a valid Quick Key: " + qk + ".\n(Verify Caps Lock is off)");
				break;		
		}	
		Console.redraw(); //redraw console in case quick keys affected window size/position
	}
}

////////////////////////////////////
ajaxFailure = function(ajaxObj){
	alert("Ajax Failure: No server response");
}

//*************************************** COOKIES ********************************************
	//load/save settings for...
	
		//window positions
			//File Nav (visible, pos, size, dividesize)
			//Detector (visible, pos, default zoom?)
			//Canvas controls (visible, pos, size)
			//help window(visible, pos, size)
			//Canvas start coordinate
			
		//Canvas Control parameters
		
		//window stack-up
		
////////////////////////////////////
HistoViewer.saveCookies = function(defaults){
	var expDate = new Date();
	expDate.setDate(expDate.getDate()+365); //expires in one year
	expDate = ' expires=' + expDate.toGMTString() + '; path=/';
	
	if(defaults){
		//document.cookie = 'default=0;' + expDate;	
		document.cookie = 'default=1;' + expDate;		
		expDate = ' expires=' + new Date().toGMTString() + '; path=/'; //erase others
	
		document.cookie = 'FileTreeWindow.win.visible=0;' + expDate;
		document.cookie = 'FileTreeWindow.win.maximized=0;' + expDate;
		document.cookie = 'FileTreeWindow.win.dwidth=100;' + expDate;
		document.cookie = 'FileTreeWindow.win.x=400;' + expDate;
		document.cookie = 'FileTreeWindow.win.y=300;' + expDate;
		document.cookie = 'FileTreeWindow.win.w=300;' + expDate;
		document.cookie = 'FileTreeWindow.win.h=200;' + expDate;
		document.cookie = 'FileTreeWindow.flatDirs=1;' + expDate;
		
		document.cookie = 'DetectorNavigator.win.visible=0;' + expDate;
		document.cookie = 'DetectorNavigator.win.x=100;' + expDate;
		document.cookie = 'DetectorNavigator.win.y=20;' + expDate;
		document.cookie = 'DetectorNavigator.zoom.0=0;' + expDate;
		document.cookie = 'DetectorNavigator.zoom.1=1;' + expDate;
		document.cookie = 'DetectorNavigator.zoom.2=0;' + expDate;		
		
		document.cookie = 'CanvasControls.win.visible=1;' + expDate;
		document.cookie = 'CanvasControls.win.x=400;' + expDate;
		document.cookie = 'CanvasControls.win.y=10;' + expDate;
		document.cookie = 'CanvasControls.win.w=300;' + expDate;
		document.cookie = 'CanvasControls.win.h=200;' + expDate;
		
		document.cookie = 'HelpWindow.win.visible=0;' + expDate;
		document.cookie = 'HelpWindow.win.maximized=0;' + expDate;
		document.cookie = 'HelpWindow.win.x=400;' + expDate;
		document.cookie = 'HelpWindow.win.y=300;' + expDate;
		document.cookie = 'HelpWindow.win.w=300;' + expDate;
		document.cookie = 'HelpWindow.win.h=200;' + expDate;
		
		document.cookie = 'Canvas.win.initx=100;' + expDate;	
		document.cookie = 'Canvas.win.inity=100;' + expDate;	
		
		for(var i=0;i<4;++i)
			document.cookie = 'Stack.'+i+'=-1;' + expDate;

		document.cookie = 'CanvasControl.params=;' + expDate;
		
		
		alert("Configuration cleared to default.");			
	}
	else{
	
		//TODO::::rank windows by lastZIndex to get z order
		
		
		var tmp;
		var state;
		var win;
		var x,y,w,h,d;
		
		var stacki = 0;
		var stack = [[-1,-1],[-1,-1],[-1,-1],[-1,-1]]; 
			//4 possible windows at start-up (front to back order)
					//0 - FileTreeWindow
					//1 - Detector
					//2 - Canvas Controls
					//3 - Help
					//-1 - none
			//also store lastZIndex for comparation
		
		document.cookie = 'default=0;' + expDate;
		
			//File Nav -----------------------------------------------
		win = FileTreeWindow.win;
		if(win.maximized)
			state = 2; //window maximized
		else if(win.isVisible())
			state = 1; //window is visible
		else
			state = 0; //window hidden
		document.cookie = 'FileTreeWindow.win.visible=' + 
			(win.isVisible()?1:0) + ';' + expDate;
		document.cookie = 'FileTreeWindow.win.maximized=' + 
			(win.maximized?1:0) + ';' + expDate;
		
		if(state == 2){ //max
			x = win.restorePos[0];
			y = win.restorePos[1];
			w = win.restoreSize.width;
			h = win.restoreSize.height;
			d = FileTreeWindow.fileTree.getInnerWidth();
		}
		else if(state == 1){ //visible
			x = win.getPosition()[0];
			y = win.getPosition()[1];
			w = win.getSize().width;
			h = win.getSize().height;
			d = FileTreeWindow.fileTree.getInnerWidth();
		}
		else{
			if(win.x){
				x = win.x;
				y = win.y;
				w = win.lastSize.width;
				h = win.lastSize.height;
				d = FileTreeWindow.fileTree.lastSize.width;
			}
			else{ //size not yet defined
				x = "";
				y = "";
				w = win.width;
				h = win.height;	
				d = FileTreeWindow.fileTree.width;
			}			
		}
		
		if(d < 50) //min width (dont allow default to collapsed because no way to navigate)
		 d = 50;
				
		document.cookie = 'FileTreeWindow.win.x=' + x + ';' + expDate;
		document.cookie = 'FileTreeWindow.win.y=' + y + ';' + expDate;
		document.cookie = 'FileTreeWindow.win.w=' + w + ';' + expDate;
		document.cookie = 'FileTreeWindow.win.h=' + h + ';' + expDate;
		document.cookie = 'FileTreeWindow.win.dwidth=' + d + ';' + expDate;
		document.cookie = 'FileTreeWindow.flatDirs=' + 
			(FileTreeWindow.flattenFileDirs?1:0) + ';' + expDate;
		
		if(state != 0){
			stack[0][0] = 0;
			stack[0][1] = win.lastZIndex;		
			++stacki;
		}
			//File Nav End -----------------------------------------------
		
			//Detector Nav -----------------------------------------------
		win = DetectorWindow.win;
		document.cookie = 'DetectorNavigator.win.visible=' + 
			(win.isVisible()?1:0) + ';' + expDate;
		if(win.isVisible()){
			document.cookie = 'DetectorNavigator.win.x=' + 
				win.getPosition()[0] + ';' + expDate;
			document.cookie = 'DetectorNavigator.win.y=' + 
				win.getPosition()[1] + ';' + expDate;
			document.cookie = 'DetectorNavigator.zoom.0=' + 
				DetectorNavigator.zoomedIndex[0] + ';' + expDate;
			document.cookie = 'DetectorNavigator.zoom.1=' + 
				DetectorNavigator.zoomedIndex[1] + ';' + expDate;
			document.cookie = 'DetectorNavigator.zoom.2=' + 
				DetectorNavigator.zoomedIndex[2] + ';' + expDate;
			
			var i = stacki;
			while(i > 0 && win.lastZIndex > stack[i-1][1]){ //new high so move old high down
				stack[i][0] = stack[i-1][0];
				stack[i][1] = stack[i-1][1];
				--i;
			}
			stack[i][0] = 1;
			stack[i][1] = win.lastZIndex;		
			++stacki;
			
		}
		else{
			document.cookie = 'DetectorNavigator.win.x=;' + expDate;
			document.cookie = 'DetectorNavigator.win.y=;' + expDate;
			document.cookie = 'DetectorNavigator.zoom.0=;' + expDate;
			document.cookie = 'DetectorNavigator.zoom.1=;' + expDate;
			document.cookie = 'DetectorNavigator.zoom.2=;' + expDate;
		}
			//Detector Nav End -----------------------------------------------
	
			//Canvas Controls -----------------------------------------------
		win = Canvas.win;
		if(win.isVisible()){ //visible
			x = win.getPosition()[0];
			y = win.getPosition()[1];
			w = win.getSize().width;
			h = win.getSize().height;
			
			var i = stacki;
			while(i > 0 && win.lastZIndex > stack[i-1][1]){ //new high so move old high down
				stack[i][0] = stack[i-1][0];
				stack[i][1] = stack[i-1][1];
				--i;
			}
			stack[i][0] = 2;
			stack[i][1] = win.lastZIndex;		
			++stacki;			
		}
		else{
			if(win.x){
				x = win.x;
				y = win.y;
			}
			else{ //size not yet defined
				x = "";
				y = "";
			}
			if(win.lastSize && win.lastSize.width)
				w = win.lastSize.width;
			else
				w = win.width;
			if(win.lastSize && win.lastSize.height)
				h = win.lastSize.height;
			else
				h = win.height;
		}
		document.cookie = 'CanvasControls.win.visible=' + 
			(win.isVisible()?1:0) + ';' + expDate;
		document.cookie = 'CanvasControls.win.x=' + x + ';' + expDate;
		document.cookie = 'CanvasControls.win.y=' + y + ';' + expDate;
		document.cookie = 'CanvasControls.win.w=' + w + ';' + expDate;
		document.cookie = 'CanvasControls.win.h=' + h + ';' + expDate;
			//Canvas Controls End -----------------------------------------------
			
			//Help Win -----------------------------------------------
		win = HelpWindow.win;
		if(!win)
			state = 0;
		else if(win.maximized)
			state = 2; //window maximized
		else
			state = 1; //window is visible
	
		d = 1; tmp = 0;
		if(state == 2){ //max
			x = win.restorePos[0];
			y = win.restorePos[1];
			w = win.restoreSize.width;
			h = win.restoreSize.height;
			tmp = 1;
		}
		else if(state == 1){ //visible
			x = win.getPosition()[0];
			y = win.getPosition()[1];
			w = win.getSize().width;
			h = win.getSize().height;
		}
		else{
			x = "";
			y = "";
			w = "";
			h = "";		
			d = 0;
		}	
		
		document.cookie = 'HelpWindow.win.visible=' + 
			d + ';' + expDate;
		document.cookie = 'HelpWindow.win.maximized=' + 
			tmp + ';' + expDate;	
		document.cookie = 'HelpWindow.win.x=' + x + ';' + expDate;
		document.cookie = 'HelpWindow.win.y=' + y + ';' + expDate;
		document.cookie = 'HelpWindow.win.w=' + w + ';' + expDate;
		document.cookie = 'HelpWindow.win.h=' + h + ';' + expDate;		
		
		if(state != 0){
			var i = stacki;
			while(i > 0 && win.lastZIndex > stack[i-1][1]){ //new high so move old high down
				stack[i][0] = stack[i-1][0];
				stack[i][1] = stack[i-1][1];
				--i;
			}
			stack[i][0] = 3;
			stack[i][1] = win.lastZIndex;		
			++stacki;		
		}
			//Help Win End -----------------------------------------------
		
			//Canvas start coordinate -----------------------------------------------
		if(Canvas.canvasVector.length > 0){ //already a canvas to align to
			document.cookie = 'Canvas.win.initx=' + 
				Canvas.canvasVector[0].win.getPosition()[0] + ';' + expDate;	
			document.cookie = 'Canvas.win.inity=' + 
				Canvas.canvasVector[0].win.getPosition()[1] + ';' + expDate;
		}
		else{
			document.cookie = 'Canvas.win.initx=;' + expDate;	
			document.cookie = 'Canvas.win.inity=;' + expDate;	
		}									 
			//Canvas start coordinate end -----------------------------------------------
			
			//window stack up -----------------------------------------------		
		for(var i=0;i<4;++i)
			document.cookie = 'Stack.'+i+'=' + stack[i][0] +';' + expDate;									 
			//window stack up end -----------------------------------------------
			
			//Canvas Controls -----------------------------------------------		
		var canvasCtrlStr = ""
		canvasCtrlStr += 'alwaysNewWindow=' + (Canvas.alwaysNewWindow?1:0) + '+';
		canvasCtrlStr += 'useCanvasCache=' + Canvas.useCanvasCache + '+';
		canvasCtrlStr += 'updateOnResize=' + (Canvas.updateOnResize?1:0) + '+';
		canvasCtrlStr += 'initToDefault=' + (Canvas.initToDefault?1:0) + '+';
		canvasCtrlStr += 'xmin=' + Canvas.xmin + '+';
		canvasCtrlStr += 'xmax=' + Canvas.xmax + '+';
		canvasCtrlStr += 'ymin=' + Canvas.ymin + '+';
		canvasCtrlStr += 'ymax=' + Canvas.ymax + '+';
		canvasCtrlStr += 'zmin=' + Canvas.zmin + '+';
		canvasCtrlStr += 'zmax=' + Canvas.zmax + '+';
		canvasCtrlStr += 'marginLt=' + Canvas.marginLt + '+';
		canvasCtrlStr += 'marginUp=' + Canvas.marginUp + '+';
		canvasCtrlStr += 'marginRt=' + Canvas.marginRt + '+';
		canvasCtrlStr += 'marginDn=' + Canvas.marginDn + '+';
		canvasCtrlStr += 'xlog=' + Canvas.xlog + '+';
		canvasCtrlStr += 'ylog=' + Canvas.ylog + '+';
		canvasCtrlStr += 'zlog=' + Canvas.zlog + '+';
		canvasCtrlStr += 'xtick=' + Canvas.xtick + '+';
		canvasCtrlStr += 'ytick=' + Canvas.ytick + '+';
		canvasCtrlStr += 'xgrid=' + Canvas.xgrid + '+';
		canvasCtrlStr += 'ygrid=' + Canvas.ygrid + '+';
		canvasCtrlStr += 'draw=' + Canvas.draw + '+';
		canvasCtrlStr += 'prependA=' + Canvas.prependA + '+';  
		
		document.cookie = 'CanvasControl.params=' + canvasCtrlStr + ';' + expDate;
				//Canvas Controls end -----------------------------------------------	
		
		alert("Configuration successfully saved to cookies.");	
	}	
	//alert(document.cookie);
	
}

////////////////////////////////////
HistoViewer.loadCookies = function(){	
	
	tmp = HistoViewer.getCookie("default",document.cookie);
	if(tmp == "" || parseInt(tmp))
		return; //use default settings
	
	var tmp;
	var win;
	
	//load settings
	
		//File Nav -----------------------------------------------
	if((tmp = HistoViewer.getCookie("FileTreeWindow.flatDirs",document.cookie)) != "")
		FileTreeWindow.flattenFileDirs = parseInt(tmp);
			
	FileTreeWindow.minimizeMaximizeWindow(); //prep window for changes	
	win = FileTreeWindow.win;
	if((tmp = HistoViewer.getCookie("FileTreeWindow.win.visible",document.cookie)) != "")
		win.setVisible(parseInt(tmp));
	if((tmp = HistoViewer.getCookie("FileTreeWindow.win.dwidth",document.cookie)) != "")
		FileTreeWindow.fileTree.setWidth(parseInt(tmp));
	if((tmp = HistoViewer.getCookie("FileTreeWindow.win.x",document.cookie)) != "")
		win.setPosition(parseInt(tmp),win.getPosition()[1]);
	if((tmp = HistoViewer.getCookie("FileTreeWindow.win.y",document.cookie)) != "")
		win.setPosition(win.getPosition()[0],parseInt(tmp));
	if((tmp = HistoViewer.getCookie("FileTreeWindow.win.w",document.cookie)) != "")
		win.setWidth(parseInt(tmp));
	if((tmp = HistoViewer.getCookie("FileTreeWindow.win.h",document.cookie)) != "")
		win.setHeight(parseInt(tmp));
	if((tmp = HistoViewer.getCookie("FileTreeWindow.win.maximized",document.cookie)) != "")
		if(parseInt(tmp))
			win.maximize();
	win.syncSize();
		//File Nav End -----------------------------------------------
		
		//Detector Nav -----------------------------------------------
	win = DetectorWindow.win;
	if((tmp = HistoViewer.getCookie("DetectorNavigator.win.visible",document.cookie)) != "")	
		if(parseInt(tmp)){ //only set position if window is visible to avoid unnessarily loading full navigator
			var z = new Array();
			if( (z[0] = HistoViewer.getCookie("DetectorNavigator.zoom.0",document.cookie)) != "" &&
					(z[1] = HistoViewer.getCookie("DetectorNavigator.zoom.1",document.cookie)) != "" &&
					(z[2] = HistoViewer.getCookie("DetectorNavigator.zoom.2",document.cookie)) != "" )
				DetectorNavigator.zoomedIndex = [parseInt(z[0]),parseInt(z[1]),parseInt(z[2])];			
			DetectorWindow.minimizeMaximizeWindow(); //after zoomIndices are set
			if((tmp = HistoViewer.getCookie("DetectorNavigator.win.x",document.cookie)) != "")
				win.setPosition(parseInt(tmp),win.getPosition()[1]);
			if((tmp = HistoViewer.getCookie("DetectorNavigator.win.y",document.cookie)) != "")
				win.setPosition(win.getPosition()[0],parseInt(tmp));
			
		}		
		//Detector Nav End -----------------------------------------------
	
		//Canvas Controls -----------------------------------------------
	Canvas.minimizeMaximizeWindow(); //prep window for changes	
	win = Canvas.win;
	if((tmp = HistoViewer.getCookie("CanvasControls.win.visible",document.cookie)) != "")
		win.setVisible(parseInt(tmp));
	if((tmp = HistoViewer.getCookie("CanvasControls.win.x",document.cookie)) != "")
		win.setPosition(parseInt(tmp),win.getPosition()[1]);
	if((tmp = HistoViewer.getCookie("CanvasControls.win.y",document.cookie)) != "")
		win.setPosition(win.getPosition()[0],parseInt(tmp));
	if((tmp = HistoViewer.getCookie("CanvasControls.win.w",document.cookie)) != "")
		win.setWidth(parseInt(tmp));
	if((tmp = HistoViewer.getCookie("CanvasControls.win.h",document.cookie)) != "")
		win.setHeight(parseInt(tmp));
		//Canvas Controls End -----------------------------------------------
		
		//Help Win -----------------------------------------------
	if((tmp = HistoViewer.getCookie("HelpWindow.win.visible",document.cookie)) != "")
		if(parseInt(tmp)){
			HelpWindow.createWindow();
			win = HelpWindow.win;
			if((tmp = HistoViewer.getCookie("HelpWindow.win.x",document.cookie)) != "")
				win.setPosition(parseInt(tmp),win.getPosition()[1]);
			if((tmp = HistoViewer.getCookie("HelpWindow.win.y",document.cookie)) != "")
				win.setPosition(win.getPosition()[0],parseInt(tmp));
			if((tmp = HistoViewer.getCookie("HelpWindow.win.w",document.cookie)) != "")
				win.setWidth(parseInt(tmp));
			if((tmp = HistoViewer.getCookie("HelpWindow.win.h",document.cookie)) != "")
				win.setHeight(parseInt(tmp));
			if((tmp = HistoViewer.getCookie("HelpWindow.win.maximized",document.cookie)) != "")
				if(parseInt(tmp))
					win.maximize();
		}
		//Help Win End -----------------------------------------------
		
		//Canvas start coordinate -----------------------------------------------
	if((tmp = HistoViewer.getCookie("Canvas.win.initx",document.cookie)) != "")
		Canvas.defaultCanvasX = parseInt(tmp);
	if((tmp = HistoViewer.getCookie("Canvas.win.inity",document.cookie)) != "")
		Canvas.defaultCanvasY = parseInt(tmp);	
		//Canvas start coordinate -----------------------------------------------
		
		//window stack up -----------------------------------------------
	for(var i=3;i>=0;--i)
		if((tmp = HistoViewer.getCookie("Stack."+i,document.cookie)) != "" &&
			tmp != "-1"){
			
			switch(parseInt(tmp)){
			case 0:
				FileTreeWindow.win.setVisible(true);
				break;
			case 1: 
				DetectorWindow.win.setVisible(true);
				break;
			case 2: 
				Canvas.win.setVisible(true);
				break;
			case 3: 
				HelpWindow.win.setVisible(true);
				break;
			default:
				alert("Imposible Cookie Monster");
				return;
			}
		}
		//window stack up end -----------------------------------------------
		
		//Canvas Controls -----------------------------------------------		
		
		if((tmp = HistoViewer.getCookie("CanvasControl.params",document.cookie)) != ""){
	
			Canvas.xmin = parseInt(parseCanvasControlString("xmin",tmp));			
			Canvas.xmax = parseInt(parseCanvasControlString("xmax",tmp));
			Canvas.ymin = parseInt(parseCanvasControlString("ymin",tmp));
			Canvas.ymax = parseInt(parseCanvasControlString("ymax",tmp));
			Canvas.zmin = parseInt(parseCanvasControlString("zmin",tmp));
			Canvas.zmax = parseInt(parseCanvasControlString("zmax",tmp));
			Canvas.marginLt = parseInt(parseCanvasControlString("marginLt",tmp));
			Canvas.marginUp = parseInt(parseCanvasControlString("marginUp",tmp));
			Canvas.marginRt = parseInt(parseCanvasControlString("marginRt",tmp));
			Canvas.marginDn = parseInt(parseCanvasControlString("marginDn",tmp));
			Canvas.xlog = parseInt(parseCanvasControlString("xlog",tmp));
			Canvas.ylog = parseInt(parseCanvasControlString("ylog",tmp));
			Canvas.zlog = parseInt(parseCanvasControlString("zlog",tmp));
			Canvas.xtick = parseInt(parseCanvasControlString("xtick",tmp));
			Canvas.ytick = parseInt(parseCanvasControlString("ytick",tmp));
			Canvas.xgrid = parseInt(parseCanvasControlString("xgrid",tmp));
			Canvas.ygrid = parseInt(parseCanvasControlString("ygrid",tmp));
			Canvas.draw = parseCanvasControlString("draw",tmp);
			Canvas.prependA = parseInt(parseCanvasControlString("prependA",tmp));
			Canvas.alwaysNewWindow = parseInt(parseCanvasControlString("alwaysNewWindow",tmp))?true:false;
			Canvas.useCanvasCache = parseInt(parseCanvasControlString("useCanvasCache",tmp));
			Canvas.updateOnResize = parseInt(parseCanvasControlString("updateOnResize",tmp))?true:false;
			Canvas.initToDefault = parseInt(parseCanvasControlString("initToDefault",tmp))?true:false;
		
			Canvas.TypeSelect(0,0,0);
		
		}			
	
		//Canvas Controls end -----------------------------------------------		
		
}

////////////////////////////////////
HistoViewer.getCookie = function(needle,haystack){
	var i;	
	if((i =	haystack.indexOf(needle)) < 0) return "";
	haystack = haystack.slice(i);	
	if((i =	haystack.indexOf("=")) < 0)	return "";	
	haystack = haystack.slice(i+1);	
	if((i =	haystack.indexOf(";")) < 0)
		return haystack; //its the last cookie so no ';'	
	return haystack.slice(0,i);
}

////////////////////////////////////
parseCanvasControlString = function(needle,haystack){
	var i;	
	if((i =	haystack.indexOf(needle)) < 0) return "";
	haystack = haystack.slice(i);	
	if((i =	haystack.indexOf("=")) < 0)	return "";	
	haystack = haystack.slice(i+1);	
	if((i =	haystack.indexOf("+")) < 0)
		return haystack; //its the last cookie so no '+'	
	return haystack.slice(0,i);
}

////////////////////////////////////
HistoViewer.turtleRedraw = function(){
	
	HistoViewer.turtle.initx = document.body.clientWidth/2 - HistoViewer.turtle.offsetWidth/2;
	HistoViewer.turtle.inity = document.body.clientHeight/6 - HistoViewer.turtle.offsetHeight/2;
	if(HistoViewer.turtle.x == 0){
		HistoViewer.turtle.style.left = HistoViewer.turtle.initx + "px";		
		HistoViewer.turtle.style.top = HistoViewer.turtle.inity + "px";
	}

}

////////////////////////////////////
HistoViewer.turtleClick = function(){
	
	HistoViewer.turtle.motion  = !HistoViewer.turtle.motion;
	
	if(HistoViewer.turtle.motion){
		HistoViewer.turtle.speed = 10;
		HistoViewer.turtle.velx = Math.random()*2-1;
		HistoViewer.turtle.vely = Math.random()*2-1;
		var mag = HistoViewer.turtle.velx*HistoViewer.turtle.velx + HistoViewer.turtle.vely*HistoViewer.turtle.vely;
		if(mag == 0)
			HistoViewer.turtle.velx = -1;
		else{
			mag = Math.sqrt(mag);
			HistoViewer.turtle.velx /= mag;
			HistoViewer.turtle.vely /= mag;
		}
		setTimeout(HistoViewer.turtleMove,100);
	}
	else{
		//change turtle colors
		Ext.Ajax.request({
			url:     'XGI_Turtle',
			success: HistoViewer.ajaxTurtleResponse,
		});
	}
}

////////////////////////////////////
HistoViewer.turtleMove = function(){
	HistoViewer.turtle.x = HistoViewer.turtle.x + HistoViewer.turtle.velx*HistoViewer.turtle.speed;
	HistoViewer.turtle.y = HistoViewer.turtle.y + HistoViewer.turtle.vely*HistoViewer.turtle.speed;
	
	if(HistoViewer.turtle.initx + HistoViewer.turtle.x + HistoViewer.turtle.offsetWidth < 0) //test x extreme wrap-arounds
		HistoViewer.turtle.x = document.body.clientWidth-1-HistoViewer.turtle.initx;
	else if(HistoViewer.turtle.initx + HistoViewer.turtle.x > document.body.clientWidth)
		HistoViewer.turtle.x = 1-HistoViewer.turtle.initx-HistoViewer.turtle.offsetWidth;
	
	if(HistoViewer.turtle.inity + HistoViewer.turtle.y + HistoViewer.turtle.offsetHeight < 0) //test y eytreme wrap-arounds
		HistoViewer.turtle.y = document.body.clientHeight-1-HistoViewer.turtle.inity;
	else if(HistoViewer.turtle.inity + HistoViewer.turtle.y > document.body.clientHeight)
		HistoViewer.turtle.y = 1-HistoViewer.turtle.inity-HistoViewer.turtle.offsetHeight;
	
	if(HistoViewer.turtle.initx + HistoViewer.turtle.x + 
		HistoViewer.turtle.offsetWidth >= document.body.clientWidth || 
		HistoViewer.turtle.inity + HistoViewer.turtle.y + 
		HistoViewer.turtle.offsetHeight >= document.body.clientHeight)
		document.body.style.overflow = "hidden";
	else
		document.body.style.overflow = "auto";
		
	HistoViewer.turtle.style.left = HistoViewer.turtle.initx + HistoViewer.turtle.x + "px";
	HistoViewer.turtle.style.top = HistoViewer.turtle.inity + HistoViewer.turtle.y + "px";
	
	if(HistoViewer.turtle.motion)
		setTimeout(HistoViewer.turtleMove,100);
}

////////////////////////////////////
HistoViewer.ajaxTurtleResponse = function(ajaxObj){
	HistoViewer.turtle.childNodes[1].src = ajaxObj.responseText;
}












