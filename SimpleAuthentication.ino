#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <DNSServer.h>
#include <WiFiManager.h>  

#include "DHT.h"
#include "index.h"
#include "loginPage.h"


#define DHTTYPE DHT11 
#define PIN_RELAY 5
#define PIN_DTH 4

#define DTH_READ_DELAY 2000 // in milisec
#define MAX_HEAT_TIME 3600000 

int val = 0;
unsigned long lastDTHRead, lastHeatON;
bool isHeatTimer = false;
float h = 0,t = 0;
String heatingState = "OFF";

DHT dht(PIN_DTH, DHTTYPE);
ESP8266WebServer server(80);
WiFiManager wifiManager;

//Check if header is present and correct
bool is_authenticated() {
  Serial.println("Enter is_authenticated");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  server.send(200, "text/html", LOGIN_page);
}

void handleLoginValidate(){
  if (!server.hasArg("user") || !server.hasArg("psw")) {
    server.send(200, "text/plane", "failed no args"); 
    return;
  }
  if (server.arg("user") == "kuba" &&  server.arg("psw") == "admin") {
    
    server.sendHeader("Location", "/");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
    server.send(301);
    Serial.println("Log in Successful");

    server.send(200, "text/plane", "loged in"); 
    return;
  }
  Serial.println("Log in Failed");
  server.send(200, "text/plane", "failed"); 
}

//root page can be accessed only if authentication is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authenticated()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }

  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page

}

void handleHeating() {
  if(!server.hasArg("state")) return;
  String state = server.arg("state"); 
  Serial.println(state);
  if(state == "1") {
    digitalWrite(PIN_RELAY, HIGH); //LED ON
    lastHeatON = millis();
    isHeatTimer = true;
    heatingState = "ON"; //Feedback parameter
  }
  else {
    digitalWrite(PIN_RELAY, LOW); //LED OFF
    isHeatTimer = false;
    heatingState = "OFF"; //Feedback parameter  
  }
  server.send(200, "text/plane", heatingState); 
}

//no need authentication
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setupArduinoOTAUpdate(){
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}


void setup(void) {
  Serial.begin(115200);
  
  // prepare Relay and turn off by default
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, 0);
  val = 0;

  dht.begin();

  /*if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
  }

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Updated over the internet");
  int trys = 12;
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(trys-- == 0 ){
      Serial.print("\n connecting to :");
      Serial.print(STASSID2);
      WiFi.config(local_IP2, gateway, subnet, primaryDNS, secondaryDNS);
      WiFi.begin(STASSID2, STAPSK2);
    }
  }
*/
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); 
  //wifiManager.setSTAStaticIPConfig(IPAddress(192,168,0,150), IPAddress(192,168,0,1), IPAddress(255,255,255,0)); // optional DNS 4th argument
  wifiManager.autoConnect("AutoConnectAP");

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wifiManager.getWiFiSSID(true));
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //This is taken care of in WifiManager
 //WiFi.setAutoReconnect(true);
 //WiFi.persistent(true);

  setupArduinoOTAUpdate();


  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/loginValidate", handleLoginValidate);
  server.on("/setHeat", handleHeating);
  server.on("/readtemp", []() {server.send(200, "text/plane", String(t));});
  server.on("/readhumi", []() {server.send(200, "text/plane", String(h));});
  server.on("/readheatIndex", []() {
    server.send(200, "text/plane", String(dht.computeHeatIndex(t, h, false)));
    });
  server.on("/readheating", []() {server.send(200, "text/plane", heatingState);});
  server.on("/readTimer", []() {
    if((millis() - lastHeatON) <= MAX_HEAT_TIME && isHeatTimer){
      server.send(200, "text/plane", String( MAX_HEAT_TIME - millis() + lastHeatON));
    }
    else{
      server.send(200, "text/plane", "100");
    }
    });
  /*server.on("/inline", []() {
    server.send(200, "text/plain", "this works without need of authentication");
  });
  */

  server.onNotFound(handleNotFound);
  //ask server to track these headers
  server.collectHeaders("User-Agent", "Cookie");
  server.begin();
  Serial.println("HTTP server started");
  
}

void loop(void) {
  ArduinoOTA.handle();
  server.handleClient();

  if((millis() - lastDTHRead) > DTH_READ_DELAY){
    h = dht.readHumidity();
    t = dht.readTemperature();

    lastDTHRead = millis();
  }

  if((millis() - lastHeatON) > MAX_HEAT_TIME){
      digitalWrite(PIN_RELAY, LOW); //LED OFF
      heatingState = "OFF"; //Feedback parameter  
  }

}
