var Canvas = {};

Canvas.canvasVector = new Array(); //vector of canvasWindow objects
Canvas.nameVector = new Array(); //vector of canvas name strings for lookup
Canvas.win = 0;

Canvas.defaultCanvasX = 10;
Canvas.defaultCanvasY = 10;

Canvas.controlPanel;
// a window object is defined to have
//	.win
//  .minimizeWindow()
//  .showWindow()


////////////////////////////////////
Canvas.createControlPanel = function(){

	var form = new Ext.form.FormPanel({
      baseCls: 'x-plain',
      defaultType: 'textfield',
			autoWidth: true,
			//autoScroll:   true,
      items: [{
          name: 'histo',
					hidden: true,
					hideLabel: true,
      },{
          name: 'histoname',
					hideLabel: true,
					readOnly: true,
          anchor:'100%'
      },{
					xtype: 'combo',
          name: 'type',
					fieldLabel: 'Type',
          width: 150,
					mode: 'local',
					triggerAction: 'all',
					typeAhead: true,
					emptyText: 'Select Command Type...',
					selectOnFocus: true,
					displayField: 'commands',
					store: new Ext.data.SimpleStore({
							fields: ['commands'],
							data: [																
											['Global Options'],
											['Axis Min/Max'],											
											['Axis Options'],										
											['Draw Options'],
											['Margins'],
										]
						}),
					listeners: {select: Canvas.TypeSelect}
      }]
  });

  Canvas.win = new Ext.Window({
      title:				'Canvas Controls',
			id: 					'Canvas Controls',
      width:				320,
      height: 			320,
			resizable: 		true,
      layout: 			'fit',
			closable: 		false,
			minimizable:  true,
			//autoScroll:   true,
      plain:				true,
      bodyStyle:		'padding:5px;',
      buttonAlign:	'center',
      items:  			form,

      buttons: [{
          text: 'Defaults',
					listeners: {click: Canvas.ControlDefaults}
      },{
          text: 'Refresh',
					listeners: {click: Canvas.ControlSubmit}
      }]
  });

	Canvas.win.formPanel = form;
	Canvas.win.form = form.getForm();
	Canvas.win.addListener("minimize",Canvas.minimizeWindow);	
	this.minimizeMaximizeWindow = Canvas.minimizeMaximizeWindow;
	
		//initialize histogram parameters to default view paramaters
	Canvas.InitControlParameters();
	
		//set initial state as 'Global Options'
	Canvas.win.formPanel.items.items[2].setValue("Global Options");
	Canvas.win.formPanel.add({
        xtype: 'checkbox',
				fieldLabel: 'Always New Canvas Window',
				checked: Canvas.alwaysNewWindow,
				height: 30,
				width: 20,
				listeners: {check: Canvas.UpdateNewWindowFlag}
    });
	Canvas.win.formPanel.add({
        xtype: 'checkbox',
				fieldLabel: 'Utilize Canvas Caching',
				checked: Canvas.useCanvasCache,
				height: 30,
				width: 20,
    });
	Canvas.win.formPanel.add({
        xtype: 'checkbox',
				fieldLabel: 'Update on Resize',
				checked: Canvas.updateOnResize,
				height: 30,
				width: 20,
				listeners: {check: Canvas.UpdateUpdateOnResizeFlag}
    });
	Canvas.win.formPanel.add({
        xtype: 'checkbox',
				fieldLabel: 'Init Canvas with Default Settings',
				checked: Canvas.initToDefault,
				height: 30,
				width: 20,
				listeners: {check: Canvas.UpdateInitToDefaultFlag}
    });
}

////////////////////////////////////
//initialize histogram parameters to default view paramaters
Canvas.InitControlParameters = function(){

	Canvas.xmin = 0;
	Canvas.xmax = 0;
	Canvas.ymin = 0;
	Canvas.ymax = 0;
	Canvas.zmin = 0;
	Canvas.zmax = 0;
	Canvas.marginLt = 1;
	Canvas.marginUp = 1;
	Canvas.marginRt = 1;
	Canvas.marginDn = 1;
	Canvas.xlog 		= 0;
	Canvas.ylog 		= 0;
	Canvas.zlog 		= 0;
	Canvas.xtick		= 0;
	Canvas.ytick		= 0;
	Canvas.xgrid		= 0;
	Canvas.ygrid		= 0;
	Canvas.draw 		= "";
	Canvas.prependA = 0;
	
	Canvas.alwaysNewWindow  = false;
	Canvas.useCanvasCache 	= 1;
	Canvas.updateOnResize 	= true;
	Canvas.initToDefault 	  = true;
}

