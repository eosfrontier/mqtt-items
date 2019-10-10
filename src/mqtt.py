from utime import ticks_ms, ticks_diff
from umqtt.simple import MQTTClient
import ubinascii, machine, re

class MQTT:
    def __init__(self, config, pwd, Leds, Btns):
        self.leds = Leds
        self.btns = Btns
        self.name = config['name']
        self.buttonnames = config['buttons']
        self.buttonstates = [False]*len(self.buttonnames)
        self.state = self._connect

        self.battery = machine.ADC(0)
        self.laststatus = ticks_ms()
        self.wait = 0
        self.ack = None

        self.tstatus = b'%s/status' % config["name"]
        self.tset = b'%s/set' % config["name"]
        self.tack = b'%s/ack' % config["name"]
        self.tbtn = b'%s/button/%%s' % config["name"]
        self.client = MQTTClient(client_id = ubinascii.hexlify(machine.unique_id()),
                server = config["server"],
                user = pwd["user"],
                password = pwd["password"])
        self.client.set_callback(self._receive)
        self.client.set_last_will(topic=self.tstatus, msg=b'{"battery":0.0,"connected":false}', retain=True, qos=1)
        self.client.subscribe(self.tset)
    
    def _connect(self):
        try:
            self.client.connect()
            self.state = self._check
        except:
            self.wait = 20
            self.state = self._retry

    def _retry(self):
        if self.wait > 0:
            self.wait -= 1
        else:
            self.state = self._connect

    def _check(self):
        self.client.check_msg()
        if self.ack:
            self.client.publish(topic = self.tack, msg = self.ack, qos=1)
            self.ack = None
        self._check_status()
        btns = self.btns.read()
        pushed = False
        for i in range(len(self.buttonnames)):
            if btns[i] != self.buttonstates[i]:
                if btns[i]:
                    pushed = True
        if pushed:
            self._sendbtn('+'.join([self.buttonnames[b] for b in btns if b]))
            self.buttonstates = btns
                    
    def _sendbtn(self, topic):
        try:
            self.client.publish(topic = topic, msg=b'1', qos=1)
            return True
        except:
            self.state = _retry
            return False

    def _check_status(self):
        now = ticks_ms()
        if ticks_diff(now, self.laststatus) > (10 * 1000):
            vlt = adc.read() * 0.0059
            try:
                self.client.publish(topic = tstatus, msg=b'{"battery":%1.2f,"connected":true}' % vlt, retain=True, qos=1)
                self.laststatus = now
            except:
                self.state = _retry

    def _receive(self, topic, msg):
        if topic == self.tset:
            self._mqtt_set(msg)

    def _mqtt_set(self, msg):
        self.leds.set(msg)
        self.ack = msg
