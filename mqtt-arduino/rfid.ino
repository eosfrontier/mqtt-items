#ifdef MQTT_RFID
#include <Wire.h>

#define PN532 0x24
#define I2C_SDA 0
#define I2C_SCL 4

#define R_reset 0
#define R_resetwait 1
#define R_getfirmware 2
#define R_waitgetfirmware 3
#define R_configsam 4
#define R_waitconfigsam 5
#define R_configretry 6
#define R_waitconfigretry 7
#define R_idle 8
#define R_scancard 9
#define R_waitscancard 10
#define R_toidle 11
int rfid_state;

int rfid_timeout;
uint32_t rfid_cardid = 0;

void rfid_setup()
{
    rfid_state = R_reset;
    rfid_timeout = 2;
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);
}

uint8_t rfid_reset[]       = {0x00,0x00,0xff,0x00,0xff,0x00};
uint8_t rfid_configsam[]   = {0x14, 1, 20, 1};
uint8_t rfid_configretry[] = {0x32, 0x05, 0x02, 0x02, 200};
uint8_t rfid_getfirmware[] = {0x02};
uint8_t rfid_scancard[]    = {0x4a, 0x01, 0x00};

#define RFID_SENDFRAME(f) rfid_sendframe(sizeof(f), f)

void print_bytes(uint8_t *bytes, int len)
{
  for (int i = 0; i < len; i++) {
    Serial.printf(" %02X", bytes[i]);
  }
  Serial.println(" ");
}

bool rfid_sendframe(uint8_t len, uint8_t *frame)
{
  uint8_t buff[32] = {0x00,0xff};
  uint8_t ack[7];
  uint8_t ackok[7] = {0x01,0x00,0x00,0xff,0x00,0xff,0x00};
  int bufp = 2;
  buff[bufp++] = len+1;
  buff[bufp++] = 0xff-len;
  buff[bufp++] = 0xd4;
  uint8_t chk = 0;
  chk -= 0xd4;
  for (uint8_t i = 0; i < len; i++) {
    buff[bufp++] = frame[i];
    chk -= frame[i];
  }
  buff[bufp++] = chk;
  buff[bufp++] = 0;
  Wire.beginTransmission(PN532);
  if (Wire.write(buff, bufp) < bufp) {
    Serial.print("RFID Failed to send frame: short write on "); Serial.println(frame[0], HEX);
    return false;
  }
  int err = Wire.endTransmission();
  if (err != 0) {
    Serial.print("RFID transmission error "); Serial.print(err); Serial.print(" on "); Serial.println(frame[0], HEX);
    return false;
  }
  delay(1);
  int rbytes = Wire.requestFrom(PN532, sizeof(ack));
  rbytes = Wire.readBytes(ack, rbytes);
  if (rbytes < sizeof(ack)) {
    Serial.print("RFID Failed to send frame: short read on "); Serial.println(frame[0], HEX);
    Serial.print("Got bytes"); print_bytes(ack, rbytes);
    return false;
  }
  if (memcmp(ack,ackok,sizeof(ack))) {
    Serial.print("RFID Failed to send frame: got bad ack"); print_bytes(ack, sizeof(ack));
    Serial.print("RFID sent frame:"); print_bytes(buff, bufp);
    return false;
  }
  return true;
}

uint8_t *rfid_readframe(uint8_t cmd, uint8_t len)
{
  static uint8_t buf[32];
  int rbytes = Wire.requestFrom(PN532, len+8);
  rbytes = Wire.readBytes(buf, rbytes);
  if (rbytes < len+8) {
    Serial.print("RFID short read "); Serial.print(rbytes); Serial.print(" wanted "); Serial.print(len+8); Serial.print(" on cmd "); Serial.println(cmd, HEX);
    Serial.print("Got response"); print_bytes(buf, rbytes);
    return NULL;
  }
  if (buf[0] != 0x01) {
    // No data ready
    return NULL;
  }
  if (buf[7] != cmd+1) {
    Serial.print("RFID bad response on cmd "); Serial.println(cmd, HEX);
    Serial.print("Got response"); print_bytes(buf, rbytes);
    return NULL;
  }
  // Serial.print("DBG: got frame: "); print_bytes(buf, len+8);
  return buf+8;
}

int rfid_statemachine()
{
  switch(rfid_state) {
    case R_reset:
      Wire.beginTransmission(PN532);
      if (Wire.write(rfid_reset, sizeof(rfid_reset)) < sizeof(rfid_reset)) {
        Serial.println("RFID Failed to send frame: short write on RESET");
      } else {
        int err = Wire.endTransmission();
        if (err != 0) {
          Serial.print("RFID transmission error "); Serial.print(err); Serial.println(" on RESET");
        }
      }
      return 127;
    case R_resetwait:
      if (rfid_timeout < 8) {
        return 2;
      }
      /*
      Wire.beginTransmission(rfid_timeout);
      if (int err = Wire.endTransmission()) {
        if (err == 4) {
          Serial.print("RFID: I2C error at "); Serial.println(rfid_timeout, HEX);
        }
      } else {
        Serial.print("RFID: I2C device found at "); Serial.println(rfid_timeout, HEX);
      }
      */
      return 0;
    case R_configsam:
      if (RFID_SENDFRAME(rfid_configsam)) {
        return 10;
      }
      return 0;
    case R_waitconfigsam:
      if (rfid_readframe(0x14, 0)) {
        return 2;
      }
      return 0;
    case R_configretry:
      if (RFID_SENDFRAME(rfid_configretry)) {
        return 10;
      }
      return 0;
    case R_waitconfigretry:
      if (rfid_readframe(0x32, 0)) {
        return 2;
      }
      return 0;
    case R_getfirmware:
      if (RFID_SENDFRAME(rfid_getfirmware)) {
        return 10;
      }
      return 0;
    case R_waitgetfirmware:
      if (uint8_t *frame = rfid_readframe(0x02, 4)) {
        Serial.printf("RFID Firmware: IC:%02x Ver:%d.%d Supported:%02x\n", frame[0], frame[1], frame[2], frame[3]);
        return 100;
      }
      return 0;
    case R_idle:
      return 2;
    case R_scancard:
      if (RFID_SENDFRAME(rfid_scancard)) {
        return 100;
      }
      return 0;
    case R_waitscancard:
      if (uint8_t *frame = rfid_readframe(0x4a, 10)) {
        if (frame[0] == 1) {
          uint32_t cardid = (frame[6]<<24)+(frame[7]<<16)+(frame[8]<<8)+(frame[9]);
          if (cardid != rfid_cardid) {
            //Serial.print("RFID got cardid: "); Serial.println(cardid, HEX);
            rfid_cardid = cardid;
            api_got_cardid(cardid);
          }
        } else {
          rfid_cardid = 0;
        }
        return 1;
      }
      return 0;
  }
}

void rfid_check()
{
  int newtimeout = rfid_statemachine();
  rfid_timeout--;
  if (newtimeout > 0) {
    rfid_timeout = newtimeout;
    rfid_state = rfid_state + 1;
    if (rfid_state >= R_toidle) rfid_state = R_idle;
    // Serial.print("DBG: RFID step "); Serial.println(rfid_state);
  } else if (rfid_timeout <= 0) {
    Serial.print("RFID PN532 timeout on step "); Serial.println(rfid_state);
    rfid_state = R_reset;
    rfid_timeout = 2;
  }
}

#else
void rfid_setup() {}
void rfid_check() {}
#endif
