#include <FastLED.h>
#include <elapsedMillis.h>
/*
#define NUM_LEDS_CUBE 15
#define IR_PIN 13
#define LED_PIN_CUBE 11

#define LED_PIN 12
#define NUM_LEDS 17
#define GROUP_SIZE 3

CRGB leds[NUM_LEDS];
elapsedMillis elapsedTime; // Declare elapsedTime as a static variable
bool continueLoop = true;  // Flag to control loop continuation

CRGB ledscube[NUM_LEDS_CUBE];
bool ledStripState = false; // Initial state is OFF
unsigned long irSensorTimer = 0;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, LED_PIN_CUBE, GRB>(ledscube, NUM_LEDS_CUBE);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  pinMode(IR_PIN, INPUT);
}

void toggleLEDState() {
  static bool lastIrState = HIGH;
  static unsigned long sensorInterruptedTime = 0;

  // Read the current state of the IR sensor
  bool currentIrState = digitalRead(IR_PIN);

  // Check if the IR state has changed
  if (currentIrState != lastIrState) {
    // Update the timer when the state changes
    sensorInterruptedTime = millis();
    lastIrState = currentIrState;
  }

  // Check if the sensor has been interrupted for more than 10 seconds
  if (currentIrState == LOW && millis() - sensorInterruptedTime >= 10000) {
    // Toggle LED strip state
    ledStripState = !ledStripState;

    // Reset the timer
    sensorInterruptedTime = millis();
  }
}
*/

#include <Arduino.h>
#include <Servo.h>
#include <FastLED.h>

const int motorPins[] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};
const int NUM_LEDS = 15;
const int numPumps = sizeof(motorPins) / sizeof(motorPins[0]);
const int stepsPerRev = 5000; // 6400;
const float STEP_ANGLE = 1.8; // degrees per step

#define GROUP_SIZE 2
float animation = 0;
float fadeAnimation = 0;
#define ANIMATION_DELAY 100

//CRGB leds[NUM_LEDS];
elapsedMillis elapsedTime; // Declare elapsedTime as a static variable
bool continueLoop = true;  // Flag to control loop continuation

bool isReverse = true;
unsigned long pumpStartTimes[numPumps] = {0};
CRGB leds[NUM_LEDS];
Servo myServo;

const int LED_PIN_CUBE = 12;  // Define the LED pin for the cube LED strip
const int NUM_LEDS_CUBE = 15; // Define the number of LEDs in the cube LED strip
CRGB ledscube[NUM_LEDS_CUBE];  // Define the array for the cube LED strip

bool ledStripState = false;    // Initial state is OFF for the cube LED strip
unsigned long irSensorTimer = 0;

#define PULL_PIN 46    // PULL
#define DIR_PIN 45     // Direction
#define IR_PIN 13      // IR sensor
#define LED_PIN 11
#define driverOut1 84
#define driverOut2 85
#define servoPin 44

void setup() {
  for (int i = 0; i < numPumps; i++) {
    pinMode(motorPins[i], OUTPUT);
  }

  pinMode(driverOut1, OUTPUT);
  pinMode(driverOut2, OUTPUT);

  myServo.attach(servoPin);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(100);
  FastLED.clear();
  FastLED.show();

  pinMode(DIR_PIN, OUTPUT);
  pinMode(PULL_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);

  // Initialize cube LED strip
  FastLED.addLeds<WS2812B, LED_PIN_CUBE, GRB>(ledscube, NUM_LEDS_CUBE);

  Serial.begin(115200);
}

void runPumps(int pumpNumber, int runTime) {
  digitalWrite(motorPins[pumpNumber - 54], HIGH);
  pumpStartTimes[pumpNumber - 54] = millis() + runTime;
}

void runServo(int angle, int runSpeed) {
  int mappedSpeed = map(runSpeed, 0, 2000, 0, 180);

  if (angle >= 0) {
    for (int i = 0; i <= angle; ++i) {
      myServo.write(i);
      delay(mappedSpeed);
    }
  } else {
    for (int i = 0; i >= angle; --i) {
      myServo.write(i);
      delay(mappedSpeed);
    }
  }
}

void blinkLEDs() {
  const int blinkDuration = 5000;
  const int blinkInterval = 500;
  unsigned long startTime = millis();
  while (millis() - startTime < blinkDuration) {
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(blinkInterval);

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(blinkInterval);
  }
}

void runStepper(int angle, int speed) {
  int direction = (angle >= 0) ? HIGH : LOW;
  angle = abs(angle);

  digitalWrite(DIR_PIN, direction);
  int step_target_position = int(2 * (angle / STEP_ANGLE));

  int i = 0;
  while (i < step_target_position && digitalRead(IR_PIN) == HIGH) {
    digitalWrite(PULL_PIN, HIGH);
    delayMicroseconds(speed);
    digitalWrite(PULL_PIN, LOW);
    delayMicroseconds(speed);

    i++;
  }

  digitalWrite(PULL_PIN, LOW);
  blinkLEDs();

  if (digitalRead(IR_PIN) == LOW || i < step_target_position) {
    blinkLEDs();
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
  }
}

void ledStrip() {
  CRGB brownishYellow = CRGB(218, 165, 32);
  CRGB red = CRGB(255, 0, 0);

  while (true) {
    for (int i = 0; i <= NUM_LEDS - GROUP_SIZE; i++) {
      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = CRGB::Green;
      }
      FastLED.show();
      delay(ANIMATION_DELAY);

      if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        processCommand(data);
        if (data.indexOf("adiosMf,0") != -1) {
          FastLED.clear();
          FastLED.show();
          return;
        }
      }

      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = red;
      }
    }

    for (int i = NUM_LEDS - GROUP_SIZE; i >= 0; i--) {
      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = CRGB::Green;
      }
      FastLED.show();
      delay(ANIMATION_DELAY);

      if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        processCommand(data);
        if (data.indexOf("adiosMf,0") != -1) {
          FastLED.clear();
          FastLED.show();
          return;
        }
      }

      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = CRGB::Pink;
      }
    }
  }
}


