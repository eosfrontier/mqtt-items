// Async-ish websockets

#include <WiFiClientSecure.h>

#define WS_FIN            0x80
#define WS_OPCODE_TEXT    0x01
#define WS_OPCODE_BINARY  0x02

#define WS_MASK           0x80
#define WS_SIZE16         126

#define WS_HOST "192.168.1.49"
#define WS_PATH "/socket.io/?transport=websocket"
#define WS_PORT 5001

WiFiClientSecure wsclient;

enum WSState { noconn, errwait, handshake, handshaking, connected } wsstate;
int timeout = 0;

int shakes; // Check all handshake requirements
#define WSH_STATUS    (1 << 0)
#define WSH_UPGRADE   (1 << 1)
#define WSH_WEBSOCKET (1 << 2)
#define WSH_KEY       (1 << 3)
#define WSH_COMPLETE  (WSH_STATUS|WSH_UPGRADE|WSH_WEBSOCKET|WSH_KEY)

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

void ws_setup()
{
    wsstate = noconn;
    wsclient.setInsecure();
    wsclient.setBufferSizes(512, 512);
}

void ws_receive(const char *msg)
{
    Serial.print("Received WS message: <<<");
    Serial.print(msg);
    Serial.println(">>>");
    // TODO
}

void ws_check()
{
    if (WiFi.status() != WL_CONNECTED) {
        wsclient.stop();
        wsstate = noconn;
        return;
    }
    if (wsstate == errwait) {
        if (timeout-- > 0) return;
        wsstate = noconn;
    }
    if (wsstate == noconn) {
        if (!wsclient.connect(WS_HOST, WS_PORT)) {
            wsstate = errwait;
            timeout = 100;
            return;
        }
        wsstate = handshake;
        return;
    }
    if (!wsclient.connected()) {
        wsstate = errwait;
        timeout = 20;
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
        wsclient.write(hs.c_str());
        shakes = 0;
        wsstate = handshaking;
        return;
    }
    if (wsstate == handshaking) {
        while (wsclient.available()) {
            String s = wsclient.readStringUntil('\n');
            Serial.print("WS received: ");
            Serial.println(s);
            const char *ss = s.c_str();
            if (s == "\r") {
                if (shakes != WSH_COMPLETE) {
                    Serial.print("WS End of handshake, status not ok: ");
                    Serial.println(shakes);
                    wsclient.stop();
                    return;
                }
                Serial.println("WS End of handshake");
                break;
            } else if (s.indexOf("HTTP/") != -1) {
                if (!memcmp(ss+9, "101", 3)) {
                    shakes |= WSH_STATUS;
                    Serial.println("WS Status is OK");
                } else {
                    Serial.print("WS Wrong status received: ");
                    Serial.println(s);
                    wsclient.stop();
                    return;
                }
            } else if (!strncmp(ss, "Connection: ", 12)) {
                if (!strncmp(ss+13, "pgrade", 6)) {
                    shakes |= WSH_UPGRADE;
                    Serial.println("WS Upgrade OK");
                }
            } else if (!strncmp(ss, "Sec-WebSocket-Accept:", 21)) {
                shakes |= WSH_KEY;
                Serial.println("WS key OK");
            } else if (!strncmp(ss, "Upgrade: websocket", 18)) {
                shakes |= WSH_WEBSOCKET;
                Serial.println("WS Websocket OK");
            }
        }
        if (shakes == WSH_COMPLETE) {
            Serial.println("WS is connected successfully");
            wsstate = connected;
        }
        return;
    }
    if (wsstate == connected) {
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
    }
}
