/*
========================================
Title: ESP8266_ADC_Test
========================================

Program Detail:
----------------------------------------
Purpose: Measure the voltage of a LiPo battery using the
         ESP8266 analog input and a voltage divider.
Inputs: A0 analog input
Outputs: Battery voltage displayed in the serial terminal
Date: 10/03/2026
Compiler: PlatformIO / Visual Studio Code
Author: Tyler Klahr

Versions:
V1 - Initial ADC voltage measurement program

========================================
File Dependencies:
----------------------------------------
Arduino.h

========================================
Main Program
========================================
*/

#include <Arduino.h>

// Voltage divider resistor values
const float R1 = 10000.0;   // 10 kOhm
const float R2 = 22000.0;   // 22 kOhm

// Maximum voltage represented by the ADC
const float ADC_VOLTAGE = 3.3;

// Maximum ADC reading for the ESP8266
const int ADC_MAX = 1023;

void setup() {

  // Start serial communication
  Serial.begin(9600);

  delay(1000);

  Serial.println("ESP8266 Battery Voltage Test");
  Serial.println("----------------------------");
}

void loop() {

  // Read the analog value from A0
  int adcValue = analogRead(A0);

  // Convert ADC reading to voltage at A0
  float adcVoltage = adcValue * (ADC_VOLTAGE / ADC_MAX);

  // Convert A0 voltage back to actual battery voltage
  float batteryVoltage = adcVoltage * ((R1 + R2) / R2);

  // Display the results
  Serial.print("ADC: ");
  Serial.print(adcValue);

  Serial.print("   A0 Voltage: ");
  Serial.print(adcVoltage, 3);

  Serial.print(" V   Battery Voltage: ");
  Serial.print(batteryVoltage, 3);
  Serial.println(" V");

  // Wait one second before next measurement
  delay(1000);
}
