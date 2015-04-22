var FileTreeWindow = {};

FileTreeWindow.win;   						//window element object
FileTreeWindow.ajaxResponseList;
FileTreeWindow.fileTree;
FileTreeWindow.fileRoot;
FileTreeWindow.contentTree;
FileTreeWindow.contentRoot;
FileTreeWindow.savedCanvasName;
FileTreeWindow.flattenFileDirs = 0;

////////////////////////////////////
FileTreeWindow.createWindow = function(){
	if(!FileTreeWindow.win){
		var win;
			
		// var flattenForm = new Ext.form.FormPanel({
// 	      baseCls: 'x-plain',
// 				autoScroll:   false,
// 				labelAlign: 	'right',
// 	      items: [{
//   					xtype: 'checkbox',
// 						fieldLabel: 'Flatten',
// 						checked: FileTreeWindow.flattenFileDirs,
// 						listeners: {check: FileTreeWindow.updateFlattenFileFlag}
//     		  }],
// 	  });
		
	  var fileTree = new Ext.tree.TreePanel({
									   region:       'west',
									   animate:      false,
									   enableDD:     false,
									   rootVisible:  false,
									   autoScroll:   true,
									   width:        450,
									   split:        true,
	  							   collapsible:  true,
									   title:        "Files in /nfshome0/pixelpro/TriDAS/pixel/PixelRuns/Runs",
										 loader:       new Ext.tree.TreeLoader(
												              { 																					
			    													    url: "XGI_RefreshFileDirectory",
																			}),
									   buttonAlign:  'center',																					
									   buttons:      [{
									                   text: 'Refresh',
									                   listeners: {click: FileTreeWindow.refreshFiles}
									                 }
																	 ,{
									                    text: 'Toggle Dir.',
									                    listeners: {click: FileTreeWindow.updateFlattenFileFlag}
									                  }
//																	 ,{
//									                   text: 'Merge',
//									                   listeners: {click: FileTreeWindow.mergeFiles}
//									                 }
																	 ],
										 //items: 			 [flattenForm],
	  							 });

		// set the root node
		var fileRoot = new Ext.tree.AsyncTreeNode({
		    						 text:      'fileRoot',
		    						 draggable: false,
		    						 id:        'fileRoot'
		               });
	
		fileTree.setRootNode(fileRoot);

		
		var contentTree = new Ext.tree.TreePanel({
												region: 			'center', 																													
												animate:			false,																															
												enableDD: 		false,																															
												rootVisible:  false,																															
												autoScroll: 	true, 																															
												split:  			true, 																															
												title:				"File Content", 																										
												loader: 			new Ext.tree.TreeLoader(
												              { 																					
			    													    url: "XGI_RefreshContentList",
																			})
	                    });
	

		// set the root node
		var contentRoot = new Ext.tree.AsyncTreeNode({
		    						    text:      'contentRoot',  													
		    						    draggable: false,         													
		    						    id:        'contentRoot'  													
		                  }); 											                            

		contentTree.setRootNode(contentRoot);
	
		win = new Ext.Window({
					  title:        'File Navigator', 						
					  id:           'File Navigator', 						
			  	  closable:     false,												
					  maximizable:  true, 												
					  minimizable:  true, 												
					  collapsible:  false,												
			  	  width:        800,													
			  	  height:       400,													
			  	  border:       true, 												
			  	  plain:        true, 												
			  	  layout:       'border', 										
			  	  items:        [fileTree,contentTree] 							
			    });														       
		        
		win.addListener("minimize",FileTreeWindow.minimizeMaximizeWindow);
		
		FileTreeWindow.win         = win;	
		FileTreeWindow.fileTree    = fileTree;	
		FileTreeWindow.contentTree = contentTree;
		FileTreeWindow.fileRoot    = fileRoot;		
		FileTreeWindow.contentRoot = contentRoot;
		
		FileTreeWindow.refreshFiles();
	}

};

