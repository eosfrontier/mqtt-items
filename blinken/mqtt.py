from umqtt.simple import MQTTClient
import ubinascii, machine

client = None
leds = None

def mqtt_rec(topic,msg):
    global leds
    if topic == b'eos/portal/status':
        if msg == b'good':
            leds[0] = (0,255,0)
            leds[1] = (0,255,0)
            leds[2] = (0,255,0)
            leds[3] = (0,255,0)
            leds.write()
        elif msg == b'bad':
            leds[0] = (255,0,0)
            leds[1] = (255,0,0)
            leds[2] = (255,0,0)
            leds[3] = (255,0,0)
            leds.write()
        else:
            leds[0] = (0,0,0)
            leds[1] = (0,0,0)
            leds[2] = (0,0,0)
            leds[3] = (0,0,0)
            leds.write()

def mainloop(np, host='192.168.1.58'):
    global client,leds
    leds = np
    cid = ubinascii.hexlify(machine.unique_id())
    client = MQTTClient(client_id=cid, server=host)
    client.set_callback(mqtt_rec)
    client.connect()
    client.subscribe('eos/portal/status')
    while True:
        client.wait_msg()
