import paho.mqtt.client as mqtt


# settings
broker = "broker.hivemq.com"
sub_topic = [("test/light",0),("test/door",1)]


#app
def disconnect():
    client.disconnect()
    print("--receiving enterrupted!")
    exit()

def on_message(client,userdata,msg):
    msg = msg.payload.decode()
    if msg == "!":
        disconnect()
        return
    print(msg)


client = mqtt.Client()
client.connect(broker,1883,60)
client.on_message = on_message

client.subscribe(sub_topic)

print("ready to receive from light:")
client.loop_forever()