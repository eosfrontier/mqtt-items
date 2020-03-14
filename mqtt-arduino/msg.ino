#include <WiFiUdp.h>
#include <FS.h>

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
  SPIFFS.begin();
}

// Strings vergelijken met een beperkte wildcard optie
// * matcht alles tot aan de volgende slash (werkt dus alleen met */)
bool strmatch(const char *patt, const char *match, bool partial = false)
{
  // Serial.print("strmatch("); Serial.print(patt); Serial.print(", "); Serial.print(match); Serial.print(", "); Serial.print(partial); Serial.println(")");
  
  const char *p = patt, *m = match;
  int slashidx = 0;
  while (*p && *m) {
    if (*p == '*') {
      p++;
      while (*m && *m != '/') m++;
    } else {
      if (*p != *m) {
        // Serial.print("strmatch: mismatch at <"); Serial.print(p); Serial.print("> || <"); Serial.print(m); Serial.print(">, num = "); Serial.println(slashidx);
        return false;
      }
      p++;
      m++;
    }
  }
  return (partial || (*p == *m));
}

void msg_send_sub(const char *topic, const char *msg, int idx)
{
  if (subscribers[idx].topic[0]) {
    if (strmatch(subscribers[idx].topic, topic)) {
      udp.beginPacket(subscribers[idx].ip, mqtt_port);
      udp.write(MSG_NAME "/", strlen(MSG_NAME)+1);
      udp.write(topic, strlen(topic));
      udp.write('\n');
      udp.write(msg, strlen(msg));
      udp.endPacket();
    }
  }
}

void msg_send(const char *topic, const char *msg)
{
  for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
    msg_send_sub(topic, msg, i);
  }
}

unsigned long lastsub = 0;
char lastack[33];

void msg_add_sub(const char *topic)
{
  // Serial.print("Add sub <"); Serial.print(topic); Serial.println(">");
  if (strmatch(topic, MSG_NAME "/", true)) {
    const char *postfix = topic;
    for (int i = 0; i < MSG_NAME_NUM_PARTS; i++) {
      postfix = strchr(postfix, '/');
      if (!postfix) {
        Serial.print("ERROR: Subscribe topic too few parts: <"); Serial.print(topic); Serial.println(">");
        return; // Sanity
      }
      postfix++;
    }
    if (strlen(postfix) >= sizeof(subscribers[0].topic)) {
      Serial.print("ERROR: Subscribe topic too long: <"); Serial.print(postfix); Serial.println(">");
      return; 
    }
    IPAddress ip = udp.remoteIP();
    int idx = -1;
    bool resub = false;
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
      if (subscribers[i].ip == ip && !strcmp(subscribers[i].topic, postfix)) {
        idx = i;
        resub = true;
        break;
      }
      if (idx < 0 && (!subscribers[i].topic[0])) {
        idx = i;
      }
    }
    if (idx >= 0) {
      if (!resub) {
        Serial.print("Subscribed "); Serial.print(ip); Serial.print(" on <"); Serial.print(postfix); Serial.println(">");
        strcpy(subscribers[idx].topic, postfix);
        subscribers[idx].ip = ip;
        lastsub = lasttick;
      }
      subscribers[idx].lastseen = lasttick;
      if (lastack[0]) {
        msg_send_sub("ack", lastack, idx);
      }
    } else {
      Serial.print("ERROR: More than "); Serial.print(MAX_SUBSCRIBERS); Serial.print(" subscribers, cannot subscribe "); Serial.print(ip); Serial.print(" <"); Serial.print(topic); Serial.println(">");
    }
  }
}

