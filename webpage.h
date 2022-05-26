//=====================
//HTML code for webpage
//=====================
const char webpageCont[] PROGMEM = 
R"=====(
<!DOCTYPE HTML>
<html>
<title>Solar panel voltage</title>
<!---------------------------CSS-------------------------->
<style>
    #dynRectangle
    {
        width:0px;
        height:12px;
        top: 9px;
        background-color: red;
        z-index: -1;
        margin-top:8px;
    }
    body   {background-color:rgba(128,128,128,0.322); font-family:calibri}
    h1     {font-size: 40px; color: black; text-align: center}
    h2     {font-size: 30px; color: blue}
    h3     {font-size: 17px; color:blue}
    div.h1 {background-color: whitesmoke;}
</style>
<!--------------------------HTML-------------------------->
<body>
    <h1><div class="h1">Solar panel voltage</div></h1>
    <h2>
        Voltage: <span style="color:rgb(216, 3, 3)" id="POTvalue">0</span> V  <br>
        Temperature: <span style="color:rgb(216, 3, 3)" id="TEMPvalue">0</span> V
    </h2>
    <h3>
        0V &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;
        &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; 24V
        <div id="dynRectangle"></div>
    </h3>
    <h3>
       <p>Current Date and Time is <span id='date-time'></span>.</p>
    </h3>
</body>
<!----------------------JavaScript------------------------>
<script>
  InitWebSocket()
  function InitWebSocket()
  {
    websock = new WebSocket('ws://'+window.location.hostname+':81/');
    websock.onmessage=function(evt)
    {
       JSONobj = JSON.parse(evt.data);
       document.getElementById('POTvalue').innerHTML = JSONobj.POT;
       var pot = parseInt(JSONobj.POT * 20);
       document.getElementById('TEMPvalue').innerHTML = JSONobj.TEMPC;
       var tempc = parseInt(JSONobj.TEMPC * 20);
       document.getElementById("dynRectangle").style.width = pot+"px";
    }
  }
</script>
</html>
<script>
var dt = new Date();
document.getElementById('date-time').innerHTML=dt;
</script>
)=====";
