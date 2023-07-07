#include <FastLED.h>
#include <WiFi.h>
#include <WebSocketServer.h>
#include <ArduinoJson.h>
#include "secret_constants.h"

// todo split file into parts plz

WiFiServer server(80);
WebSocketServer webSocketServer;

#define NUM_LEDS 30
CRGB leds[NUM_LEDS];

void handleReceivedMessage(String message){
    StaticJsonBuffer<2500> JSONBuffer;
    JsonObject& parsed = JSONBuffer.parseObject(message);
    if (!parsed.success()) { 
        Serial.println("Parsing failed");
        return;
    }

    const char * red_line = parsed["RD"];

    double curr_pos[20] = {-1};
    process(red_line, curr_pos);

    for (int i = 0; i < 20; i ++) {
      if (curr_pos[i] >= 0) {
        int led_to_light = round(curr_pos[i] * NUM_LEDS);
        Serial.println(curr_pos[i]);
        Serial.println(led_to_light);
        leds[led_to_light] = CRGB::Red;
      }
    }
    FastLED.show();
    delay(500);
  }

// todo make int return for err handling
void process(const char * str_in, double * curr_pos) { 
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
}

void setup() {

    Serial.begin(115200);
    delay(2000);

    FastLED.addLeds<NEOPIXEL, 25>(leds, NUM_LEDS);
    
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
