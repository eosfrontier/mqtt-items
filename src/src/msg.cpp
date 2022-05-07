#include "settings.h"
#include "msg.h"
#include "main.h"
#include "leds.h"
#include "buttons.h"
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Strings vergelijken met een beperkte wildcard optie
// * matcht alles tot aan de volgende slash (werkt dus alleen met */)
static bool strmatch(const char *patt, const char *match, bool partial = false)
{
  // Serial.print("strmatch("); Serial.print(patt); Serial.print(", "); Serial.print(match); Serial.print(", "); Serial.print(partial); Serial.println(")");
  
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
  return (partial || (*p == *m));
}

#ifdef MQTT_SOFTAP

static WiFiServer server(mqtt_port);

static WiFiClient clients[MAX_CLIENTS];

static struct {
    int16_t clientidx;
    char topic[62];
} subscribers[MAX_SUBSCRIBERS];

static void cprintf(WiFiClient client, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char s[256];

    vsnprintf(s, sizeof(s), fmt, args);
    client.print(s);
    va_end(args);
}

static void send_topics(WiFiClient client)
{
}

static void new_client(WiFiClient client)
{
    for (int idx = 0; idx < MAX_CLIENTS; idx++) {
        if (!clients[idx]) {
            clients[idx] = client;
            cprintf(client, "HELLO\r\n");
            send_topics(client);
            return;
        }
    }
    // No free space
    cprintf(client, "All connections full, disconnecting\r\n");
    client.stop();
}

static void server_send_message(const char *topic, const char *msg)
{
    for (int idx = 0; idx < MAX_SUBSCRIBERS; idx++) {
        if (subscribers[idx].clientidx >= 0) {
            if (strmatch(subscribers[idx].topic, topic)) {
                WiFiClient client = clients[subscribers[idx].clientidx];
                if (client && client.connected()) {
                    cprintf(client, "%s %s\r\n", topic, msg);
                }
            }
        }
    }
}

static void handle_message(int clientidx, const char *topic, const char *msg)
{
    if (!strcmp(topic, "SUB")) {
        if (strlen(msg) >= (sizeof(subscribers[0].topic)-1)) {
            cprintf(clients[clientidx], "ERROR Subscribe topic too long\r\n");
            return;
        }
        for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
            if (subscribers[si].clientidx < 0) {
                subscribers[si].clientidx = clientidx;
                strcpy(subscribers[si].topic, msg);
                return;
            }
        }
        cprintf(clients[clientidx], "ERROR No room for subscriptions\r\n");
    } else {
        server_send_message(topic, msg);
        msg_receive(topic, msg);
    }
}

static void check_client(int clientidx)
{
    WiFiClient client = clients[clientidx];
    char buf[256];
    size_t av = client.available();
    if (!av) return;
    if (av > 255) av = 255;
    if (client.peekBytes(buf, av) < av) return;
    char *nl = (char *)memchr(buf, '\n', av);
    if (nl) {
        int n = nl - buf + 1;
        if (client.read(buf, n) != n) return;
        while (buf[n-1] == '\r' || buf[n-1] == '\n' || buf[n-1] == ' ')
            n--;
        buf[n] = 0;
        char *msg = strchr(buf, ' ');
        if (msg) {
            *msg++ = 0;
            handle_message(clientidx, buf, msg);
        }
        return check_client(client);
    } else {
        client.read(buf, av);
        return check_client(client);
    }
}

static void server_check()
{
    // Connect new clients
    while (server.hasClient()) {
        new_client(server.available());
    }
    // Handle existing clients
    for (int idx = 0; idx < MAX_CLIENTS; idx++) {
        if (clients[idx]) {
            if (clients[idx].connected()) {
                check_client(clients[idx]);
            } else {
                for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
                    if (subscribers[si].clientidx == idx) {
                        subscribers[si].clientidx = -1;
                    }
                }
                clients[idx].stop();
            }
        }
    }
}

static void server_setup()
{
    for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
        subscribers[si].clientidx = -1;
    }
    server.begin();
}

