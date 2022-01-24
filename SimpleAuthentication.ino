#define WEBSERVER_H //to avoid  duplicated definisions in WifiManager and AsyncWebServer
#include <WiFiManager.h>  
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <AsyncElegantOTA.h>

//Debug option can be removed to see serial output connect to "/webserial"
//#include <WebSerial.h>

//#include <ArduinoOTA.h>


#include <DHT_U.h>
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
AsyncWebServer server(80);

WiFiManager wifiManager;

//Check if header is present and correct
bool is_authenticated(AsyncWebServerRequest *request) {
  Serial.println("Enter is_authenticated");
  if (request->hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = request->header("Cookie");
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
void handleLogin(AsyncWebServerRequest *request) {
  String msg;
  if (request->hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = request->header("Cookie");
    Serial.println(cookie);
  }
  if (request->hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    AsyncWebServerResponse *response = request->beginResponse(301);
    response->addHeader("Location", "/login");
    response->addHeader("Cache-Control", "no-cache");
    response->addHeader("Set-Cookie", "ESPSESSIONID=0");
    request->send(response);
    return;
  }
  request->send_P(200, "text/html", LOGIN_page);
}

void handleLoginValidate(AsyncWebServerRequest *request){
  if (!request->hasArg("user") || !request->hasArg("psw")) {
    request->send(200, "text/plane", "failed no args"); 
    return;
  }
  if (request->arg("user") == "kuba" &&  request->arg("psw") == "admin") {
    AsyncWebServerResponse *response = request->beginResponse(301);
    response->addHeader("Location", "/");
    response->addHeader("Cache-Control", "no-cache");
    response->addHeader("Set-Cookie", "ESPSESSIONID=1");
    request->send(response);

    Serial.println("Log in Successful");
    request->send(200, "text/plane", "loged in"); 
    return;
  }
  Serial.println("Log in Failed");
  request->send(200, "text/plane", "failed"); 
}

//root page can be accessed only if authentication is ok
void handleRoot(AsyncWebServerRequest *request) {
  String header;
  if (!is_authenticated(request)) {
    //request->redirect("/login");
    AsyncWebServerResponse *response = request->beginResponse(301);
    response->addHeader("Location", "/login");
    response->addHeader("Cache-Control", "no-cache");
    request->send(response);
    return;
  }

  request->send_P(200, "text/html", MAIN_page); //Send web page

}

void handleHeating(AsyncWebServerRequest *request) {
  if(!request->hasArg("state")) return;
  String state = request->arg("state"); 
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
  request->send(200, "text/plane", heatingState); 
}

//no need authentication
void handleNotFound(AsyncWebServerRequest *request) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", message);
}

void setupWiFi(){
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
}

void setupWebServer(){
  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/loginValidate", handleLoginValidate);
  server.on("/setHeat", handleHeating);
  server.on("/readtemp", [](AsyncWebServerRequest *request) {request->send(200, "text/plane", String(t));});
  server.on("/readhumi", [](AsyncWebServerRequest *request) {request->send(200, "text/plane", String(h));});
  server.on("/readheatIndex", [](AsyncWebServerRequest *request) {
    request->send(200, "text/plane", String(dht.computeHeatIndex(t, h, false)));
    });
  server.on("/readheating", [](AsyncWebServerRequest *request) {request->send(200, "text/plane", heatingState);});
  server.on("/readTimer", [](AsyncWebServerRequest *request) {
    if((millis() - lastHeatON) <= MAX_HEAT_TIME && isHeatTimer){
      request->send(200, "text/plane", String( MAX_HEAT_TIME - millis() + lastHeatON));
    }
    else{
      request->send(200, "text/plane", "100");
    }
    });

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

/*
void setupOTA(){
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

*/

void setup(void) {
  Serial.begin(115200);
  
  // prepare Relay and turn off by default
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, 0);
  val = 0;

  dht.begin();

  setupWiFi();
  //setupOTA();
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  setupWebServer();

  
  //WebSerial.begin(&server);

  
}

void loop(void) {
 // ArduinoOTA.handle();
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