void msg_receive(const char *topic, const char *msg)
{
  Serial.print("receive <"); Serial.print(topic); Serial.print("> = <"); Serial.print(msg); Serial.println(">");
  // SUB topic is een meta-topic waar anderen op onze messages subscriben
  if (!strcmp(topic, "SUB")) {
    msg_add_sub(msg);
    return;
  }
  // Tset topic is het topic wat zegt dat we een specifieke kleur moeten gaan doen
  if (!strcmp(topic, MSG_NAME "/set")) {
    leds_set(msg);
    msg_send("ack", msg);
    // ws_send_ack(msg);
    if (strlen(msg) < (sizeof(lastack)-1)) {
      strcpy(lastack, msg);
    } else {
      lastack[0] = 0;
    }
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

void msg_subscribe(const char *topic)
{
  IPAddress bcast = WiFi.localIP();
  if (bcast) {
    IPAddress smask = WiFi.subnetMask();
    Serial.print("Sending sub from ip "); Serial.print(bcast); Serial.print(" mask "); Serial.println(smask);
    for (int i = 0; i < 4; i++) bcast[i] |= ~smask[i];
    Serial.print("Broadcast to "); Serial.println(bcast);
    udp.beginPacket(bcast, mqtt_port);
    udp.write("SUB\n", 4);
    udp.write(topic, strlen(topic));
    udp.endPacket();
  }
  
  bcast = WiFi.softAPIP();
  if (bcast) {
    Serial.print("Sending sub from SoftAP ip "); Serial.println(bcast);
    bcast[3] = 255;
    Serial.print("Broadcast to "); Serial.println(bcast);
    udp.beginPacket(bcast, mqtt_port);
    udp.write("SUB\n", 4);
    udp.write(topic, strlen(topic));
    udp.endPacket();
  }
}

unsigned long lastscan = 0;
char wifiidx = 0;

void msg_check()
{
  if (WiFi.status() != WL_CONNECTED) {
      if (!strcmp(state, "nosubs")) { // Als we van nosubs anders naar nowifi gaan (NB: initieel is nosubs omdat ESP automatisch connecten ij powerup)
          Serial.println("No WiFi connection");
          leds_set("nowifi");
          lastscan = lasttick + 5 * 1000; // 5 seconden wachten tot active reconnect
      }
      if (lasttick > lastscan) {
          lastscan = lasttick + 5 * 1000; // Elke 10 seconden een ssid proberen
#ifdef MQTT_SOFTAP
          if (wifiidx == 0) {
              wifiidx = 1;
              File wifitxt = SPIFFS.open("/wifiA.txt", "r");
              String assid = wifitxt.readStringUntil('\n');
              String apwd = wifitxt.readStringUntil('\n');
              wifitxt.close();
              WiFi.softAP(assid, apwd);
              Serial.print("SoftAP Listening on '"); Serial.print(assid); Serial.print("' pwd='"); Serial.print(apwd); Serial.print("' IP="); Serial.println(WiFi.softAPIP());
              Serial.print("Currently we have "); Serial.print(WiFi.softAPgetStationNum()); Serial.println(" clients connected to SoftAP");
          }
#endif
          wifiidx++;
          char wififn[11] = "/wifiA.txt";
          wififn[5] = 'A' + (wifiidx-1);
          if (!SPIFFS.exists(wififn)) {
              Serial.print("Scanned wifi up to "); Serial.print(wififn); Serial.println(" redo from start");
              lastscan = lasttick + 30 * 1000; // 60 seconden wachten en opnieuw beginnen
              wifiidx = 0;
              int nap = WiFi.scanNetworks();
              if (nap == 0) {
                Serial.println("WiFi scan found zero networks!");
              }
              for (int i = 0; i < nap; i++) {
                Serial.print("Found network "); Serial.print(WiFi.SSID(i)); Serial.print(" on Channel "); Serial.print(WiFi.channel(i)); Serial.print(" ("); Serial.print(WiFi.RSSI()); Serial.println(")");
              }
              WiFi.scanDelete();
          } else {
              File wifitxt = SPIFFS.open(wififn, "r");
              String wssid = wifitxt.readStringUntil('\n');
              String wpwd = wifitxt.readStringUntil('\n');
              wifitxt.close();
              Serial.print("Wifi trying to connect to '"); Serial.print(wssid); Serial.print("' pwd '"); Serial.print(wpwd); Serial.println("'...");
              WiFi.begin(wssid, wpwd);
          }
      }
  } else if (!strcmp(state, "nowifi")) {
#ifdef MQTT_SOFTAP
      if (WiFi.softAPIP()) {
          Serial.println("Connected to real WiFI, disconnecting soft AP");
          WiFi.softAPdisconnect(true);
      }
#endif
      Serial.print("Connected to WiFi, IP: "); Serial.println(WiFi.localIP());
      leds_set("idle");
      lastsub = lasttick + 500; // Halve seconde afwachten
  }
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
  int numsubs = 0;
  for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
    if (subscribers[i].topic[0]) {
      if ((lasttick - subscribers[i].lastseen) > MSG_TIMEOUT) {
        subscribers[i].topic[0] = 0;
      } else {
        numsubs++;
      }
    }
  }
  // Resubscribe every N seconds
  if ((signed)(lasttick - lastsub) >= 0) { // ipv. lasttick >= lastsub ivm overflow effecten
    lastsub = lasttick + MSG_TIMEOUT/3;
    for (int i = 0; MSG_SUBSCRIPTIONS[i]; i++) {
      msg_subscribe(MSG_SUBSCRIPTIONS[i]);
    }
  }
  if (strcmp(state, "nowifi")) {
    if (numsubs == 0) {
      if (strcmp(state, "nosubs")) {
        Serial.println("No subs, not live, waiting a while and then disconnecting from WiFi");
        lastscan = lasttick + random(60 * 1000, 90 * 1000); // 60-90 seconden wachten op subs
        leds_set("nosubs");
      } else if (lasttick > lastscan) {
        leds_set("nowifi");
        Serial.println("No subs for a while, disconnecting from wifi");
        WiFi.disconnect();
      }
    } else {
      lastscan = lasttick + 10 * 1000;
      if (!strcmp(state, "nosubs")) {
        Serial.println("We have subscribers, we are live!");
        leds_set("idle");
      }
    }
  }
}
