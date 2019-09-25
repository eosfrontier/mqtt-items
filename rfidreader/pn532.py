import time,machine

class PN532:
    def __init__(self):
        self._i2c = machine.I2C(scl=machine.Pin(4),sda=machine.Pin(0),freq=100000)
        self._state = 'configsam'
        self._timeout = 10
        self.firmware = None
        self.cardid = None
        self.verbose = False

    def _reset(self):
        if self._timeout <= 10:
            return 'configsam',2

    def _configsam(self):
        if self._sendframe([0x14, 1, 20, 1]):
            return 'waitconfigsam',10

    def _waitconfigsam(self):
        if self._readframe(0x14, 0) != None:
            return 'configretry',2
    
    def _configretry(self):
        if self._sendframe([0x32, 0x05, 0x02, 0x02, 10]):
            return 'waitconfigretry',10

    def _waitconfigretry(self):
        if self._readframe(0x32, 0) != None:
            return 'getfirmware',2

    def _getfirmware(self):
        if self._sendframe([0x02]):
            return 'waitgetfirmware',10

    def _waitgetfirmware(self):
        res = self._readframe(0x02, 4)
        if res != None:
            self.firmware = "Firmware: IC:%02x Ver:%d.%d Supported:%02x" % (res[0],res[1],res[2],res[3])
            if self.verbose:
                print(self.firmware)
            return 'idle',100

    def _idle(self):
        return 'scancard',2

    def _scancard(self):
        if self._sendframe([0x4a, 0x01, 0x00]):
            return 'waitscancard',100

    def _waitscancard(self):
        res = self._readframe(0x4a, 10)
        if res != None:
            if res[0] == 1:
                self.cardid = (res[6]<<24)+(res[7]<<16)+(res[8]<<8)+res[9]
                if self.verbose:
                    print("Got cardid 0x%08x" % self.cardid)
            else:
                self.cardid = None
            return 'idle',100
        if self._timeout <= 1:
            self.cardid = None
            return 'idle',100

    def _sendframe(self,cmd):
        frame = [0x00,0xff,len(cmd)+1, 255-len(cmd)]
        chk = 0
        for c in [0xd4]+cmd:
            frame.append(c)
            chk -= c
        frame.append(chk%256)
        frame.append(0)
        if self.verbose:
            print("Send frame %s" % bytes(frame))
        if self._i2c.writeto(0x24, bytearray(frame)) < len(frame):
            print("Failed to send frame for 0x%02x" % cmd[0])
            return False
        ack = self._i2c.readfrom(0x24, 7)
        if ack != b'\x01\x00\x00\xff\x00\xff\x00':
            print("Failed to send frame, got ack %s" % ack)
            print("Wanted to send frame %s" % bytes(frame))
            return False
        return True

    def _readframe(self,cmd,count):
        if self.verbose:
            print("Read frame")
        result = self._i2c.readfrom(0x24, count+8)
        if self.verbose:
            print("Got frame %s" % result)
        if len(result) >= 1 and result[0] == 0x01:
            if result[7]-1 != cmd:
                print("Unexpected command answer (%02x <> %02x)" % (cmd,result[7]))
                print("Result: %s" % result)
                return None
            return result[8:]
        return None
    _states = {
        'reset':_reset,
        'configsam':_configsam,
        'waitconfigsam':_waitconfigsam,
        'configretry':_configretry,
        'waitconfigretry':_waitconfigretry,
        'getfirmware':_getfirmware,
        'waitgetfirmware':_waitgetfirmware,
        'idle':_idle,
        'scancard':_scancard,
        'waitscancard':_waitscancard
    }
    def mainloop(self):
        if self.verbose:
            print("Step %s" % self._state)
        newstate = self._states[self._state](self)
        self._timeout -= 1
        if newstate:
            self._state = newstate[0]
            self._timeout = newstate[1]
        elif self._timeout <= 0:
            print("PN532 timeout on %s" % self._state)
            self._state = 'reset'
            self._timeout = 100

def test(msec):
    pn = PN532()
    cardid = None
    for r in range(msec):
        pn.mainloop()
        if pn.cardid != cardid:
            cardid = pn.cardid
            if cardid:
                print("Card: %08x" % cardid)
