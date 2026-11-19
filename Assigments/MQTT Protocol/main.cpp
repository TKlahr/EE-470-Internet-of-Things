//-----------------------------
// Title: MQTT - EE470 Assignment
//-----------------------------
// Based on the ESP8266 MQTT example provided by Farid Farahmand
//
// Part A: Potentiometer, pushbutton, LED, Serial Monitor
// Part B: MQTT potentiometer publishing
// Part C: MQTT LED control and button publishing
// Part D: Smartphone MQTT communication
// Part E: ESP8266-to-ESP8266 communication
// Part F: HiveMQ and Python database relay
// Part G: Smartphone dashboard and LED status
//
// Dependencies: PubSubClient
//-----------------------------

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//------------------------------------------
// WiFi settings
//------------------------------------------

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

//------------------------------------------
// MQTT settings
//------------------------------------------

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Change to 2 when uploading to the second ESP8266
#define DEVICE_ID 1

#if DEVICE_ID == 1

const char* publishTopic = "ee470/klahr470/esp1/pot";
const char* buttonTopic = "ee470/klahr470/esp1/button";
const char* peerButtonTopic = "ee470/klahr470/esp2/button";
const char* subscribeTopic = "ee470/klahr470/esp1/led/set";
const char* ledStatusTopic = "ee470/klahr470/esp1/led/status";

#elif DEVICE_ID == 2

const char* publishTopic = "ee470/klahr470/esp2/pot";
const char* buttonTopic = "ee470/klahr470/esp2/button";
const char* peerButtonTopic = "ee470/klahr470/esp1/button";
const char* subscribeTopic = "ee470/klahr470/esp2/led/set";
const char* ledStatusTopic = "ee470/klahr470/esp2/led/status";

#else
#error "DEVICE_ID must be 1 or 2"
#endif

//------------------------------------------
// Pin definitions
//------------------------------------------

#define POT_PIN A0
#define BUTTON_PIN D7
#define LED_PIN D2

//------------------------------------------
// Timing
//------------------------------------------

#define publishTimeInterval 15000

const unsigned long readInterval = 1000;
const unsigned long debounceInterval = 50;
const unsigned long buttonResetInterval = 5000;

//------------------------------------------
// Variables
//------------------------------------------

unsigned long lastMsg = 0;
unsigned long lastRead = 0;
unsigned long lastButtonChange = 0;
unsigned long buttonPublishTime = 0;

#define MSG_BUFFER_SIZE 50
char msg[MSG_BUFFER_SIZE];

int potValue = 0;
int ledStatus = 0;

int buttonState = HIGH;
int lastButtonState = HIGH;

bool buttonWasPressed = false;
bool buttonResetPending = false;

// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

//------------------------------------------
// Publish LED status
//------------------------------------------

void publishLEDStatus() {

    if (client.connected()) {

        const char* status = ledStatus ? "1" : "0";

        // Retain latest LED status for new subscribers
        if (client.publish(ledStatusTopic, status, true)) {

            Serial.print("LED status published: ");
            Serial.println(status);
        }
    }
}

//------------------------------------------
// Set LED state
//------------------------------------------

void setLED(int state) {

    if (state != 0 && state != 1) {
        return;
    }

    if (ledStatus != state) {

        ledStatus = state;

        digitalWrite(LED_PIN, ledStatus ? HIGH : LOW);

        Serial.print("LED changed to: ");
        Serial.println(ledStatus ? "ON" : "OFF");

        publishLEDStatus();
    }
}

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

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

//------------------------------------------
// MQTT callback
//------------------------------------------

void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }

    Serial.println();

    // Accept LED commands from MyMQTT or the other ESP8266
    if ((strcmp(topic, subscribeTopic) == 0 ||
         strcmp(topic, peerButtonTopic) == 0) &&
         length == 1) {

        if (payload[0] == '1') {

            Serial.println("MQTT: LED ON command");
            setLED(1);
        }
        else if (payload[0] == '0') {

            Serial.println("MQTT: LED OFF command");
            setLED(0);
        }
    }
}

//------------------------------------------
// MQTT reconnect
//------------------------------------------

void reconnect() {

    while (!client.connected()) {

        Serial.print("Attempting MQTT connection...");

        String clientId = "ESP8266Client-";
        clientId += String(ESP.getChipId(), HEX);
        clientId += "-";
        clientId += String(random(0xffff), HEX);

        if (client.connect(clientId.c_str())) {

            Serial.println("connected");

            // Subscribe to smartphone LED commands
            client.subscribe(subscribeTopic);

            // Subscribe to the other ESP8266
            client.subscribe(peerButtonTopic);

            // Publish current LED status
            publishLEDStatus();

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

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    digitalWrite(LED_PIN, LOW);

    // Read initial button state
    buttonState = digitalRead(BUTTON_PIN);
    lastButtonState = buttonState;

    Serial.begin(115200);

    Serial.println();
    Serial.print("EE470 MQTT Assignment - ESP");
    Serial.println(DEVICE_ID);

    setup_wifi();

    client.setServer(mqtt_server, mqtt_port);
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

    //--------------------------------------
    // Part A: Read potentiometer
    //--------------------------------------

    if (now - lastRead >= readInterval) {

        lastRead = now;

        potValue = analogRead(POT_PIN);

        Serial.print("Potentiometer: ");
        Serial.println(potValue);
    }

    //--------------------------------------
    // Parts C/E: Button detection
    //--------------------------------------

    int reading = digitalRead(BUTTON_PIN);

    if (reading != buttonState) {

        lastButtonChange = now;
        buttonState = reading;
    }

    if (now - lastButtonChange >= debounceInterval &&
        buttonState != lastButtonState) {

        lastButtonState = buttonState;

        if (buttonState == LOW) {

            // Button pressed
            buttonWasPressed = true;

            Serial.println("Button pressed: 1");

        } else if (buttonWasPressed) {

            // Button released
            buttonWasPressed = false;

            Serial.println("Button released");

            // Publish 1 after press and release
            if (client.connected()) {

                if (client.publish(buttonTopic, "1")) {

                    Serial.println("MQTT: Published button 1");

                    buttonPublishTime = now;
                    buttonResetPending = true;
                }
            }
        }
    }

    //--------------------------------------
    // Publish 0 five seconds after 1
    //--------------------------------------

    if (buttonResetPending &&
        now - buttonPublishTime >= buttonResetInterval) {

        if (client.connected()) {

            if (client.publish(buttonTopic, "0")) {

                Serial.println("MQTT: Published button 0");

                buttonResetPending = false;
            }
        }
    }

    //--------------------------------------
    // Part A: Serial Monitor LED control
    //--------------------------------------

    if (Serial.available() > 0) {

        char command = Serial.read();

        if (command == '1') {

            Serial.println("Serial: LED ON");
            setLED(1);

        } else if (command == '0') {

            Serial.println("Serial: LED OFF");
            setLED(0);
        }
    }

    //--------------------------------------
    // Parts B/F: Publish potentiometer
    //--------------------------------------

    if (now - lastMsg >= publishTimeInterval) {

        lastMsg = now;

        potValue = analogRead(POT_PIN);

        snprintf(msg, MSG_BUFFER_SIZE, "%d", potValue);

        Serial.print("Publish potentiometer: ");
        Serial.println(msg);

        if (client.publish(publishTopic, msg)) {

            Serial.println("MQTT publish successful");

        } else {

            Serial.println("MQTT publish failed");
        }
    }
}
