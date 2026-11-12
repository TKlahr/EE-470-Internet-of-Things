#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DHT.h>
#include <time.h>
#include <math.h>

/*
 * EE-470 ESP8266 IoT Project
 * Reliable Sensor Transmission
 * Version 2.1
 *
 * Inputs:
 *   DHT11: D5 (GPIO14)
 *   Photoresistor: A0
 *   Pushbutton: D6 (GPIO12)
 *   Tilt Switch: D7 (GPIO13)
 *
 * Outputs:
 *   Sensor measurements to Hostinger MySQL
 *   Wi-Fi RSSI and transmission status to Serial Monitor
 *
 * Features:
 *   Switch debouncing
 *   Automatic retransmission every 30 seconds
 *   Original measurements retained during retries
 *   Duplicate response handling
 */

// --------------------------------------------------
// Wi-Fi and server configuration
// --------------------------------------------------

const char* wifiSSID = "raidernation75";
const char* wifiPassword = "raiderswin";

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

// Pacific Standard Time (UTC-8)
// Serial time zone selection will be added later.
const long utcOffsetSeconds = -8 * 3600;

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

// One pending measurement per node
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
// Connect to Wi-Fi
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
// Synchronize time using NTP
// --------------------------------------------------

void synchronizeTime() {
    configTime(
        utcOffsetSeconds,
        0,
        "pool.ntp.org"
    );

    Serial.print("Synchronizing time");

    unsigned long startTime = millis();

    while (time(nullptr) < 1000000000 &&
           millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (time(nullptr) >= 1000000000) {
        Serial.println("Time synchronized!");
    } else {
        Serial.println("Time synchronization failed.");
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

    struct tm* timeInfo = localtime(&now);

    char timestamp[20];

    strftime(
        timestamp,
        sizeof(timestamp),
        "%Y-%m-%d %H:%M:%S",
        timeInfo
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
// Capture new sensor measurement
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

    // Attempt immediate transmission
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
    Serial.println("EE-470 ESP8266 IoT Project V2.1");

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(TILT_PIN, INPUT_PULLUP);

    button.stableState = digitalRead(BUTTON_PIN);
    button.lastReading = button.stableState;

    tilt.stableState = digitalRead(TILT_PIN);
    tilt.lastReading = tilt.stableState;

    dht.begin();

    connectWiFi();

    if (WiFi.status() == WL_CONNECTED) {
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
