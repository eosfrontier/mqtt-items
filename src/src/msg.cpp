#include "settings.h"
#include "msg.h"
#include "main.h"
#include "leds.h"
#include "buttons.h"
#include "gpio.h"
#include "ws.h"
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>

static void handle_data(void *arg, AsyncClient *client, void *data, size_t len);

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

static void cprintf(AsyncClient *client, const char *fmt, ...)
{
    if (client && client->canSend()) {
        va_list args;
        va_start(args, fmt);
        char s[256];

        size_t n = vsnprintf(s, sizeof(s), fmt, args);
        if (client->space() >= n) {
            client->add(s, n);
            client->send();
        } else {
            client->stop();
        }
        va_end(args);
    }
}

static struct {
    AsyncClient *client;
    char buffer[256];
    size_t bufidx;
    bool overflow; // Buffer was full without newline.  Discard data until newline found
    bool connected;
} clients[MAX_CLIENTS];

#ifdef MQTT_SERVER

static AsyncServer *server;

const int MAX_TOPICS = 16;

static struct {
    int16_t clientidx;
    char topic[62];
} subscribers[MAX_SUBSCRIBERS];

static struct {
    unsigned long lastseen;
    char topic[62];
    char msg[62];
} topic_cache[MAX_TOPICS];

static void send_topic(AsyncClient *client, const char *topic)
{
    for (int i = 0; i < MAX_TOPICS; i++) {
        if (strmatch(topic, topic_cache[i].topic)) {
            cprintf(client, "%s %s\r\n", topic_cache[i].topic, topic_cache[i].msg);
        }
    }
}

static void new_client(void *arg, AsyncClient *client)
{
    serprintf("New client connection");
    for (int idx = 0; idx < MAX_CLIENTS; idx++) {
        if (!clients[idx].client) {
            serprintf("Put client at index %d", idx);
            client->setNoDelay(true);
            client->onData(&handle_data, (void *)idx);
            //client->onDisconnect(&handle_disconnect, (void *)idx);
            clients[idx].client = client;
            clients[idx].bufidx = 0;
            clients[idx].overflow = false;
            clients[idx].connected = true;
            cprintf(client, "HELLO\r\n");
            return;
        }
    }
    serprintf("ERROR all connections full");
    // No free space
    cprintf(client, "ERROR All connections full, disconnecting\r\n");
    client->stop();
}

static void server_send_message(const char *topic, const char *msg)
{
    serprintf("Sending message %s %s", topic, msg);
    if ((strlen(topic) < 62) && (strlen(msg) < 62)) {
        unsigned long oldest = 0;
        int cidx = -1;
        for (int ti = 0; ti < MAX_TOPICS; ti++) {
            if (!strcmp(topic_cache[ti].topic, topic)) {
                // Same topic, overwrite
                cidx = ti;
                break;
            }
            unsigned long age = lasttick - topic_cache[ti].lastseen;
            if (age > oldest) {
                // Find topic that is oldest
                oldest = age;
                cidx = ti;
            }
        }
        serprintf("Adding message %s %s to cache at index %d with age %ld", topic, msg, cidx, lasttick);
        strcpy(topic_cache[cidx].topic, topic);
        strcpy(topic_cache[cidx].msg, msg);
        topic_cache[cidx].lastseen = lasttick;
    }
    for (int idx = 0; idx < MAX_SUBSCRIBERS; idx++) {
        if (subscribers[idx].clientidx >= 0) {
            //serprintf("Matching subscriber %d client %d topic '%s' to '%s'",
            //    idx, subscribers[idx].clientidx, subscribers[idx].topic, topic);
            if (strmatch(subscribers[idx].topic, topic)) {
                if (clients[subscribers[idx].clientidx].connected) {
                    cprintf(clients[subscribers[idx].clientidx].client, "%s %s\r\n", topic, msg);
                }
            }
        }
    }
}

static void handle_message(int clientidx, const char *topic, const char *msg)
{
    if (!strcmp(topic, "SUB")) {
        serprintf("Subscribing client %d to topic %s", clientidx, msg);
        if (strlen(msg) >= (sizeof(subscribers[0].topic)-1)) {
            cprintf(clients[clientidx].client, "ERROR Subscribe topic too long\r\n");
            return;
        }
        for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
            if (subscribers[si].clientidx < 0) {
                subscribers[si].clientidx = clientidx;
                strcpy(subscribers[si].topic, msg);
                serprintf("Subscribed %d to %s at index %d", clientidx, msg, si);
                send_topic(clients[clientidx].client, msg);
                return;
            }
        }
        serprintf("ERROR No room in subscriber list");
        cprintf(clients[clientidx].client, "ERROR No room for subscriptions\r\n");
    } else {
        serprintf("Handling message from client %d: %s %s", clientidx, topic, msg);
        server_send_message(topic, msg);
        msg_receive(topic, msg);
    }
}

