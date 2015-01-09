function getXMLHttpObject()
{
  var xmlHttp=null;
  try {
    // Firefox, Opera 8.0+, Safari
    xmlHttp=new XMLHttpRequest();
  } catch (e) {
    // Internet Explorer
    try {
      xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
    } catch (e) {
      try {
        xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
      } catch (e) {
        alert("Your browser does not support AJAX!");
        return false;
      }
    }
  }
  return xmlHttp;
}

var url;
function onPageLoad(argurl)
{
 url=argurl;
 //alert(url);
}

var xmlHttp=null;
xmlHttp=getXMLHttpObject();
xmlHttp.onreadystatechange=function()
{  
  if (xmlHttp.readyState==4) {
    //alert("inside ready=4");
    document.getElementById('part_FSM').innerHTML=xmlHttp.responseText;
    setTimeout("fsmCommand('None')",2000);
  }
}

function fsmCommand(command)
{
  xmlHttp.open("GET", url+"/ExperimentalXgiHandler?FSMInput="+command, true);
  xmlHttp.send(null);
}
