#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <DHT.h>
#include <WebSocketsServer.h>
#include "webpage.h"
#include <ArduinoJson.h>
#include "time.h"
//-----------------------------------------------
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

//How many loops until refresh params
int refresh_loops = 10;
int actual_loop = refresh_loops; //first set at refresh_loop value to inmediate refresh

//-----------------------------------------------
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
//-----------------------------------------------
String JSONtxt;
//-----------------------------------------------
void handleRoot()
{
  server.send(200,"text/html", webpageCont);
}
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
//====================================================================
void setup()
{
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  dht_sensor.begin(); 

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("."); 
    delay(500);  
  }
  WiFi.mode(WIFI_STA);
  Serial.print(" Local IP: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.begin(); webSocket.begin();
  delay(1000);

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
//====================================================================
void loop() 
{
  delay(5000); 
    
  if(chargeGo())
  {
    if (actual_loop == refresh_loops)
    {
      actual_loop = 0; //reset to 0 
      refresh_params();  
    }
    else
    {
      actual_loop++;
    }        
     float avgvolt = handlecharge();
     webSocket.loop(); server.handleClient();
     String POTvalString = String(avgvolt);    
     float tempC = dht_sensor.readTemperature();
        
     JSONtxt = "{\"POT\":\"" + POTvalString + "\", \"TEMPC\":\"" + tempC + "\"}";
     webSocket.broadcastTXT(JSONtxt);  
  }
    else
  {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Charge standby!");
  }
}

bool chargeGo()
{
   if(checktime())
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


void refresh_params()
{      
    HTTPClient http;    
    http.begin("http://websoftec.net/param.json");
    http.GET();
    
    // Parse response
    StaticJsonDocument<200> doc;    
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
      Serial.print(refresh_loops);

      Serial.print(" | Adj. volts: ");
      Serial.println(vadjust);
      // Disconnect
      http.end();
    }
}

bool checktime(){
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
    return false;
   }
   else if (t_hour >= 7)
   {
    return true;
   }
}
