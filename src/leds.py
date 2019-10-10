from utime import ticks_ms, ticks_diff
from machine import Pin
from neopixel import NeoPixel

class Leds:
    def __init__(self, config):
        self.num = config['num']
        self.order = [x for x in ['RGBW'.find(x) for x in config['order']] if x >= 0]
        self.np = NeoPixel(Pin(config['pin']),num,len(self.order))
        self.defcols = [_parsecol(x) for x in config['colors']]
        self.anim = None
        self.tick = 0
        self.curcol = [(0,0,0)]
        self.clear()

    def _parsecol(colors):
        vals = [int(colors[i:i+2],16) for i in range(0,len(colors),2)]
        return tuple([vals[i] for i in self.order])

    def animate():
        if self.anim:
            self.anim()

    def a_set():
        elaps = ticks_diff(ticks_ms() - self.tick)
        for i in range(len(atim)):
            if elaps < self.atim[i+1]:
                _setinter(self.acol[i-1],self.acol[i],
                    (elaps-self.atim[i])/(self.atim[i+1]-self.atim[i]))
                return
        _setcols(self.acol[-1])
        self.anim = None

    def _setcols(cols):
        nc = len(cols)
        for n in range(self.num):
            np[n] = cols[n%nc]
        np.write()    

    def _interp(a, b, frac):
        return (
            a[0] * frac + b[0] * (1-frac),
            a[1] * frac + b[1] * (1-frac),
            a[2] * frac + b[2] * (1-frac))

    def _setinter(colsa, colsb, frac):
        na = len(colsa)
        nb = len(colsb)
        for n in range(self.num):
            self.np[n] = _interp(colsa[n%na],colsb[n%nb]
        np.write()

    def set(colors):
        self.atim = [0,1000]
        self.acol = [self.curcol,[_parsecol(c) for c in colors]]
        self.anim = a_set
        self.tick = ticks_ms()

    def clear():
        self.atim = [0,5000]
        self.acol = self.defcols
        self.anim = a_set
        self.tick = ticks_ms()
