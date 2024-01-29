#include <FastLED.h>

#define NUM_LEDS_CUBE 15
#define IR_PIN 13
#define LED_PIN_CUBE 11

CRGB ledscube[NUM_LEDS_CUBE];

// Global variables
bool ledStripOn = false;
unsigned long sensorBlockedStartTime = 0;
unsigned long defaultSpeed = 150;
unsigned long lastToggleTime = 0;

void movingRainbowEffect(unsigned long speed, int ledStripState) {
  if (ledStripState == 1) {
    // Turn on the LED strip
    if (!ledStripOn) {
      ledStripOn = true;
      FastLED.showColor(CRGB::Black);  // Clear any previous colors
    }

    // Perform rainbow effect
    static uint8_t startIndex = 0;
    static uint8_t hue = 0;

    // Calculate the delay based on the received speed or use the default speed
    unsigned long delayTime = (speed > 0) ? speed : defaultSpeed;

    if (millis() - lastToggleTime >= delayTime) {
      // Move the rainbow effect from left to right
      for (int i = 0; i < NUM_LEDS_CUBE; i++) {
        ledscube[i] = CHSV(hue++, 255, 255);
      }

      FastLED.show();
      lastToggleTime = millis();
    }
  } else {
    // Turn off the LED strip
    if (ledStripOn) {
      ledStripOn = false;
      FastLED.clear();
      FastLED.show();
    }
  }
}

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN_CUBE, GRB>(ledscube, NUM_LEDS_CUBE);
  pinMode(IR_PIN, INPUT);
  Serial.begin(115200);
}

void loop() {
  // Check if the IR sensor is interrupted
  if (digitalRead(IR_PIN) == LOW) {
    if (sensorBlockedStartTime == 0) {
      // Initialize the timer when the sensor is first blocked
      sensorBlockedStartTime = millis();
    } else if (millis() - sensorBlockedStartTime >= 3000) {
      // Toggle the LED strip state after 3 seconds of continuous blocking
      movingRainbowEffect(defaultSpeed, ledStripOn ? 0 : 1);
      sensorBlockedStartTime = 0;  // Reset the timer
    }
  } else {
    sensorBlockedStartTime = 0;  // Reset the timer if the sensor is not blocked
  }

  // Check if there is serial data available
  if (Serial.available() > 0) {
    // Read the serial command
    String data = Serial.readStringUntil('\n');
    
    // Parse the received command
    int speed = defaultSpeed;
    int ledStripState = 0;

    if (data.startsWith("movingRainbowEffect(")) {
      // Extract speed and ledStripState from the serial command
      int openParenIndex = data.indexOf('(');
      int closeParenIndex = data.indexOf(')');

      if (openParenIndex != -1 && closeParenIndex != -1) {
        String args = data.substring(openParenIndex + 1, closeParenIndex);
        String speedStr = args.substring(0, args.indexOf(','));
        String ledStripStateStr = args.substring(args.indexOf(',') + 1);

        // Convert speed and ledStripState to integers
        speed = speedStr.toInt();
        ledStripState = ledStripStateStr.toInt();
      }
    }

    // Call the movingRainbowEffect function with the received parameters
    movingRainbowEffect(speed, ledStripState);
  }
}