#else // MQTT_SOFTAP
static void server_check() { }
static void server_setup() { }
static void server_send_message(const char *topic, const char *msg)
{
    // TODO: client connect to server and send
}
#endif // MQTT_SOFTAP

unsigned long lastsub = 0;
unsigned long lastacksend = 0;
char lastack[33];

void msg_setup()
{
#ifndef MQTT_SOFTAP
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
#else
  WiFi.mode(WIFI_AP_STA);
#endif
  int nap = WiFi.scanNetworks(false, true);
  if (nap == 0) {
      Serial.println("WiFi scan found zero networks!");
  }
  for (int i = 0; i < nap; i++) {
      Serial.print("Found network "); Serial.print(WiFi.SSID(i)); Serial.print(" on Channel "); Serial.print(WiFi.channel(i)); Serial.print(" ("); Serial.print(WiFi.RSSI(i)); Serial.println(")");
  }
  WiFi.scanDelete();
  LittleFS.begin();
  server_setup();
}

void msg_send(const char *topic, const char *msg)
{
#ifdef MQTT_SOFTAP
    if (!strcmp(topic, "SUB")) {
        server_send_message(topic, msg);
    }
#else // MQTT_SOFTAP
    // TODO: Client send message
#endif // MQTT_SOFTAP
}

unsigned long lastscan = 0;
char wifiidx = 3;
char gotssid = 0;
char gotrssi = 0;

#ifdef MQTT_SOFTAP
static void send_ssid(void)
{
    if (LittleFS.exists("/wifiD.txt")) {
        gotssid = 1;
        File wifitxt = LittleFS.open("/wifiD.txt", "r");
        String ssidtxt = wifitxt.readString();
        wifitxt.close();
        // const char *msg = ssidtxt.c_str();
        Serial.println("Sending SSID to soft AP subscribers");
                /*
        for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
            if (subscribers[i].topic[0]) {
                Serial.print("Sending SSID to soft AP subscriber "); Serial.println(subscribers[i].ip);
                msg_udp.beginPacket(subscribers[i].ip, mqtt_port);
                msg_udp.write("SSID", 4);
                msg_udp.write('\n');
                msg_udp.write(msg, strlen(msg));
                msg_udp.endPacket();
            }
        }
                */
    }
}
#endif

static void add_ssid(const char *msg)
{
    File wifitxt = LittleFS.open("/wifiD.txt", "w");
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
  // SSID topic is een meta-topic om de ssid om te zetten
  if (!strcmp(topic, "SSID")) {
    add_ssid(msg);
    return;
  }
  // Tset topic is het topic wat zegt dat we een specifieke kleur moeten gaan doen
  if (!strcmp(topic, MSG_NAME "/set")) {
    leds_set(msg);
    msg_send("ack", msg);
    if (!strcmp(msg, lastack)) return;  // Don't stop retrying if the set is the same as the previous one
    if (strlen(msg) < (sizeof(lastack)-1)) {
      strcpy(lastack, msg);
    } else {
      lastack[0] = 0;
    }
    buttons_ack();
    // return;  // Don't return: also allow chaining via mappings
  }
#ifdef MQTT_GPIO
  if (strmatch(MSG_NAME "/gpio/*", topic)) {
    gpio_set(topic+strlen(MSG_NAME "/gpio/"), msg);
    // return;  // Don't return: also allow chaining via mappings
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
        return;  // Later actions can be generic fallbacks
      }
    }
  }
}

static void msg_subscribe(const char *topic)
{
#ifndef MQTT_SOFTAP
    // TODO: Client send subscription
#endif
}

