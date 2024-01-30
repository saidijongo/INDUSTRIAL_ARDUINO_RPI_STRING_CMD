void setup() {
  Serial.begin(115200);

  String inputCommand = "(ROTATECW 1647eba3-a6b0-42a7-8a08-ffef8ab07065),(54,1000),(55,3500),(56,2600),(57,1000),(58,2500),(59,4000),(59,1000),(60,5500),(61,500),(62,3600),(64,1000),(65,2500),(66,4000),(67,1000),(68,5500),(69,7000),(70,3600),(71,2000),(75,2500),(80,4000),(83,1000),(78,5500)";

  // Find the maximum runTime
  int maxRunTime = 0;

  // Start parsing from index 0 to skip the initial command
  int startIndex = 0;

  // Iterate through the string to find subsequent runTime values
  while (startIndex < inputCommand.length()) {
    // Find the next occurrence of '(' and ')'
    int openParenIndex = inputCommand.indexOf('(', startIndex);
    int closeParenIndex = inputCommand.indexOf(')', openParenIndex);

    if (openParenIndex != -1 && closeParenIndex != -1) {
      // Extract the substring between '(' and ')'
      String bracketContent = inputCommand.substring(openParenIndex + 1, closeParenIndex);

      // Find the second parameter (runTime) within the substring
      int commaIndex = bracketContent.indexOf(',');
      if (commaIndex != -1) {
        String runTimeString = bracketContent.substring(commaIndex + 1);
        int runTime = runTimeString.toInt();

        // Update maxRunTime if the current runTime is greater
        maxRunTime = max(maxRunTime, runTime);
      }

      // Move the startIndex to the next character after ')'
      startIndex = closeParenIndex + 1;
    } else {
      // If no more brackets are found, exit the loop
      break;
    }
  }

  // Display the maximum runTime
  Serial.print("Maximum runTime: ");
  Serial.println(maxRunTime);
}

void loop() {
    // Display the maximum runTime

}
