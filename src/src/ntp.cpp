#include <stdint.h>
#include "settings.h"
#include "ntp.h"
#ifdef MQTT_RFID
#include <WiFiUdp.h>

WiFiUDP ntp_udp;

const unsigned int ntp_port = 2390;

uint32_t ntp_offset;
unsigned long ntp_last_check;
unsigned long ntp_next_check;

IPAddress ntp_ip;
const char *ntp_name = "europe.pool.ntp.org";

uint32_t ntp_time(uint32_t time)
{
  return time + ntp_offset;
}

uint32_t ntp_now()
{
  return ntp_time(millis()/1000);
}

void ntp_setup()
{
  ntp_last_check = 0;
  ntp_next_check = 0;
  ntp_offset = 0;
  WiFi.hostByName(ntp_name, ntp_ip);
  ntp_udp.begin(ntp_port);
}

void ntp_check()
{
  int pak;
  uint8_t packet[48];
  unsigned long now = millis();
  while ((pak = ntp_udp.parsePacket()) > 0) {
    int rd = ntp_udp.read(packet, sizeof(packet));
    if (rd < pak) {
      debugE("NTP Short read, got %d, expected %d", rd, pak);
      ntp_udp.flush();
      if (rd < 48) break;
    }
    // Serial.print("NTP time packet: "); for (int i = 0; i < rd; i++) { Serial.printf("%02x ", packet[i]); }
    uint32_t osecs = (packet[40] << 24) | (packet[41] << 16) | (packet[42] << 8) | (packet[43]);
    uint32_t secs = osecs - 2208988800UL; // 1900 -> 1970 for epoch time
    ntp_offset = secs - (now / 1000);
    ntp_last_check = now;
    ntp_next_check = now + 3600000;
    debugD("NTP time result: %d shifted: %d, offset = %d", osecs, secs, ntp_offset);
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (ntp_next_check < lasttick) {
    ntp_next_check = lasttick + 5000;
    if (!ntp_ip || (ntp_next_check > (ntp_last_check + 30000))) {
      WiFi.hostByName(ntp_name, ntp_ip);
    }

    memset(packet, 0, sizeof(packet));
    packet[0] = 0xE3;
    packet[1] = 0x00;
    packet[2] = 0x06;
    packet[3] = 0xEC;

    packet[12] = 0x31;
    packet[13] = 0x4E;
    packet[14] = 0x31;
    packet[15] = 0x34;

    ntp_udp.beginPacket(ntp_ip, 123);
    ntp_udp.write(packet, sizeof(packet));
    ntp_udp.endPacket();
    // Serial.print("NTP time packet sent: "); for (int i = 0; i < sizeof(packet); i++) { Serial.printf("%02x ", packet[i]); }
    debugD("NTP sent request to: %s", ntp_ip.toString().c_str());
  }
}

#else
void ntp_setup() {}
void ntp_check() {}
uint32_t ntp_now() { return 0; }
#endif
