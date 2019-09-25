from machine import Pin
from neopixel import NeoPixel
from utime import sleep_ms

def clear():
    np = NeoPixel(Pin(2),4)
    np.write()

def test():
    for k in range(10):
        for j in range(4):
            for i in range(18):
                np[j] = (15*i,0,0)
                np.write()
                sleep_ms(50)
            for i in range(18):
                np[j] = (255-15*i,15*i,0)
                np.write()
                sleep_ms(50)
            for i in range(18):
                np[j] = (0,255-15*i,15*i)
                np.write()
                sleep_ms(50)
            for i in range(18):
                np[j] = (0,0,255-15*i)
                np.write()
                sleep_ms(50)

clear()
