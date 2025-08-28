#include <Wire.h>
#include <FastLED.h>
#include "Adafruit_VL53L0X.h"

// ===== LED MATRIX SETTINGS =====
#define LED_PIN 13
#define NUM_LEDS 64
#define BRIGHTNESS 60
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// ===== SENSOR SETTINGS =====
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Define the min/max range for the distance visualizer (in millimeters)
#define MIN_DISTANCE 30   // At this distance or closer, very few LEDs will be lit
#define MAX_DISTANCE 400  // At this distance or farther, all LEDs will be lit

// Variables for the scrolling effect
int currentLedsToLight = 0;
int previousLedsToLight = 0;
int animationSpeed = 20; // Lower value = faster animation

void setup() {
  Serial.begin(115200);
  delay(1000); // Power-on delay

  // ----- Initialize FastLED -----
  // The correct syntax for addLeds is <chipset, data pin, color order>
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  Serial.println("FastLED Matrix Initialized");

  // ----- Initialize VL53L0X Sensor -----
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
  Serial.println("VL53L0X Sensor Initialized");
  
  // Start continuous ranging
  lox.startRangeContinuous();
}

void loop() {
  // Check if a new sensor measurement is available
  if (lox.isRangeComplete()) {
    int distance = lox.readRangeResult();

    if (distance >= MIN_DISTANCE && distance <= MAX_DISTANCE) {
      // Map the distance to the target number of LEDs to light up.
      int targetLedsToLight = map(distance, MIN_DISTANCE, MAX_DISTANCE, 0, NUM_LEDS);

      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.print("mm, Target LEDs: ");
      Serial.print(targetLedsToLight);
      Serial.println(" LEDs");

      // Animate the change in the number of lit LEDs
      if (targetLedsToLight > currentLedsToLight) {
        currentLedsToLight++;
      } else if (targetLedsToLight < currentLedsToLight) {
        currentLedsToLight--;
      }
      
      // Update the LEDs based on the current animated state
      FastLED.clear(); // Clear the matrix for each frame
      for (int i = 0; i < currentLedsToLight; i++) {
        // First row (0-7) is red
        if (i < 8) {
          leds[i] = CRGB::Red;
        } 
        // Second row (8-15) is yellow
        else if (i < 16) {
          leds[i] = CRGB::Yellow;
        } 
        // Third row (16-23) is green
        else if (i < 24) {
          leds[i] = CRGB::Green;
        }
      }
    
    } else {
      // If out of range, animate the display turning off
      if (currentLedsToLight > 0) {
        currentLedsToLight--;
        FastLED.clear();
        for (int i = 0; i < currentLedsToLight; i++) {
          if (i < 8) {
            leds[i] = CRGB::Red;
          } else if (i < 16) {
            leds[i] = CRGB::Yellow;
          } else if (i < 24) {
            leds[i] = CRGB::Green;
          }
        }
      } else {
        Serial.println("Out of range");
        FastLED.clear();
      }
    }
    
    // Update the physical LEDs and control animation speed
    FastLED.show();
    delay(animationSpeed);
  }
}