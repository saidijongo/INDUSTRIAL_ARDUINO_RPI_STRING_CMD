#include <Arduino.h>

const int motorPins[] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};
const int dirPin = 45;
const int pulPin = 46;
const int servoPWM = 44;

const int driverOut1 = 84;
const int driverOut2 = 85;
int maxRunTime = 0;

const int numPumps = sizeof(motorPins) / sizeof(motorPins[0]);

bool isReverse = true;

unsigned long pumpStartTimes[numPumps] = {0};
unsigned long pumpEndTimes[numPumps] = {0};

void setup() {
  for (int i = 0; i < numPumps; i++) {
    pinMode(motorPins[i], OUTPUT);
    digitalWrite(motorPins[i], LOW);
  }

  pinMode(dirPin, OUTPUT);
  pinMode(pulPin, OUTPUT);
  pinMode(servoPWM, OUTPUT);

  Serial.begin(115200);
}

void runPumps(int pumpNumber, int runTime) {
  digitalWrite(motorPins[pumpNumber - 54], HIGH);
  pumpStartTimes[pumpNumber - 54] = millis();
  pumpEndTimes[pumpNumber - 54] = pumpStartTimes[pumpNumber - 54] + runTime; // Set the time when the pump should be turned off
  Serial.print("Running pump: ");
  Serial.println(pumpNumber);

  // Update maxRunTime if the current runTime is greater
  maxRunTime = max(maxRunTime, runTime);
}

void runServo(int angle, int runTime) {
  // Move the servo to the specified angle
  analogWrite(servoPWM, angle);

  // Wait for the specified time
  delay(runTime);
}

void runStepper(int angle, int runTime) {
  int direction = (angle >= 0) ? HIGH : LOW;
  angle = abs(angle);
  digitalWrite(dirPin, direction);

  // Calculate the number of steps based on the angle
  int steps = int(2 * (angle / 1.8));

  // Run the stepper motor
  for (int i = 0; i < steps; i++) {
    digitalWrite(pulPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pulPin, LOW);
    delayMicroseconds(500);
  }

  // Run for the requested time
  delay(runTime);
}

void processCommand(String command) {
  char separator = ',';

  int firstBracketIndex = command.indexOf('(');
  int secondBracketIndex = command.indexOf(')', firstBracketIndex + 1);

  if (firstBracketIndex != -1 && secondBracketIndex != -1) {
    // Extract motor type and SID from the first set of brackets
    String motorTypeAndSID = command.substring(firstBracketIndex + 1, secondBracketIndex);
    int spaceIndex = motorTypeAndSID.indexOf(' ');

    if (spaceIndex != -1) {
      String motorType = motorTypeAndSID.substring(0, spaceIndex);
      String SID = motorTypeAndSID.substring(spaceIndex + 1);

      Serial.print("SID: ");
      Serial.println(SID);

      // Relay control logic based on motorType
      if (motorType == "PUMPMOTOR_OPERATION") {
        digitalWrite(driverOut1, HIGH);
        isReverse = false;
      } else if (motorType == "REVERSE_PUMPMOTOR_OPERATION") {
        digitalWrite(driverOut2, HIGH);
        isReverse = true;
      }

      // Extract pump number/angle and run time from subsequent brackets
      int index = secondBracketIndex + 1;
      while (index < command.length()) {
        int nextBracketIndex = command.indexOf('(', index);
        int endIndex = command.indexOf(')', nextBracketIndex + 1);

        if (nextBracketIndex != -1 && endIndex != -1) {
          String inputData = command.substring(nextBracketIndex + 1, endIndex);

          int commaIndex = inputData.indexOf(separator);
          if (commaIndex != -1) {
            int param1 = inputData.substring(0, commaIndex).toInt();
            int param2 = inputData.substring(commaIndex + 1).toInt();

            Serial.print(motorType);
            Serial.print(": Param1: ");
            Serial.print(param1);
            Serial.print(", Param2: ");
            Serial.println(param2);

            // Update maxRunTime if the current runTime is greater
            maxRunTime = max(maxRunTime, param2);
            Serial.print(", maxRunTime: ");
            Serial.println(maxRunTime);
            if (motorType == "PUMPMOTOR_OPERATION") {
              runPumps(param1, param2);
            } else if (motorType == "REVERSE_PUMPMOTOR_OPERATION") {
              runPumps(param1, param2);
            } else if (motorType == "SERVOMOTOR_OPERATION") {
              runServo(param1, param2);
            } else if (motorType == "STEPPERMOTOR_OPERATION") {
              runStepper(param1, param2);
            } else if (motorType == "LEDSTRIP_OPERATION") {
              runServo(param1, param2); // param1 =speed, param2 ON(1) or OFF(0)
            } else {
              Serial.println("Unknown motor type");
            }
          } else {
            Serial.println("Invalid pump data format");
          }

          index = endIndex + 1;
        } else {
          break;
        }
      }
    } else {
      Serial.println("Invalid motor type and SID format");
    }
  } else {
    Serial.println("Invalid command format");
  }
}

void loop() {
  // Check and deactivate pumps if the run time has elapsed
  for (int i = 0; i < numPumps; i++) {
    if (millis() >= pumpEndTimes[i] && digitalRead(motorPins[i]) == HIGH) {
      digitalWrite(motorPins[i], LOW);
    }
  }

  // Stop driverOut1 and driverOut2 at maxRunTime
  if (millis() >= maxRunTime) {
    digitalWrite(driverOut1, LOW);
    digitalWrite(driverOut2, LOW);
  }

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    processCommand(data);
  }
}


// "(PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(56,3000),(59,4250),(60,9000),(65,4000),(70,3000),(75,4250),(80,9000),(80,4000),(59,4250),(60,9000),(65,4000),(70,3000),(75,4250),(82,6000),(81,4000)"
//"(REVERSE_PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(56,3000),(59,4250),(60,9000),(65,4000),(70,3000),(75,4250),(80,9000),(80,4000),(59,4250),(60,9000),(65,4000),(70,3000),(75,4250),(82,6000),(81,4000)"