////////////////////////////////////
Canvas.ControlDefaults = function(){
	Canvas.InitControlParameters();
	Canvas.TypeSelect(0,0,0);
	Canvas.ControlSubmit();
}

////////////////////////////////////
Canvas.ControlSubmit = function(t,w,getFullImgTag,imgTagWindow){
	
	var histoTarget;
	var histoWin;
	var isResize = false;
	if(typeof t == "string")
	{
		histoTarget = t; isResize = true;
		histoWin = w;
	}
	else
	{
		histoTarget = Canvas.win.form.items.items[0].getValue();
		histoWin = Canvas.win.form.items.items[1].getValue();		
	}
	
	if(!isResize)
	{
			//update current type parameters
		var selType = Canvas.win.form.items.items[2].lastSelectionText;
		if(selType == "Global Options")
		{
			//items[3] is alwaysNewCanvas
			Canvas.useCanvasCache  = Canvas.win.form.items.items[4].getValue()?1:0; //convert to 1/0 for value in ajax request
			//items[5] is updateOnResize
			//items[6] is initToDefault
		}
		else if(selType == "Axis Min/Max")
		{
			Canvas.xmin 	  = Canvas.win.form.items.items[3].getValue();
			Canvas.xmax 	  = Canvas.win.form.items.items[4].getValue();
			Canvas.ymin 	  = Canvas.win.form.items.items[5].getValue();
			Canvas.ymax 	  = Canvas.win.form.items.items[6].getValue();	
			Canvas.zmin     = Canvas.win.form.items.items[7].getValue();
			Canvas.zmax     = Canvas.win.form.items.items[8].getValue();      
		}
		else if(selType == "Margins")
		{
			Canvas.marginLt = Canvas.win.form.items.items[3].getValue();
			Canvas.marginUp = Canvas.win.form.items.items[4].getValue();
			Canvas.marginRt = Canvas.win.form.items.items[5].getValue();
			Canvas.marginDn = Canvas.win.form.items.items[6].getValue();
		}
		else if(selType == "Axis Options")
		{
			Canvas.xlog 	  = Canvas.win.form.items.items[3].getValue()?1:0;
			Canvas.ylog 	  = Canvas.win.form.items.items[4].getValue()?1:0;
			Canvas.zlog 	  = Canvas.win.form.items.items[5].getValue()?1:0;
			Canvas.xtick	  = Canvas.win.form.items.items[6].getValue()?1:0;
			Canvas.ytick	  = Canvas.win.form.items.items[7].getValue()?1:0;
			Canvas.xgrid	  = Canvas.win.form.items.items[8].getValue()?1:0;
			Canvas.ygrid	  = Canvas.win.form.items.items[9].getValue()?1:0;
		}
		else if(selType == "Draw Options")
		{
			Canvas.prependA = Canvas.win.form.items.items[4].getValue()?1:0;			
			Canvas.draw 		= (Canvas.prependA?"A":"") + Canvas.win.form.items.items[3].getValue();
			
			//remove ; and + cause they will screw up cookies
			while(Canvas.draw.indexOf(";") != -1)
				Canvas.draw = Canvas.draw.replace(";",",");
			while(Canvas.draw.indexOf("+") != -1)
				Canvas.draw = Canvas.draw.replace("+","-");
		}
		else if(selType != undefined) //for undefined pass through request with no updates
		{
			alert("Unrecognized type selected!");
			return;
		}
	}	
	
	if(getFullImgTag){ //special case for init canvas window for first time with config parameters
		imgTagWindow.load({
  		  url: 'XGI_CanvasControl',
			  method: 'POST',
			  params: 
					"getFullImgTag=1" + "&" +
				  "histoname=" +  escape(histoTarget) + "&" +
				  "useCache=" +   Canvas.useCanvasCache + "&" +
				  "w=" +          (imgTagWindow.body.getWidth() - 2) + "&" +
				  "h=" +          (imgTagWindow.body.getHeight() - 2) + "&" +
				  "draw=" +       escape(Canvas.draw) + "&" +
				  "xmin=" +       Canvas.xmin + "&" +
				  "xmax=" +       Canvas.xmax + "&" +
				  "ymin=" +       Canvas.ymin + "&" +
				  "ymax=" +       Canvas.ymax + "&" +
				  "zmin=" +       Canvas.zmin + "&" +
				  "zmax=" +       Canvas.zmax + "&" +
				  "marginLt=" +   Canvas.marginLt + "&" +
				  "marginUp=" +   Canvas.marginUp + "&" +
				  "marginRt=" +   Canvas.marginRt + "&" +
				  "marginDn=" +   Canvas.marginDn + "&" +
				  "xlog=" +       Canvas.xlog + "&" +
				  "ylog=" +       Canvas.ylog + "&" +
				  "zlog=" +       Canvas.zlog + "&" +
				  "xtick=" +      Canvas.xtick + "&" +
				  "ytick=" +      Canvas.ytick + "&" +
				  "xgrid=" +      Canvas.xgrid + "&" +
				  "ygrid=" +      Canvas.ygrid
			});
	}
	else{
		if(histoTarget == "") //no histogram target
			return;
	
		var pad = document.getElementById('pad-' + histoWin);
		if(pad == undefined) return; //should only happen on first resize event
	
		Ext.Ajax.request({
			url:     'XGI_CanvasControl',
			success: Canvas.ajaxControlResponse,
			failure: ajaxFailure,
			params:  {
									sourceWin: histoWin,
									histoname: histoTarget,
									useCache:  Canvas.useCanvasCache,
									w:				 pad.offsetWidth,
									h:				 pad.offsetHeight,
									draw: 		 Canvas.draw,
									xmin: 		 Canvas.xmin,
									xmax: 		 Canvas.xmax,
									ymin: 		 Canvas.ymin,
									ymax: 		 Canvas.ymax,
									zmin: 		 Canvas.zmin,
									zmax: 		 Canvas.zmax,
									marginLt:  Canvas.marginLt,
									marginUp:  Canvas.marginUp,
									marginRt:  Canvas.marginRt,
									marginDn:  Canvas.marginDn,
									xlog: 		 Canvas.xlog,
									ylog: 		 Canvas.ylog,
									zlog: 		 Canvas.zlog,
									xtick:		 Canvas.xtick,
									ytick:		 Canvas.ytick,
									xgrid:		 Canvas.xgrid,
									ygrid:		 Canvas.ygrid,
								}
		});
	}
}

