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

    double curr_pos_red[40];
    memset(curr_pos_red, -1, 40);
    
    process(red_line, curr_pos_red);

    clear_leds();

    for (int i = 0; i < 40; i ++) {
      if (curr_pos_red[i] >= 0) {
        int led_to_light = round(curr_pos_red[i] * (NUM_LEDS-1));
        leds[led_to_light] = CRGB::Red;
      }
    }
    FastLED.show();
    delay(100);

    Serial.println("Processing Complete");
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

// todo figure out faster and better solution
void clear_leds() {
  for (int i = 0; i < NUM_LEDS; i ++) {
      leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(100);
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
        // todo dont use String obj
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
