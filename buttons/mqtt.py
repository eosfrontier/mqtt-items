from umqtt.simple import MQTTClient
import ubinascii, machine, re
from utime import sleep_ms

client = None
leds = None
tset = None
tport = b'eos/portal/incoming'

buttonpins = [machine.Pin(pn,machine.Pin.IN,machine.Pin.PULL_UP) for pn in [12,13,15]]
buttonlabels = [b'1',b'2',b'3']
buttonstates = [1,1,1]

def mqtt_rec(topic,msg):
    global leds,tset,tport
    if topic == tport:
        if msg == b'good':
            leds[0] = (0,255,0)
            leds[1] = (0,255,0)
            leds[2] = (0,255,0)
            leds.write()
        elif msg == b'bad':
            leds[0] = (255,0,0)
            leds[1] = (255,0,0)
            leds[2] = (255,0,0)
            leds.write()
        else:
            leds[0] = (50,0,0)
            leds[1] = (0,50,0)
            leds[2] = (0,0,50)
            leds.write()
    elif topic == tset:
        try:
            rgb = int(msg, 16)
            col = ((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255)
            leds[0] = col
            leds[1] = col
            leds[2] = col
            leds.write()
        except:
            pass

def read_pins():
    global buttonpins, buttonlabels, buttonstates, leds
    for i in range(len(buttonpins)):
        val = buttonpins[i].value()
        if val != buttonstates[i]:
            buttonstates[i] = val
            msg = [b'up',b'down'][val]
            client.publish(topic = b'eos/portal/buttons01/b%s' % i, msg=msg, qos=1)

def mainloop(np, host='192.168.1.58', myname='buttons01'):
    global client,leds,tset
    tset = b'eos/portal/%s/set' % myname
    tstatus = b'eos/portal/%s/status' % myname
    leds = np
    cid = ubinascii.hexlify(machine.unique_id())
    client = MQTTClient(client_id=cid, server=host)
    client.set_callback(mqtt_rec)
    client.set_last_will(topic=tstatus, msg=b'{"battery":0.0,"connected":false}', retain=True, qos=1)
    client.connect()
    client.subscribe(tport)
    client.subscribe(tset)
    adc = machine.ADC(0)
    while True:
        ctr = 1200 # Every minute
        while ctr > 0:
            ctr -= 1
            client.check_msg()
            read_pins()
            sleep_ms(50)
        vlt = adc.read() * 0.0059
        client.publish(topic = tstatus, msg=b'{"battery":%1.2f,"connected":true}' % vlt, retain=True, qos=1)
