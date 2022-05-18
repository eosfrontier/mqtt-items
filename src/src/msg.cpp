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

#define LOCAL_IP_STRING(client) ((client)->localIP().toString().c_str())
#define REMOTE_IP_STRING(client) ((client)->remoteIP().toString().c_str())

static void handle_data(void *arg, AsyncClient *client, void *data, size_t len);

// Strings vergelijken met een beperkte wildcard optie
// * matcht alles tot aan de volgende slash (werkt dus alleen met */)
static bool strmatch(const char *patt, const char *match, bool partial = false)
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
  return (partial || (*p == *m));
}

#ifdef MQTT_SERVER
static const int MAX_QUEUE = 64;
#else
static const int MAX_QUEUE = 32;
#endif
static const int MAX_LINE_SIZE = 244;
static const size_t MAX_QUEUE_AGE = 5000;

static struct {
    AsyncClient *client;
    char buffer[256];
    size_t bufidx;
    bool overflow; // Buffer was full without newline.  Discard data until newline found
    bool connected;
    bool tosend; // Client has data to send
} clients[MAX_CLIENTS];

// Linked list that's always a complete circular chain
static struct {
    int clientidx;
    unsigned long tick;
    char line[MAX_LINE_SIZE];
    uint8_t next;
} client_queue[MAX_QUEUE];

static uint8_t queueroot;

#if 0
#ifdef MQTT_SERVER
#define DEBUG_QUEUE
#endif
#ifdef MQTT_SONOFF
#define DEBUG_QUEUE
#endif
#endif

#ifdef DEBUG_QUEUE
static bool queuechanged = false;
#endif

static void queue_check()
{
    uint8_t q_end = queueroot;
    if (!client_queue[queueroot].line[0]) { return; }
#ifdef DEBUG_QUEUE
    if (queuechanged) {
        uint8_t qidx = queueroot;
        Serial.print("Queue: ");
        while (client_queue[qidx].next != queueroot) {
            Serial.print(qidx); Serial.print(":"); Serial.print(client_queue[qidx].line);
            qidx = client_queue[qidx].next;
        }
        Serial.println("");
        queuechanged = false;
    }
#endif
    while (client_queue[q_end].next != queueroot) {
        // Find end of queue (or previous of root)
        q_end = client_queue[q_end].next;
    }
    uint8_t idx = queueroot;
    uint8_t pidx = q_end;  // Needed for relinking

    while (client_queue[idx].line[0]) {
        // Try to send item on queue
        int clientidx = client_queue[idx].clientidx;
        AsyncClient *client = clients[clientidx].client;
        bool done = false;
        unsigned long age = (lasttick - client_queue[idx].tick);
        if (!client || !client->connected() || (age > MAX_QUEUE_AGE)) {
            if (age > MAX_QUEUE_AGE) {
                debugE("Queue timeout (%ld - %ld = %ld) on %s wanting to send %s", lasttick, client_queue[idx].tick, age, REMOTE_IP_STRING(client), client_queue[idx].line);
                // Stop so it disconnects when too old
                if (client && client->connected()) { client->close(true); }
            } else {
                debugE("Connection lost wanting to send %s", client_queue[idx].line);
            }
            // Drop if not connected or too old
            client_queue[idx].line[0] = 0;
            done = true;
        } else if (client->canSend()) {
            char *line = client_queue[idx].line;
            size_t n = strlen(line);
            size_t spc = client->space();
            if (n <= spc) {
                debugV("Queue send: %s", line);
                client->add(line, n);
                clients[clientidx].tosend = true;
                done = true;
            } else {
                debugV("Queue send partial %d: %s", spc, client_queue[idx].line);
                if (!clients[clientidx].tosend) {
                    // Only send partial if we haven't sent a full line yet
                    client->add(line, spc);
                    memmove(line, line+spc, n - spc + 1);
                    clients[clientidx].tosend = true;
                }
            }
        }
        if (done) {
            // Empty entry
            client_queue[idx].line[0] = 0;
            if (idx != queueroot) {
                uint8_t next = client_queue[idx].next;
                // Swap around next indices to put the current index at queueroot
                client_queue[pidx].next = next;
                client_queue[q_end].next = idx;
                q_end = idx;
                client_queue[q_end].next = queueroot;
                idx = next;
            } else {
                // When at the beginning, just skip to next
                pidx = idx;
                idx = client_queue[idx].next;
                q_end = pidx;
                queueroot = idx;
            }
        } else {
            if (idx == q_end) { break; } // Prevent endless loop
            pidx = idx;
            idx = client_queue[idx].next;
        }
    }
    // Only send at the end so it's one packet
    for (int clientidx = 0; clientidx < MAX_CLIENTS; clientidx++) {
        if (clients[clientidx].tosend) {
            clients[clientidx].client->send();
            clients[clientidx].tosend = false;
#ifdef DEBUG_QUEUE
            queuechanged = true;
#endif
        }
    }
}

