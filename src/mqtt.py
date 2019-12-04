from utime import ticks_ms, ticks_diff
from umqtt.simple import MQTTClient
import ubinascii, machine, re

class mqtt:
    def __init__(self, config, sub, receive):
        f = open('password.json', 'r')
        pwd = ujson.loads(f.read())
        f.close()

        self._client = MQTTClient(client_id = ubinascii.hexlify(machine.unique_id()),
                server = config["server"],
                user = pwd["user"],
                password = pwd["password"],
                keepalive = 10
                )
        self._client.set_callback(self._callback)
        self._client.set_last_will(topic=self.tstatus, msg=b'{"battery":0.0,"connected":false,"load":0.0}', retain=True, qos=1)
        self._sub = sub
        self._receive = receive
        self.wait = 1

    def check(self):
        if self.wait == 1:
            try:
                self._client.connect()
                self._client.subscribe(self._sub)
                self.state = self._check
                self.wait = 0
            except Exception as e:
                print('connect', e)
                self.wait = 20
        elif self.wait > 0:
            self.wait -= 1
        else:
            try:
                self._client.check_msg()
            except Exception as e:
                print('check', e)
                self.wait = 4

    def publish(self, topic, msg, retain=False):
        try:
            self._client.publish(topic = topic, msg = msg, retain = retain)
            return True
        except Exception as e:
            print('publish', e)
            self.wait = 4
            return False
                    
    def _callback(self, topic, msg):
        print('%s -> %s' % (topic, msg))
        if topic == self.tset:
            self._receive(msg)