////////////////////////////////////
Canvas.ajaxControlResponse = function(ajaxObj){

	var targetWinId = ajaxObj.responseText.slice(0,ajaxObj.responseText.indexOf("---"));
	var newSrc = ajaxObj.responseText.slice(ajaxObj.responseText.indexOf("---")+3);

	var pad = document.getElementById('pad-' + targetWinId);
	
	if(pad)
		pad.src = newSrc;			
}

////////////////////////////////////
Canvas.TypeSelect = function(box,record,sel){

	var sz = Canvas.win.form.items.items.length;
	
	for(var i=3;i<sz;++i) //remove all fields to prepare for new selection
		Canvas.win.formPanel.remove(3);
		
	if(sel < 0) //nothing selected
		return;
	
	var selType = Canvas.win.form.items.items[2].lastSelectionText;
	if(selType == "Global Options"){
		Canvas.win.formPanel.add({
  	      xtype: 'checkbox',
		      fieldLabel: 'Always New Canvas Window',
		      checked: Canvas.alwaysNewWindow,
		      height: 30,
		      width: 20,
		      listeners: {check: Canvas.UpdateNewWindowFlag}
  	  });
		Canvas.win.formPanel.add({
  	      xtype: 'checkbox',
		      fieldLabel: 'Utilize Canvas Caching',
		      checked: Canvas.useCanvasCache,
		      height: 30,
		      width: 20,
  	  });
		Canvas.win.formPanel.add({
  	      xtype: 'checkbox',
		      fieldLabel: 'Update on Resize',
		      checked: Canvas.updateOnResize,
		      height: 30,
		      width: 20,
		      listeners: {check: Canvas.UpdateUpdateOnResizeFlag}
  	  });
		Canvas.win.formPanel.add({
  	      xtype: 'checkbox',
		      fieldLabel: 'Init Canvas with Default Settings',
		      checked: Canvas.initToDefault,
		      height: 30,
		      width: 20,
		      listeners: {check: Canvas.UpdateInitToDefaultFlag}
  	  });
	}
	else if(selType == "Axis Min/Max"){
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'X Min',
					value: Canvas.xmin,
      });
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'X Max',
					value: Canvas.xmax,
      });
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'Y Min',
					value: Canvas.ymin,
      });
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'Y Max',
					value: Canvas.ymax,
      });
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'Z Min',
					value: Canvas.zmin,
      });
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'Z Max',
					value: Canvas.zmax,
      });
	}
	else if(selType == "Margins"){	
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'Left Margin',
					value: Canvas.marginLt,
      });
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'Top Margin',
					value: Canvas.marginUp,
      });
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'Right Margin',
					value: Canvas.marginRt,
      });
		Canvas.win.formPanel.add({
          xtype: 'numberfield',
					fieldLabel: 'Bottom Margin',
					value: Canvas.marginDn,
      });
	}
	else if(selType == "Axis Options"){
		Canvas.win.formPanel.add({
          xtype: 'checkbox',
					fieldLabel: 'Log X',
					checked: Canvas.xlog,
      });
		Canvas.win.formPanel.add({
          xtype: 'checkbox',
					fieldLabel: 'Log Y',
					checked: Canvas.ylog,
      });
		Canvas.win.formPanel.add({
          xtype: 'checkbox',
					fieldLabel: 'Log Z',
					checked: Canvas.zlog,
      });
		Canvas.win.formPanel.add({
          xtype: 'checkbox',
					fieldLabel: 'Tick X',
					checked: Canvas.xtick,
      });
		Canvas.win.formPanel.add({
          xtype: 'checkbox',
					fieldLabel: 'Tick Y',
					checked: Canvas.ytick,
      });
		Canvas.win.formPanel.add({
          xtype: 'checkbox',
					fieldLabel: 'Grid X',
					checked: Canvas.xgrid,
      });
		Canvas.win.formPanel.add({
          xtype: 'checkbox',
					fieldLabel: 'Grid Y',
					checked: Canvas.ygrid,
      });
	} 
	else if(selType == "Draw Options"){
		Canvas.win.formPanel.add({
	      xtype: 'combo',
	      fieldLabel: 'Draw Options',
	      mode: 'local',
	      triggerAction: 'all',
	      typeAhead: true,
	      emptyText: 'Select or Manual Edit...',
	      selectOnFocus: true,
	      displayField: 'commands',
				width: 150,
	      store: new Ext.data.SimpleStore({
	          fields: ['commands'],
	          data: [
										['Default'],
	                  ['LEGO'],
	                  ['LEGO1'],
	                  ['LEGO2'],
	                  ['AXIS'],
	                  ['AXIG'],
	                  ['HIST'],
	                  ['FUNC'],
	                  ['SAME'],
	                  ['CYL'],
	                  ['POL'],
	                  ['SPH'],
	                  ['PSR'],
	                  ['SURF'],
	                  ['SURF1'],
	                  ['SURF2'],
	                  ['SURF3'],
	                  ['SURF4'],
	                  ['SURF5'],
	                  ['TEXT'],
	                  ['TEXTnn'],
	                  ['X+'],
	                  ['Y+'],
										['ARR'],
										['AH'],
										[']['],
										['B'],
										['C'],
										['E'],
										['E0'],
										['E1'],
										['E2'],
										['E3'],
										['E4'],
										['E5'],
										['E6'],
										['X0'],
										['L'],
										['P'],
										['P0'],
										['PIE'],
										['*H'],
										['LF2'],
										['9'],
										['ARR'],
										['BOX'],
										['BOX1'],
										['COL'],
										['COLZ'],
										['CONT'],
										['CONT0'],
										['CONT1'],
										['CONT2'],
										['CONT3'],
										['CONT4'],
										['CONT5'],
										['LIST'],
										['FB'],
										['BB'],
										['A'],
										['SCAT'],
										['[cutg]'],
										['ISO'],
										['BOX'],
										['NOSTACK'],
										['PADS'],
	                ]
	        })
    });
		
		Canvas.win.formPanel.items.items[3].setValue(Canvas.draw);
		
		Canvas.win.formPanel.add({
          xtype: 'checkbox',
					fieldLabel: 'Remove Axes',
					checked: Canvas.prependA,
      });
	} 
	else{
		alert("Unrecognized type selected!");	
	}
	
	Canvas.win.formPanel.doLayout();
	
}

