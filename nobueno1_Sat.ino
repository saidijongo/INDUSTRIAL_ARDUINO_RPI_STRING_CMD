//new 2
#include <Arduino.h>
#include <Servo.h>
#include <FastLED.h>

const int motorPins[] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};

#define PULL_PIN 11    // PULL
#define DIR_PIN 12     // Direction
#define IR_PIN 13      // IR sensor
#define LED_PIN 45 //cubed 46

const int NUM_LEDS = 37;

const int driverPin = 84;
const int driverReversePin = 85;
const int servoPin = 44;

Servo myServo;

const int numPumps = sizeof(motorPins) / sizeof(motorPins[0]);
const int numStepsPerRevolution = 5000;
const int stepsPerRev = 5000;
const float STEP_ANGLE = 1.8;

#define GROUP_SIZE 5
float animation = 0;
float fadeAnimation = 0;
#define ANIMATION_DELAY 100

bool isReverse = true;
unsigned long maxRunTime = 0; // Maximum runtime for all pump motors
unsigned long pumpRunTimes[numPumps] = {0};  // Array to store individual pump run times

CRGB leds[NUM_LEDS];

void setup() {
  for (int i = 0; i < numPumps; i++) {
    pinMode(motorPins[i], OUTPUT);
  }

  pinMode(driverPin, OUTPUT);
  pinMode(driverReversePin, OUTPUT);

  myServo.attach(servoPin);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(100);
  FastLED.clear();
  FastLED.show();

  pinMode(DIR_PIN, OUTPUT);
  pinMode(PULL_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);

  Serial.begin(115200);
}

void runPumps(int pumpNumber, int runTime) {
  // Activate the relay for the specified pump
  digitalWrite(motorPins[pumpNumber - 1], HIGH);

  // Check the pumpNumber and control the corresponding driver pin
  if (digitalRead(driverReversePin) == LOW) {
    digitalWrite(driverPin, HIGH); // Turn on driverCW_PIN (ROTATECW)
  }
  if (digitalRead(driverPin) == LOW) {
    digitalWrite(driverReversePin, HIGH); // Turn on driverCCW_PIN (ROTATECCW)
  }

  // Update pump run time in the array
  pumpRunTimes[pumpNumber - 1] = runTime;

  // Update maxRunTime if the current pump's runTime is greater
  if (runTime > maxRunTime) {
    maxRunTime = runTime;
  }
}

void stopPumpMotors() {
  for (int i = 0; i < numPumps; i++) {
    digitalWrite(motorPins[i], LOW);
  }

  digitalWrite(driverPin, LOW);
  digitalWrite(driverReversePin, LOW);
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
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(blinkInterval);

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(blinkInterval);
  }
}

//void runStepper() {
  // Your stepper motor control logic goes here
  // This function should be called when all pump motors finish their individual run times
//}

void runStepper(int angle, int speed) {
  int direction = (angle >= 0) ? HIGH : LOW;
  angle = abs(angle);

  digitalWrite(DIR_PIN, direction);
  int step_target_position = int(2 * (angle / 1.8));

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

  if (digitalRead(IR_PIN) == LOW) {
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
        leds[i + j] = CRGB::Yellow;
      }
      FastLED.show();
      delay(ANIMATION_DELAY);

      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = CRGB::Blue;
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
    String commandTypeAndSID = command.substring(firstBracketIndex + 1, secondBracketIndex);
    commandTypeAndSID.trim();
    int spaceIndex = commandTypeAndSID.indexOf(',');

    if (spaceIndex != -1) {
      String commandType = commandTypeAndSID.substring(0, spaceIndex);
      String SID = commandTypeAndSID.substring(spaceIndex + 1);

      Serial.print("cmd Type: ");
      Serial.println(commandType);
      Serial.print("SID: ");
      Serial.println(SID);

      if (commandType == "PUMPMOTOR_OPERATION") {
        digitalWrite(driverPin, HIGH);
        isReverse = false;
      } else if (commandType == "REVERSE_PUMPMOTOR_OPERATION") {
        digitalWrite(driverReversePin, HIGH);
        isReverse = true;
      }

      if (commandType == "PUMPMOTOR_OPERATION" ||
          commandType == "REVERSE_PUMPMOTOR_OPERATION" ||
          commandType == "SERVOMOTOR_OPERATION" ||
          commandType == "STEPPERMOTOR_OPERATION" ||
          commandType == "LEDSTRIP_OPERATION") {

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

              if (commandType == "PUMPMOTOR_OPERATION" ||
                  commandType == "REVERSE_PUMPMOTOR_OPERATION") {
                runPumps(param1.toInt(), param2.toInt());
              } else if (commandType == "SERVOMOTOR_OPERATION") {
                runServo(param1.toInt(), param2.toInt());
              } else if (commandType == "STEPPERMOTOR_OPERATION") {
                runStepper(param1.toInt(), param2.toInt());
              } else if (commandType == "LEDSTRIP_OPERATION") {
                if (param2.toInt() == 1) {
                  ledStrip();
                } else if (param2.toInt() == 0) {
                  FastLED.clear();
                  FastLED.show();
                } else {
                  Serial.println("Invalid LED strip state");
                }

                index = endIndex + 1;
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

    firstBracketIndex = command.indexOf('(', secondBracketIndex + 1);
    secondBracketIndex = command.indexOf(')', firstBracketIndex + 1);
  }
}

void loop() {
  for (int i = 0; i < numPumps; i++) {
    if (millis() >= pumpRunTimes[i] && digitalRead(motorPins[i]) == HIGH) {
      digitalWrite(motorPins[i], LOW);
    }
  }

  if (millis() >= maxRunTime) {
    stopPumpMotors();
    FastLED.clear();
    FastLED.show();
    //runStepper(param1.toInt(), param2.toInt());
  }

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    Serial.println(data);
    processCommand(data);
  }
}


//"(PUMPMOTOR_OPERATION,1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500)";(SERVOMOTOR_OPERATION,1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION,1647eba3-a6b0-42a7-8a08-ffef8ab07065),(1045,1000);(LEDSTRIP_OPERATION,1647eba3-a6b0-42a7-8a08-ffef8ab07065),(100,1))"
//"(REVERSE_PUMPMOTOR_OPERATION,1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500)";(SERVOMOTOR_OPERATION,1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,1000);(STEPPERMOTOR_OPERATION,1647eba3-a6b0-42a7-8a08-ffef8ab07065),(1045,1000);(LEDSTRIP_OPERATION,1647eba3-a6b0-42a7-8a08-ffef8ab07065),(100,0))"
