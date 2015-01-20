var DetectorWindow = {};

DetectorWindow.win = 0;   						//window element object

////////////////////////////////////
DetectorWindow.showWindow = function(){
	if(DetectorWindow.win){
		DetectorWindow.win.setVisible(true);
		DetectorWindow.win.show();	
		if(DetectorWindow.win.getPosition()[0] < 0 || DetectorWindow.win.getPosition()[1] < 0){
			DetectorWindow.win.setPosition(0,0); //in case window gets offscreen
		}
	}
	else{
		DetectorWindow.createWindow();
	}	
};

////////////////////////////////////
DetectorWindow.createWindow = function(){

	if(!DetectorWindow.win){
		var win;	
	
		win = new Ext.Window({
				title:  			'Detector Navigator',
				id: 					'Detector Navigator',
			  closable: 		false,
				maximizable:	false,
				minimizable:  true,
				collapsible:	false,
				autoScroll: 	false,
				resizable: 		false,
			  width:				DetectorNavigator.WEB_LARGE_WIDTH+15,
			  height: 			DetectorNavigator.WEB_LARGE_HEIGHT+56+
					(navigator.platform.indexOf("Mac") < 0?8:0), //add 8 pixels if not MAC
			  border: 			true,
			  plain:				true,
			  autoLoad: 		"XGI_DetectorNavigator?requestId=" + 
					escape(HistoViewer.globalRequestId)
			});
		win.addListener("minimize",DetectorWindow.minimizeMaximizeWindow);
		DetectorWindow.win = win;	
	
		DetectorNavigator.init(this);
	}
};

////////////////////////////////////
DetectorWindow.minimizeMaximizeWindow = function(){
	if(DetectorWindow.win){
		if(DetectorWindow.win.isVisible()){
			DetectorWindow.win.setVisible(false);	
		}
		else{
//			DetectorWindow.win.expand();
			DetectorWindow.showWindow(); 
			//DetectorWindow.win.setVisible(true);
		}
	}
	else{
		alert("DetectorWindow.minimizeMaximizeWindow: No Window!");
	}	
};