////////////////////////////////////
Canvas.UpdateNewWindowFlag = function(){
	Canvas.alwaysNewWindow = this.getValue();
}

////////////////////////////////////
Canvas.UpdateUpdateOnResizeFlag = function(){
	Canvas.updateOnResize = this.getValue();
}

////////////////////////////////////
Canvas.UpdateInitToDefaultFlag = function(){
	Canvas.initToDefault = this.getValue();
}
	
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
//define Canvas.CanvasWindow "class" ***********************************************************
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------





////////////////////////////////////
Canvas.CanvasWindow = function(canvasName,histoString){

	var controlAction = new Ext.Action({
		id: 'btn-'+canvasName,
    text: 'Controls',
    handler: Canvas.accessControls,
	});
	var svgAction = new Ext.Action({
		id: 'btn-'+histoString,
    text: 'SVG View',
    handler: Canvas.svgView,
	});
	var printAction = new Ext.Action({
		id: 'btn-'+histoString,
    text: 'Print',
    handler: Canvas.printView,
	});

	this.win = new Ext.Window({
		title:  			canvasName, //histoString,
		id: 					canvasName,
	  closable: 		true,
		minimizable:  true,
		maximizable:	true,
		collapsible:	false,
		autoScroll:   true,
	  width:				500,
	  height: 			400,
	  border: 			true,
	  plain:				true,
//		autoLoad: 		{ url: "XGI_RequestHistogram",
//		                method: 'POST',
//										params: "histoname="+escape(histoString)+
//											"&w=" + 484 +
//											"&h=" + 342 +
//											"&sourceWin=" + escape(canvasName)
//									},
		buttonAlign:  'center',
		tbar: [
        	// Add the actions directly to a toolbar as a menu button
        controlAction,
				svgAction,
				printAction
    ],
	});
	this.win.histoString = histoString;
			
	Canvas.oldTime = (new Date()).getTime(); //used to prevent unnessary refreshing due to resize
	
	this.win.setSize(500,400); //for closing bug (still caches window size when closed)...
	
	this.showWindow = Canvas.showWindow;
	this.minimizeWindow = Canvas.minimizeWindow;
	this.minimizeMaximizeWindow = Canvas.minimizeMaximizeWindow;
	
	this.win.addListener("minimize",Canvas.minimizeWindow);
	this.win.addListener("close",Canvas.closeWindow);
	this.win.addListener("resize",Canvas.resizeWindow);
	
	this.showWindow();
	
	if(Canvas.initToDefault) //if global option is set to init to default parameters
		this.win.load({
  	  url: "XGI_RequestHistogram",
		  method: 'POST',
		  params: "histoname="+escape(histoString)+
		   "&w=" + (this.win.body.getWidth() - 2) +
		   "&h=" + (this.win.body.getHeight() - 2) +
			 "&sourceWin=" + escape(canvasName)
		 });
	else		//init to control parameters
		Canvas.ControlSubmit(histoString,canvasName,1,this.win);
}


