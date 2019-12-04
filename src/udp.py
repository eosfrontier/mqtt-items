from utime import ticks_ms, ticks_diff
import ubinascii, machine, re
import socket, network

def _broadcastaddr():
    ipc = network.WLAN(network.STA_IF).ifconfig()
    ipt = [int(i) for i in ipc[0].split('.')]
    nmt = [int(i) for i in ipc[1].split('.')]
    return '.'.join([str(ipt[i] | ~nmt[i] & 255) for i in range(len(ipt))])

class udp:
    def __init__(self, config, _, receive):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._socket.setblocking(0)
        self._port = 1883
        self._socket.bind(('0.0.0.0', self._port))

        self._receive = receive

    def check(self):
        try:
            data, addr = self._socket.recvfrom(1024)
            topic, msg = data.split(b'\n', 1)
            self._receive(topic, msg)
        except OSError as e:
            if e.args[0] != 110:
                print('check', e)
        except Exception as e:
            print('check', e)

    def publish(self, topic, msg, retain=False):
        try:
            self._socket.sendto(b'%s\n%s' % (topic, msg), (_broadcastaddr(), self._port))
            return True
        except Exception as e:
            print('publish', e)
            return False
