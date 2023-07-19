#include "firmware.h"

// todo split file into parts plz

WiFiServer server(80);
WebSocketServer webSocketServer;

CRGB leds_red[NUM_LEDS_RD];
CRGB leds_blue[NUM_LEDS_BL];
CRGB leds_green[NUM_LEDS_GR];
CRGB leds_silver[NUM_LEDS_SV];
CRGB leds_yellow[NUM_LEDS_YL];
CRGB leds_orange[NUM_LEDS_OR];

// todo make int return for err handling
void process(char * str_in, double * curr_pos) { 
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

void handleReceivedMessage(String message){
    StaticJsonBuffer<2500> JSONBuffer;
    JsonObject& parsed = JSONBuffer.parseObject(message);
    if (!parsed.success()) { 
        Serial.println("Parsing failed");
        return;
    }

    char * line;
    double curr_pos[MAX_LED_LEN];
    memset(curr_pos, -1, MAX_LED_LEN);
    int length;
    CRGB color;

    for (int i = 0 ; i < NUM_LINES; i ++) {
      switch(i) {
        case 0:
          line = parsed['RD'];
          length = NUM_LEDS_RD;
          color = CRGB::Red;
          break;
        case 1:
          line = parsed['BL'];
          lengh = NUM_LEDS_BL;
          color = CRBG::Blue;
          break;
        case 2:
          line = parsed['YL'];
          length = NUM_LEDS_YL;
          color = CRGB::Yellow;
          break;
        case 3:
          line = parsed['SV'];
          lengh = NUM_LEDS_SV;
          color = CRBG::SlateGrey;
          break;
        case 4:
          line = parsed['OR'];
          length = NUM_LEDS_OR;
          color = CRGB::Orange;
          break;
        case 5:
          line = parsed['GR'];
          lengh = NUM_LEDS_GR;
          color = CRBG::Green;
          break;
        default:
          line = parsed['RD'];
          length = NUM_LEDS_RD;
          color = CRGB::Red;
      }
      
      process(line, curr_pos);

      FastLED.clear();
      //FastLED.show();
      delay(LED_DELAY);
      
      for (int i = 0; i < length; i ++) {
        if (curr_pos_red[i] >= 0) {
          int led_to_light = round(curr_pos_red[i] * (length-1));
          Serial.println(led_to_light);
          leds_red[led_to_light] = color;
        }
      }
      FastLED.show();
      delay(LED_DELAY);

      memset(curr_pos, -1, MAX_LED_LEN);
    }
  
    Serial.println("Processing Complete");
  }

void setup() {

    Serial.begin(115200);
    delay(2000);

    FastLED.setMaxPowerInVoltsAndMilliamps(5,1000); 
    FastLED.setDither(0);

    FastLED.addLeds<NEOPIXEL, 23>(leds_red, NUM_LEDS_RD);

    FastLED.clear();
    FastLED.show();
    delay(LED_DELAY);
    
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
