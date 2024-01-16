#include <FastLED.h>
#include <elapsedMillis.h>

#define LED_PIN 11
#define NUM_LEDS 32

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
}

void blinkLEDs() {
  const int blinkDuration = 20000;  // 20 seconds
  const int blinkInterval = 500;   // 500 milliseconds (0.5 seconds)

  static elapsedMillis elapsedTime; // Track elapsed time

  while (elapsedTime < blinkDuration) {
    // Turn on all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(blinkInterval);

    // Turn off all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(blinkInterval);
  }

  // Turn off LEDs after the specified duration
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  blinkLEDs();
}