////////////////////////////////////
Canvas.accessControls = function(){

	Canvas.win.show();

		//access correct canvas
	var myCan = Canvas.canvasVector[Canvas.nameVector.indexOf(this.getId().slice(4))];
	
	Canvas.win.form.items.items[0].setValue(myCan.win.histoString);//CHANGE win.title);
	Canvas.win.form.items.items[1].setValue(myCan.win.id);
};

////////////////////////////////////
Canvas.newView = function(paramString){

	paramString += "&w=" +				 1000;
	paramString += "&h=" +				 1000;
	paramString += "&draw=" + 		 Canvas.draw;
	paramString += "&xmin=" + 		 Canvas.xmin;
	paramString += "&xmax=" + 		 Canvas.xmax;
	paramString += "&ymin=" + 		 Canvas.ymin;
	paramString += "&ymax=" + 		 Canvas.ymax;
	paramString += "&marginLt=" +  Canvas.marginLt;
	paramString += "&marginUp=" +  Canvas.marginUp;
	paramString += "&marginRt=" +  Canvas.marginRt;
	paramString += "&marginDn=" +  Canvas.marginDn;
	paramString += "&xlog=" +      Canvas.xlog;
	paramString += "&ylog=" +      Canvas.ylog;
	paramString += "&zlog=" +      Canvas.zlog;
	paramString += "&xtick=" +		 Canvas.xtick;
	paramString += "&ytick=" +		 Canvas.ytick;
	paramString += "&xgrid=" +		 Canvas.xgrid;
	paramString += "&ygrid=" +		 Canvas.ygrid;
	
	window.open('XGI_PrintView?' + paramString,'printView',
		'width=400,height=200,toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes,copyhistory=no,resizable=yes');
}

////////////////////////////////////
Canvas.printView = function(){
	var paramString = "print=1";
	var histoString = this.getId().slice(4);
	
	paramString += "&histo=" +		 escape(histoString);
	Canvas.newView(paramString);
};

