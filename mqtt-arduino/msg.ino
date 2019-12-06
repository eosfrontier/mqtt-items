#include <WiFiUdp.h>

unsigned long loadavg = 0;
unsigned long laststatus = 0;

const char *tstatus = MSG_NAME "/status";
const char *tack = MSG_NAME "/ack";
const char *tset = MSG_NAME "/set";

unsigned int mqtt_port = 1883;
WiFiUDP udp;

void msg_setup()
{
  udp.begin(mqtt_port);
}

void msg_send(const char *topic, const char *msg)
{
  IPAddress bcast = WiFi.localIP();
  IPAddress smask = WiFi.subnetMask();
  for (int i = 0; i < 4; i++) bcast[i] |= ~smask[i];
  udp.beginPacket(bcast, mqtt_port);
  udp.write(topic, strlen(topic));
  udp.write('\n');
  udp.write(msg, strlen(msg));
  udp.endPacket();
}

void msg_receive(const char *topic, const char *msg)
{
  if (!strcmp(topic, tset)) {
    leds_set(msg);
    msg_send(tack, msg);
    return;
  }
  for (int m = 0; m < sizeof(MSG_MAPPING)/sizeof(*MSG_MAPPING); m += 4) {
    if (!strcmp(MSG_MAPPING[m], topic)) {
      if (!MSG_MAPPING[m+1] || !strcmp(MSG_MAPPING[m+1], msg)) {
        const char *smsg = MSG_MAPPING[m+3];
        if (!smsg) smsg = msg;
        msg_receive(MSG_MAPPING[m+2],smsg);
        return;
      }
    }
  }
}

void msg_check()
{
  char buf[1025];
  int pak = udp.parsePacket();
  if (pak > 0) {
    int rd = udp.read(buf, sizeof(buf)-1);
    if (rd < pak) {
      Serial.print("Short read, got "); Serial.print(rd); Serial.print(", expected "); Serial.println(pak);
      udp.flush();
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
      msg_receive(topic, msg);
    }
  }
}
