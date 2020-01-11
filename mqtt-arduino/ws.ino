// Async-ish websockets
#ifdef MQTT_LIGHTS

#include <ESPAsyncTCP.h>

#define WS_FIN            0x80
#define WS_OPCODE_TEXT    0x01
#define WS_OPCODE_BINARY  0x02

#define WS_MASK           0x80
#define WS_SIZE16         126

#define WS_HOST "192.168.1.49"
#define WS_PATH "/socket.io/?transport=websocket"
#define WS_PORT 5001

AsyncClient wsclient;

enum WSState { noconn, errwait, handshake, handshaking, connected } wsstate;
int ws_timeout = 0;

unsigned long ws_ping_time = 0;

int ws_shakes; // Check all handshake requirements
#define WSH_STATUS    (1 << 0)
#define WSH_UPGRADE   (1 << 1)
#define WSH_WEBSOCKET (1 << 2)
#define WSH_KEY       (1 << 3)
#define WSH_COMPLETE  (WSH_STATUS|WSH_UPGRADE|WSH_WEBSOCKET|WSH_KEY)

bool justgotack = false;

String generateKey() {
  String key = "";
  for (int i = 0; i < 23; ++i) {
    int r = random(0, 3);
    if (r == 0)
      key += (char) random(48, 58);
    else if (r == 1)
      key += (char) random(65, 91);
    else if (r == 2)
      key += (char) random(97, 128);
  }
  return key;
}

static void ws_handleConnect(void *arg, void *data)
{
  Serial.println("WS Connected to client");
}

void ws_setup()
{
    wsstate = noconn;
    // wsclient.setInsecure();
    // wsclient.setBufferSizes(512, 512);
    wsclient.onConnect(&ws_handleConnect, NULL);
    wsclient.onData(&ws_handleData, NULL);
}

void ws_send(const char *msg)
{
  if (!wsclient.connected()) {
    return;
  }
  int size = strlen(msg);
  uint8_t buf[size+8];
  Serial.print("WS Trying to send("); Serial.print(size); Serial.print("): "); Serial.println(msg);
  uint8_t *bufptr = buf;
  *bufptr++ = WS_FIN | WS_OPCODE_TEXT;
  if (size > 125) {
    *bufptr++ = WS_MASK | WS_SIZE16;
    *bufptr++ = size >> 8;
    *bufptr++ = size & 0xff;
  } else {
    *bufptr++ = WS_MASK | (uint8_t)size;
  }
  uint8_t *mask = bufptr;
  *bufptr++ = random(0,256);
  *bufptr++ = random(0,256);
  *bufptr++ = random(0,256);
  *bufptr++ = random(0,256);

  for (int i = 0; i < size; i++) {
    *bufptr++ = msg[i] ^ mask[i%4];
  }
  Serial.print("WS Writing "); Serial.print(bufptr-buf); Serial.println(" bytes");
  wsclient.write((const char *)buf, (bufptr-buf));
}

static inline bool startswith(const char *str, const char *prefix)
{
  return (!strncmp(str, prefix, strlen(prefix)));
}

void ws_receive_broadcast(const char *bc)
{
  for (int i = 0; WS_BROADCAST_RECEIVE[i]; i += 3) {
    if (!strcmp(bc, WS_BROADCAST_RECEIVE[i])) {
      justgotack = true;
      msg_receive(WS_BROADCAST_RECEIVE[i+1], WS_BROADCAST_RECEIVE[i+2]);
      justgotack = false;
      return;
    }
  }
}

void ws_receive(char *msg)
{
    Serial.print("Received WS message: <<<");
    Serial.print(msg);
    Serial.println(">>>");
    if (msg[0] == '0') {
      Serial.print("TODO: Do something with connection info: ");
      Serial.println(msg+1);
    }
    if (msg[0] == '2') {
      ws_send("3");
    } else if (msg[0] == '4') {
      if (msg[1] == '0') {
        ws_ping_time = lasttick;
        Serial.println("WS Connected");
      } else if (msg[1] == '2') {
        if (startswith(msg+2, "[\"broadcastReceive\"")) {
          char *bcfile = strstr(msg+24, "\"file\":\"");
          if (bcfile) {
            bcfile += 8;
            char *bcfend = strchr(bcfile, '"');
            if (bcfend) {
              *bcfend = 0;
              ws_receive_broadcast(bcfile);
            }
          }
        } else {
          Serial.print("TODO: Do something with <<<");
          Serial.print(msg+2);
          Serial.println(">>>");
        }
      }
    }
}