static MDNSResponder mdns;

static void server_check()
{
    // Handle existing clients
    for (int idx = 0; idx < MAX_CLIENTS; idx++) {
        if (clients[idx].connected) {
            if (!clients[idx].client->connected()) {
                serprintf("Connection lost on %d", idx);
                for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
                    if (subscribers[si].clientidx == idx) {
                        serprintf("Unsubscribing %d from %d = %s", idx, si, subscribers[si].topic);
                        subscribers[si].clientidx = -1;
                    }
                }
                clients[idx].client->stop();
                clients[idx].connected = false;
                clients[idx].client = nullptr;
            }
        }
    }
    int numsubs = 0;
    for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
        if (subscribers[si].clientidx >= 0) {
            numsubs++;
        }
    }
    if (numsubs == 0) {
        if (WiFi.status() == WL_CONNECTED) {
            leds_set("nosubs");
        }
    } else if (!strcmp(state, "nosubs")) {
        leds_set("idle");
        serprintf("We have %d subscribers, we are live!", numsubs);
    }
    mdns.update();
}

static void server_setup()
{
    for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
        subscribers[si].clientidx = -1;
    }
    for (int ti = 0; ti < MAX_TOPICS; ti++) {
        topic_cache[ti].lastseen = 0;
        topic_cache[ti].topic[0] = 0;
        topic_cache[ti].msg[0] = 0;
    }
    server = new AsyncServer(mqtt_port);
    server->onClient(new_client, (void *)0);
    server->begin();
    serprintf("Setting up mdns responder on %s", OTA_NAME);
    if (!mdns.begin(OTA_NAME)) {
        serprintf("Error setting up mdns responder");
    }
    serprintf("Adding mdns service %s %s %d", "eos-portal", "tcp", mqtt_port);
    if (!mdns.addService("eos-portal", "tcp", mqtt_port)) {
        serprintf("Error adding mdns service");
    }
}

#else // MQTT_SERVER
static void server_check() { }

static void handle_message(int clientidx, const char *topic, const char *msg)
{
    msg_receive(topic, msg);
}

#endif // MQTT_SERVER

static void handle_data(void *arg, AsyncClient *client, void *data, size_t len)
{
    int idx = (int)arg;
    char *buffer = clients[idx].buffer;
    size_t bufidx = clients[idx].bufidx;
    static const size_t bufend = sizeof(clients[idx].buffer);
    char *ptr = (char *)data;
    serprintf("Client %d, received %d bytes: '%.*s'", idx, len, len, ptr);
    while (len > 0) {
        char *nl = (char *)memchr(ptr, '\n', len);
        if (!nl) {
            if (clients[idx].overflow) { return; }
            // Store in buffer
            if (len > (bufend-bufidx)) {
                // Overflow
                clients[idx].overflow = true;
                return;
            }
            memcpy(buffer + bufidx, ptr, len);
            clients[idx].bufidx = bufidx + len;
            serprintf("Stored in buffer, total %d: '%.*s'", clients[idx].bufidx, clients[idx].bufidx, buffer);
            return;
        }
        if (!clients[idx].overflow) {
            size_t linelen = nl-ptr+1;
            serprintf("Found line of length %d: '*.*s'", linelen, ptr, ptr);
            if (linelen < (bufend-bufidx)) {
                memcpy(buffer+bufidx, ptr, linelen);
                char *bnl = buffer+bufidx+linelen-1;
                while (isSpace(*bnl)) *bnl-- = 0;
                char *topic = buffer;
                while (isSpace(*topic)) topic++;
                char *msg = strchr(topic, ' ');
                serprintf("Trimmed, space at %d: '%s'", (msg-topic), topic);
                if (msg) {
                    *msg++ = 0;
                    handle_message(idx, topic, msg);
                    // *(msg-1) = ' ';
                }
            }
        }
        clients[idx].overflow = false;
        clients[idx].bufidx = 0;
        nl = nl+1;
        len = len - (nl-ptr);
        ptr = nl;
        serprintf("Extra %d bytes: '%.*s'", len, len, ptr);
        while ((len > 0) && isSpace(*(char *)ptr)) {
            ptr++;
            len--;
        }
        serprintf("Extra %d bytes after trim: '%.*s'", len, len, ptr);
    }
}

unsigned long lastacksend = 0;
char lastack[33];

static bool msgdebug = false;

