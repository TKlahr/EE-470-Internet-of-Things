#include <Arduino.h>
#include <ESP8266WiFi.h>

/*
 * EE-470 IoT Project
 * Wi-Fi Connection Test
 * Version 1.5
 *
 * Input: Wi-Fi network credentials
 * Output: Connection status, IP address, RSSI
 */

const char* wifiSSID = "raidernation75";
const char* wifiPassword = "raiderswin";

void connectWiFi() {
    Serial.print("Connecting to Wi-Fi");

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID, wifiPassword);

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < 15000) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Wi-Fi connected successfully!");

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        Serial.print("RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("Wi-Fi connection failed!");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("EE-470 ESP8266 Wi-Fi Test");

    connectWiFi();
}

void loop() {
    // No continuous sensor readings needed for this test.
}