static void hexdump(Stream& s, const char *data, size_t len)
{
  size_t i = 0;
  for (; i < len; i++) {
    s.printf(" %02X", (unsigned char)data[i]);
    if ((i%8) == 7) {
      s.print("        ");
      for (size_t j = i-7; j <= i; j++) {
        s.print(data[j]>31?data[j]:'.');
      }
      s.println("");
    }
  }
  for (size_t j = i%8; j < 7; j++) {
    s.print("   ");
  }
  s.print("        ");
  for (size_t j = i-i%8; j < len; j++) {
    s.print(data[j]>31?data[j]:'.');
  }
  s.println("");
}

static void ws_handleData(void *arg, void *client, void *data, size_t len)
{
  const char *msg = (const char *)data;
  // Serial.println("WS received data: ");
  // hexdump(Serial, msg,len);
  if (wsstate == handshaking) {
    const char *eol;
    // Serial.print("WS received handshake: ");
    // Serial.println(msg);
    while ((eol = (char *)memchr(msg, '\n', len))) {
      if (*msg == '\r') {
        if (ws_shakes != WSH_COMPLETE) {
            Serial.print("WS End of handshake, status not ok: ");
            Serial.println(ws_shakes);
            wsclient.stop();
            return;
        }
        Serial.println("WS End of handshake");
        break;
      } else if (!strncmp(msg, "HTTP/", 5)) {
        if ((len > 12) && !strncmp(msg+9, "101", 3)) {
            ws_shakes |= WSH_STATUS;
            Serial.println("WS Status is OK");
        } else {
            Serial.print("WS Wrong status received: ");
            Serial.println(msg);
            wsclient.stop();
            return;
        }
      } else if (!strncmp(msg, "Connection: ", 12)) {
        if ((len>18) && !strncmp(msg+13, "pgrade", 6)) {
            ws_shakes |= WSH_UPGRADE;
            Serial.println("WS Upgrade OK");
        }
      } else if (!strncmp(msg, "Sec-WebSocket-Accept:", 21)) {
        ws_shakes |= WSH_KEY;
        Serial.println("WS key OK");
      } else if (!strncmp(msg, "Upgrade: websocket", 18)) {
        ws_shakes |= WSH_WEBSOCKET;
        Serial.println("WS Websocket OK");
      }
      len -= (eol+1-msg);
      msg = eol+1;
    }
    if (ws_shakes == WSH_COMPLETE) {
      Serial.println("WS is connected successfully");
      wsstate = connected;
    }
  }
  if (wsstate == connected) {
    const unsigned char *d = (const unsigned char *)msg;
    const unsigned char *dend = d+len;
    while (d < dend) {
      unsigned int msgtype = *d++;
      int length = *d++;
      bool masked = false;
      if (length & WS_MASK) {
          length &= ~WS_MASK;
          masked = true;
      }
      if (length == WS_SIZE16) {
          length = (*d++ << 8) | *d++;
      }
      const uint8_t *mask = (uint8_t *)"\0\0\0\0";
      if (masked) {
          mask = d;
          d += 4;
      }
      char sio_msg[length+1];
      for (int i = 0; i < length; i++) {
          sio_msg[i] = *d++ ^ mask[i%4];
      }
      sio_msg[length] = 0;
      ws_receive(sio_msg);
    }
  }
}

