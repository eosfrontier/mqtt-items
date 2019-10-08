from umqtt.simple import MQTTClient
import ubinascii, machine, re
from utime import sleep_ms

client = None
leds = None
tset = None
tport = b'eos/portal/incoming'

def mqtt_rec(topic,msg):
    global leds,tset,tport
    if topic == tport:
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
    elif topic == tset:
        try:
            rgb = int(msg, 16)
            col = ((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255)
            leds[0] = col
            leds[1] = col
            leds[2] = col
            leds[3] = col
            leds.write()
        except:
            pass

def mainloop(np, host='192.168.1.58', myname='light01'):
    global client,leds,tset
    tset = b'eos/portal/%s/set' % myname
    tstatus = b'eos/portal/%s/status' % myname
    leds = np
    cid = ubinascii.hexlify(machine.unique_id())
    client = MQTTClient(client_id=cid, server=host, user='blinken', password='Chocola')
    client.set_callback(mqtt_rec)
    client.set_last_will(topic=tstatus, msg=b'{"battery":0.0,"connected":false}', retain=True, qos=1)
    client.connect()
    client.subscribe(tport)
    client.subscribe(tset)
    adc = machine.ADC(0)
    while True:
        ctr = 600 # Every minute
        while ctr > 0:
            ctr -= 1
            client.check_msg()
            sleep_ms(100)
        vlt = adc.read() * 0.0059
        client.publish(topic = tstatus, msg=b'{"battery":%1.2f,"connected":true}' % vlt, retain=True, qos=1)
