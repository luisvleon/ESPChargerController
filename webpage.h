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
    <h1><div class="h1">Solar battery charger controller</div></h1>
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

/*
 * Server Index Page
 */
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>"; 

/*
 * Login page
 */
const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>Solar Charge controller Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='logan' && form.pwd.value=='targus25')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>"; 
