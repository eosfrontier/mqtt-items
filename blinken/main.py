from machine import Pin
from neopixel import NeoPixel
from utime import sleep_ms
import wifiportal
import mqtt

np = NeoPixel(Pin(0),4)
np.write()
sleep_ms(100)
np[0] = (20,0,0)
np.write()
sleep_ms(100)
np[1] = (10,10,0)
np.write()
sleep_ms(100)
np[2] = (0,20,0)
np.write()
sleep_ms(100)
np[3] = (0,0,20)
np.write()

wifiportal.captive_portal("Blinken", "blinken_portal")

for i in range(4):
    sleep_ms(500)
    np[0]=(0,0,255)
    np[1]=(0,255,0)
    np[2]=(255,255,0)
    np[3]=(255,0,0)
    np.write()
    sleep_ms(500)
    np[0]=(0,0,0)
    np[1]=(0,0,0)
    np[2]=(0,0,0)
    np[3]=(0,0,0)
    np.write()

mqtt.mainloop(np)
