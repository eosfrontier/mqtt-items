from utime import ticks_ms, ticks_diff
import ubinascii, machine, re
import udp, mqtt

class msg:
    def __init__(self, config, Leds, Btns):
        self.leds = Leds
        self.btns = Btns
        self.name = config['name']
        self.buttonnames = [bytes(bn,'utf-8') for bn in config['buttons']]
        self.num = len(self.buttonnames)
        self.buttonstates = [False]*self.num
        self.loadavg = 0.0

        self.mapping = {bytes(k,'utf-8'):[bytes(x,'utf-8') for x in v] for (k,v) in config["mapping"].items()}

        self.battery = machine.ADC(0)
        self.laststatus = ticks_ms()
        self.wait = 0
        self.ack = None

        self.adc = machine.ADC(0)

        self.tstatus = b'%s/status' % config["name"]
        self.tack = b'%s/ack' % config["name"]
        self.tbtn = b'%s/button/%%s' % config["name"]
        self.tset = b'%s/set' % config["name"]
        #self.net = mqtt.mqtt(config, self.tset, self._receive)
        self.net = udp.udp(config, self.tset, self._receive)

    def check(self):
        self.net.check()
        if self.ack != None:
            self.net.publish(topic = self.tack, msg = self.ack)
            self.ack = None
        self._check_status()
        btns = self.btns.read()
        pushed = False
        for i in range(len(self.buttonnames)):
            if btns[i] != self.buttonstates[i]:
                if btns[i]:
                    pushed = True
        if pushed:
            if self._sendbtn(b'_'.join([self.buttonnames[i] for i in range(self.num) if btns[i]])):
                self.buttonstates = btns
        else:
            self.buttonstates = btns
                    
    def _sendbtn(self, bname):
        return self.net.publish(topic = (self.tbtn % bname), msg=b'1')

    def _check_status(self):
        now = ticks_ms()
        if ticks_diff(now, self.laststatus) > (10 * 1000):
            vlt = self.adc.read() * 0.0059
            self.net.publish(topic = self.tstatus, msg=b'{"battery":%1.2f,"connected":true,"load":%1.2f}' % (vlt, self.loadavg), retain=True)
            self.laststatus = now

    def _receive(self, topic, msg):
        if topic in self.mapping:
            tm = self.mapping[topic]
            topic = b'%s/%s' % (self.name, tm[0])
            msg = tm[1].replace(b'&',msg)
        if topic == self.tset:
            self.do_set(msg)

    def do_set(self, msg):
        if msg == b'green':
            self.leds.pulse_green()
        elif msg == b'flashred':
            self.leds.flash_red()
        elif len(msg) > 0:
            self.leds.set(msg.split(b','))
        else:
            self.leds.set([])
        self.ack = msg
