from pn532 import PN532
from machine import Pin
from neopixel import NeoPixel
from utime import sleep_ms,ticks_ms,ticks_add,ticks_diff,time
import ntptime
from umqtt.simple import MQTTClient
import ubinascii, machine
import urequests as requests
import wifiportal

np = NeoPixel(Pin(2),24)
np.write()

for r in range(8):
    np[r] = (20,0,0)
    np[r+8] = (0,20,0)
    np[r+16] = (0,0,20)
    np.write()
    sleep_ms(50)

wifiportal.captive_portal("RfidLock")

try:
    ntptime.settime()
except:
    pass

for r in range(8):
    np[r] = (0,0,0)
    np[r+8] = (0,0,0)
    np[r+16] = (0,0,0)
    np.write()
    sleep_ms(50)

pn = PN532()
cardid = None
colors = [(20,0,0),(15,15,0),(0,20,0),(0,15,15),(0,0,20),(15,0,15)]
statecols = [(0,0,0),(0,0,20),(0,20,0),(20,0,0)]
cardstate = 0

def mqtt_rec(topic,msg):
    global cardstate
    if topic == b'eos/rfid/color':
        if msg == b'blue':
            cardstate = 124
        elif msg == b'green':
            cardstate = 224
        elif msg == b'red':
            cardstate = 324
        else:
            cardstate = 23

tstatus = b'eos/rfid/status'
mqttid = ubinascii.hexlify(machine.unique_id())
mqtt = MQTTClient(client_id=mqttid, server='192.168.1.58', user='blinken', password='Chocola')
mqtt.set_callback(mqtt_rec)
mqtt.set_last_will(tstatus, b'{"battery":0.0,"connected":false}', retain=True, qos=1)
try:
    mqtt.connect()
    mqtt.subscribe(b'eos/rfid/color')
except:
    pass

adc = machine.ADC(0)
cnt = 1500
while True:
    ct = ticks_add(ticks_ms(),40)
    try:
        pn.mainloop()
    except:
        sleep_ms(100)
        continue
    if pn.cardid != cardid:
        cardid = pn.cardid
        if cardid:
            print("Card: %08x" % cardid)
            if cardid == 0x933b1a20:
                cardstate = 124
            elif cardid == 0xc07e60a8:
                cardstate = 224
            else:
                cardstate = 324
            try:
                res = requests.post("http://test.medicorum.space/roster/api/register_access.php", data=('{"cardid":"%08x","timestamp":%d}' % (cardid,time())), headers = {'Content-Type': 'application/json'})
                print("POST: %s" % (res.text.strip()))
            except:
                pass
            try:
                mqtt.publish(topic=b'eos/rfid/card', msg=(b'%08x' % cardid))
            except:
                pass

    if cardstate > 0 and cardstate < 400 and (cardstate % 100) <= 24:
        cardstate = cardstate-1
        if (cardstate % 100) == 99:
            cardstate = 23
        np[cardstate % 100] = statecols[int(cardstate/100)]
        np.write()
    try:
        mqtt.check_msg()
    except:
        pass
    cnt -= 1
    if cnt <= 0:
        cnt = 1500
        try:
            vlt = adc.read() * 0.0059
            mqtt.publish(topic = tstatus, msg = b'{"battery":%1.2f,"connected":true}' % vlt, retain=True, qos=1)
        except:
            pass

    et = ticks_diff(ct,ticks_ms())
    if et > 0:
        sleep_ms(et)
