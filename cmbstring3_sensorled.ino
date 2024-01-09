
#include <Arduino.h>
#include <Servo.h>
#include <FastLED.h>

const int motorPins[] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};
const int dirPin = 13;
const int pulPin = 12;

const int IR_PIN = 46;
const int NUM_LEDS = 32;

const int driverOut1 = 82;
const int driverOut2 = 83;
const int servoPin = 12; 

Servo myServo;

const int numPumps = sizeof(motorPins) / sizeof(motorPins[0]);
const int numStepsPerRevolution = 6400;

bool iswashing = true;

unsigned long pumpStartTimes[numPumps] = {0};
CRGB leds[NUM_LEDS];

void runPumps(int pumpNumber, int runTime) {
  Serial.print("Running pump ");
  Serial.print(pumpNumber);
  Serial.print(" for ");
  Serial.print(runTime);
  Serial.println(" ms");

  // Start the pump
  digitalWrite(motorPins[pumpNumber], HIGH);

  // Record the start time
  pumpStartTimes[pumpNumber] = millis();

  // Wait for the specified run time
  delay(runTime);

  // Stop the pump
  digitalWrite(motorPins[pumpNumber], LOW);
}

void runServo(int angle, int runSpeed) {
  Serial.print("Running servo at angle ");
  Serial.print(angle);
  Serial.print(" with run speed ");
  Serial.println(runSpeed);

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
  Serial.print("Running stepper motor at angle ");
  Serial.print(angle);
  Serial.print(" for ");
  Serial.print(runTime);
  Serial.println(" ms");

  int direction = (angle >= 0) ? HIGH : LOW;
  angle = abs(angle);
  digitalWrite(dirPin, direction);

  // Calculate the number of steps based on the angle
  int steps = angle * numStepsPerRevolution / 360;

  // Run the stepper motor
  for (int i = 0; i < steps; i++) {
    digitalWrite(pulPin, HIGH);
    delayMicroseconds(runTime);
    digitalWrite(pulPin, LOW);
    delayMicroseconds(runTime);

    // Check IR sensor interruption while running the stepper
    if (digitalRead(IR_PIN) == LOW) {
      Serial.println("IR Sensor interrupted, stopping stepper motor");
      break;
    }
  }
}

void ledStrip1() {
  static uint8_t startIndex = 0;  // Starting index of the green LED group
  static bool moveRight = true;   // Direction flag

  // Set the background color to blue
  fill_solid(leds, NUM_LEDS, CRGB::Blue);

  // Set the color of the moving green LEDs
  for (int i = 0; i < 5; ++i) {
    leds[(startIndex + i) % NUM_LEDS] = CRGB::Green;
  }

  FastLED.show();
  FastLED.delay(100);  

  // Clear the LEDs at the current position
  for (int i = 0; i < 5; ++i) {
    leds[(startIndex + i) % NUM_LEDS] = CRGB::Black;
  }

  // Move the green LED group
  if (moveRight) {
    startIndex = (startIndex + 1) % NUM_LEDS;
  } else {
    startIndex = (startIndex + NUM_LEDS - 1) % NUM_LEDS;
  }

  // Change direction when reaching the end
  if (startIndex == 0 || startIndex == NUM_LEDS - 5) {
    moveRight = !moveRight;
  }

  // Check if all pumps have finished running
  bool allPumpsStopped = true;
  for (int i = 0; i < numPumps; ++i) {
    if (digitalRead(motorPins[i]) == HIGH) {
      allPumpsStopped = false;
      break;
    }
  }

  // If all pumps have stopped, start blinking the whole LED strip
  if (allPumpsStopped) {
    for (int i = 0; i < NUM_LEDS; ++i) {
      if (i % 2 == 0) {
        leds[i] = CRGB::Blue;
      } else {
        leds[i] = CRGB::Black;
      }
    }
    FastLED.show();
    FastLED.delay(500);
    FastLED.clear();
    FastLED.show();
  }
}


void processCommand(String command) {
  Serial.println(command);
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
        // Start LED strip pattern while pumps are running
        ledStrip1();
        // Stop LED strip blinking after pumps finish
        FastLED.clear();
        FastLED.show();
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
                ledStrip1();
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



void setup() {
  FastLED.addLeds<NEOPIXEL, 11>(leds, NUM_LEDS);
  Serial.begin(115200);
  myServo.attach(servoPin);

  for (int i = 0; i < numPumps; ++i) {
    pinMode(motorPins[i], OUTPUT);
  }
  pinMode(dirPin, OUTPUT);
  pinMode(pulPin, OUTPUT);
  pinMode(IR_PIN, INPUT);
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    processCommand(data);
  }
}

//"(PUMP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065), (55,3000),(58,4250),(56,3000),(70,4250);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(100,3000)"      
//"(PUMP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065), (56,3000),(58,4250),(56,3000),(58,4250);(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(45,1000);(LEDSTRIP_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065)"