static void queue_setup()
{
    for (uint8_t idx = 0; idx < MAX_QUEUE; idx++) {
        client_queue[idx].line[0] = 0;
        client_queue[idx].next = (idx+1) % MAX_QUEUE;
    }
    queueroot = 0;
}

static bool queue_add(int clientidx, const char *str)
{
    uint8_t idx = queueroot;
    while (client_queue[idx].line[0]) {
        idx = client_queue[idx].next;
        if (idx == queueroot) {
            return false;
        }
    }
    debugV("Queued: %s", str);
    client_queue[idx].clientidx = clientidx;
    strcpy(client_queue[idx].line, str);
    client_queue[idx].tick = lasttick;
#ifdef DEBUG_QUEUE
    queuechanged = true;
#endif
    return true;
}

static void cprintf(int clientidx, const char *fmt, ...)
{
    if (clientidx >= 0 && clients[clientidx].connected && clients[clientidx].client) {
        AsyncClient *client = clients[clientidx].client;
        va_list args;
        va_start(args, fmt);
        char s[MAX_LINE_SIZE];

#if 0
        size_t n = vsnprintf(s, sizeof(s), fmt, args);
        if (!client->canSend()) {
            if (client->connected()) {
                if (!queue_add(client, s)) {
                    debugE("ERROR Queue overflow sending %d bytes", n);
                    client->close(true);
                }
            }
        } else {
            if (client->space() < n) {
                debugE("ERROR Client can't accept %d bytes", n);
                client->close(true);
                return;
            }
            client->add(s, n);
            client->send();
        }
#else // always queue
        if (client->connected()) {
            vsnprintf(s, sizeof(s), fmt, args);
            if (!queue_add(clientidx, s)) {
                debugE("ERROR Queue overflow sending message to client %d", clientidx);
                client->close(true);
            }
        }
#endif
        va_end(args);
    }
}

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

static void send_topic(int clientidx, const char *topic)
{
    for (int i = 0; i < MAX_TOPICS; i++) {
        if (strmatch(topic, topic_cache[i].topic)) {
            cprintf(clientidx, "%s %s\r\n", topic_cache[i].topic, topic_cache[i].msg);
        }
    }
}

static void new_client(void *arg, AsyncClient *client)
{
    // debugI("New client connection");
    for (int idx = 0; idx < MAX_CLIENTS; idx++) {
        if (!clients[idx].client) {
            debugI("Put client at index %d (local %s)->(remote %s)", idx, LOCAL_IP_STRING(client), REMOTE_IP_STRING(client));
            client->setNoDelay(true);
            client->onData(&handle_data, (void *)idx);
            //client->onDisconnect(&handle_disconnect, (void *)idx);
            clients[idx].client = client;
            clients[idx].bufidx = 0;
            clients[idx].overflow = false;
            clients[idx].connected = true;
            clients[idx].tosend = false;
            cprintf(idx, "HELLO\r\n");
            return;
        }
    }
    debugE("ERROR all connections full (local %s remote %s)", 
        LOCAL_IP_STRING(client), REMOTE_IP_STRING(client));
    // No free space
    client->write("ERROR All connections full, disconnecting\r\n");
    client->send();
    client->stop();
}

