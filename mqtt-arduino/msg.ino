#include <WiFiUdp.h>

unsigned long loadavg = 0;
unsigned long laststatus = 0;

const char *tstatus = MSG_NAME "/status";
const char *tack = MSG_NAME "/ack";
const char *tbtn = MSG_NAME "/button/%s";
const char *tset = MSG_NAME "/set";

unsigned int mqtt_port = 1883;
WiFiUDP udp;

void msg_setup()
{
  udp.begin(mqtt_port);
}

void msg_check()
{
  char buf[1025];
  int pak = udp.parsePacket();
  if (pak > 0) {
    int rd = udp.read(buf, sizeof(buf)-1);
    if (rd < pak) {
      Serial.print("Short read, got "); Serial.print(rd); Serial.print(", expected "); Serial.println(pak);
    }
    if (rd > 0) {
      buf[rd] = 0;
      char *topic = buf;
      char *msg = (char *)memchr(buf, '\n', rd);
      if (msg) {
        *msg++ = 0;
      }
      Serial.print("Topic: "); Serial.println(topic);
      Serial.print("Msg: "); Serial.println(msg);
      if (!strcmp(topic, tset)) {
        leds_set(msg);
      }
    }
  }
}
