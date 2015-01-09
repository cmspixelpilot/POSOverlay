var xmlHttp

function ResetROCs_AJAX(URI, vmeBaseAddress, mFEC, mFECChannel, TBMChannel, HubAddress){
	xmlHttp=GetXmlHttpObject()
	if (xmlHttp==null){
		alert ("Your browser does not support AJAX!");
		return;
	}
	URI=URI+"/Panel_XgiHandler";
	URI=URI+"?Command=ResetROCs";
	URI=URI+"&FECBaseAddress="+vmeBaseAddress;
	URI=URI+"&mFEC="+mFEC;
	URI=URI+"&mFECChannel="+mFECChannel;
	URI=URI+"&TBMChannel="+TBMChannel;
	URI=URI+"&HubAddress="+HubAddress;
	xmlHttp.open("GET", URI, true);
	xmlHttp.send(null);
}

function ResetTBM_AJAX(URI, vmeBaseAddress, mFEC, mFECChannel, TBMChannel, HubAddress){
	xmlHttp=GetXmlHttpObject()
	if (xmlHttp==null){
		alert ("Your browser does not support AJAX!");
		return;
	}
	URI=URI+"/ROC_XgiHandler";
	URI=URI+"?Command=ResetTBM";
	URI=URI+"&FECBaseAddress="+vmeBaseAddress;
	URI=URI+"&mFEC="+mFEC;
	URI=URI+"&mFECChannel="+mFECChannel;
	URI=URI+"&TBMChannel="+TBMChannel;
	URI=URI+"&HubAddress="+HubAddress;
	xmlHttp.open("GET", URI, true);
	xmlHttp.send(null);
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
