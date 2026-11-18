# EE470 - MQTT Python Relay
# Receives MQTT potentiometer readings
# and sends them to the Hostinger database

import paho.mqtt.client as mqtt
import requests

# MQTT settings
BROKER_URL = "broker.hivemq.com"
BROKER_PORT = 1883
TOPIC = "ee470/klahr470/esp1/pot"

# Hostinger PHP endpoint
PHP_URL = "https://klahree.com/MQTT_Assignment/insert_mqtt.php"


def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Connected to HiveMQ broker!")
        client.subscribe(TOPIC)
        print(f"Subscribed to topic: {TOPIC}")
    else:
        print(f"Connection failed: {reason_code}")


def on_message(client, userdata, msg):
    try:
        value = int(msg.payload.decode().strip())

        if not 0 <= value <= 1023:
            print("Invalid potentiometer value")
            return

        print(f"Received potentiometer value: {value}")

        data = {
            "node_name": "esp1",
            "pot_value": value
        }

        response = requests.post(
            PHP_URL,
            data=data,
            timeout=10
        )

        response.raise_for_status()

        print("Database response:", response.text)
        print("-----------------------------")

    except ValueError:
        print("Received invalid MQTT data")

    except requests.RequestException as error:
        print("Database upload failed:", error)


client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    transport="tcp"
)

client.on_connect = on_connect
client.on_message = on_message

print("Connecting to broker...")
client.connect(BROKER_URL, BROKER_PORT, 60)

client.loop_forever()
