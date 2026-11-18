# EE470 - MQTT Python Relay
# Based on the MQTT subscriber example provided by Farid Farahmand

import paho.mqtt.client as mqtt

# MQTT broker settings
BROKER_URL = "broker.hivemq.com"
BROKER_PORT = 1883

# ESP8266 potentiometer topic
TOPIC = "ee470/klahr470/esp1/pot"


# Called when connected to the broker
def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Connected to HiveMQ broker!")

        client.subscribe(TOPIC)
        print(f"Subscribed to topic: {TOPIC}")

    else:
        print(f"Connection failed: {reason_code}")


# Called when a message is received
def on_message(client, userdata, msg):
    value = msg.payload.decode()

    print(f"Received potentiometer value: {value}")
    print(f"Topic: {msg.topic}")
    print("-----------------------------")


# Create MQTT client
client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    transport="tcp"
)

# Assign callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connect to broker
print("Connecting to broker...")
client.connect(BROKER_URL, BROKER_PORT, 60)

# Keep listening for messages
client.loop_forever()
