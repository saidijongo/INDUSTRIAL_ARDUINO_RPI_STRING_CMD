//MODIFIED FNCS, bueno so far except pumps

#include <Arduino.h>
#include <Servo.h>
#include <FastLED.h>

const int motorPins[] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};

#define PULL_PIN 46    // PULL
#define DIR_PIN 45     // Direction
#define IR_PIN 13      // IR sensor
#define LED_PIN 11

//const int IR_PIN = 13;
const int NUM_LEDS = 32;

const int driverOut1 = 84;
const int driverOut2 = 85;
const int servoPin = 5;

const int numPumps = sizeof(motorPins) / sizeof(motorPins[0]);
const int stepsPerRev = 5000; // 6400;
const float STEP_ANGLE = 1.8; // degrees per step

#define GROUP_SIZE 5
float animation = 0;
float fadeAnimation = 0;
#define ANIMATION_DELAY 100

bool isReverse = true;
unsigned long pumpStartTimes[numPumps] = {0};
CRGB leds[NUM_LEDS];
Servo myServo;


void setup() {
  // Initialize motor pins as OUTPUT
  for (int i = 0; i < numPumps; i++) {
    pinMode(motorPins[i], OUTPUT);
  }

  // Initialize driver output pins as OUTPUT
  pinMode(driverOut1, OUTPUT);
  pinMode(driverOut2, OUTPUT);

  myServo.attach(servoPin);

  FastLED.addLeds<WS2812,LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(100);
  FastLED.clear();
  FastLED.show();

  // Initialize other pins
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PULL_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);

  // Initialize serial communication
  Serial.begin(115200);
}

void runPumps(int pumpNumber, int runTime) {
  digitalWrite(motorPins[pumpNumber - 54], HIGH);
  pumpStartTimes[pumpNumber - 54] = millis() + runTime; // Set the time when the pump should be turned off
  //Serial.print("Running pump: ");
  //Serial.println(pumpNumber);
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
  const int blinkDuration = 5000; // 5 seconds blink
  const int blinkInterval = 500;  // 500 milliseconds (0.5 seconds)
  unsigned long startTime = millis();
  while (millis() - startTime < blinkDuration) {
    // Turn on all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(blinkInterval);

    // Turn off all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(blinkInterval);
  }
}

void runStepper(int angle, int speed) {
  //int step_target_position = static_cast<int>(angle / STEP_ANGLE);
  int direction = (angle >= 0) ? HIGH : LOW;
  angle = abs(angle);

  digitalWrite(DIR_PIN, direction);
  int step_target_position = int(2 * (angle / STEP_ANGLE));


  int i = 0;
  while (i < step_target_position && digitalRead(IR_PIN) == HIGH) {
    digitalWrite(PULL_PIN, HIGH);  // Pulse the motor step
    delayMicroseconds(speed);
    digitalWrite(PULL_PIN, LOW);
    delayMicroseconds(speed);

    i++; // Increment step count
  }

  // Turn off the stepper motor after completing the rotation
  digitalWrite(PULL_PIN, LOW);
  blinkLEDs();

  // Check if the IR sensor is interrupted or the desired angle is reached
  if (digitalRead(IR_PIN) == LOW || i < step_target_position) {
    // Blink the LED strip for 5 seconds
    blinkLEDs();

    // Turn off the LED strip
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

      //Serial.print("Motor Type: ");
      //Serial.println(motorType);
      //Serial.print("SID: ");
      //Serial.println(SID);

      // Relay control logic based on motorType
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
          motorType == "LEDSTRIP_OPERATION") {

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

              Serial.print("Parameter 1: ");
              Serial.println(param1);
              Serial.print("Parameter 2: ");
              Serial.println(param2);

              // Call a function based on motorType
              if (motorType == "PUMPMOTOR_OPERATION") {
                runPumps(param1.toInt(), param2.toInt());

              }else if (motorType == "REVERSE_PUMPMOTOR_OPERATION") {
                runPumps(param1.toInt(), param2.toInt());
                ledStrip();               
              } else if (motorType == "SERVOMOTOR_OPERATION") {
                runServo(param1.toInt(), param2.toInt());
              } else if (motorType == "STEPPERMOTOR_OPERATION") {
                runStepper(param1.toInt(), param2.toInt());
              } else if (motorType == "LEDSTRIP_OPERATION") {
                if (param2.toInt() == 1) {
                  ledStrip();
                } else if (param2.toInt() == 0) {
                  // Turn off the LEDs
                  FastLED.clear();
                  FastLED.show();
                } else {
                  Serial.println("Invalid LED strip state");
                }

                index = endIndex + 1;  // Move to the next command
                continue;
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

    // Find the next command within the string
    firstBracketIndex = command.indexOf('(', secondBracketIndex + 1);
    secondBracketIndex = command.indexOf(')', firstBracketIndex + 1);
  }
}


void loop() {
  // Check and deactivate pumps if the run time has elapsed
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
    Serial.println(data);
    processCommand(data);
  }
}


//"(PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500)";(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(1045,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065)"
//"(REVERSE_PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500)";(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(1045,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065)"


//"(PUMP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065), (56,3000),(58,4250),(76,3000),(80,4250);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(1045,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065)"
//Jongo"(PUMP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065), (55,3000),(58,4250),(56,3000),(70,4250);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(45,1000)"      
//"(PUMP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065), (56,3000),(58,4250),(56,3000),(58,4250);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(45,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065)"
