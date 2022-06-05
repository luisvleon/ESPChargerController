#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <DHT.h>
#include <WebSocketsServer.h>
#include "webpage.h"
#include <ArduinoJson.h>
#include "time.h"
#include <ESPmDNS.h>
#include <Update.h>
//-----------------------------------------------
const char* host = "esp32";
const char* ssid = "ATT4CmRyNa";
const char* password = "cnqbmh?hz=yx";
const int VoltGPIO = 34;
const int RELAY_PIN = 5;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

#define DHT_SENSOR_PIN  21
#define DHT_SENSOR_TYPE DHT22

//Deafult voltage values
float vcut = 12.80;
float vstart = 10.50;
float vref = 4.74;
float vadjust = 0.00;
int haltTemp = 50;
int safeTemp = 30;
int tempLoops = 0;
int restartLoops = 0;
String JSONtxt;
bool offline = false;

//How many loops until refresh params
int refresh_loops = 0;
int actual_loop = refresh_loops; //first set at refresh_loop value to inmediate refresh

//-----------------------------------------------
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
//====================================================================
WebServer serverOTA(82);


void setup()
{
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  dht_sensor.begin(); 
  int conAttemps;

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED && !offline)
  {
    for(; conAttemps < 5; conAttemps++)
    {
      Serial.print("."); 
      delay(500); 
      offline = true;
    }     
  }
  if(!offline)
  {
     WiFi.mode(WIFI_STA);
     Serial.print(" Local IP: ");
     Serial.println(WiFi.localIP());
  
     server.on("/", handleRoot);
     server.begin(); webSocket.begin();
     delay(1000);

     // Init and get the time
     configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
  }

//========================================================
//OTA
 /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  serverOTA.on("/", HTTP_GET, []() {
    serverOTA.sendHeader("Connection", "close");
    serverOTA.send(200, "text/html", loginIndex);
  });
  serverOTA.on("/serverIndex", HTTP_GET, []() {
    serverOTA.sendHeader("Connection", "close");
    serverOTA.send(200, "text/html", serverIndex);
  });
/*handling uploading firmware file */
  serverOTA.on("/update", HTTP_POST, []() {
    serverOTA.sendHeader("Connection", "close");
    serverOTA.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = serverOTA.upload();
  if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  serverOTA.begin();  
}
//====================================================================
void loop() 
{
  serverOTA.handleClient();
  
  delay(5000); 
    
  if(chargeGo())
  {
    if (actual_loop == refresh_loops)
    {
      actual_loop = 0; //reset to 0 
      if(!offline)
      {
        refreshParams();  
      }
    }
    else
    {
      actual_loop++;
    }        
     float avgvolt = handlecharge();     
     String POTvalString = String(avgvolt);    
     float tempC = dht_sensor.readTemperature();
     
     webSocket.loop(); server.handleClient();
     JSONtxt = "{\"POT\":\"" + POTvalString + "\", \"TEMPC\":\"" + tempC + "\"}";
     webSocket.broadcastTXT(JSONtxt);  
  }
    else
  {
      if(!offline)
      {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("System offline, charging!");
        restartLoops++;
        if (restartLoops > 10) ESP.restart();
      }
      else
      {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("Charge standby!");
      }      
  }
}

void handleRoot()
{
  server.send(200,"text/html", webpageCont);
}

bool chargeGo()
{
   if(checkTime() && checkTemp())
   {
    return true;    
   }
   return false;     
}

float handlecharge()
{
   float voltavg = 0;
   int itera = 500;
   float tempvolt = 0;   
      
   for(int i = 0; i< itera; i++)
   {
     // vref is the value between PWR and Ground of the ESP32 board
     tempvolt = tempvolt + (float(analogRead(VoltGPIO))* vref/1023);
     delay(10);
   }    
     float avgvolt = (tempvolt / itera) - 0.6;

      Serial.println(avgvolt);
      
      if (avgvolt < vcut)
      {
          digitalWrite(RELAY_PIN, HIGH);
          Serial.println("CHARGING.....  ");
      }
      else
      {
          digitalWrite(RELAY_PIN, LOW);
          Serial.print("Standby at voltage");
      }      
     return avgvolt;
}


void refreshParams()
{      
    HTTPClient http;    
    http.begin("http://websoftec.net/param.json");
    http.GET();
    
    // Parse response
    StaticJsonDocument<300> doc;    
    //deserializeJson(doc, http.getStream());
    DeserializationError error = deserializeJson(doc, http.getString());
    
    if (error) {
       Serial.print(F("deserializeJson() failed: "));
       Serial.println(error.f_str());
       return;
    }
    else
    {
      // Read values
      String vcut_s = doc["CUT"];
      String vstart_s = doc["START"];
      String vstopcharge = doc["STOP"];
      String vref_s = doc["REF"];
      String vadjust_s = doc["ADJUST"];
      refresh_loops = doc["LOOPS"];
      haltTemp = doc["HALTTEMP"];
      safeTemp = doc["SAFETEMP"];
      
      vcut = vcut_s.toFloat();
      vstart = vstart_s.toFloat();
      vref = vref_s.toFloat();
      vadjust = vadjust_s.toFloat();
      
      Serial.print("Cut: ");
      Serial.print(vcut);
      
      Serial.print(" | Start: ");
      Serial.print(vstart);
      
      Serial.print(" | Stop: ");
      Serial.print(vstopcharge);

      Serial.print(" | Ref. Volt: ");
      Serial.print(vref);
      
      Serial.print(" | # Loops: ");
      Serial.println(refresh_loops);

      Serial.print("Adj. volts: ");
      Serial.print(vadjust);

      Serial.print(" | Halt Temp: ");
      Serial.print(haltTemp);

      Serial.print(" | Safe Temp: ");
      Serial.println(safeTemp);
      // Disconnect
      http.end();
    }
}

bool checkTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");    
    return false;
  }
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  int t_hour = atoi(timeHour);

   if(t_hour < 7)
   {
    return false;
   }
   else if(t_hour >= 18)
   {
    return true;
   }
   else if (t_hour >= 7)
   {
    return true;
   }
}

bool checkTemp()
{
  if(tempLoops > 0)
  {
    tempLoops--;
    Serial.println("Cooling battery down");
    return false;
  }
  else
  {
    float tempNow = dht_sensor.readTemperature();
    Serial.print("Temp now: ");
    Serial.println(tempNow);
    if (tempNow > haltTemp)
    {
      Serial.println("Unsafe temp reached"); 
      tempLoops = 5;
      return false;         
    }
    else if(tempNow < safeTemp)
    {
      return true;
    }  
  }   
}
