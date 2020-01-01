#include <WiFiUdp.h>

unsigned int mqtt_port = 1883;
WiFiUDP udp;

struct {
    IPAddress ip;
    unsigned long lastseen;
    char topic[24];
} subscribers[MAX_SUBSCRIBERS];

void msg_setup()
{
  udp.begin(mqtt_port);
}

// Strings vergelijken met een beperkte wildcard optie
// * matcht alles tot aan de volgende slash (werkt dus alleen met */)
bool strmatch(const char *patt, const char *match, int prefixidx = 0)
{
  const char *p = patt, *m = match;
  int slashidx = 0;
  while (*p && *m) {
    if (*p == '*') {
      p++;
      while (*m && *m != '/') m++;
    } else {
      if (*p == '/') {
        if (++slashidx == prefixidx) {
          return true; // Partial match
        }
      }
      if (*p != *m) {
        return false;
      }
      p++;
      m++;
    }
  }
  return (*p == *m);
}

void msg_send(const char *topic, const char *msg)
{
  for (int i = 0; i < MAX_SUBSCRIBERS) {
    if (subscribers[i].lastseen && ((lasttick - subscribers[i].lastseen) < MSG_TIMEOUT)) {
      if (strmatch(subscribers[i].topic, topic)) {
        udp.beginPacket(subscribers[i].ip, mqtt_port);
        udp.write(MSG_NAME "/", strlen(MSG_NAME)+1);
        udp.write(topic, strlen(topic));
        udp.write('\n');
        udp.write(msg, strlen(msg));
        udp.endPacket();
      }
    }
  }
}

void msg_add_sub(const char *topic)
{
  if (strmatch(topic, MSG_NAME, MSG_NAME_NUM_PARTS)) {
    IPAddress ip = udp.remoteIP();
    int idx = -1;
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
      if (subscribers[i].ip == ip && !strcmp(subscribers[i].topic, topic)) {
        idx = i;
        break;
      }
      if (idx < 0 && (!subscribers[i].lastseen || ((lasttick - subscribers[i].lastseen) > MSG_TIMEOUT))) {
        idx = i;
      }
    }
    if (idx >= 0) {
      subscribers[idx].ip = ip;
      strcpy(subscribers[idx].topic, topic);
      subscribers[idx].lastseen = lasttick;
    } else {
      Serial.print("ERROR: More than "); Serial.print(MAX_SUBSCRIBERS); Serial.print(" subscribers, cannot subscribe "); Serial.print(ip); Serial.print(" <"); Serial.print(topic); Serial.println(">");
    }
  }
}

void msg_receive(const char *topic, const char *msg)
{
  Serial.print("receive <"); Serial.print(topic); Serial.print("> = <"); Serial.print(msg); Serial.println(">");
  // SUB topic is for subscribing
  if (!strcmp(topic, "SUB")) {
    msg_add_sub(msg);
    return;
  }
  // Tset topic is het topic wat zegt dat we een specifieke kleur moeten gaan doen
  if (!strcmp(topic, MSG_NAME "/set")) {
    leds_set(msg);
    msg_send("ack", msg);
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

unsigned long lastsub = 0;

void msg_subscribe(const char *topic)
{
  IPAddress bcast = WiFi.localIP();
  IPAddress smask = WiFi.subnetMask();
  for (int i = 0; i < 4; i++) bcast[i] |= ~smask[i];
  udp.beginPacket(bcast, mqtt_port);
  udp.write("SUB\n", 4);
  udp.write(topic, strlen(topic));
  udp.endPacket();
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
      // String termineren en zoeken naar newline voor topic + message
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
  // Resubscribe every 20 seconds
  if ((lasttick - lastsub) > (MSG_TIMEOUT/3)) {
    lastsub = lasttick;
    for (int i = 0; MSG_SUBSCRIPTIONS[i]; i++) {
      msg_subscribe(MSG_SUBSCRIPTIONS[i]);
    }
  }
}
