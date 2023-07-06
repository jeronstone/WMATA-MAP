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

    const char * red_line = parsed["RD"];
    
    process(red_line);
    
    Serial.println(red_line);
}

void process(const char * str_in) {
  if (strcmp(str_in, strstr(str_in, "!!!"))) {
    Serial.println("Error: formatting");
    return;
  }

  char * c = (char *) (str_in + 3);
  float curr_pos[20]; // this is per line for now
  int i = 0;
  while (!strcmp(c, "!!!")) {
    char c_cpy [10];
    int j = 0;
    while (c[0] != ';') {
      c_cpy[j++] = c[0];
      c++;
    }
    curr_pos[i++] = atof(c_cpy);
    c++;
  }
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
