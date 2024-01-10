#include <FastLED.h>
#include <elapsedMillis.h>

// LED strip at the top
#define LED_PIN_TOP 11
#define NUM_LEDS_TOP 18
CRGB ledsTop[NUM_LEDS_TOP];
elapsedMillis elapsedTimeTop; // Declare elapsedTimeTop as a static variable

// LED strip at the bottom
#define LED_PIN_BOTTOM 12
#define NUM_LEDS_BOTTOM 32
CRGB ledsBottom[NUM_LEDS_BOTTOM];
elapsedMillis elapsedTimeBottom; // Declare elapsedTimeBottom as a static variable

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN_TOP, GRB>(ledsTop, NUM_LEDS_TOP);
  FastLED.addLeds<WS2812B, LED_PIN_BOTTOM, GRB>(ledsBottom, NUM_LEDS_BOTTOM);
}

void ledStripTop(int speed, int runTime) {
  unsigned long startTime = millis();
  unsigned long currentTime;

  // Set the background color to black
  fill_solid(ledsTop, NUM_LEDS_TOP, CRGB::Black);
  FastLED.show();

  while (elapsedTimeTop < runTime) {
    currentTime = millis(); // Update currentTime

    // Move the group of 5 green LEDs from left to right
    for (int i = 0; i <= NUM_LEDS_TOP - 5; ++i) {
      fill_solid(ledsTop + i, 5, CRGB::Green); // Set a group of 5 LEDs to green
      FastLED.show();
      delay(speed);
      fill_solid(ledsTop + i, 5, CRGB::Blue); // Set the same group back to blue
    }

    // Move the group of 5 green LEDs from right to left
    for (int i = NUM_LEDS_TOP - 5; i >= 0; --i) {
      fill_solid(ledsTop + i, 5, CRGB::Green); // Set a group of 5 LEDs to green
      FastLED.show();
      delay(speed);
      fill_solid(ledsTop + i, 5, CRGB::Blue); // Set the same group back to blue
    }

    // Check if runtime exceeded
    if (currentTime - startTime >= runTime) {
      break;
    }
  }

  // Turn off LEDs after the runTime
  fill_solid(ledsTop, NUM_LEDS_TOP, CRGB::Black); // Set all LEDs to black (off)
  FastLED.show();
}

void blinkLEDsBottom(int speed, int runTime) {
  unsigned long startTime = millis();
  unsigned long currentTime;

  // Set the background color to black
  fill_solid(ledsBottom, NUM_LEDS_BOTTOM, CRGB::Black);
  FastLED.show();

  while (elapsedTimeBottom < runTime) {
    currentTime = millis(); // Update currentTime

    // Move the group of 5 green LEDs from left to right
    for (int i = 0; i <= NUM_LEDS_BOTTOM - 5; ++i) {
      fill_solid(ledsBottom + i, 5, CRGB::Green); // Set a group of 5 LEDs to green
      FastLED.show();
      delay(speed);
      fill_solid(ledsBottom + i, 5, CRGB::Blue); // Set the same group back to blue
    }

    // Move the group of 5 green LEDs from right to left
    for (int i = NUM_LEDS_BOTTOM - 5; i >= 0; --i) {
      fill_solid(ledsBottom + i, 5, CRGB::Green); // Set a group of 5 LEDs to green
      FastLED.show();
      delay(speed);
      fill_solid(ledsBottom + i, 5, CRGB::Blue); // Set the same group back to blue
    }

    // Check if runtime exceeded
    if (currentTime - startTime >= runTime) {
      break;
    }
  }

  // Turn off LEDs after the runTime
  fill_solid(ledsBottom, NUM_LEDS_BOTTOM, CRGB::Black); // Set all LEDs to black (off)
  FastLED.show();
}

void loop() {
  ledStripTop(100, 20000);
  blinkLEDsBottom(100, 20000);
}
