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

      if (motorType == "PUMP_OPERATION" ||
          motorType == "SERVOMOTOR_OPERATION" ||
          motorType == "STEPPERMOTOR_OPERATION" ||
          motorType == "LEDSTRIP_OPERATION") {

        int index = secondBracketIndex + 1;
        while (index < command.length() && index < semiColonIndex) {
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
              if (motorType == "PUMP_OPERATION") {
                runPumps(param1.toInt(), param2.toInt());
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