////////////////////////////////////
FileTreeWindow.updateFlattenFileFlag = function(){
	FileTreeWindow.flattenFileDirs = !FileTreeWindow.flattenFileDirs;
	FileTreeWindow.refreshFiles();
}

////////////////////////////////////
FileTreeWindow.mergeFiles = function(){
	FileTreeWindow.flattenFileDirs = !FileTreeWindow.flattenFileDirs;
	FileTreeWindow.refreshFiles();
}

////////////////////////////////////
FileTreeWindow.showWindow = function(){
	if(FileTreeWindow.win){
		FileTreeWindow.win.setVisible(true);
	}
	else{
		alert("FileTreeWindow.showWindow: No Window!");
	}
};

////////////////////////////////////
FileTreeWindow.minimizeMaximizeWindow = function(){
	if(FileTreeWindow.win){
		if(FileTreeWindow.win.isVisible()){
			FileTreeWindow.win.setVisible(false);	
		}
		else{
			FileTreeWindow.win.setVisible(true);
		}
	}
	else{
		alert("FileTreeWindow.minimizeMaximizeWindow: No Window!");
	}	
};

////////////////////////////////////
FileTreeWindow.onFileClick = function(node){

	if(!FileTreeWindow.oldFileClickTime) FileTreeWindow.oldFileClickTime = 0;
	var newTime = (new Date()).getTime();
	if(newTime - FileTreeWindow.oldFileClickTime < 1000) //only allow every second at most
		return;
	FileTreeWindow.oldFileClickTime = newTime;	
	
	FileTreeWindow.contentRoot.id = node.id;
	FileTreeWindow.contentTree.loader.load(FileTreeWindow.contentTree.getRootNode());
}

////////////////////////////////////
FileTreeWindow.onContentClick = function(node){
	if(node.id.indexOf("SummaryTrees") >= 0){ //has "SummaryTrees" in the text from folder name - assumed to be TTree entry
		DetectorNavigator.paintWithTree(node);
	}
	else{ //assume clicked entry is for a histogram
		Canvas.add(node.text,node.id);
	}
}

////////////////////////////////////
FileTreeWindow.refreshFiles = function(){	
  while(FileTreeWindow.contentRoot.firstChild){ //clear list
	  FileTreeWindow.contentRoot.firstChild.remove();	
	}
  if(!FileTreeWindow.flattenFileDirs){
  	while(FileTreeWindow.fileRoot.firstChild){ //clear list
	  	FileTreeWindow.fileRoot.firstChild.remove();	
	  }
    FileTreeWindow.fileTree.getLoader().load(FileTreeWindow.fileRoot);
  }
	else{
	  Ext.Ajax.request({
		url:     'XGI_RefreshFileList',
		method:  'GET',
//		  url:     'XGI_RefreshFileDirectory',
//		  method:  'POST',
//		  params:  "node=fileRoot",
		  success: FileTreeWindow.ajaxFileResponse,
		  failure: ajaxFailure,
	  });
  }
}
////////////////////////////////////
FileTreeWindow.ajaxFileResponse = function(ajaxObj){
	while(FileTreeWindow.fileRoot.firstChild){ //clear list
		FileTreeWindow.fileRoot.firstChild.remove();	
	}
	
	var fileArray = new Array();
	
	var ajaxStr = ajaxObj.responseText;
	if(ajaxStr == "[]"){
		//alert("FileTreeWindow.ajaxFileResponse: No Producers Connected.");
		FileTreeWindow.fileRoot.appendChild(node = new Ext.tree.TreeNode({
  		text: 			"< no files >",
  		id: 				"< no files >",
  		leaf: 			true,
  		cls:  			'file',
		}));
		return;
	}
	
	var start = 2;//after ['
	var end = ajaxStr.indexOf("','",start);
	while(end != -1){
		fileArray.push(ajaxStr.slice(start,end));
		start = end + 3;
		end = ajaxStr.indexOf("','",start);
	}
  fileArray.push(ajaxStr.slice(start,ajaxStr.length-2));

	
	for(var i=0;i<fileArray.length;++i){
		var node = FileTreeWindow.fileRoot;
		var child;
		var fileName = fileArray[i].slice(fileArray[i].indexOf("/")+1);
    var firstSlash = 0;
		var lastSlash;
		while(!FileTreeWindow.flattenFileDirs && (lastSlash = fileName.indexOf("/",firstSlash)) != -1){
		  if(lastSlash == 0){
				firstSlash = 1;
				continue;
			}
			var name = fileName.slice(firstSlash,lastSlash);
			if(child = node.findChild('text',name)){
				node = child;
			}
			else{
//				node.expand(true);
				node.appendChild(node = new Ext.tree.TreeNode({
																							   text:       name,
																							   id:         name,
																							   expandable: true,
																							   leaf:       false,
																							  }));
				
			}
		  firstSlash = lastSlash + 1;
		}

		node.appendChild(node = new Ext.tree.TreeNode({
																					   text:       fileName.slice(firstSlash,fileName.length),
																					   id:         fileArray[i],
																					   leaf:       false,
																					   cls:        'file',
																					  }));
		node.addListener("click",FileTreeWindow.onFileClick);
	}
	delete fileArray;
}