////////////////////////////////////
Canvas.svgView = function(){
	var paramString = "print=0";
	var histoString = this.getId().slice(4);
	
	paramString += "&histo=" + escape(histoString);
	Canvas.newView(paramString);
};

////////////////////////////////////
Canvas.resizeWindow = function(){

	if(!Canvas.updateOnResize)
		return;
		
		//check time to see if resize acceptable
	if(Canvas.oldTime == undefined) Canvas.oldTime = 0;
		
	var newTime = new Date();
	var secWait = 1; //seconds to wait between accepting resize picture requests
	if(newTime.getTime() - Canvas.oldTime > secWait*1000)
	{	
		Canvas.oldTime = newTime;
		Canvas.ControlSubmit(this.histoString,this.getId());
	}
};

////////////////////////////////////
Canvas.closeWindow = function(){
	Canvas.remove(this.getId());
};

////////////////////////////////////
Canvas.showWindow = function(){
	this.win.show();
};
	
////////////////////////////////////
Canvas.minimizeWindow = function(){
	var winObj = this;
	if(this.win){  //condition for if called as a member function (not as event handler)
		winObj = this.win;
	}
	
	//toggle visibility and animate
	if(winObj.isVisible()){
		winObj.setVisible(false);
	}
//	else{
//		winObj.setVisible(true);
//		winObj.expand(true);
//	}	
};

////////////////////////////////////
Canvas.minimizeMaximizeWindow = function(){
	var winObj = this;
	if(this.win){  //condition for if called as a member function (not as event handler)
		winObj = this.win;
	}
	//toggle visibility and animate
	if(winObj.isVisible()){
		winObj.setVisible(false);
	}
	else{
		winObj.setVisible(true);
	}	
};

////////////////////////////////////
Canvas.activateWindow = function(){
	this.collapse(false);
	this.expand(false);
}
//-----------------------------------------------------------
//-----------------------------------------------------------

	
////////////////////////////////////
Canvas.add = function(canvasName,histoString){

	var i = Canvas.nameVector.indexOf(canvasName);
	
	if(i >= 0 && !Canvas.alwaysNewWindow){ //already exists, update
		
		if(Canvas.initToDefault) //if global option is set to init to default parameters
			Canvas.canvasVector[i].win.load({
  		  url: "XGI_RequestHistogram",
			  method: 'POST',
			  params: "histoname="+escape(histoString)+
			   "&w=" + (Canvas.canvasVector[i].win.body.getWidth() - 2) +
			   "&h=" + (Canvas.canvasVector[i].win.body.getHeight() - 2) +
				 "&sourceWin=" + escape(canvasName)
			 });
		else		//init to control parameters
			Canvas.ControlSubmit(histoString,canvasName);
			
		Canvas.canvasVector[i].win.histoString = histoString; //CHANGE title
		Canvas.canvasVector[i].win.setVisible(true);
	}
	else{
		if(i >= 0)
		{
			var tag = 1; //determine tag number
			while((tmpName = canvasName + "--" + tag) &&
				Canvas.nameVector.indexOf(tmpName) >= 0)
				++tag;
			canvasName += "--" + tag;
		}
		
		Canvas.canvasVector.push(new Canvas.CanvasWindow(canvasName,histoString));
		Canvas.nameVector.push(canvasName);
	
		Console.add(Canvas.canvasVector[Canvas.canvasVector.length - 1]);
		if(Canvas.canvasVector.length > 1){ //already a canvas to align to
			Canvas.canvasVector[Canvas.canvasVector.length-1].win.
				setPosition(
										Canvas.canvasVector[Canvas.canvasVector.length-2].win.getPosition()[0]+15,
										Canvas.canvasVector[Canvas.canvasVector.length-2].win.getPosition()[1]+15
									 );
		}
		else{ //first canvas
			Canvas.canvasVector[0].win.setPosition(Canvas.defaultCanvasX,Canvas.defaultCanvasY);
		}
	}
};
	
////////////////////////////////////
Canvas.remove = function(canvasName){
	var i = Canvas.nameVector.indexOf(canvasName);
	
	if(i < 0){
		alert("Canvas.remove: " + canvasName + " not found");
		return;
	}
	
	Console.remove(Canvas.canvasVector[i]);
	delete Canvas.canvasVector[i].win;
	delete Canvas.canvasVector[i];
	
	Canvas.canvasVector.splice(i,1);
	Canvas.nameVector.splice(i,1);
}
















