from time import sleep
import socket

"""
ns = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ns.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
ns.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
ns.setblocking(0)
ns.bind(('0.0.0.0', 2390))

def hex_dump(my_string):
    return " ".join(map("{0:0>2X}".format,map(ord,my_string)))

ntppack = b'\xE3\x00\x06\xEC\0\0\0\0\0\0\0\0\x31\x4E\x31\x34\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0'

while True:
    ns.sendto(ntppack, ('194.192.112.20', 123))
    print('Sent packet', hex_dump(ntppack))
    for r in range(100):
        try:
            data,addr = ns.recvfrom(1024)
            print('nsdata',hex_dump(data),'addr',addr)
        except:
            sleep(0.1)
"""

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
s.setblocking(0)
s.bind(('0.0.0.0', 1883))
msgs = [
    #b'eos/portal/buttons_in/set\n%s' % b'flashred',
    #b'eos/portal/light/set\n%s' % b'red',
    #b'eos/portal/buttons_in/set\n%s' % b'green',
    #b'eos/portal/light/set\n%s' % b'green',
    #b'eos/portal/buttons_in/set\n%s' % b'',
    #b'eos/portal/light/set\n%s' % b''
    #b'eos/portal/light/set\n%s' % b'r 1000:00ff00,00aa00,005500 2000:00aa00,005500,00ff00 3000:005500,00ff00,00aa00',
    #b'eos/portal/light/set\n%s' % b'r 1000:ff0000 2000:ff0000 3000:000000',
    #b'eos/portal/light/set\n%s' % b'3000:000000'
    #b'eos/portal/light/set\nr 500:000000 1000:224488,000000,000000,000000 1500:44cccc,224488,000000,000000 2000:6622ff,44cccc,224488,000000 2500:6622ff,6622ff,44cccc,224488 3000:cc44cc,6622ff,6622ff,44cccc 3500:442288,cc44cc,6622ff,6622ff 4000:000000,442288,cc44cc,6622ff 4500:000000,000000,442288,cc44cc 5000:000000,000000,000000,442288 5500:000000',
    #b'eos/portal/light/ack\nred',
    #b'eos/portal/light/ack\n',
    #b'eos/portal/buttons_in/button/b1\n1',
    #b'eos/portal/buttons_in/button/b2\n1',
    #b'eos/portal/buttons_in/button/b3\n1',
    #b'eos/portal/monitor/status\n000',
    #b'eos/portal/light/set\nidle',
    #b'eos/portal/light/set\nred',
    #b'eos/portal/buttons_in/set\n1000:ff0000,00ff00,0000ff,ffffff',
    b'SUB\neos/*/*/*',
    b'SUB\neos/*/*/*/*',
    #b'eos/rfid/armory/set\nr 200:ff0000,00ff00 400:00ff00,ff0000',
    #b'eos/rfid/armory/set\nred',
    #b'eos/rfid/armory/set\nidle',
    #b'eos/rfid/armory/set\n1000:000000,000008',
    #b'eos/portal/sonoff_A/beacon\nred',
    #b'eos/portal/sonoff_A/gpio/led\nH',
    #b'eos/portal/sonoff_A/gpio/led\nL',
    #b'eos/portal/sonoff_B/gpio/relais\nH',
    #b'eos/portal/sonoff_B/gpio/relais\nL',
    #b'eos/portal/buttons_in/set\nr 1000:ff0000,00ff00,0000ff,000000 2000:000000,ff0000,00ff00,0000ff 3000:0000ff,000000,ff0000,00ff00 4000:00ff00,0000ff,000000,ff0000 ',
    #b'eos/portal/buttons_in/set\nr 1000:ffff00,00ffff,ff00ff,aaaaaa 2000:aaaaaa,ffff00,00ffff,ff00ff 3000:ff00ff,aaaaaa,ffff00,00ffff 4000:00ffff,ff00ff,aaaaaa,ffff00 ',
    #b'SUB\neos/portal/*/button/*'
]

while True:
    for msg in msgs:
        print('send',msg)
        s.sendto(msg, ('<broadcast>', 1883))
        s.sendto(msg, ('192.168.178.199', 1883))
        for r in range(200):
            try:
                data,addr = s.recvfrom(1024)
                print('data',data,'addr',addr)
            except:
                sleep(0.1)
