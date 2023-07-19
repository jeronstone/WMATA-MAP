#include <FastLED.h>
#include <WiFi.h>
#include <WebSocketServer.h>
#include <ArduinoJson.h>
#include "secret_constants.h"

#define LED_DELAY 100

#define NUM_LINES 6

#define NUM_LEDS_OR 1
#define NUM_LEDS_BL 1
#define NUM_LEDS_YL 1
#define NUM_LEDS_RD 1
#define NUM_LEDS_GR 1
#define NUM_LEDS_SV 1

#define MAX_LED_LEN 1

// class Line {
//     public:
//         int num_leds;
//         const char code[3];
//         CRGB color;
//         CRGB ** leds;
//         int data_pin;
// };

// #define NUM_LEDS_A 51
// #define NUM_LEDS_B 59
// #define NUM_LEDS_C 4
// #define NUM_LEDS_D 27
// #define NUM_LEDS_E 3
// #define NUM_LEDS_F 16
// #define NUM_LEDS_G 5
// #define NUM_LEDS_H 25

// CRGB leds_A[NUM_LEDS_A];
// CRGB leds_B[NUM_LEDS_B];
// CRGB leds_C[NUM_LEDS_C];
// CRGB leds_D[NUM_LEDS_D];
// CRGB leds_E[NUM_LEDS_E];
// CRGB leds_F[NUM_LEDS_F];
// CRGB leds_G[NUM_LEDS_G];
// CRGB leds_H[NUM_LEDS_H];

// void perc_to_strip(const char * line, float perc) {
//   switch (line) {
//     case "RD":
//       leds_A[round(perc * (NUM_LEDS_A-1))] = CRGB::Red;
//       return;
//     case "SV":
//       leds_B[round(perc * (NUM_LEDS_B-1))] = CRGB::SlateGrey;
//       return;
//     case "OR":
//       int to_light = round(perc * NUM_LEDS_OR);
//       if (to_light <= 3) {
//         leds_C[to_light] = CRGB::Orange;
//       } else if (to_light <= 43) {
//         leds_B[to_light - (NUM_LEDS_C-1) + 12] = CRGB::Orange;
//       } else {
//         leds_G[to_light - 44] = CRGB::Orange;
//       }
//       return;
//     case "BL":
//       int to_light = round(perc * NUM_LEDS_BL);
//       if (to_light <= 26) {
//         leds_D[to_light] = CRGB::Blue;
//       } else {
//         leds_B[to_light - 27 + 21] = CRGB::Blue;
//       }
//       return;
//     case "YL":
//       int to_light = round(perc * NUM_LEDS_YL);
//       if (to_light <= 2) {
//         leds_E[to_light] = CRGB::Yellow;
//       } else if (to_light <= 13) {
//         leds_D[to_light - 2 + 11] = CRGB::Yellow;
//       } else {
//         leds_H[(NUM_LEDS_H-1) - (to_light - 22)] = CRGB::Yellow;
//       }
//       return;
//       //etc.
//   }
// }