void toggleLEDState() {
  static bool lastIrState = HIGH;
  static unsigned long sensorInterruptedTime = 0;

  bool currentIrState = digitalRead(IR_PIN);

  if (currentIrState != lastIrState) {
    sensorInterruptedTime = millis();
    lastIrState = currentIrState;
  }

  if (currentIrState == LOW && millis() - sensorInterruptedTime >= 10000) {
    ledStripState = !ledStripState;
    sensorInterruptedTime = millis();
  }
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

void movingRainbowEffect(int speed = 150, bool defaultState = false) {
  static uint8_t startIndex = 0;
  static uint8_t hue = 0;

  // Fill the LED strip with a moving rainbow gradient
  fill_rainbow(ledscube, NUM_LEDS_CUBE, hue, 1);

  // Move the rainbow effect from left to right
  for (int i = 0; i < NUM_LEDS_CUBE; i++) {
    ledscube[i] = ledscube[(i + startIndex) % NUM_LEDS_CUBE];
  }

  FastLED.show();

  // Increment the rainbow hue to change colors
  hue++;

  // Move the rainbow gradient index
  startIndex++;
  if (startIndex >= NUM_LEDS_CUBE) {
    startIndex = 0;
  }

  // Introduce a delay to control the speed
  delay(speed);
}




void processCommand(String command) {
  char separator = ',';
  char semiColon = ';';

  int firstBracketIndex = command.indexOf('(');
  int secondBracketIndex = command.indexOf(')', firstBracketIndex + 1);

  while (firstBracketIndex != -1 && secondBracketIndex != -1) {
    String motorTypeAndSID = command.substring(firstBracketIndex + 1, secondBracketIndex);
    motorTypeAndSID.trim();
    int spaceIndex = motorTypeAndSID.indexOf(' ');

    if (spaceIndex != -1) {
      String motorType = motorTypeAndSID.substring(0, spaceIndex);
      String SID = motorTypeAndSID.substring(spaceIndex + 1);

      if (motorType == "PUMPMOTOR_OPERATION") {
        digitalWrite(driverOut1, HIGH);
        isReverse = false;
      } else if (motorType == "REVERSE_PUMPMOTOR_OPERATION") {
        digitalWrite(driverOut2, HIGH);
        isReverse = true;
      }

      if (motorType == "PUMPMOTOR_OPERATION" ||
          motorType == "REVERSE_PUMPMOTOR_OPERATION" ||
          motorType == "SERVOMOTOR_OPERATION" ||
          motorType == "STEPPERMOTOR_OPERATION" ||
          motorType == "LEDSTRIP_OPERATION" ||
          motorType == "LEDSTRIP_CUBE_OPERATION") {

        int index = secondBracketIndex + 1;
        while (index < command.length()) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              String param1 = inputData.substring(0, commaIndex);
              String param2 = inputData.substring(commaIndex + 1);

              // Call a function based on motorType
              if (motorType == "PUMPMOTOR_OPERATION") {
                runPumps(param1.toInt(), param2.toInt());
              } else if (motorType == "REVERSE_PUMPMOTOR_OPERATION") {
                runPumps(param1.toInt(), param2.toInt());
                ledStrip();
              } else if (motorType == "SERVOMOTOR_OPERATION") {
                runServo(param1.toInt(), param2.toInt());
              } else if (motorType == "STEPPERMOTOR_OPERATION") {
                runStepper(param1.toInt(), param2.toInt());
              } else if (motorType == "LEDSTRIP_OPERATION") {
                if (param2.toInt() == 1) {
                  ledStripWithBlink(100, 10000);
                } else if (param2.toInt() == 0) {
                  FastLED.clear();
                  FastLED.show();
                } else {
                  Serial.println("Invalid LED strip state");
                }
              } else if (motorType == "LEDSTRIP_CUBE_OPERATION") {
                if (param2.toInt() == 1) {
                  ledStripState = true;
                } else if (param2.toInt() == 0) {
                  ledStripState = false;
                  FastLED.clear();
                  FastLED.show();
                } else {
                  Serial.println("Invalid cube LED strip state");
                }
              }

              index = endIndex + 1;
            } else {
              Serial.println("Invalid data format");
              break;
            }
          } else {
            break;
          }
        }
      } else {
        Serial.println("Unknown motor type");
      }
    } else {
      Serial.println("Invalid motor type and SID format");
    }

    firstBracketIndex = command.indexOf('(', secondBracketIndex + 1);
    secondBracketIndex = command.indexOf(')', firstBracketIndex + 1);
  }
}

void loop() {
  unsigned long currentTime = millis();
  bool allPumpsOff = true;

  for (int i = 0; i < numPumps; i++) {
    if (currentTime >= pumpStartTimes[i] && digitalRead(motorPins[i]) == HIGH) {
      digitalWrite(motorPins[i], LOW);
    }

    if (currentTime < pumpStartTimes[i] || digitalRead(motorPins[i]) == HIGH) {
      allPumpsOff = false;
    }
  }

  if (allPumpsOff) {
    digitalWrite(driverOut1, LOW);
    digitalWrite(driverOut2, LOW);
  }

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    processCommand(data);
  }

  toggleLEDState();  // Check and toggle the cube LED strip state
}


//"(PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(845,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065)"
//"(REVERSE_PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(1045,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065)"
//"(PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(845,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(100,1))"
