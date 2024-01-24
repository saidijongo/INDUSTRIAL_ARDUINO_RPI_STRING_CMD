#include <FastLED.h>
#include <elapsedMillis.h>

#define LED_PIN 12
#define NUM_LEDS 17

#define GROUP_SIZE 3
CRGB leds[NUM_LEDS];


elapsedMillis elapsedTime; // Declare elapsedTime as a static variable
bool continueLoop = true;  // Flag to control loop continuation

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
}

void ledStripWithBlink(int speed, int runTime) {
  unsigned long startTime = millis();
  unsigned long currentTime;

  const int blinkDuration = 10000;  // 10 seconds
  const int blinkInterval = 500;   // 500 milliseconds (0.5 seconds)

  while (continueLoop && (millis() - startTime < runTime)) {
    // Move the group of 3 blue LEDs from left to right
    for (int i = 0; i <= NUM_LEDS - GROUP_SIZE; ++i) {
      fill_solid(leds + i, GROUP_SIZE, CRGB::Blue); // Set a group of 3 LEDs to blue
      fill_solid(leds, i, CRGB::Red); // Set the background to red
      FastLED.show();
      delay(speed);
      fill_solid(leds + i, GROUP_SIZE, CRGB::Red); // Set the same group back to red
    }

    // Move the group of 3 blue LEDs from right to left
    for (int i = NUM_LEDS - GROUP_SIZE; i >= 0; --i) {
      fill_solid(leds + i, GROUP_SIZE, CRGB::Blue); // Set a group of 3 LEDs to blue
      fill_solid(leds, i, CRGB::Red); // Set the background to red
      FastLED.show();
      delay(speed);
      fill_solid(leds + i, GROUP_SIZE, CRGB::Red); // Set the same group back to red
    }

    // Check if runtime exceeded
    currentTime = millis(); // Update currentTime
    if (currentTime - startTime >= runTime) {
      break;
    }
  }

  // Turn off LEDs after the runTime
  fill_solid(leds, NUM_LEDS, CRGB::Black); // Set all LEDs to black (off)
  FastLED.show();

  delay(3000);

  // Reset elapsedTime for blinking
  elapsedTime = 0;


  startTime = millis(); // Reset the start time for blinking

  while (continueLoop && elapsedTime < blinkDuration) {
    // Turn on all LEDs to green
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

  continueLoop = false;  // Stop further looping
}

void loop() {
  ledStripWithBlink(100, 10000);
}
