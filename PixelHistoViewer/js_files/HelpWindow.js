var HelpWindow = {};

HelpWindow.win = 0;   						//window element object

////////////////////////////////////
HelpWindow.createWindow = function(){

	if(!HelpWindow.win){
		var win;	
	
		win = new Ext.Window({
				title:  			'Help Window',
				id: 					'Help Window',
			  closable: 		true,
				maximizable:	true,
				minimizable:  false,
				collapsible:	false,
				autoScroll: 	true,
				resizable: 		true,
			  width:				500,
			  height: 			400,
			  border: 			true,
			  plain:				true,
			  autoLoad: 		"/pixel/PixelHistoViewer/html_files/HelpWindow.html",
				listeners:  	{close: function(){HelpWindow.win=0;}}
			});
		HelpWindow.win = win;	
		HelpWindow.win.show();
	}
	else{ //if already open then close the Help pop-up window. Toggling effect.
		HelpWindow.win.close();	
	}

};

