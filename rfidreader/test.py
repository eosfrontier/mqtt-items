from pn532 import PN532
from machine import Pin
from neopixel import NeoPixel
from utime import sleep_ms,ticks_ms,ticks_add,ticks_diff

np = NeoPixel(Pin(2),24)
np.write()
pn = PN532()
cardid = None
colors = [(20,0,0),(15,15,0),(0,20,0),(0,15,15),(0,0,20),(15,0,15)]
for r in range(6000):
    ct = ticks_add(ticks_ms(),10)
    try:
        pn.mainloop()
    except:
        sleep_ms(100)
        continue
    if pn.cardid != cardid:
        cardid = pn.cardid
        if cardid:
            print("Card: %08x" % cardid)
            for i in range(24):
                np[i] = colors[i%6]
    if (r%2)==0:
        for i in range(24):
            np[i] = (max(0,np[i][0]-1),max(0,np[i][1]-1),max(0,np[i][2]-1))
        np.write()
    et = ticks_diff(ct,ticks_ms())
    if et > 0:
        sleep_ms(et)
