import paho.mqtt.client as mqtt


# settings
broker = "151.242.7.203"
pub_topic = "test/light"


#app
client = mqtt.Client()
client.connect(broker,1883,60)

def publish(msg):
    client.publish(pub_topic,msg)

print("ready to pub(light):")

while True:
    msg = input()
    if msg == "~":
        break
    publish(msg)


print("--- publishing Ended!")
client.disconnect()