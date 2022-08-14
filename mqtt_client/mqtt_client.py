import random
import paho.mqtt.client as mqtt_client
from configparser import ConfigParser

# Read broker and client_id from aws_config.pem
# Pem files have been set in the git ignore
config_object = ConfigParser()
config_object.read("./certificates/aws_config.pem")
broker = config_object["config"]["broker"]
client_id = config_object["config"]["client_id"]

port = 8883
topic = "KA/LAB/DO/TEMP"
caPath = "./certificates/AmazonRootCA1.pem"
certPath = "./certificates/cert.pem"
keyPath = "./certificates/private.pem.key"


def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.tls_set(ca_certs=caPath, certfile=certPath, keyfile=keyPath)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received '{msg.payload.decode()}' from '{msg.topic}' topic.")

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()


if __name__ == "__main__":
    run()
