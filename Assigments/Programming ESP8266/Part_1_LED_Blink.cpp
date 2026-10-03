/*
========================================
Title: ESP8266_LED_Blink
========================================

Program Detail:
----------------------------------------
Purpose: Blink the built-in LED on the ESP8266 and display
         the LED status in the serial terminal.
Inputs: None
Outputs: Built-in LED and serial terminal messages
Date: 10/03/2026
Compiler: PlatformIO / Visual Studio Code
Author: Tyler Klahr

Versions:
V1 - Initial version of LED blink program

========================================
File Dependencies:
----------------------------------------
Arduino.h

========================================
Main Program
========================================
*/

#include <Arduino.h>

void setup() {
  // Set the built-in LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);

  // Start serial communication
  Serial.begin(9600);
}

void loop() {
  // Turn LED ON
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("LED is ON");
  delay(1000);

  // Turn LED OFF
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("LED is OFF");
  delay(1000);
}