void ws_check()
{
    if (WiFi.status() != WL_CONNECTED) {
        wsclient.stop();
        wsstate = noconn;
        return;
    }
    if (wsstate == errwait) {
        if (ws_timeout-- > 0) return;
        wsstate = noconn;
    }
    if (wsstate == noconn) {
        Serial.println("WS Connecting");
        if (!wsclient.connect(WS_HOST, WS_PORT, true)) {
            Serial.println("WS Connection error");
            wsstate = errwait;
            ws_timeout = 100;
            return;
        }
        ws_timeout = 100;
        wsstate = handshake;
        return;
    }
    if (!wsclient.connected()) {
        if (ws_timeout-- > 0) return;
        wsstate = errwait;
        ws_timeout = 20;
        return;
    }
    if (wsstate == handshake) {
        String hs = "GET " WS_PATH " HTTP/1.1\r\n"
            "Host: " WS_HOST "\r\n"
            "Connection: Upgrade\r\n"
            "Upgrade: websocket\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "Sec-WebSocket-Key: " + generateKey() + "=\r\n\r\n";
        Serial.println("WS Send handshake");
        Serial.println(hs);
        wsclient.add(hs.c_str(), hs.length());
        wsclient.send();
        Serial.println("WS Handshake sent");
        ws_shakes = 0;
        wsstate = handshaking;
        return;
    }
    if (wsstate == handshaking) {
      /*
        while (wsclient.available()) {
            String s = wsclient.readStringUntil('\n');
            Serial.print("WS received: ");
            Serial.println(s);
            const char *ss = s.c_str();
            if (s == "\r") {
                if (ws_shakes != WSH_COMPLETE) {
                    Serial.print("WS End of handshake, status not ok: ");
                    Serial.println(ws_shakes);
                    wsclient.stop();
                    return;
                }
                Serial.println("WS End of handshake");
                break;
            } else if (s.indexOf("HTTP/") != -1) {
                if (!memcmp(ss+9, "101", 3)) {
                    ws_shakes |= WSH_STATUS;
                    Serial.println("WS Status is OK");
                } else {
                    Serial.print("WS Wrong status received: ");
                    Serial.println(s);
                    wsclient.stop();
                    return;
                }
            } else if (!strncmp(ss, "Connection: ", 12)) {
                if (!strncmp(ss+13, "pgrade", 6)) {
                    ws_shakes |= WSH_UPGRADE;
                    Serial.println("WS Upgrade OK");
                }
            } else if (!strncmp(ss, "Sec-WebSocket-Accept:", 21)) {
                ws_shakes |= WSH_KEY;
                Serial.println("WS key OK");
            } else if (!strncmp(ss, "Upgrade: websocket", 18)) {
                ws_shakes |= WSH_WEBSOCKET;
                Serial.println("WS Websocket OK");
            }
        }
        if (ws_shakes == WSH_COMPLETE) {
            Serial.println("WS is connected successfully");
            wsstate = connected;
        }
        return;
        */
    }
    if (wsstate == connected) {
        /*
        if (wsclient.available()) {
            unsigned int msgtype = wsclient.read();
            int length = wsclient.read();
            bool masked = false;
            if (length & WS_MASK) {
                length &= ~WS_MASK;
                masked = true;
            }
            if (length == WS_SIZE16) {
                length = wsclient.read() << 8 | wsclient.read();
            }
            uint8_t mask[4] = {0,0,0,0};
            if (masked) {
                mask[0] = wsclient.read();
                mask[1] = wsclient.read();
                mask[2] = wsclient.read();
                mask[3] = wsclient.read();
            }
            char msg[length+1];
            for (int i = 0; i < length; i++) {
                msg[i] = wsclient.read() ^ mask[i%4];
            }
            ws_receive(msg);
        }
        */
        if ((lasttick - ws_ping_time) > 25000) {
            ws_send("2ping");
            ws_ping_time = lasttick;
        }
    }
}

void ws_send_ack(const char *ack)
{
  if (!justgotack) {
    for (int i = 0; WS_BROADCAST_SEND[i]; i += 2) {
      if (!strcmp(WS_BROADCAST_SEND[i], ack)) {
        ws_send(WS_BROADCAST_SEND[i+1]);
        return;
      }
    }
  }
}
#else
void ws_send_ack(const char *ack)
{
  // NOTHING 
}
void ws_setup() {}
void ws_check() {}
#endif
