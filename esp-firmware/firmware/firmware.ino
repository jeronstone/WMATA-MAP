#include <FastLED.h>
#include <WiFi.h>
#include <WebSocketServer.h>
#include <ArduinoJson.h>
#include "secret_constants.h"


WiFiServer server(80);
WebSocketServer webSocketServer;

void process(const char * str_in);

double curr_pos[20];
StaticJsonBuffer<2500> JSONBuffer;

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

void handleReceivedMessage(String message){
    JsonObject& parsed = JSONBuffer.parseObject(message);
    if (!parsed.success()) { 
        Serial.println("Parsing failed");
        return;
    }

    const char * red_line = parsed["RD"];
    
    process(red_line);

    JSONBuffer.clear();
}

void process(const char * str_in) { 
  if (strcmp(str_in, strstr(str_in, "!!!"))) {
    Serial.println("Error: json format");
    return;
  }

  char * c = (char *) (str_in + 3);
  int i = 0;
  while (strcmp(c, "!!!")) {
    char c_cpy [10];
    int j = 0;
    while (c[0] != ';') {
      c_cpy[j++] = c[0];
      c++;
    }
    c_cpy[j] = '\0';
    curr_pos[i++] = atof(c_cpy);
    c++;
  }

  // blink 1 for now
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(2000);
  leds[0] = CRGB::Black;
  FastLED.show();
}

void setup() {

    Serial.begin(115200);
    delay(2000);

    FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS);
    
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
                webSocketServer.sendData("rx confirm");
            }
    
            delay(10);
        }
        
        Serial.println("The client disconnected");
        delay(100);
      }
      
    delay(100);
}
