/*
 * ------------------------------------------------------------
 * Project/Program Name : EE-470 ESP8266 IoT Sensor Monitoring
 * File Name            : main.cpp
 * Author               : Tyler Klahr
 * Date                 : 11/11/2026
 * Version              : 2.2
 *
 * Purpose:
 *   Read temperature, humidity, and light measurements using
 *   an ESP8266. Transmit measurements to a MySQL database when
 *   a pushbutton or tilt switch is activated.
 *
 * Inputs:
 *   DHT11 Temperature/Humidity Sensor - D5 (GPIO14)
 *   Photoresistor Sensor              - A0
 *   Pushbutton                        - D6 (GPIO12)
 *   Tilt Switch                       - D7 (GPIO13)
 *   Time Zone Selection               - USB Serial Interface
 *
 * Outputs:
 *   Sensor measurements to Hostinger MySQL database
 *   Wi-Fi RSSI and transmission status through USB Serial
 *   Time zone menu and generated timestamps
 *
 * Example Application:
 *   Event-triggered environmental monitoring using an ESP8266
 *   with reliable database transmission.
 *
 * Dependencies:
 *   ESP8266 NodeMCU development board
 *   Arduino framework and PlatformIO
 *   ESP8266WiFi, ESP8266HTTPClient, BearSSL
 *   Adafruit DHT Sensor Library
 *   Adafruit Unified Sensor Library
 *   timeapi.io and Hostinger PHP/MySQL server
 *
 * Usage Notes:
 *   Select a time zone through the USB Serial Monitor.
 *   Default selection is Pacific Time (option 4).
 *   Pushbutton sends measurements as node_1.
 *   Tilt switch sends measurements as node_2.
 *   Failed transmissions retry every 30 seconds.
 *   Pending measurements are stored in RAM.
 *
 * ------------------------------------------------------------
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DHT.h>
#include <time.h>
#include <math.h>

// --------------------------------------------------
// Wi-Fi and server configuration
// --------------------------------------------------

const char* wifiSSID = "Tyler's iPhone (2)";
const char* wifiPassword = "Tylerhotspot";

const char* serverURL =
    "https://klahree.com/ESP8266_IoT/insert.php";

// --------------------------------------------------
// Pin configuration
// --------------------------------------------------

#define DHT_PIN D5
#define DHT_TYPE DHT11
#define LIGHT_PIN A0
#define BUTTON_PIN D6
#define TILT_PIN D7

DHT dht(DHT_PIN, DHT_TYPE);

// --------------------------------------------------
// Timing configuration
// --------------------------------------------------

const unsigned long debounceDelay = 50;
const unsigned long retryInterval = 30000;
const unsigned long serialSelectionTimeout = 10000;

// --------------------------------------------------
// Time zone configuration
// --------------------------------------------------

// Time zones listed in Appendix A
const char* timeZoneNames[] = {
    "Eastern Time - New York",
    "Central Time - Chicago",
    "Mountain Time - Denver",
    "Pacific Time - Los Angeles",
    "Alaska Time - Anchorage",
    "Hawaii-Aleutian Time - Honolulu",
    "Atlantic Time - San Juan"
};

const char* timeZoneAPI[] = {
    "America/New_York",
    "America/Chicago",
    "America/Denver",
    "America/Los_Angeles",
    "America/Anchorage",
    "Pacific/Honolulu",
    "America/Puerto_Rico"
};

// POSIX time zone rules for ESP8266 localtime()
// These account for daylight saving time where applicable.
const char* timeZoneRules[] = {
    "EST5EDT,M3.2.0/2,M11.1.0/2",
    "CST6CDT,M3.2.0/2,M11.1.0/2",
    "MST7MDT,M3.2.0/2,M11.1.0/2",
    "PST8PDT,M3.2.0/2,M11.1.0/2",
    "AKST9AKDT,M3.2.0/2,M11.1.0/2",
    "HST10",
    "AST4"
};

// Default is Pacific Time (option 4)
int selectedTimeZone = 4;

// --------------------------------------------------
// Measurement structure
// --------------------------------------------------

struct Measurement {
    float temperature;
    float humidity;
    int lightLevel;
    String timestamp;
    bool pending;
    unsigned long lastAttempt;
};

Measurement node1 = {};
Measurement node2 = {};

// --------------------------------------------------
// Switch structure
// --------------------------------------------------

struct SwitchState {
    uint8_t pin;
    bool stableState;
    bool lastReading;
    unsigned long lastChange;
};

SwitchState button = {
    BUTTON_PIN, HIGH, HIGH, 0
};

SwitchState tilt = {
    TILT_PIN, HIGH, HIGH, 0
};

// --------------------------------------------------
// Wi-Fi connection
// --------------------------------------------------

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
        Serial.println("Wi-Fi connected!");

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        Serial.print("RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("Wi-Fi connection failed.");
    }
}

// --------------------------------------------------
// Display time zone menu
// --------------------------------------------------

void displayTimeZoneMenu() {
    Serial.println();
    Serial.println("Select Your Time Zone:");
    Serial.println();

    for (int i = 0; i < 7; i++) {
        Serial.print(i + 1);
        Serial.print(") ");
        Serial.println(timeZoneNames[i]);
    }

    Serial.println();
    Serial.println(
        "Select Your Time Zone (Default: 4 - PT):"
    );
}

// --------------------------------------------------
// Read time zone selection from USB Serial
// --------------------------------------------------

void selectTimeZone() {
    displayTimeZoneMenu();

    unsigned long startTime = millis();

    while (millis() - startTime <
           serialSelectionTimeout) {

        if (Serial.available() > 0) {
            char selection = Serial.read();

            if (selection >= '1' && selection <= '7') {
                selectedTimeZone = selection - '0';
                break;
            }
        }

        delay(10);
    }

    // Clear remaining serial input
    while (Serial.available() > 0) {
        Serial.read();
    }

    Serial.println();
    Serial.print("Selected: ");
    Serial.println(
        timeZoneNames[selectedTimeZone - 1]
    );
}

// --------------------------------------------------
// Retrieve selected time zone from external API
// --------------------------------------------------

void retrieveTimeFromAPI() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Time API unavailable: No Wi-Fi.");
        return;
    }

    String url =
        "https://timeapi.io/api/Time/current/zone?timeZone=";

    url += timeZoneAPI[selectedTimeZone - 1];

    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(10000);

    Serial.println("Contacting timeapi.io...");

    if (!http.begin(client, url)) {
        Serial.println("Time API connection failed.");
        return;
    }

    int httpCode = http.GET();

    Serial.print("Time API HTTP code: ");
    Serial.println(httpCode);

    if (httpCode == 200) {
        String response = http.getString();

        // The API returns JSON containing dateTime.
        // For this version, NTP will maintain the actual
        // clock used for measurement timestamps.
        if (response.indexOf("\"dateTime\"") >= 0) {
            Serial.println("Time API response received.");
        } else {
            Serial.println("Unexpected time API response.");
        }
    } else {
        Serial.println("Time API request unsuccessful.");
    }

    http.end();
}

// --------------------------------------------------
// Synchronize internal clock using NTP
// --------------------------------------------------

void synchronizeTime() {
    // Configure selected time zone including DST
    setenv(
        "TZ",
        timeZoneRules[selectedTimeZone - 1],
        1
    );

    tzset();

    // NTP provides UTC; localtime() applies TZ rules
    configTime(
        0,
        0,
        "pool.ntp.org",
        "time.nist.gov"
    );

    Serial.print("Synchronizing clock");

    unsigned long startTime = millis();

    while (time(nullptr) < 1000000000 &&
           millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (time(nullptr) >= 1000000000) {
        Serial.println("Clock synchronized!");
    } else {
        Serial.println("Clock synchronization failed.");
    }
}

// --------------------------------------------------
// Generate timestamp
// --------------------------------------------------

String getTimestamp() {
    time_t now = time(nullptr);

    if (now < 1000000000) {
        return "";
    }

    struct tm timeInfo;

    if (localtime_r(&now, &timeInfo) == nullptr) {
        return "";
    }

    char timestamp[20];

    strftime(
        timestamp,
        sizeof(timestamp),
        "%Y-%m-%d %H:%M:%S",
        &timeInfo
    );

    return String(timestamp);
}

// --------------------------------------------------
// Transmit saved measurement
// --------------------------------------------------

bool transmitMeasurement(
    const char* node,
    Measurement &data
) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi disconnected. Will retry.");
        return false;
    }

    String encodedTime = data.timestamp;
    encodedTime.replace(" ", "%20");

    String url = String(serverURL) +
        "?nodeId=" + String(node) +
        "&nodeTemp=" + String(data.temperature, 2) +
        "&nodeHumidity=" + String(data.humidity, 2) +
        "&lightLevel=" + String(data.lightLevel) +
        "&timeReceived=" + encodedTime;

    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(10000);

    Serial.println();
    Serial.print("Transmitting: ");
    Serial.println(node);

    if (!http.begin(client, url)) {
        Serial.println("HTTP setup failed.");
        return false;
    }

    int httpCode = http.GET();

    Serial.print("HTTP code: ");
    Serial.println(httpCode);

    bool delivered = false;

    if (httpCode > 0) {
        String response = http.getString();
        response.trim();

        Serial.print("Server response: ");
        Serial.println(response);

        if (httpCode == 200 &&
            (response == "STORED" ||
             response == "DUPLICATE")) {
            delivered = true;
        }
    } else {
        Serial.print("HTTP error: ");
        Serial.println(http.errorToString(httpCode));
    }

    http.end();

    return delivered;
}

// --------------------------------------------------
// Capture new measurement
// --------------------------------------------------

void captureMeasurement(
    const char* node,
    Measurement &data
) {
    if (data.pending) {
        Serial.print(node);
        Serial.println(
            " already has a pending measurement."
        );
        return;
    }

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int lightLevel = analogRead(LIGHT_PIN);

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("DHT11 reading failed.");
        return;
    }

    String timestamp = getTimestamp();

    if (timestamp == "") {
        Serial.println("No valid timestamp.");
        return;
    }

    data.temperature = temperature;
    data.humidity = humidity;
    data.lightLevel = lightLevel;
    data.timestamp = timestamp;
    data.pending = true;

    Serial.println();
    Serial.print("New measurement: ");
    Serial.println(node);

    Serial.print("Temperature: ");
    Serial.println(data.temperature);

    Serial.print("Humidity: ");
    Serial.println(data.humidity);

    Serial.print("Light: ");
    Serial.println(data.lightLevel);

    Serial.print("Timestamp: ");
    Serial.println(data.timestamp);

    data.lastAttempt = millis();

    if (transmitMeasurement(node, data)) {
        data.pending = false;
        Serial.println("Measurement delivered!");
    } else {
        Serial.println(
            "Delivery failed. Retry scheduled."
        );
    }
}

// --------------------------------------------------
// Retry pending measurements
// --------------------------------------------------

void checkRetries() {
    unsigned long now = millis();

    if (node1.pending &&
        now - node1.lastAttempt >= retryInterval) {

        node1.lastAttempt = now;

        Serial.println("Retrying node_1...");

        if (transmitMeasurement("node_1", node1)) {
            node1.pending = false;
            Serial.println("node_1 delivered!");
        }
    }

    if (node2.pending &&
        now - node2.lastAttempt >= retryInterval) {

        node2.lastAttempt = now;

        Serial.println("Retrying node_2...");

        if (transmitMeasurement("node_2", node2)) {
            node2.pending = false;
            Serial.println("node_2 delivered!");
        }
    }
}

// --------------------------------------------------
// Debounced switch detection
// --------------------------------------------------

bool switchActivated(SwitchState &sw) {
    bool reading = digitalRead(sw.pin);

    if (reading != sw.lastReading) {
        sw.lastChange = millis();
    }

    if (millis() - sw.lastChange >= debounceDelay) {
        if (reading != sw.stableState) {
            sw.stableState = reading;

            if (sw.stableState == LOW) {
                sw.lastReading = reading;
                return true;
            }
        }
    }

    sw.lastReading = reading;
    return false;
}

// --------------------------------------------------
// Check switches
// --------------------------------------------------

void checkSwitches() {
    if (switchActivated(button)) {
        Serial.println("Pushbutton activated!");
        captureMeasurement("node_1", node1);
    }

    if (switchActivated(tilt)) {
        Serial.println("Tilt switch activated!");
        captureMeasurement("node_2", node2);
    }
}

// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("EE-470 ESP8266 IoT Project V2.2");

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(TILT_PIN, INPUT_PULLUP);

    button.stableState = digitalRead(BUTTON_PIN);
    button.lastReading = button.stableState;

    tilt.stableState = digitalRead(TILT_PIN);
    tilt.lastReading = tilt.stableState;

    dht.begin();

    connectWiFi();

    selectTimeZone();

    if (WiFi.status() == WL_CONNECTED) {
        retrieveTimeFromAPI();
        synchronizeTime();
    }
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------

void loop() {
    checkSwitches();
    checkRetries();

    delay(10);
}
