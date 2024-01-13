//Two strips connected together
#include <FastLED.h>

#define NUM_LEDS 135
#define LED_PIN 46
#define LED_PIN_BTM 45

CRGB leds[NUM_LEDS];

// Number of LEDs in the moving group
#define GROUP_SIZE 10
float animation = 0;
float fadeAnimation = 0;

// Animation delay in milliseconds
#define ANIMATION_DELAY 15 // Decreased delay for faster movement

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812, LED_PIN_BTM, GRB>(leds, NUM_LEDS); //MTM = Bottom led strip

  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  //FastLED.setBrightness(200);
  FastLED.setBrightness(100);

  Serial.begin(115200);
  FastLED.clear();
  FastLED.show();
}
void ledStrip() {
  // Initialize color values
  CRGB brownishYellow = CRGB(0, 255, 0); // Brownish Yellow color
  CRGB red = CRGB(255, 0, 255);

  // Move the group of brownish yellow LEDs from left to right on a red background
  for (int i = 0; i <= NUM_LEDS - GROUP_SIZE; i++) {
    for (int j = 0; j < GROUP_SIZE; j++) {
      leds[i + j] = brownishYellow; // Set LEDs to brownish yellow
    }
    FastLED.show();
    delay(ANIMATION_DELAY);
    // Clear the group
    for (int j = 0; j < GROUP_SIZE; j++) {
      leds[i + j] = red; // Set LEDs to red background
    }
  }

  // Move the group of LEDs from right to left on a background
  for (int i = NUM_LEDS - GROUP_SIZE; i >= 0; i--) {
    for (int j = 0; j < GROUP_SIZE; j++) {
      leds[i + j] = brownishYellow;
    }
    FastLED.show();
    delay(ANIMATION_DELAY);
    // Clear the group
    for (int j = 0; j < GROUP_SIZE; j++) {
      leds[i + j] = red; // Set LEDs to red background
    }
  }
}

void loop() {
  ledStrip();


}