////////////////////////////////////
FileTreeWindow.addFolderContent = function(i,depth,parentFolderNode,contentArray){
	if(i >= contentArray.length){
		return i;
	}
	
	var nodeName;
	var folderName;
	var depthSlashIndex;
	var folderStartIndex;
	while(i < contentArray.length){
		folderStartIndex = 0;
		depthSlashIndex = contentArray[i].indexOf("/");
		nodeName = contentArray[i].slice(depthSlashIndex+1);		
		
		for(var d=0;d<depth;++d){ 
			folderStartIndex = depthSlashIndex+1;
			depthSlashIndex	= nodeName.indexOf("/");
			nodeName = nodeName.slice(depthSlashIndex+1);
		}
		
		folderName = contentArray[i].slice(folderStartIndex,folderStartIndex+depthSlashIndex);
	
			//have correct folderName and nodeName
			
		if(depth != 0 && folderName != parentFolderNode.text){ //reached end of folder
			return i;
		}
		
		if(nodeName.indexOf("/") >= 0){ //found folder
			//add folder node
			var folder = new Ext.tree.TreeNode({
				 text:  				nodeName.slice(0,nodeName.length-1),
				 id:	  				contentArray[i],
				 cls:   				'folder',
				 leaf:  				false,
				 renderChilden: true,
			 }); 
			 var tmpi = i+1;
			 i = FileTreeWindow.addFolderContent(tmpi,depth+1,folder,contentArray); //recursively descend structure			
			 if(tmpi == i){ //no nodes were added to display as folder insert dummy node
				 folder.appendChild(FileTreeWindow.dummyNode);
			 }
			 else{
			  folder.expand(true);
			 }
			 parentFolderNode.appendChild(folder);
			 
		}
		else{ //create leaf
			var node = new Ext.tree.TreeNode({
  		  text:       nodeName,
  		  id:         contentArray[i],
  		  leaf:       true,
  		  cls:        'file',
			})
			parentFolderNode.appendChild(node);
			node.addListener("click",FileTreeWindow.onContentClick);
			++i;
		}
	}

	return i;
}

////////////////////////////////////
FileTreeWindow.activateWindow = function(){
	FileTreeWindow.fileTree.setVisible(true);
	FileTreeWindow.contentTree.setVisible(true);
	this.collapse(false);
	this.expand(false);	
}

////////////////////////////////////
FileTreeWindow.deactivateWindow = function(){
	
	FileTreeWindow.fileTree.setVisible(false);
	FileTreeWindow.contentTree.setVisible(false);
}












