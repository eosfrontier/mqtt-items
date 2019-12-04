from utime import ticks_ms, ticks_diff
import ubinascii, machine, re
import socket

class udp:
    def __init__(self, config, sub, receive):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._socket.setblocking(0)
        self._port = 1883
        self._socket.bind(('0.0.0.0', self._port))

        self._sub = sub
        self._receive = receive

    def check(self):
        try:
            data, addr = self._socket.recvfrom(1024)
            topic, msg = data.split(b'\n', 1)
            if topic == self._sub:
                self._receive(topic, msg)
        except OSError as e:
            if e.args[0] != 110:
                print('check', e)
        except Exception as e:
            print('check', e)

    def publish(self, topic, msg, retain=False):
        try:
            self._socket.sendto(b'%s\n%s' % (topic, msg), ('192.168.1.255', self._port))
            return True
        except Exception as e:
            print('publish', e)
            return False
                    
    def _callback(self, topic, msg):
        print('%s -> %s' % (topic, msg))
        if topic == self.tset:
            self._receive(msg)
