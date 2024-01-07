

#include <Arduino.h>
#include <Servo.h>
#include <FastLED.h>

const int motorPins[] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};
const int dirPin = 12;
const int pulPin = 11;
const int IR_PIN = 13;
const int NUM_LEDS = 32;

const int driverOut1 = 82;
const int driverOut2 = 83;
const int servoPin = 5;

Servo myServo;

const int numPumps = sizeof(motorPins) / sizeof(motorPins[0]);
const int numStepsPerRevolution = 5000; //6400

#define GROUP_SIZE 5
float animation = 0;
float fadeAnimation = 0;
#define ANIMATION_DELAY 100

bool iswashing = true;
unsigned long pumpStartTimes[numPumps] = {0};
CRGB leds[NUM_LEDS];


void setup() {
  // Initialize motor pins as OUTPUT
  for (int i = 0; i < numPumps; i++) {
    pinMode(motorPins[i], OUTPUT);
  }

  // Initialize driver output pins as OUTPUT
  pinMode(driverOut1, OUTPUT);
  pinMode(driverOut2, OUTPUT);

  // Initialize servo motor
  myServo.attach(servoPin);

  // Initialize LED strip
  FastLED.addLeds<WS2812, pulPin, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(50);
  FastLED.clear();
  FastLED.show();

  // Initialize other pins
  pinMode(dirPin, OUTPUT);
  pinMode(pulPin, OUTPUT);
  pinMode(IR_PIN, INPUT);

  // Initialize serial communication
  Serial.begin(115200);
}

