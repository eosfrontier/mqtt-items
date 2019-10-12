from utime import ticks_ms, ticks_diff
from machine import Pin
from neopixel import NeoPixel

class Leds:
    def __init__(self, config):
        self.num = config['num']
        self.order = [x for x in ['RGBW'.find(x) for x in config['order']] if x >= 0]
        self.np = NeoPixel(Pin(config['pin']),self.num,len(self.order))
        self.defcols = [self._parsecol(x) for x in config['colors']]
        self.anim = None
        self.tick = 0
        self.curcol = [(0,0,0)]
        self.clear()

    def _parsecol(self,colors):
        vals = [int(colors[i:i+2],16) for i in range(0,len(colors),2)]
        return tuple([vals[i] for i in self.order])

    def animate(self):
        if self.anim:
            self.anim()

    def a_set(self):
        elaps = ticks_diff(ticks_ms(), self.tick)
        for i in range(len(self.atim)-1):
            if elaps < self.atim[i+1]:
                self._setinter(self.acol[i-1],self.acol[i],
                    (elaps-self.atim[i])/(self.atim[i+1]-self.atim[i]))
                return
        self._setcols(self.acol[-1])
        self.anim = None

    def _setcols(self,cols):
        nc = len(cols)
        for n in range(self.num):
            self.np[n] = cols[n%nc]
        self.np.write()    

    def _interp(self, a, b, frac):
        return (
            int(a[0] * frac + b[0] * (1-frac)),
            int(a[1] * frac + b[1] * (1-frac)),
            int(a[2] * frac + b[2] * (1-frac)))

    def _setinter(self, colsa, colsb, frac):
        na = len(colsa)
        nb = len(colsb)
        self.curcol = [self._interp(colsa[n%na],colsb[n%nb],frac) for n in range(self.num)]
        for n in range(self.num):
            self.np[n] = self.curcol[n]
        self.np.write()

    def set(self, colors):
        if len(colors) > 0:
            self.atim = [0,2000]
            self.acol = [self.curcol,[self._parsecol(c) for c in colors]]
            self.anim = self.a_set
            self.tick = ticks_ms()
        else:
            self.clear()

    def clear(self):
        self.atim = [0,2000]
        self.acol = [self.curcol,self.defcols]
        self.anim = self.a_set
        self.tick = ticks_ms()
