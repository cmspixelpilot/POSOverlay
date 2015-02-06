//every window in the console must have 
// minimizeMaximizeWindow()
//implemented...

var Console = {};

Console.bar;
Console.barBkgnd;
Console.barHeight = 30;
Console.barX;
Console.barY;
Console.rows;
Console.winMax = 50;
Console.winList = new Array(Console.winMax);
Console.winListUsed;
Console.buttons = new Array(Console.winMax);

////////////////////////////////////////////////////////
Console.init = function(){
	Console.bar = Ext.get('console');
	Console.barBkgnd = Ext.get('console_bg');
	Console.barBkgnd.setWidth("100%");	
	Console.barBkgnd.setHeight(Console.barHeight);
	Console.winListUsed = 0;
	Console.rows = 1;
	
	Console.initButtons();
	Console.redraw();

};

////////////////////////////////////////////////////////
Console.redraw = function(){
	Console.barX = document.body.scrollLeft;
	Console.barY = document.body.scrollTop + document.body.clientHeight	- Console.barHeight*Console.rows;

	Console.barBkgnd.position("absolute",1000,Console.barX,Console.barY);

	//draw buttons
	var saveRows = Console.rows;
	Console.rows = 1;
	var i;
	for(i=0;i<Console.winListUsed;++i){
		Console.drawButton(i);	
	}
	for(i=Console.winListUsed;i<Console.winMax;++i){
		Console.hideButton(i);	
	}
	
	if(saveRows != Console.rows){ //number of rows changed
		Console.barBkgnd.setHeight(Console.barHeight*Console.rows);
		Console.redraw();
	}	
};

////////////////////////////////////////////////////////
Console.drawButton = function(i){
//	Console.buttons[i].setText(Console.winList[i].win.getId());
	Console.buttons[i].setText(Console.winList[i].win.id);
	var btnDiv = Ext.get("btn_div"+i);
	
	if(i>0){
		var alignToBtnDiv = Ext.get("btn_div"+(i-1));
		var btnX = alignToBtnDiv.getLeft() + alignToBtnDiv.getWidth() + 2;
		var btnY = alignToBtnDiv.getTop();
		if(btnX + btnDiv.getWidth() >= document.body.clientWidth){ //new row
			btnX = 10;
			btnY += alignToBtnDiv.getHeight() + 2;
			++Console.rows;
		}	
		btnDiv.position("absolute",1001,btnX,btnY);
	}
	else{ //First window (File Manager)
		btnDiv.position("absolute",1001,Console.barX + 10,Console.barY+4);
	}
	
};
////////////////////////////////////////////////////////
Console.hideButton = function(i){
	var btnDiv = Ext.get("btn_div"+i);
	btnDiv.setTop(-100);
};

////////////////////////////////////////////////////////
Console.initButtons = function(){

	for(var i=0;i<Console.winMax;++i){
		Console.bar.insertHtml("afterEnd","<div id='btn_div" + i + "' />");
		var btnDiv = Ext.get("btn_div"+i);
		btnDiv.position("absolute",1001,0,-100);
		Console.buttons[i] = new Ext.Toolbar.Button({
			text: 			"btn"+i,
			id: 				"btn"+i,
			minWidth:		100,
			renderTo: 	btnDiv,
		//iconCls:  	<background img>,
		});		
		
		Console.buttons[i].addListener("click",Console.buttonClicked);
	}
	
};

////////////////////////////////////////////////////////
Console.buttonClicked = function(btn){
	var i = parseInt(btn.getId().slice(3));
	Console.winList[i].minimizeMaximizeWindow();
};

////////////////////////////////////////////////////////
Console.add = function(winHandle){
	if(Console.winListUsed >= Console.winMax){
		alert("max window count reached: " + Console.winMax);
		return;
	}
	Console.winList[Console.winListUsed] = winHandle;
	++Console.winListUsed;
	Console.redraw();
};

////////////////////////////////////////////////////////
Console.remove = function(winHandle){
	if(Console.winListUsed == 0){
		alert("empty list");
		return;
	}
	
	var i = Console.winList.indexOf(winHandle);
	
	if(i<0){
		alert("not found");
		return;
	}
	
	for(i;i<Console.winListUsed-1;++i){
		Console.winList[i] = Console.winList[i+1];
	}
	Console.winList[i] = 0;
	--Console.winListUsed;
	
	Console.redraw();
};

////////////////////////////////////////////////////////
Console.autoArrangeWindows = function(style){

	var xoff,yoff;
	if(typeof(Console.winList[1].win.body) != "undefined" && Console.winList[1].win.isVisible())
		xoff = Console.winList[1].win.width;
	else
		xoff = 0;
		
	Console.winList[0].win.setPosition(xoff,0); //file tree window
	if(Console.winList[1].win)
		Console.winList[1].win.setPosition(0,0); //detector navigator
	
	if(style == 1){
		yoff = Console.winList[0].win.getSize().height;		
	}
	else{ //style == 2
		yoff = 0;
		xoff += Console.winList[0].win.getSize().width;
	}
	
	for(var i=2;i<Console.winListUsed;++i){ //all other windows
		Console.winList[i].win.setPosition(xoff+(i-2)*15,yoff+(i-2)*15);
		Console.winList[i].showWindow();
	}

}

////////////////////////////////////////////////////////
Console.minimizeAllWindows = function(style){
	for(var i=0;i<Console.winListUsed;++i){ //for each window
		if(typeof(Console.winList[i].win.body) != "undefined" && Console.winList[i].win.isVisible())
			Console.winList[i].minimizeMaximizeWindow();
	}
}

////////////////////////////////////////////////////////
Console.showAllWindows = function(style){
	for(var i=0;i<Console.winListUsed;++i){ //for each window
		Console.winList[i].showWindow();
	}
}

////////////////////////////////////////////////////////
Console.closeAllOthers = function(style){
	var n = Console.winListUsed
	for(var i=n-1;i>=3;--i){ //for each window not the navigators or console control
		Console.winList[i].win.close();		
	}
}

Console.histoFocusIndex = 0;
////////////////////////////////////////////////////////
Console.scrollWindowFocus = function(dir){
		//get new focus index based on direction
	Console.histoFocusIndex += dir;
	if(Console.histoFocusIndex < 0)
		Console.histoFocusIndex = Console.winListUsed - 1;
	Console.histoFocusIndex %= Console.winListUsed;
	Console.winList[Console.histoFocusIndex].showWindow();
}