static void server_send_message(const char *topic, const char *msg)
{
    debugD("Sending message %s %s", topic, msg);
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
        debugD("Adding message %s %s to cache at index %d with age %ld", topic, msg, cidx, lasttick);
        strcpy(topic_cache[cidx].topic, topic);
        strcpy(topic_cache[cidx].msg, msg);
        topic_cache[cidx].lastseen = lasttick;
    }
    for (int idx = 0; idx < MAX_SUBSCRIBERS; idx++) {
        if (subscribers[idx].clientidx >= 0) {
            //debugV("Matching subscriber %d client %d topic '%s' to '%s'",
            //    idx, subscribers[idx].clientidx, subscribers[idx].topic, topic);
            if (strmatch(subscribers[idx].topic, topic)) {
                if (clients[subscribers[idx].clientidx].connected) {
                    cprintf(subscribers[idx].clientidx, "%s %s\r\n", topic, msg);
                }
            }
        }
    }
}

static void handle_message(int clientidx, const char *topic, const char *msg)
{
    if (!strcmp(topic, "SUB")) {
        debugD("Subscribing client %d to topic %s", clientidx, msg);
        if (strlen(msg) >= (sizeof(subscribers[0].topic)-1)) {
            cprintf(clientidx, "ERROR Subscribe topic too long\r\n");
            return;
        }
        for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
            if (subscribers[si].clientidx < 0) {
                subscribers[si].clientidx = clientidx;
                strcpy(subscribers[si].topic, msg);
                debugD("Subscribed %d to %s at index %d", clientidx, msg, si);
                send_topic(clientidx, msg);
                return;
            }
        }
        debugE("ERROR No room in subscriber list");
        cprintf(clientidx, "ERROR No room for subscriptions\r\n");
    } else if (!strcmp(topic, "SENDSSID")) {
        // For debugging ssid sharing code
        debugI("Sending SSID message to clients");
        IPAddress softapip = WiFi.softAPIP();
        if (!strcmp(msg, "local")) {
            const char *ssid = WiFi.softAPSSID().c_str();
            const char *psk = WiFi.softAPPSK().c_str();
            for (int idx = 0; idx < MAX_CLIENTS; idx++) {
                if (clients[idx].connected) {
                    if (clients[idx].client->localIP() != softapip) {
                        debugI("Sending local SSID to client at idx %d (local %s / remote %s)",
                            idx, LOCAL_IP_STRING(clients[idx].client),
                            REMOTE_IP_STRING(clients[idx].client));
                        cprintf(idx, "SSID %s %s\r\n", ssid, psk);
                    }
                }
            }
        } else {
            for (int idx = 0; idx < MAX_CLIENTS; idx++) {
                if (clients[idx].connected) {
                    if (clients[idx].client->localIP() == softapip) {
                        debugI("Sending SSID to client at idx %d (local %s / remote %s)",
                            idx, LOCAL_IP_STRING(clients[idx].client),
                            REMOTE_IP_STRING(clients[idx].client));
                        if (!strcmp(msg, "remote")) {
                            cprintf(idx, "SSID %s %s\r\n", WiFi.SSID().c_str(), WiFi.psk().c_str());
                        } else {
                            cprintf(idx, "SSID %s\r\n", msg);
                        }
                    }
                }
            }
        }
    } else {
        debugD("Handling message from client %d: %s %s", clientidx, topic, msg);
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
                debugE("Connection lost on %d", idx);
                for (int si = 0; si < MAX_SUBSCRIBERS; si++) {
                    if (subscribers[si].clientidx == idx) {
                        debugI("Unsubscribing %d from %d = %s", idx, si, subscribers[si].topic);
                        subscribers[si].clientidx = -1;
                    }
                }
                clients[idx].connected = false;
                clients[idx].client = NULL;
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
        debugI("We have %d subscribers, we are live!", numsubs);
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
    debugI("Setting up mdns responder on %s", OTA_NAME);
    if (!mdns.begin(OTA_NAME)) {
        debugE("Error setting up mdns responder");
    }
    debugI("Adding mdns service %s %s %d", "eos-portal", "tcp", mqtt_port);
    if (!mdns.addService("eos-portal", "tcp", mqtt_port)) {
        debugE("Error adding mdns service");
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
    // debugV("Client %d, received %d bytes: '%.*s'", idx, len, len, ptr);
    while (len > 0) {
        char *nl = (char *)memchr(ptr, '\n', len);
        if (!nl) {
            if (clients[idx].overflow) { return; }
            // Store in buffer
            if (len > (bufend-bufidx)) {
                // Overflow
                debugE("Client %d line overflow, dropping %d bytes", idx, len);
                clients[idx].overflow = true;
                return;
            }
            memcpy(buffer + bufidx, ptr, len);
            clients[idx].bufidx = bufidx + len;
            // debugV("Stored in buffer, total %d: '%.*s'", clients[idx].bufidx, clients[idx].bufidx, buffer);
            return;
        }
        if (!clients[idx].overflow) {
            size_t linelen = nl-ptr+1;
            // debugV("Found line of length %d: '%.*s'", linelen, linelen, ptr);
            if (linelen < (bufend-bufidx)) {
                memcpy(buffer+bufidx, ptr, linelen);
                char *bnl = buffer+bufidx+linelen-1;
                while (isSpace(*bnl)) *bnl-- = 0;
                char *topic = buffer;
                while (isSpace(*topic)) topic++;
                char *msg = strchr(topic, ' ');
                // debugV("Trimmed, space at %d: '%s'", (msg-topic), topic);
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
        // debugV("Extra %d bytes: '%.*s'", len, len, ptr);
        while ((len > 0) && isSpace(*(char *)ptr)) {
            ptr++;
            len--;
        }
        if (len > 0) {
            // debugV("Extra %d bytes after trim: '%.*s'", len, len, ptr);
        }
    }
}

unsigned long lastacksend = 0;
char lastack[33];

#ifdef MQTT_SERVER
static void ap_setup()
{
    WiFi.mode(WIFI_AP_STA);
    File wifitxt = LittleFS.open("/wifiA.txt", "r");
    String wssid = wifitxt.readStringUntil('\n');
    String wpwd = wifitxt.readStringUntil('\n');
    wifitxt.close();
    WiFi.softAP(wssid, wpwd);
}
#endif

unsigned long lastscan = 0;
char wifiidx = 0;
char gotssid = 0;
char gotrssi = 0;
#ifdef MQTT_SERVER
static const char wifiskip = 1;
#else
static const char wifiskip = 0;
#endif

void msg_setup()
{
    LittleFS.begin();
    char wififn[11] = "/wifiA.txt";
    wifiidx = 0;
    while (LittleFS.exists(wififn)) {
        wifiidx++;
        wififn[5] = 'A' + wifiidx;
    }
    queue_setup();
#ifndef MQTT_SERVER
    WiFi.mode(WIFI_STA);
    clients[0].client = new AsyncClient;
    clients[0].client->onData(&handle_data, (void *)0);
    //client->onDisconnect(&handle_disconnect, (void *)0);
#else
    ap_setup();
    server_setup();
#endif
    /*
       int nap = WiFi.scanNetworks(false, true);
       if (nap == 0) {
       debugE("WiFi scan found zero networks!");
       }
       for (int i = 0; i < nap; i++) {
       Serial.print("Found network "); Serial.print(WiFi.SSID(i)); Serial.print(" on Channel "); Serial.print(WiFi.channel(i)); Serial.print(" ("); Serial.print(WiFi.RSSI(i)); Serial.println(")");
       }
       WiFi.scanDelete();
     */
}

void msg_send(const char *topic, const char *msg)
{
    debugD("Sending %s %s", topic, msg);
#ifdef MQTT_SERVER
    char fulltopic[128];
    strcpy(fulltopic, MSG_NAME "/");
    strcpy(fulltopic + strlen(MSG_NAME) + 1, topic);
    server_send_message(fulltopic, msg);
#else // MQTT_SERVER
    cprintf(0, "%s/%s %s\r\n", MSG_NAME, topic, msg);
#endif // MQTT_SERVER
}

static void add_ssid(const char *msg)
{
    char *pwd = strchr(msg, ' ');
    if (!pwd || (!pwd[1])) {
        debugE("Ignoring ssid without space: '%s'", msg);
        return;
    }
    // Hardcoded index 4, maybe fix later
    File wifitxt = LittleFS.open("/wifiD.txt", "w");
    wifitxt.write(msg, pwd-msg);
    wifitxt.write('\n');
    wifitxt.write(pwd+1, strlen(pwd)-1);
    wifitxt.write('\n');
    wifitxt.close();
    debugI("Got SSID");
#if 0 // def MQTT_SERVER
    send_ssid();
#endif
    wifiidx = 4;
    if (WiFi.status() == WL_CONNECTED) {
      debugI("Forcing disconnect");
      WiFi.disconnect();
      leds_set("nowifi");
      lastscan = lasttick + 100;
    }
}

void msg_receive(const char *topic, const char *msg)
{
  debugD("receive <%s> = <%s>", topic, msg);
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
    debugI("gpio_set(%s, %s)", topic+strlen(MSG_NAME "/gpio/"), msg);
    gpio_set(topic+strlen(MSG_NAME "/gpio/"), msg);
    // return;  // Don't return: also allow chaining via mappings
  }
#endif
#ifdef MQTT_WEBSOCKETS
  if (!strcmp(topic, WS_BROADCAST_ACK)) {
    debugI("ws_send_ack(%s)", msg);
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
              // lastscan = lasttick + 60 * 1000; // Na de hele lijst 60 seconden wachten
          }
          wifiidx--;
          wififn[5] = 'A' + wifiidx;
          debugI("Trying wifi creds from %s", wififn);
          File wifitxt = LittleFS.open(wififn, "r");
          String wssid = wifitxt.readStringUntil('\n');
          String wpwd = wifitxt.readStringUntil('\n');
          wifitxt.close();
          if (wifiidx >= wifiskip) {
              // skip = wifiA but only on server because that's our own softap
              Serial.print("Wifi trying to connect to '"); Serial.print(wssid); Serial.print("' pwd '"); Serial.print(wpwd); Serial.println("'...");
              WiFi.begin(wssid, wpwd);
              gotrssi = 0;
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
        debugI("Querying mdns for %s - %s", "eos-portal", "tcp");
        int n = MDNS.queryService("eos-portal", "tcp");
        debugD("Found %d services", n);
        if (n) {
            debugI("Trying to connect to %s at %s:%d", MDNS.answerHostname(0), MDNS.answerIP(0).toString().c_str(), MDNS.answerPort(0));
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
            leds_set("nosubs");
            connect_client();
        }
    } else if (!clients[0].connected) {
        clients[0].connected = true;
        debugI("Connected, subscribing");
        for (int i = 0; MSG_SUBSCRIPTIONS[i]; i++) {
            debugD("Send SUB %s", MSG_SUBSCRIPTIONS[i]);
            cprintf(0, "SUB %s\r\n", MSG_SUBSCRIPTIONS[i]);
        }
#ifdef MQTT_GPIO
        debugD("Send SUB %s/gpio/*", MSG_NAME);
        cprintf(0, "SUB %s/gpio/*\r\n", MSG_NAME);
#endif
        debugD("Send SUB %s/set", MSG_NAME);
        cprintf(0, "SUB %s/set\r\n", MSG_NAME);
        if (!strcmp(state, "nosubs")) {
            leds_set("idle");
            debugI("We are connected, we are live!");
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
    queue_check();
    server_check();
}
