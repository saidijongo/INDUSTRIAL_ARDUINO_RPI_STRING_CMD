
#include <Arduino.h>
#include <Servo.h>

const int motorPins[] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};
const int dirPin = 12;
const int pulPin = 11;

const int driverOut1 = 83;
const int driverOut2 = 82;
const int servoPin = 12; 
const int servoPWM = 5; //3rd section in the MPAINO PWM

Servo myServo;


const int numPumps = sizeof(motorPins) / sizeof(motorPins[0]);
const int numStepsPerRevolution = 6400;


bool iswashing = true;

unsigned long pumpStartTimes[numPumps] = {0};

void setup() {
  for (int i = 0; i < numPumps; i++) {
    pinMode(motorPins[i], OUTPUT);
    digitalWrite(motorPins[i], LOW);
  }

  pinMode(dirPin, OUTPUT);
  pinMode(pulPin, OUTPUT);
  //pinMode(servoPWM, OUTPUT);
  myServo.attach(servoPin);


  Serial.begin(115200);
}

void runPumps(int pumpNumber, int runTime) {
  digitalWrite(motorPins[pumpNumber - 54], HIGH);
  pumpStartTimes[pumpNumber - 54] = millis() + runTime; // Set the time when the pump should be turned off
  Serial.print("Running pump: ");
  Serial.println(pumpNumber);
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
  //Serial.print(angle);
  //Serial.print(", Run time: ");
  //Serial.println(runTime);

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
  }

  // Run for the requested time
  //delay(runTime);
}



//only comma separated
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

      //Serial.print("Motor Type: ");
      //Serial.println(motorType);
      Serial.print("SID: ");
      Serial.println(SID);

      
      // Relay control logic based on motorType
      if (motorType == "PUMPMOTOR_OPERATION") {
        digitalWrite(driverOut1, HIGH);
        iswashing = true;
      } else if (motorType == "WASHING_OPERATION") {
        digitalWrite(driverOut2, HIGH);
        iswashing = true;
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

            if (motorType == "PUMPMOTOR_OPERATION") runPumps(param1, param2);
            else if (motorType == "SERVOMOTOR_OPERATION") runServo(param1, param2);
            else if (motorType == "STEPPERMOTOR_OPERATION") runStepper(param1, param2);
            else Serial.println("Unknown motor type");
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
    if (millis() >= pumpStartTimes[i] && digitalRead(motorPins[i]) == HIGH) {
      digitalWrite(motorPins[i], LOW);
    }
  }

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

    //"(PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500)"

//"(PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,5000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500)"

// "(PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(56,3000),(58,4250),(56,3000),(60,4000)"
//"(PUMPMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,2000),(56,3000),(57,4000),(58,3000)"
//"(SERVOMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(90,3000)"
//"(STEPPERMOTOR_OPERATION 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(360,2000)"
    processCommand(data);
  }
}


