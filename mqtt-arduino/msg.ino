#include <WiFiUdp.h>
#include <FS.h>

unsigned int mqtt_port = 1883;
WiFiUDP msg_udp;

struct {
    IPAddress ip;
    unsigned long lastseen;
    char topic[24];
} subscribers[MAX_SUBSCRIBERS];

void msg_setup()
{
  msg_udp.begin(mqtt_port);
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
      msg_udp.beginPacket(subscribers[idx].ip, mqtt_port);
      msg_udp.write(MSG_NAME "/", strlen(MSG_NAME)+1);
      msg_udp.write(topic, strlen(topic));
      msg_udp.write('\n');
      msg_udp.write(msg, strlen(msg));
      msg_udp.endPacket();
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
    IPAddress ip = msg_udp.remoteIP();
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

unsigned long lastscan = 0;
char wifiidx = 3;
char gotssid = 0;

#ifdef MQTT_SOFTAP
void send_ssid(void)
{
    if (SPIFFS.exists("/wifiD.txt")) {
        gotssid = 1;
        File wifitxt = SPIFFS.open("/wifiD.txt", "r");
        String ssidtxt = wifitxt.readString();
        wifitxt.close();
        const char *msg = ssidtxt.c_str();
        Serial.println("Sending SSID to soft AP subscribers");
        for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
            if (subscribers[i].topic[0]) {
              if (subscribers[i].ip[3] == 4) {
                msg_udp.beginPacket(subscribers[i].ip, mqtt_port);
                msg_udp.write("SSID", 4);
                msg_udp.write('\n');
                msg_udp.write(msg, strlen(msg));
                msg_udp.endPacket();
              }
            }
        }
    }
}
#endif

void add_ssid(const char *msg)
{
    File wifitxt = SPIFFS.open("/wifiD.txt", "w");
    wifitxt.print(msg);
    wifitxt.close();
    Serial.println("Got SSID");
#ifdef MQTT_SOFTAP
    send_ssid();
#endif
    wifiidx = 4;
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Forcing disconnect");
      WiFi.disconnect();
      leds_set("nowifi");
      lastscan = lasttick + 100;
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
  if (!strcmp(topic, "SSID")) {
    add_ssid(msg);
    return;
  }
  // Tset topic is het topic wat zegt dat we een specifieke kleur moeten gaan doen
  if (!strcmp(topic, MSG_NAME "/set")) {
    leds_set(msg);
    msg_send("ack", msg);
    if (strlen(msg) < (sizeof(lastack)-1)) {
      strcpy(lastack, msg);
    } else {
      lastack[0] = 0;
    }
    buttons_ack();
    return;
  }
#ifdef MQTT_GPIO
  if (strmatch(MSG_NAME "/gpio/*", topic)) {
    gpio_set(topic+strlen(MSG_NAME "/gpio/"), msg);
    return;
  }
#endif
#ifdef MQTT_WEBSOCKETS
  if (!strcmp(topic, WS_BROADCAST_ACK)) {
    ws_send_ack(msg);
  }
#endif
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
    msg_udp.beginPacket(bcast, mqtt_port);
    msg_udp.write("SUB\n", 4);
    msg_udp.write(topic, strlen(topic));
    msg_udp.endPacket();
  }
  
  bcast = WiFi.softAPIP();
  if (bcast) {
    Serial.print("Sending sub from SoftAP ip "); Serial.println(bcast);
    bcast[3] = 255;
    Serial.print("Broadcast to "); Serial.println(bcast);
    msg_udp.beginPacket(bcast, mqtt_port);
    msg_udp.write("SUB\n", 4);
    msg_udp.write(topic, strlen(topic));
    msg_udp.endPacket();
  }

#ifdef MQTT_SOFTAP
    if (WiFi.softAPIP()) {
        if (WiFi.softAPgetStationNum() > 0) {
            if (gotssid) {
                send_ssid();
            }
        } else {
            gotssid = 0;
            if (WiFi.status() != WL_CONNECTED) {
                leds_set("nowifi");
                lastscan = lasttick + 100;
            } else {
                Serial.println("Connected to real WiFI, no subscribers, disconnecting soft AP");
                WiFi.softAPdisconnect(true);
            }
        }
    }
#endif
}