void runPumps(int pumpNumber, int runTime) {
  Serial.println("Running pump " + String(pumpNumber) + " for " + String(runTime) + " milliseconds");
  digitalWrite(motorPins[pumpNumber], HIGH);
  pumpStartTimes[pumpNumber] = millis();
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

void runStepper(int angle, int runTime) {
  Serial.print("Running stepper motor, Angle: " + String(angle) + " Run Time: " + String(runTime) + "\r\n");

  int direction = (angle >= 0) ? HIGH : LOW;
  angle = abs(angle);
  digitalWrite(dirPin, direction);

  int steps = angle * numStepsPerRevolution / 360;

  for (int i = 0; i < steps; i++) {
    digitalWrite(pulPin, HIGH);
    delayMicroseconds(runTime);
    digitalWrite(pulPin, LOW);
    delayMicroseconds(runTime);
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

/*
void processCommand(String command) {
  char separator = ',';
  char semiColon = ';';

  int firstBracketIndex = command.indexOf('(');
  int secondBracketIndex = command.indexOf(')', firstBracketIndex + 1);

  if (firstBracketIndex != -1 && secondBracketIndex != -1) {
    String motorTypeAndSID = command.substring(firstBracketIndex + 1, secondBracketIndex);
    motorTypeAndSID.trim();
    int spaceIndex = motorTypeAndSID.indexOf(' ');

    if (spaceIndex != -1) {
      String motorType = motorTypeAndSID.substring(0, spaceIndex);
      String SID = motorTypeAndSID.substring(spaceIndex + 1);

      Serial.print("Motor Type: ");
      Serial.println(motorType);
      Serial.print("SID: ");
      Serial.println(SID);

      if (motorType == "PUMP_OPERATION") {
        int index = secondBracketIndex + 1;
        while (index < command.length()) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              int pumpNumber = inputData.substring(0, commaIndex).toInt();
              int runTime = inputData.substring(commaIndex + 1).toInt();

              Serial.print("Pump Number: ");
              Serial.println(pumpNumber);
              Serial.print("Run Time: ");
              Serial.println(runTime);

              runPumps(pumpNumber, runTime);

              index = endIndex + 1;
            } else {
              Serial.println("Invalid pump data format");
            }
          } else {
            break;
          }
        }
      } else if (motorType == "SERVOMOTOR_OPERATION") {
        int index = secondBracketIndex + 1;
        while (index < command.length()) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              int angle = inputData.substring(0, commaIndex).toInt();
              int runSpeed = inputData.substring(commaIndex + 1).toInt();

              Serial.print("Servo Angle: ");
              Serial.println(angle);
              Serial.print("Run Speed: ");
              Serial.println(runSpeed);

              runServo(angle, runSpeed);

              index = endIndex + 1;
            } else {
              Serial.println("Invalid servo data format");
            }
          } else {
            break;
          }
        }
      } else if (motorType == "STEPPERMOTOR_OPERATION") {
        int index = secondBracketIndex + 1;
        while (index < command.length()) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              int angle = inputData.substring(0, commaIndex).toInt();
              int runTime = inputData.substring(commaIndex + 1).toInt();

              Serial.print("Stepper Angle: ");
              Serial.println(angle);
              Serial.print("Run Time: ");
              Serial.println(runTime);

              runStepper(angle, runTime);

              index = endIndex + 1;
            } else {
              Serial.println("Invalid stepper data format");
            }
          } else {
            break;
          }
        }
      } else if (motorType == "LEDSTRIP_OPERATION") {
        int index = secondBracketIndex + 1;
        while (index < command.length()) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              String patternName = inputData.substring(0, commaIndex);
              int stripState = inputData.substring(commaIndex + 1).toInt();

              Serial.print("LED Strip Pattern: ");
              Serial.println(patternName);
              Serial.print("Strip State: ");
              Serial.println(stripState);

              if (stripState == 1) {
                ledStrip();
              } else if (stripState == 0) {
                // Turn off the LEDs
                FastLED.clear();
                FastLED.show();
              } else {
                Serial.println("Invalid LED strip state");
              }

              index = endIndex + 1;
            } else {
              Serial.println("Invalid LED strip data format");
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
  } else {
    Serial.println("Invalid command format");
  }
}
*/

void processCommand(String command) {
  char separator = ',';
  char semiColon = ';';

  int firstBracketIndex = command.indexOf('(');
  int secondBracketIndex = command.indexOf(')', firstBracketIndex + 1);

  // Check if there is a semicolon in the command
  int semiColonIndex = command.indexOf(semiColon);

  while (firstBracketIndex != -1 && secondBracketIndex != -1) {
    String motorTypeAndSID = command.substring(firstBracketIndex + 1, secondBracketIndex);
    motorTypeAndSID.trim();
    int spaceIndex = motorTypeAndSID.indexOf(' ');

    if (spaceIndex != -1) {
      String motorType = motorTypeAndSID.substring(0, spaceIndex);
      String SID = motorTypeAndSID.substring(spaceIndex + 1);

      Serial.print("Motor Type: ");
      Serial.println(motorType);
      Serial.print("SID: ");
      Serial.println(SID);

      if (motorType == "PUMP_OPERATION") {
        int index = secondBracketIndex + 1;
        while (index < command.length() && index < semiColonIndex) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              int pumpNumber = inputData.substring(0, commaIndex).toInt();
              int runTime = inputData.substring(commaIndex + 1).toInt();

              Serial.print("Pump Number: ");
              Serial.println(pumpNumber);
              Serial.print("Run Time: ");
              Serial.println(runTime);

              runPumps(pumpNumber, runTime);

              index = endIndex + 1;
            } else {
              Serial.println("Invalid pump data format");
              break;
            }
          } else {
            break;
          }
        }
      } else if (motorType == "SERVOMOTOR_OPERATION") {
        int index = secondBracketIndex + 1;
        while (index < command.length() && index < semiColonIndex) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              int angle = inputData.substring(0, commaIndex).toInt();
              int runSpeed = inputData.substring(commaIndex + 1).toInt();

              Serial.print("Servo Angle: ");
              Serial.println(angle);
              Serial.print("Run Speed: ");
              Serial.println(runSpeed);

              runServo(angle, runSpeed);

              index = endIndex + 1;
            } else {
              Serial.println("Invalid servo data format");
              break;
            }
          } else {
            break;
          }
        }
      } else if (motorType == "STEPPERMOTOR_OPERATION") {
        int index = secondBracketIndex + 1;
        while (index < command.length() && index < semiColonIndex) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              int angle = inputData.substring(0, commaIndex).toInt();
              int runTime = inputData.substring(commaIndex + 1).toInt();

              Serial.print("Stepper Angle: ");
              Serial.println(angle);
              Serial.print("Run Time: ");
              Serial.println(runTime);

              runStepper(angle, runTime);

              index = endIndex + 1;
            } else {
              Serial.println("Invalid stepper data format");
              break;
            }
          } else {
            break;
          }
        }
      } else if (motorType == "LEDSTRIP_OPERATION") {
        int index = secondBracketIndex + 1;
        while (index < command.length() && index < semiColonIndex) {
          int nextBracketIndex = command.indexOf('(', index);
          int endIndex = command.indexOf(')', nextBracketIndex + 1);

          if (nextBracketIndex != -1 && endIndex != -1) {
            String inputData = command.substring(nextBracketIndex + 1, endIndex);

            int commaIndex = inputData.indexOf(separator);
            if (commaIndex != -1) {
              String patternName = inputData.substring(0, commaIndex);
              int stripState = inputData.substring(commaIndex + 1).toInt();

              Serial.print("LED Strip Pattern: ");
              Serial.println(patternName);
              Serial.print("Strip State: ");
              Serial.println(stripState);

              if (stripState == 1) {
                ledStrip();
              } else if (stripState == 0) {
                // Turn off the LEDs
                FastLED.clear();
                FastLED.show();
              } else {
                Serial.println("Invalid LED strip state");
              }

              index = endIndex + 1;
            } else {
              Serial.println("Invalid LED strip data format");
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
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    processCommand(data);
  }
}

//"(PUMP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065), (55,3000),(58,4250),(56,3000),(70,4250);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(45,1000)"      
//"(PUMP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065), (56,3000),(58,4250),(56,3000),(58,4250);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(45,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065)"
