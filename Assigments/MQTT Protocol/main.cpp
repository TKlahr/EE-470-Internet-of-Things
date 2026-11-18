//-----------------------------
// Title: MQTT - EE470 Assignment
//-----------------------------
// Based on the ESP8266 MQTT example provided by Farid Farahmand
//
// Purpose: Connect to HiveMQ, publish potentiometer values,
//          and prepare for MQTT subscriptions.
//
// Part A: Potentiometer, pushbutton, and LED
// Part B: MQTT publishing
//
// Dependencies: PubSubClient
//-----------------------------

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

// WiFi variables
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT variables
const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* publishTopic = "testtopic/temp/outTopic/klahr470";
const char* subscribeTopic = "testtopic/temp/inTopic/klahr470";

#define publishTimeInterval 15000 // 15 seconds (milliseconds)

// Pin definitions
#define POT_PIN A0
#define BUTTON_PIN D7
#define LED_PIN D2

// Definitions
unsigned long lastMsg = 0;
unsigned long lastRead = 0;

#define MSG_BUFFER_SIZE (50)

char msg[MSG_BUFFER_SIZE];

int potValue = 0;
int buttonState = HIGH;
int lastButtonState = HIGH;
int ledStatus = 0;

WiFiClient espClient;
PubSubClient client(espClient); // Define MQTT client


//------------------------------------------
// Connect to WiFi
//------------------------------------------

void setup_wifi() {
    delay(10);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


//------------------------------------------
// MQTT callback - received messages
//------------------------------------------

void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }

    Serial.println();

    // LED control will be implemented in Part C
}


//------------------------------------------
// Reconnect to MQTT broker
//------------------------------------------

void reconnect() {

    while (!client.connected()) {

        Serial.print("Attempting MQTT connection...");

        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);

        // Attempt to connect
        if (client.connect(clientId.c_str())) {

            Serial.println("connected");

            // Resubscribe to topic
            client.subscribe(subscribeTopic);

        } else {

            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");

            delay(5000);
        }
    }
}


//------------------------------------------
// Setup
//------------------------------------------

void setup() {

    // Initialize hardware
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);

    Serial.println();
    Serial.println("EE470 MQTT Assignment - Part B");

    setup_wifi();

    // MQTT broker configuration
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}


//------------------------------------------
// Main loop
//------------------------------------------

void loop() {

    // Maintain MQTT connection
    if (!client.connected()) {
        reconnect();
    }

    client.loop();

    unsigned long now = millis();

    // Read potentiometer every second
    if (now - lastRead >= 1000) {

        lastRead = now;

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

    // LED control using Serial Monitor
    if (Serial.available() > 0) {

        char command = Serial.read();

        if (command == '1') {

            digitalWrite(LED_PIN, HIGH);
            ledStatus = 1;

            Serial.println("LED ON");
        }
        else if (command == '0') {

            digitalWrite(LED_PIN, LOW);
            ledStatus = 0;

            Serial.println("LED OFF");
        }
    }

    // Publish potentiometer value every 15 seconds
    if (now - lastMsg >= publishTimeInterval) {

        lastMsg = now;

        potValue = analogRead(POT_PIN);

        snprintf(msg, MSG_BUFFER_SIZE, "%d", potValue);

        Serial.print("Publish message: ");
        Serial.println(msg);

        if (client.publish(publishTopic, msg)) {
            Serial.println("MQTT publish successful");
        } else {
            Serial.println("MQTT publish failed");
        }
    }
}
