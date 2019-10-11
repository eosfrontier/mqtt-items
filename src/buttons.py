from machine import Pin

class Buttons:
    def __init__(self,config):
        self.pins = [Pin(pn,Pin.IN,Pin.PULL_UP) for pn in config['pins']]
    
    def read(self):
        return [p.value() == 0 for p in self.pins]
