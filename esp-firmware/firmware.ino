#include <WiFi.h>
#include <WebSocketServer.h>
#include <ArduinoJson.h>
#include "secret_constants.h"

WiFiServer server(80);
WebSocketServer webSocketServer;

void handleReceivedMessage(String message){
    StaticJsonBuffer<500> JSONBuffer;
    JsonObject& parsed = JSONBuffer.parseObject(message);
    if (!parsed.success()) { 
        Serial.println("Parsing failed");
        return;
    }
}


