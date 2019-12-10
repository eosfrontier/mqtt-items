#include <WiFiUdp.h>

const char *tack = MSG_NAME "/ack";
const char *tset = MSG_NAME "/set";

const char *wanttopic = NULL;
const char *wantmsg = NULL;

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

// Strings vergelijken met een beperkte wildcard optie
// * matcht alles tot aan de volgende slash (werkt dus alleen met */)
bool strmatch(const char *patt, const char *match)
{
  const char *p = patt, *m = match;
  while (*p && *m) {
    if (*p == '*') {
      p++;
      while (*m && *m != '/') m++;
    } else {
      if (*p != *m) {
        return false;
      }
      p++;
      m++;
    }
  }
  return (*p == *m);
}

void msg_receive(const char *topic, const char *msg)
{
  Serial.print("receive <"); Serial.print(topic); Serial.print("> = <"); Serial.print(msg); Serial.println(">");
  // Tset topic is het topic wat zegt dat we een specifieke kleur moeten gaan doen
  if (!strcmp(topic, tset)) {
    leds_set(msg);
    msg_send(tack, msg);
    buttons_ack();
    return;
  }
  // De mapping array bevat mappings van andere topics,
  // dit is effectief de koppeling tussen acties en hun reacties
  for (int m = 0; MSG_MAPPING[m]; m += 4) {
    if (strmatch(MSG_MAPPING[m], topic)) {
      if (strmatch(MSG_MAPPING[m+1], msg)) {
        const char *smsg = MSG_MAPPING[m+3];
        // Als blanco, dan geven we het originele bericht mee
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
  // Kijken of er packets zijn
  int pak;
  while ((pak = udp.parsePacket()) > 0) {
    int rd = udp.read(buf, sizeof(buf)-1);
    if (rd < pak) {
      Serial.print("Short read, got "); Serial.print(rd); Serial.print(", expected "); Serial.println(pak);
      udp.flush();
    }
    if (rd > 0) {
      // Strint termineren en zoeken naar newline voor topic + message
      buf[rd] = 0;
      char *topic = buf;
      char *msg = (char *)memchr(buf, '\n', rd);
      if (msg) {
        *msg++ = 0;
      }
      // Serial.print("Topic: "); Serial.println(topic);
      // if (msg) Serial.print("Msg: "); Serial.println(msg);
      msg_receive(topic, msg);
    }
  }
}
