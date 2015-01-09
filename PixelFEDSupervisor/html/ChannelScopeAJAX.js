var xmlHttp=GetXmlHttpObject();

function toggleChannelScope(URI, vmeBaseAddress, fednumber, channelId, channel) {	

  URI=URI+"/LowLevelXgiHandler";
  URI=URI+"?Command=UpdateChannelScope";
  URI=URI+"&FEDBaseAddress="+vmeBaseAddress;
  URI=URI+"&Channel="+channel;
  xmlHttp.open("GET", URI, true);
  xmlHttp.send(null);
  xmlHttp.onreadystatechange=function() {
    if (xmlHttp.readyState==4) {
      document.getElementById(channelId).src="/pixel/PixelRun/FIFO1Signal_"+fednumber+"_"+channel+".gif?"+Math.random();
    }
  }  
    
}

function GetXmlHttpObject(){
	var xmlHttp=null;
	try{
		// Firefox, Opera 8.0+, Safari
		xmlHttp=new XMLHttpRequest();
	}
	catch (e){
		// Internet Explorer
		try{
			xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e){
			xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
	    }
	}
	return xmlHttp;
}