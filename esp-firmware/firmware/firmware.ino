#include <WiFi.h>
#include <WebSocketServer.h>
#include <ArduinoJson.h>
#include "secret_constants.h"

WiFiServer server(80);
WebSocketServer webSocketServer;

void handleReceivedMessage(String message){
    StaticJsonBuffer<2500> JSONBuffer;
    JsonObject& parsed = JSONBuffer.parseObject(message);
    if (!parsed.success()) { 
        Serial.println("Parsing failed");
        return;
    }

    const char * rd = parsed["RD"];  
    Serial.println(rd);
}

void setup() {
 
    Serial.begin(115200);
    delay(2000);
    WiFi.begin(ssid, password); 
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
 
    Serial.println("Connected to the WiFi network");
    Serial.println(WiFi.localIP());
 
    server.begin();
    delay(100);
}

void loop() {

    WiFiClient client = server.available();
     
    if (client.connected() && webSocketServer.handshake(client)) {
        String data;      
        
        while (client.connected()) {
    
            data = webSocketServer.getData();
    
            if (data.length() > 0) {
                handleReceivedMessage(data);
                webSocketServer.sendData(data);
            }
    
            delay(10);
        }
        
        Serial.println("The client disconnected");
        delay(100);
      }
      
    delay(100);
}