static void msg_connect_wifi()
{
  if (WiFi.status() != WL_CONNECTED) {
      if (!strcmp(state, "nosubs")) { // Als we van nosubs anders naar nowifi gaan (NB: initieel is nosubs omdat ESP automatisch connecten bij powerup)
          Serial.println("No WiFi connection");
          leds_set("nowifi");
          lastscan = lasttick + 5 * 1000; // 5 seconden wachten tot active reconnect
      }
      if (lasttick > lastscan) {
          Serial.print("WiFi Status "); Serial.print(WiFi.status()); Serial.print(" ("); Serial.print(WiFi.SSID()); Serial.print(":"); Serial.print(WiFi.psk()); Serial.println(")");
          Serial.print("WiFi IP / mac: "); Serial.print(WiFi.localIP()); Serial.print(" / "); Serial.println(WiFi.macAddress());
          Serial.print("WiFi mode: "); Serial.println(WiFi.getMode());
          int8_t rssi = WiFi.RSSI();
          Serial.print("WiFi bssid/rssi: "); Serial.print(WiFi.BSSIDstr()); Serial.print(" / "); Serial.println(rssi);
          if (!gotrssi) {
            if (rssi < 0) {  // Lager is beter
              gotrssi = 1;
              lastscan = lasttick + 60 * 1000; // 60 seconden wachten als er signaal is
              Serial.print("RSSI = "); Serial.print(rssi); Serial.println(", waiting for connection to resolve");
              return;
            }
          }
          lastscan = lasttick + 15 * 1000; // Elke 15 seconden een ssid proberen
          char wififn[11] = "/wifiA.txt";
          if (wifiidx <= 0) {
            wifiidx = 0;
            while (LittleFS.exists(wififn)) {
              wifiidx++;
              wififn[5] = 'A' + wifiidx;
            }
            Serial.print("Scanned wifi down from "); Serial.print(wififn); Serial.println(" redo from start");
            int nap = WiFi.scanNetworks(false, true);
            if (nap == 0) {
                Serial.println("WiFi scan found zero networks!");
            }
            for (int i = 0; i < nap; i++) {
                Serial.print("Found network "); Serial.print(WiFi.SSID(i)); Serial.print(" on Channel "); Serial.print(WiFi.channel(i)); Serial.print(" ("); Serial.print(WiFi.RSSI(i)); Serial.println(")");
            }
            WiFi.scanDelete();
            lastscan = lasttick + 60 * 1000; // Na de hele lijst 60 seconden wachten
          } else {
            wifiidx--;
            wififn[5] = 'A' + wifiidx;
            File wifitxt = LittleFS.open(wififn, "r");
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
              Serial.print("Wifi trying to connect to '"); Serial.print(wssid); Serial.print("' pwd '"); Serial.print(wpwd); Serial.println("'...");
              WiFi.begin(wssid, wpwd);
              gotrssi = 0;
            }
          }
      }
  } else if (!strcmp(state, "nowifi")) {
      Serial.print("Connected to WiFi, IP: "); Serial.println(WiFi.localIP());
      leds_set("idle");
      lastsub = lasttick + 500; // Halve seconde afwachten
  }
}

void msg_check()
{
  msg_connect_wifi();
  // Resubscribe every N seconds
  if ((signed)(lasttick - lastsub) >= 0) { // ipv. lasttick >= lastsub ivm overflow effecten
    lastsub = lasttick + MSG_SUB_INTERVAL;
    for (int i = 0; MSG_SUBSCRIPTIONS[i]; i++) {
      msg_send("SUB", MSG_SUBSCRIPTIONS[i]);
    }
    for (int i = 0; MSG_SUBSCRIPTIONS[i]; i++) {
      msg_subscribe(MSG_SUBSCRIPTIONS[i]);
    }
  }
  // Send our status every M seconds
  if (lastack[0] && ((signed)(lasttick - lastacksend) >= 0)) { // ipv. lasttick >= lastacksend ivm overflow effecten
    lastacksend = lasttick + MSG_ACK_INTERVAL;
    msg_send("ack", lastack);
  }
  if (strcmp(state, "nowifi")) {
    if (!gotssid) {
        lastscan = lasttick + 10 * 1000;
    }
    if (!strcmp(state, "nosubs")) {
      Serial.println("We have subscribers, we are live!");
      leds_set("idle");
    }
  }
  server_check();
}
