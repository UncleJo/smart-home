#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "SinricPro.h"
#include "SinricProFanUS.h"

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

#define MyApiKey "20##a27c-##################fb3ed"
#define MySSID "######" 
#define MyWifiPassword "######" 
#define API_ENDPOINT "http://sinric.com"
#define HEARTBEAT_INTERVAL 300000

#define APP_KEY           "################d919d7"      
#define APP_SECRET        "fef6c926-####################a582-0a#######c80"   
#define FAN_ID            "5f5#######c45d"
 
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

struct {
  bool powerState = false;
  int fanSpeed = 1;
} device_state;

bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Sanxchep's Room FC Fan 1 turned %s\r\n", state?"on":"off");
  device_state.powerState = state;
  return true; // request handled properly
}

bool onRangeValue(const String &deviceId, int &rangeValue) {
  device_state.fanSpeed = rangeValue;
  Serial.printf("FC Fan 1 speed changed to %d\r\n", device_state.fanSpeed);
  return true;
}


bool onAdjustRangeValue(const String &deviceId, int rangeValueDelta) {
  device_state.fanSpeed += rangeValueDelta;
  Serial.printf("FC Fan 1 speed changed about %i to %d\r\n", rangeValueDelta, device_state.fanSpeed);

  rangeValueDelta = device_state.fanSpeed; // return absolute fan speed
  return true;
}


void turnOn(String deviceId) {
  if (deviceId == "##################")
  {  
    Serial.print("Turn on device Sanxchep's Room FC Light 1 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "####################")
  {  
    Serial.print("Turn on device Sanxchep's Room FC Light 2 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "############")
  {  
    Serial.print("Turn on device Sanxchep's Room FC Light 3 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "######################")
  {  
    Serial.print("Turn on device Sanxchep's Room FC Light 4 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "##################")
  {  
    Serial.print("Turn on device Sanxchep's Room FC Light 5 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "###################")
  {  
    Serial.print("Turn on device Sanxchep's CP Switch  id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "#################")
  {  
    Serial.print("Turn on device Sanxchep's Room Wall Mounted Halogen Bulb id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "####################")
  {  
    Serial.print("Turn on device Sanxchep's Room Node MCU Test id: ");
    Serial.println(deviceId);
    digitalWrite(LED_BUILTIN, LOW);    
  }
}

void turnOff(String deviceId) {
  if (deviceId == "######################")
  {  
    Serial.print("Turn off device Sanxchep's Room FC Light 1 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "################3")
  {  
    Serial.print("Turn off device Sanxchep's Room FC Light 2 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "#######################")
  {  
    Serial.print("Turn off device Sanxchep's Room FC Light 3 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "#########################")
  {  
    Serial.print("Turn off device Sanxchep's Room FC Light 4 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "###########################3")
  {  
    Serial.print("Turn off device Sanxchep's Room FC Light 5 id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "###########################")
  {  
    Serial.print("Turn off device Sanxchep's CP Switch  id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "###########################3")
  {  
    Serial.print("Turn off device Sanxchep's Room Wall Mounted Halogen Bulb id: ");
    Serial.println(deviceId);    
  }
  else if (deviceId == "#############################")
  {  
    Serial.print("Turn off device Sanxchep's Room Node MCU Test id: ");
    Serial.println(deviceId);
    digitalWrite(LED_BUILTIN, HIGH);    
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        
#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);      
#endif        
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "setPowerState") { 
            String value = json ["value"];
            if(value == "ON") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }
        else if(action == "SetColor") {
            String hue = json ["value"]["hue"];
            String saturation = json ["value"]["saturation"];
            String brightness = json ["value"]["brightness"];

            Serial.println("[WSc] hue: " + hue);
            Serial.println("[WSc] saturation: " + saturation);
            Serial.println("[WSc] brightness: " + brightness);
        }
        else if(action == "SetBrightness") {
          
        }
        else if(action == "AdjustBrightness") {
          
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
    default: break;
  }
}

void setupSinricPro() {
  SinricProFanUS &myFan = SinricPro[FAN_ID];

  myFan.onPowerState(onPowerState);
  myFan.onRangeValue(onRangeValue);
  myFan.onAdjustRangeValue(onAdjustRangeValue);

  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); 
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);  

  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  setupSinricPro();
  
  webSocket.begin("iot.sinric.com", 80, "/");

  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

void loop() {
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }
  SinricPro.handle();   
}
