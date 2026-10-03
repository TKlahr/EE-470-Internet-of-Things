/*
========================================
Title: ESP8266_Battery_Data_Logger
========================================

Program Detail:
----------------------------------------
Purpose: Automatically measure the voltage of a LiPo battery
         every 60 seconds using the ESP8266 ADC and a voltage
         divider. Output is formatted for CSV data collection.
Inputs: A0 analog input
Outputs: Time and battery voltage through the serial terminal
Date: 10/03/2026
Compiler: PlatformIO / Visual Studio Code
Author: Tyler Klahr

Versions:
V1 - Automatic 60-second battery voltage data logger

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

// ESP8266 ADC values
const float ADC_VOLTAGE = 3.3;
const int ADC_MAX = 1023;

// Keeps track of elapsed minutes
unsigned long minuteCount = 0;

void setup() {

  // Start serial communication
  Serial.begin(9600);

  delay(1000);

  // CSV column headings
  Serial.println("Time(min),BatteryVoltage(V)");
}

void loop() {

  // Read analog input
  int adcValue = analogRead(A0);

  // Convert ADC reading to voltage at A0
  float adcVoltage = adcValue * (ADC_VOLTAGE / ADC_MAX);

  // Calculate actual battery voltage
  float batteryVoltage =
      adcVoltage * ((R1 + R2) / R2);

  // Print data in CSV format
  Serial.print(minuteCount);
  Serial.print(",");
  Serial.println(batteryVoltage, 3);

  // Increase elapsed time by one minute
  minuteCount++;

  // Wait 60 seconds
  delay(60000);
}
