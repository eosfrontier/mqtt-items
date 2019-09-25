from pn532 import PN532
from machine import Pin
from neopixel import NeoPixel
from utime import sleep_ms,ticks_ms,ticks_add,ticks_diff,time
import ntptime
import wifiportal
import urequests as requests

np = NeoPixel(Pin(2),24)
np.write()

for r in range(8):
    np[r] = (20,0,0)
    np[r+8] = (0,20,0)
    np[r+16] = (0,0,20)
    np.write()
    sleep_ms(50)

wifiportal.captive_portal("RfidLock")

ntptime.settime()

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
            res = requests.post("http://test.medicorum.space/roster/api/register_access.php", data=('{"cardid":"%08x","timestamp":%d}' % (cardid,time())), headers = {'Content-Type': 'application/json'})
            print("POST: %s" % (res.text.strip()))

    if cardstate > 0 and cardstate < 400 and (cardstate % 100) <= 24:
        cardstate = cardstate-1
        if (cardstate % 100) == 99:
            cardstate = 23
        np[cardstate % 100] = statecols[int(cardstate/100)]
        np.write()
    et = ticks_diff(ct,ticks_ms())
    if et > 0:
        sleep_ms(et)
