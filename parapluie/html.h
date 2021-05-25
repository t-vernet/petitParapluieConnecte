R"rawText(
<!DOCTYPE HTML>
<SCRIPT>
var xmlHttp=createXmlHttpObject();
function createXmlHttpObject(){
 if(window.XMLHttpRequest){
    xmlHttp=new XMLHttpRequest();
 }else{
    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');
 }
 return xmlHttp;
}
var click;
function handleServerResponse(){
   xmlResponse=xmlHttp.responseXML;
   xmldoc = xmlResponse.getElementsByTagName('response');
   message = xmldoc[0].firstChild.nodeValue;
   if(message == 1){
     click = 1;
     message = 'ouvert';
     document.getElementById('button').style.background='#FFA200';
   }else{
     click = 0;
     message='ferm&eacute;';
     document.getElementById('button').style.background='#111111';
   }
   document.getElementById('button').innerHTML=message;
}
function process(){
   xmlHttp.open('PUT','xml',true);
   xmlHttp.onreadystatechange=handleServerResponse; // no brackets?????
   xmlHttp.send(null);
 setTimeout('process()',200);
}
function process2(){
    xmlHttp.open('SET','set1ESPval?Start='+click,true);
    xmlHttp.send(null);
 setTimeout('process2()',400);
}
function RunButtonWasClicked(){
click = (click==1)?0:1;
    xmlHttp.open('SET','set1ESPval?Start='+click,true);
    xmlHttp.send(null);
}
</SCRIPT>
<HTML>
<style>
#button {
  background-color: #E6E6FA;
  border: none;
  color: white;
  padding: 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 168px;
  display:block;
  margin:0 auto;
  margin-top:130px;
  cursor: pointer;
  width:524px;
  height:400px;
}
p.thicker{
  font-weight:900;
}
#runtime{
  font-weight:900;
  font-size: 147%;
  color:RED;
}
</style>

<BODY bgcolor='#E6E6FA' onload='process()'>
<button onClick='RunButtonWasClicked()' id='button'></button>
</BODY>

</HTML>
)rawText"