void msg_setup()
{
#ifndef MQTT_SERVER
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  clients[0].client = new AsyncClient;
  client->onData(&handle_data, (void *)0);
  //client->onDisconnect(&handle_disconnect, (void *)0);
#else
  server_setup();
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
}

void msg_send(const char *topic, const char *msg)
{
    serprintf("Sending %s %s", topic, msg);
#ifdef MQTT_SERVER
    char fulltopic[128];
    strcpy(fulltopic, MSG_NAME "/");
    strcpy(fulltopic + strlen(MSG_NAME) + 1, topic);
    server_send_message(fulltopic, msg);
#else // MQTT_SERVER
    cprintf(clients[0].client, "%s/%s %s\r\n", MSG_NAME, topic, msg);
#endif // MQTT_SERVER
}

unsigned long lastscan = 0;
char wifiidx = 3;
char gotssid = 0;
char gotrssi = 0;

#ifdef MQTT_SERVER
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
#ifdef MQTT_SERVER
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
  serprintf("receive <%s> = <%s>", topic, msg);
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
  if (!strcmp(topic, MSG_NAME "/setdebug")) {
      if (!strcmp(msg, "on") || !strcmp(msg, "true") || !strcmp(msg, "debug")) {
          msg_send("debug", "start debugging");
          msgdebug = true;
      } else {
          msg_send("debug", "stop debugging");
          msgdebug = false;
      }
  }
#ifdef MQTT_GPIO
  if (strmatch(MSG_NAME "/gpio/*", topic)) {
    serprintf("gpio_set(%s, %s)", topic+strlen(MSG_NAME "/gpio/"), msg);
    gpio_set(topic+strlen(MSG_NAME "/gpio/"), msg);
    // return;  // Don't return: also allow chaining via mappings
  }
#endif
#ifdef MQTT_WEBSOCKETS
  if (!strcmp(topic, WS_BROADCAST_ACK)) {
    serprintf("ws_send_ack(%s)", msg);
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

void msg_debug(char *msg)
{
    if (msgdebug) {
        msg_send("debug", msg);
    }
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
#ifdef MQTT_SERVER
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
      leds_set("nosubs");
  }
}


#ifndef MQTT_SERVER
static unsigned long lastconnect = 0;

#define SERVER_HOST "eos-portal-light"

static void connect_client()
{
    if (lastconnect < lasttick) {
        serprintf("Querying mdns for %s - %s", "eos-portal", "tcp");
        int n = MDNS.queryService("eos-portal", "tcp");
        serprintf("Found %d services", n);
        if (n) {
            serprintf("Trying to connect to %s:%d", MDNS.answerHostname(0), MDNS.answerPort(0));
            clients[0].client->connect(MDNS.answerIP(0), MDNS.answerPort(0));
            clients[0].client->setNoDelay(true);
        }
        lastconnect = lasttick + 30000;
    }
}
#endif

void msg_check()
{
    msg_connect_wifi();
#ifndef MQTT_SERVER
    if (!clients[0].client->connected()) {
        clients[0].connected = false;
        if (WiFi.status() == WL_CONNECTED) {
            connect_client();
            if (!strcmp(state, "nowifi")) {
                leds_set("nosubs");
            }
        }
    } else if (!clients[0].connected) {
        clients[0].connected = true;
        serprintf("Connected, subscribing");
        for (int i = 0; MSG_SUBSCRIPTIONS[i]; i++) {
            serprintf("Send SUB %s", MSG_SUBSCRIPTIONS[i]);
            cprintf(clients[0].client, "SUB %s\r\n", MSG_SUBSCRIPTIONS[i]);
        }
#ifdef MQTT_GPIO
        serprintf("Send SUB %s/gpio/*", MSG_NAME);
        cprintf(clients[0].client, "SUB %s/gpio/*\r\n", MSG_NAME);
#endif
        serprintf("Send SUB %s/set", MSG_NAME);
        cprintf(clients[0].client, "SUB %s/set\r\n", MSG_NAME);
        serprintf("Send SUB %s/setdebug", MSG_NAME);
        cprintf(clients[0].client, "SUB %s/setdebug\r\n", MSG_NAME);
        if (!strcmp(state, "nosubs")) {
            leds_set("idle");
            serprintf("We are connected, we are live!");
        }
    }
#endif
    // Send our status every M seconds
    if (lastack[0] && ((signed)(lasttick - lastacksend) >= 0)) { // ipv. lasttick >= lastacksend ivm overflow effecten
        lastacksend = lasttick + MSG_ACK_INTERVAL;
        msg_send("ack", lastack);
    }
    if (strcmp(state, "nowifi")) {
        if (!gotssid) {
            lastscan = lasttick + 10 * 1000;
        }
    }
    server_check();
}