void msg_check()
{
  if (WiFi.status() != WL_CONNECTED) {
      if (!strcmp(state, "nosubs")) { // Als we van nosubs anders naar nowifi gaan (NB: initieel is nosubs omdat ESP automatisch connecten bij powerup)
          Serial.println("No WiFi connection");
          leds_set("nowifi");
          lastscan = lasttick + 5 * 1000; // 5 seconden wachten tot active reconnect
      }
      if (lasttick > lastscan) {
          lastscan = lasttick + 5 * 1000; // Elke 5 seconden een ssid proberen
          char wififn[11] = "/wifiA.txt";
          if (wifiidx <= 0) {
            wifiidx = 0;
            while (SPIFFS.exists(wififn)) {
              wifiidx++;
              wififn[5] = 'A' + wifiidx;
            }
            Serial.print("Scanned wifi down from "); Serial.print(wififn); Serial.println(" redo from start");
            /*
            int nap = WiFi.scanNetworks(false, true);
            if (nap == 0) {
                Serial.println("WiFi scan found zero networks!");
            }
            for (int i = 0; i < nap; i++) {
                Serial.print("Found network "); Serial.print(WiFi.SSID(i)); Serial.print(" on Channel "); Serial.print(WiFi.channel(i)); Serial.print(" ("); Serial.print(WiFi.RSSI(i)); Serial.println(")");
            }
            WiFi.scanDelete();
            */
            lastscan = lasttick + 60 * 1000; // Na de hele lijst 60 seconden wachten
          } else {
            wifiidx--;
            wififn[5] = 'A' + wifiidx;
            File wifitxt = SPIFFS.open(wififn, "r");
            String wssid = wifitxt.readStringUntil('\n');
            String wpwd = wifitxt.readStringUntil('\n');
            wifitxt.close();
#ifdef MQTT_SOFTAP
            if (wifiidx == 0) {
              static int softapchannel = 1;
              if (!WiFi.softAPIP()) {
                  uint32_t seenchannels = 0;
                  int nap = WiFi.scanNetworks(false, true);
                  if (nap == 0) { Serial.println("WiFi scan found zero networks!"); }
                  for (int i = 0; i < nap; i++) {
                      int ch = WiFi.channel(i);
                      Serial.print("Found network "); Serial.print(WiFi.SSID(i)); Serial.print(" on Channel "); Serial.print(ch); Serial.print(" ("); Serial.print(WiFi.RSSI(i)); Serial.println(")");
                      seenchannels |= (1 << ch);
                  }
                  WiFi.scanDelete();
                  for (int c = 1; c <= 11; c++) {
                      if (!(seenchannels & (1 << c))) {
                          softapchannel = c;
                          break;
                      }
                  }
              }
              if (!WiFi.softAP(wssid, wpwd, softapchannel, false, 8)) {
                Serial.println("Error listening to SoftAP!");
              } else {
                Serial.print("SoftAP Listening on '"); Serial.print(wssid); Serial.print("' pwd='"); Serial.print(wpwd); Serial.print("' IP="); Serial.print(WiFi.softAPIP()); Serial.print(", channel "); Serial.println(softapchannel);
                Serial.print("Currently we have "); Serial.print(WiFi.softAPgetStationNum()); Serial.println(" clients connected to SoftAP");
              }
            } else
#endif
            {
              WiFi.begin(wssid, wpwd);
              Serial.print("Wifi trying to connect to '"); Serial.print(wssid); Serial.print("' pwd '"); Serial.print(wpwd); Serial.println("'...");
            }
          }
      }
  } else if (!strcmp(state, "nowifi")) {
      Serial.print("Connected to WiFi, IP: "); Serial.println(WiFi.localIP());
      leds_set("idle");
      lastsub = lasttick + 500; // Halve seconde afwachten
  }
  char buf[1025];
  // Kijken of er packets zijn
  int pak;
  while ((pak = msg_udp.parsePacket()) > 0) {
    int rd = msg_udp.read(buf, sizeof(buf)-1);
    if (rd < pak) {
      Serial.print("Short read, got "); Serial.print(rd); Serial.print(", expected "); Serial.println(pak);
      msg_udp.flush();
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
    lastsub = lasttick + MSG_SUB_INTERVAL;
    for (int i = 0; MSG_SUBSCRIPTIONS[i]; i++) {
      msg_subscribe(MSG_SUBSCRIPTIONS[i]);
    }
  }
  if (strcmp(state, "nowifi")) {
    if ((numsubs == 0) && (api_check_status < 0)) {
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
      if (!gotssid) {
          lastscan = lasttick + 10 * 1000;
      }
      if (!strcmp(state, "nosubs")) {
        Serial.println("We have subscribers, we are live!");
        leds_set("idle");
      }
    }
  }
}
