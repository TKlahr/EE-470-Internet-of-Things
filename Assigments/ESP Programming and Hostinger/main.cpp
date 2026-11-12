#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DHT.h>
#include <time.h>

/*
 * EE-470 ESP8266 IoT Project
 * Sensor Monitoring and Database Transmission
 * Version 2.0
 *
 * Inputs:
 *   DHT11: D5
 *   Photoresistor: A0
 *   Pushbutton: D6
 *   Tilt Switch: D7
 *
 * Outputs:
 *   Sensor measurements to Hostinger MySQL database
 *   Wi-Fi RSSI and transmission status to Serial Monitor
 */

// Wi-Fi credentials
const char* wifiSSID = "YOUR_WIFI_NAME";
const char* wifiPassword = "YOUR_WIFI_PASSWORD";

// Database endpoint
const char* serverURL =
    "https://klahree.com/ESP8266_IoT/insert.php";

// Pin assignments
#define DHT_PIN D5
#define DHT_TYPE DHT11
#define LIGHT_PIN A0
#define BUTTON_PIN D6
#define TILT_PIN D7

DHT dht(DHT_PIN, DHT_TYPE);

// Switch states
bool previousButtonState = HIGH;
bool previousTiltState = HIGH;

// Debounce timing
const unsigned long debounceDelay = 50;
unsigned long lastButtonChange = 0;
unsigned long lastTiltChange = 0;

// Time zone: Pacific Standard Time (UTC-8)
// We will make this selectable through Serial later.
const long utcOffsetSeconds = -8 * 3600;

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
// Synchronize ESP8266 clock
// --------------------------------------------------
void synchronizeTime() {
    configTime(utcOffsetSeconds, 0, "pool.ntp.org");

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
// Generate measurement timestamp
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
// Read sensors and transmit measurement
// --------------------------------------------------
void sendSensorData(const char* node) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int lightLevel = analogRead(LIGHT_PIN);

    // Reject invalid DHT readings
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("ERROR: DHT11 reading failed.");
        return;
    }

    String timestamp = getTimestamp();

    if (timestamp == "") {
        Serial.println("ERROR: No valid timestamp.");
        return;
    }

    Serial.println();
    Serial.print("Node: ");
    Serial.println(node);

    Serial.print("Temperature: ");
    Serial.println(temperature);

    Serial.print("Humidity: ");
    Serial.println(humidity);

    Serial.print("Light Level: ");
    Serial.println(lightLevel);

    Serial.print("Timestamp: ");
    Serial.println(timestamp);

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ERROR: Wi-Fi disconnected.");
        return;
    }

    // Encode spaces in timestamp for URL
    timestamp.replace(" ", "%20");

    String url = String(serverURL) +
        "?nodeId=" + String(node) +
        "&nodeTemp=" + String(temperature, 2) +
        "&nodeHumidity=" + String(humidity, 2) +
        "&lightLevel=" + String(lightLevel) +
        "&timeReceived=" + timestamp;

    BearSSL::WiFiClientSecure client;

    // Temporary TLS configuration for testing
    client.setInsecure();

    HTTPClient http;

    Serial.println("Sending measurement...");

    if (!http.begin(client, url)) {
        Serial.println("ERROR: HTTP setup failed.");
        return;
    }

    int httpCode = http.GET();

    Serial.print("HTTP response code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
        Serial.print("Server response: ");
        Serial.println(http.getString());
    } else {
        Serial.print("HTTP error: ");
        Serial.println(http.errorToString(httpCode));
    }

    http.end();
}

// --------------------------------------------------
// Detect switch activations
// --------------------------------------------------
void checkSwitches() {
    bool buttonState = digitalRead(BUTTON_PIN);
    bool tiltState = digitalRead(TILT_PIN);

    unsigned long currentTime = millis();

    // Pushbutton: node_1
    if (buttonState != previousButtonState &&
        currentTime - lastButtonChange > debounceDelay) {

        lastButtonChange = currentTime;

        if (buttonState == LOW) {
            Serial.println("Pushbutton activated!");
            sendSensorData("node_1");
        }
    }

    // Tilt switch: node_2
    if (tiltState != previousTiltState &&
        currentTime - lastTiltChange > debounceDelay) {

        lastTiltChange = currentTime;

        if (tiltState == LOW) {
            Serial.println("Tilt switch activated!");
            sendSensorData("node_2");
        }
    }

    previousButtonState = buttonState;
    previousTiltState = tiltState;
}

// --------------------------------------------------
// Setup
// --------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("EE-470 ESP8266 IoT Project");

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(TILT_PIN, INPUT_PULLUP);

    previousButtonState = digitalRead(BUTTON_PIN);
    previousTiltState = digitalRead(TILT_PIN);

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
    delay(10);
}
