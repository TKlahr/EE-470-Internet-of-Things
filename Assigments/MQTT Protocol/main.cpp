#include <Arduino.h>

// Pin definitions
#define POT_PIN A0
#define BUTTON_PIN D7
#define LED_PIN D2

// Variables
int potValue = 0;
int buttonState = HIGH;
int lastButtonState = HIGH;

unsigned long lastRead = 0;
const unsigned long readInterval = 1000;

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    digitalWrite(LED_PIN, LOW);

    Serial.println();
    Serial.println("EE-470 MQTT Project - Part A");
    Serial.println("Enter 1 to turn LED ON");
    Serial.println("Enter 0 to turn LED OFF");
}

void loop() {

    // Read potentiometer every second
    if (millis() - lastRead >= readInterval) {
        lastRead = millis();

        potValue = analogRead(POT_PIN);

        Serial.print("Potentiometer: ");
        Serial.println(potValue);
    }

    // Detect button press
    buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW && lastButtonState == HIGH) {
        Serial.println("Button pressed: 1");
    }

    lastButtonState = buttonState;

    // LED control through Serial Monitor
    if (Serial.available() > 0) {
        char command = Serial.read();

        if (command == '1') {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED ON");
        }
        else if (command == '0') {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED OFF");
        }
    }
}
