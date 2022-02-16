//#define WEBSERVER_H //to avoid  duplicated definisions in WifiManager and AsyncWebServer
#include <WiFiManager.h>  

#include <DHT_U.h>

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>

#include <Hash.h>
#include <ArduinoJson.h>

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

const char* ssid     = "Trollol";
const char* password = "asiaKUBAmartyna";

//char* host = "192.168.1.13";  //replace this ip address with the ip address of your Node.Js server
char* host = "stocc.ddns.net";  //replace this ip address with the ip address of your Node.Js server
const int espport= 8081;

WiFiManager wifiManager;

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

// Use arduinojson.org/v6/assistant to compute the capacity.
StaticJsonDocument<200> doc; // opim for this JSON is 128
String jsonData = "";


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
  
  /*Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFiMulti.addAP(ssid, password);

  //WiFi.disconnect();
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
    Serial.print('.');
  }
*/

}

void setupWebSocketConnction(){
  // server address, port and URL
  webSocket.begin(host, espport, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  
  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);
}

void handleWebSocketMessage(String msg){
  if(msg == "ON"){
    digitalWrite(PIN_RELAY, HIGH); //LED ON
    lastHeatON = millis();
    isHeatTimer = true;
    heatingState = "ON"; //Feedback parameter
  }
  else if (msg == "OFF"){
    digitalWrite(PIN_RELAY, LOW); //LED OFF
    isHeatTimer = false;
    heatingState = "OFF"; //Feedback parameter  
  }
  else if(msg == "wifi"){
    wifiManager.resetSettings();
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      Serial.printf("[WSc] Connected to url: %s\n", payload);

      // send message to server when Connected
      webSocket.sendTXT("Connected");
    }
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
      handleWebSocketMessage(String((char*)payload));
      break;

    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
        case WStype_PING:
            // pong will be send automatically
            Serial.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            Serial.printf("[WSc] get pong\n");
            break;
    }

}

void setup(void) {
  Serial.begin(115200);
  
  // prepare Relay and turn off by default
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, 0);
  val = 0;

  dht.begin();

  setupWiFi();
  //AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  //setupWebServer();

  setupWebSocketConnction();
  
  //WebSerial.begin(&server);
  doc["msgType"] = "update";
  doc["temp"] = h;
  doc["humi"] = t;
  doc["heat"] = 10.5f;
  doc["heating"] = heatingState;
  doc["time"] = 0;
  
}

void updateJsonData(){
  doc["temp"] = t;
  doc["humi"] = h;
  doc["heat"] = dht.computeHeatIndex(t, h, false);
  doc["heating"] = heatingState;
  if((millis() - lastHeatON) <= MAX_HEAT_TIME && isHeatTimer){
     doc["time"] =  MAX_HEAT_TIME - millis() + lastHeatON;
  }
  else{
   doc["time"] = 0;
  }
  jsonData = "";
  serializeJson(doc, jsonData);
}

void loop(void) {

  webSocket.loop();

  if((millis() - lastDTHRead) > DTH_READ_DELAY){
    h = dht.readHumidity();
    t = dht.readTemperature();

    lastDTHRead = millis();

    updateJsonData();
    webSocket.sendTXT(jsonData);
  }

  if((millis() - lastHeatON) > MAX_HEAT_TIME){
      digitalWrite(PIN_RELAY, LOW); //LED OFF
      heatingState = "OFF"; //Feedback parameter  
  }

}
