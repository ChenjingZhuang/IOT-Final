import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
from dotenv import load_dotenv
import os

load_dotenv()

AIO_USERNAME = os.getenv("AIO_USERNAME")
AIO_KEY = os.getenv("AIO_KEY")

BROKER = "io.adafruit.com"
PORT = 1883

TOPIC_TEMP = f"{AIO_USERNAME}/feeds/temperature"
TOPIC_HUM = f"{AIO_USERNAME}/feeds/humidity"

temps = []
hums = []
times = []
t = 0

plt.ion()
fig, ax = plt.subplots()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code", rc)
    client.subscribe([(TOPIC_TEMP, 0), (TOPIC_HUM, 0)])

def on_message(client, userdata, msg):
    global t

    value = float(msg.payload.decode())

    if msg.topic == TOPIC_TEMP:
        temps.append(value)

    elif msg.topic == TOPIC_HUM:
        hums.append(value)

    times.append(t)
    t += 1

    ax.clear()

    ax.plot(times[:len(temps)], temps, label="Temperature (°C)")
    ax.plot(times[:len(hums)], hums, label="Humidity (%)")

    ax.set_xlabel("Time")
    ax.set_ylabel("Value")
    ax.set_title("Live Sensor Data (MQTT)")
    ax.legend()
    ax.grid(True)

    plt.pause(0.1)

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
client.username_pw_set(AIO_USERNAME, AIO_KEY)

client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)
client.loop_forever()