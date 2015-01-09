var xmlHttp

function DAC_AJAX(URI, vmeBaseAddress, mFEC, mFECChannel, HubAddress, PortAddress, ROCId, ROCName, DACAddress, DACValue){
	xmlHttp=GetXmlHttpObject()
	if (xmlHttp==null){
		alert ("Your browser does not support AJAX!");
		return;
	}
	URI=URI+"/ROC_XgiHandler";
	URI=URI+"?Command=Prog_DAC";
	URI=URI+"&FECBaseAddress="+vmeBaseAddress;
	URI=URI+"&mFEC="+mFEC;
	URI=URI+"&mFECChannel="+mFECChannel;
	URI=URI+"&HubAddress="+HubAddress;
	URI=URI+"&PortAddress="+PortAddress;
	URI=URI+"&ROCId="+ROCId;
	URI=URI+"&PixelROCName="+ROCName;
	URI=URI+"&DACAddress="+DACAddress;
	URI=URI+"&DACValue="+DACValue;
	xmlHttp.open("GET", URI, true);
	xmlHttp.send(null);
}

function Cal_Pix_AJAX(URI, vmeBaseAddress, mFEC, mFECChannel, HubAddress, PortAddress, ROCId, ROCName, Row, Column, CalData){
	xmlHttp=GetXmlHttpObject()
	if (xmlHttp==null){
		alert ("Your browser does not support AJAX!");
		return;
	}
	URI=URI+"/ROC_XgiHandler";
	URI=URI+"?Command=Cal_Pix";
	URI=URI+"&FECBaseAddress="+vmeBaseAddress;
	URI=URI+"&mFEC="+mFEC;
	URI=URI+"&mFECChannel="+mFECChannel;
	URI=URI+"&HubAddress="+HubAddress;
	URI=URI+"&PortAddress="+PortAddress;
	URI=URI+"&ROCId="+ROCId;
	URI=URI+"&PixelROCName="+ROCName;
	URI=URI+"&Column="+Column;
	URI=URI+"&Row="+Row;
	URI=URI+"&CalData="+CalData;
	xmlHttp.open("GET", URI, true);
	xmlHttp.send(null);
}

function Prog_Pix_AJAX(URI, vmeBaseAddress, mFEC, mFECChannel, HubAddress, PortAddress, ROCId, ROCName, Row, Column, Mask, Trim){
	xmlHttp=GetXmlHttpObject()
	if (xmlHttp==null){
		alert ("Your browser does not support AJAX!");
		return;
	}
	URI=URI+"/ROC_XgiHandler";
	URI=URI+"?Command=Prog_Pix";
	URI=URI+"&FECBaseAddress="+vmeBaseAddress;
	URI=URI+"&mFEC="+mFEC;
	URI=URI+"&mFECChannel="+mFECChannel;
	URI=URI+"&HubAddress="+HubAddress;
	URI=URI+"&PortAddress="+PortAddress;
	URI=URI+"&ROCId="+ROCId;
	URI=URI+"&PixelROCName="+ROCName;
	URI=URI+"&Column="+Column;
	URI=URI+"&Row="+Row;
	URI=URI+"&Mask="+Mask;
	URI=URI+"&Trim="+Trim;
	xmlHttp.open("GET", URI, true);
	xmlHttp.send(null);
}

function ClrCal_AJAX(URI, vmeBaseAddress, mFEC, mFECChannel, HubAddress, PortAddress, ROCId, ROCName){
	xmlHttp=GetXmlHttpObject()
	if (xmlHttp==null){
		alert ("Your browser does not support AJAX!");
		return;
	}
	URI=URI+"/ROC_XgiHandler";
	URI=URI+"?Command=ClrCal";
	URI=URI+"&FECBaseAddress="+vmeBaseAddress;
	URI=URI+"&mFEC="+mFEC;
	URI=URI+"&mFECChannel="+mFECChannel;
	URI=URI+"&HubAddress="+HubAddress;
	URI=URI+"&PortAddress="+PortAddress;
	URI=URI+"&ROCId="+ROCId;
	URI=URI+"&PixelROCName="+ROCName;
	xmlHttp.open("GET", URI, true);
	xmlHttp.send(null);
}

function FileDACs(URI, ROCName){
	xmlHttp=GetXmlHttpObject()
        if (xmlHttp==null){
                alert ("Your browser does not support AJAX!");
                return;
        }
        URI=URI+"/ROC_XgiHandler";
	URI=URI+"?Command=FileDACs";
	URI=URI+"&PixelROCName="+ROCName;
	xmlHttp.open("GET", URI, true);
        xmlHttp.send(null);
}

function FileMasks(URI, ROCName){
        xmlHttp=GetXmlHttpObject()
        if (xmlHttp==null){
                alert ("Your browser does not support AJAX!");
                return;
        }
        URI=URI+"/ROC_XgiHandler";
        URI=URI+"?Command=FileMasks";
        URI=URI+"&PixelROCName="+ROCName;
        xmlHttp.open("GET", URI, true);
        xmlHttp.send(null);
}

function FileTrims(URI, ROCName){
        xmlHttp=GetXmlHttpObject()
        if (xmlHttp==null){
                alert ("Your browser does not support AJAX!");
                return;
        }
        URI=URI+"/ROC_XgiHandler";
        URI=URI+"?Command=FileTrims";
        URI=URI+"&PixelROCName="+ROCName;
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
