#include "settings.h"
#include "msg.h"
#include "main.h"
#include "leds.h"
#include "buttons.h"
#include "gpio.h"
#include "ws.h"
#include <LittleFS.h>
#include <user_interface.h>
#include <espconn.h>
#include <ESP8266mDNS.h>

const char *state = "";

static struct client {
    struct espconn *conn;
    const char *state;
    struct client *next;
    char buf[64 - 12];
} *clients = NULL;

static void msg_set_state(const char *st)
{
    if (!strcmp(st, state)) return;
    Serial.printf("Set state to %s\r\n", st);
    for (int i = 0; LEDS_ANIMATIONS[i]; i += 2) {
        if (!strcmp(st, LEDS_ANIMATIONS[i])) {
            Serial.printf("Set leds to %s (%s)\r\n", st, LEDS_ANIMATIONS[i+1]);
            leds_set(LEDS_ANIMATIONS[i+1]);
            state = LEDS_ANIMATIONS[i];
            return;
        }
    }
    leds_set(st);
}

static void msg_handle(const char *cmd, const char *arg)
{
    if (!strcmp(cmd, "set")) {
        return msg_set_state(arg);
    }
}

static void conn_handle(char *line, unsigned int len)
{
    char *arg = os_strchr(line, ' ');
    if (!arg) return;
    *arg++ = 0;
    while (*arg && isspace(*arg)) arg++;
    while (--len > 0 && isspace(line[len])) line[len] = 0;
    msg_handle(line, arg);
}

static void ICACHE_FLASH_ATTR conn_recv(void *arg, char *data, unsigned short len)
{
    struct espconn *conn = (struct espconn *)arg;
    struct client *client = (struct client *)conn->reverse;

    //Serial.printf("recv('%*s', %d)  client %p  buf = '%s'\r\n", len, data, len, client, client->buf);
    int bln = os_strlen(client->buf);
    while (len > 0) {
        char *nl = (char *)memchr(data, '\n', len);
        //Serial.printf("memchr(data, '\\n', %d) = %p\r\n", len, nl);
        if (nl) {
            if (bln) {
                size_t sz = nl-data;
                if (sz > (sizeof(client->buf)-bln-1)) sz = (sizeof(client->buf)-bln-1);
                os_memcpy(client->buf+bln, data, sz);
                client->buf[bln+sz] = 0;
                conn_handle(client->buf, bln+sz);
                client->buf[0] = 0;
                bln = 0;
            } else {
                *nl = 0;
                conn_handle(data, nl-data);
            }
            len -= (nl-data);
            data = nl+1;
        } else {
            size_t sz = len;
            if (sz > (sizeof(client->buf)-bln-1)) sz = sizeof(client->buf)-bln-1;
            os_memcpy(client->buf+bln, data, sz);
            client->buf[bln+sz] = 0;
            //Serial.printf("buf = '%s'\r\n", client->buf);
            len = 0;
        }
    }
}

/*
static void ICACHE_FLASH_ATTR conn_recon(void *arg, sint8 err)
{
    struct espconn *conn = (struct espconn *)arg;
    Serial.printf("conn_recon(%d.%d.%d.%d:%d) %d\r\n",
        conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
        conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
        conn->proto.tcp->remote_port, conn->link_cnt);
    struct client *client = (struct client *)conn->reverse;
    client->state = NULL;
}

static void ICACHE_FLASH_ATTR conn_discon(void *arg)
{
    struct espconn *conn = (struct espconn *)arg;

    Serial.printf("conn_discon(%d.%d.%d.%d:%d) %d\r\n",
        conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
        conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
        conn->proto.tcp->remote_port, conn->link_cnt);

    struct client **client = &clients;
    while (*client && conn->reverse != *client) {
        client = &((*client)->next);
    }
    if (*client) {
        struct client *toremove = *client;
        *client = (*client)->next;
        delete toremove;
        Serial.printf("Removed client (%d.%d.%d.%d:%d)\r\n",
            conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
            conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
            conn->proto.tcp->remote_port);
    } else {
        debugE("Disconnect callback on unknown client (%d.%d.%d.%d:%d)",
            conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
            conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
            conn->proto.tcp->remote_port);
    }
}
*/

/*
static void ICACHE_FLASH_ATTR conn_sent(void *arg)
{
}
*/

static void ICACHE_FLASH_ATTR conn_connected(void *arg)
{
    struct client *client = new struct client;

    struct espconn *conn = (struct espconn *)arg;
    client->conn = conn;
    client->conn->reverse = client;
    //client->lastsent = 0;
    client->buf[0] = 0;
    client->next = clients;
    client->state = NULL;
    clients = client;

    //espconn_set_opt(conn, ESPCONN_REUSEADDR|ESPCONN_NODELAY|ESPCONN_KEEPALIVE);
    espconn_regist_recvcb(conn, conn_recv);
    //espconn_regist_sentcb(conn, conn_sent);
    //espconn_regist_reconcb(conn, conn_recon);
    //espconn_regist_disconcb(conn, conn_discon);
    //espconn_regist_time(conn, 10, 1);

    Serial.printf("Connected client (%d.%d.%d.%d:%d) %d\r\n",
        conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
        conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
        conn->proto.tcp->remote_port, conn->link_cnt);

    Serial.printf("Heap: %d\r\n", system_get_free_heap_size());
    for (client = clients; client; client = client->next) {
        conn = client->conn;
        Serial.printf("- Client: (%d.%d.%d.%d:%d)\r\n",
            conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
            conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
            conn->proto.tcp->remote_port);
        Serial.printf("  Conn state: %d\r\n", conn->state);
        Serial.printf("  Link cnt: %d\r\n", conn->link_cnt);
        Serial.printf("  Client State: '%s'\r\n", client->state ? client->state : "");
        Serial.printf("  Buf: '%s'\r\n", client->buf);
        Serial.printf("  Con: %p\r\n", conn->proto.tcp->connect_callback);
        Serial.printf("  Discon: %p\r\n", conn->proto.tcp->disconnect_callback);
        Serial.printf("  Recon: %p\r\n", conn->proto.tcp->reconnect_callback);
        Serial.printf("  Write: %p\r\n", conn->proto.tcp->write_finish_fn);
    }
}

#ifdef MQTT_SERVER

static void server_setup()
{
    static esp_tcp tcp = {
        .local_port = mqtt_port
    };
    static struct espconn server = {
        .type = ESPCONN_TCP,
        .state = ESPCONN_NONE,
        .proto = { .tcp = &tcp }
    };
    //espconn_set_opt(&server, ESPCONN_REUSEADDR|ESPCONN_NODELAY|ESPCONN_KEEPALIVE);
    espconn_regist_connectcb(&server, conn_connected);
    //espconn_regist_reconcb(&server, conn_recon);
    //espconn_regist_disconcb(&server, conn_discon);
    espconn_tcp_set_max_con_allow(&server, MAX_CLIENTS);
    espconn_accept(&server);
    espconn_regist_time(&server, 0, 0);
    Serial.printf("Server initialized\r\n");
    //Serial.printf("  Con: %p\r\n  Discon: %p\r\n  Recon: %p\r\n  Write: %p\r\n", conn_connected, conn_recon, conn_discon, (void *)NULL);
}

static void ap_setup()
{
    if (wifi_get_opmode() != STATIONAP_MODE) wifi_set_opmode(STATIONAP_MODE);
    File wifitxt = LittleFS.open("/wifiA.txt", "r");
    String wssid = wifitxt.readStringUntil('\n');
    String wpwd = wifitxt.readStringUntil('\n');
    wifitxt.close();
    if (wssid.length() > 31) {
        debugE("wifiA ssid too long!");
        return;
    }
    if (wssid.length() > 63) {
        debugE("wifiA password too long!");
        return;
    }

    struct softap_config wificonfig;
    wifi_softap_get_config_default(&wificonfig);
    if (strcmp((char *)wificonfig.ssid, wssid.c_str()) || strcmp((char *)wificonfig.password, wpwd.c_str())) {
        strcpy((char *)wificonfig.ssid, wssid.c_str());
        strcpy((char *)wificonfig.password, wpwd.c_str());
        wificonfig.ssid_len = wssid.length();
        wificonfig.ssid_hidden = 0;
        wifi_softap_set_config(&wificonfig);
    }
}

static MDNSResponder mdns;

static void mdns_setup()
{
    if (!mdns.begin(OTA_NAME)) {
        Serial.printf("Error setting up mdns responder");
    }
    if (!mdns.addService("eos-portal", "tcp", mqtt_port)) {
        Serial.printf("Error adding mdns service");
    }
}

#else

static unsigned long lastconn;

static void sta_setup()
{
    wifi_set_opmode(STATION_MODE);
    lastconn = 0 - 30000;
}

static struct espconn client_conn;

static void client_setup()
{
    static esp_tcp tcp;
    client_conn.type = ESPCONN_TCP;
    client_conn.state = ESPCONN_NONE;
    client_conn.proto.tcp = &tcp;
    espconn_regist_recvcb(&client_conn, conn_recv);
}

static void client_check()
{
    if ((client_conn.state == ESPCONN_NONE) || (client_conn.state == ESPCONN_CLOSE)) {
        if ((lasttick - lastconn) >= 30000) {
            Serial.printf("Querying mdns for %s - %s\r\n", "eos-portal", "tcp");
            int n = MDNS.queryService("eos-portal", "tcp");
            Serial.printf("Found %d services\r\n", n);
            if (n) {
                Serial.printf("Trying to connect to %s at %s:%d\r\n", MDNS.answerHostname(0), MDNS.answerIP(0).toString().c_str(), MDNS.answerPort(0));
                client_conn.proto.tcp.remote_port = MDNS.answerPort(0);
                os_memcpy(client_conn.proto.tcp.remote_ip, MDNS.answerIP(0).ip);
                espconn_connect(&client_conn);
            }
            lastconn = lasttick + 30000;
        }
    }

}
#endif

static unsigned long lastscan;

static struct wifipwd {
    struct wifipwd *next;
    char ssid[32];
    char password[64];
} *wifipwds = NULL;

// TODO:
// - mark APs as not ok (move to end of list?) on connection failure
// - do something when connected
// - print debug info
// - use cached station api
static void ICACHE_FLASH_ATTR wifi_scan_done(void *arg, STATUS status)
{
    if (status == OK) {
        for (struct bss_info *bss = (struct bss_info *)arg; bss; bss = bss->next.stqe_next) {
            Serial.printf("WiFi-AP %02X:%02X:%02X:%02X:%02X:%02X %s on channel %d with rssi %d\r\n",
                bss->bssid[0], bss->bssid[1], bss->bssid[2],
                bss->bssid[3], bss->bssid[4], bss->bssid[5],
                bss->ssid, bss->channel, bss->rssi);
        }
        // Loop over wp first because that defines prio
        for (struct wifipwd *wp = wifipwds; wp; wp = wp->next) {
            for (struct bss_info *bss = (struct bss_info *)arg; bss; bss = bss->next.stqe_next) {
                if (!strcmp(wp->ssid, (char *)bss->ssid)) {
                    struct station_config sconf;
                    os_memset(&sconf, 0, sizeof(sconf));
                    wifi_station_get_config(&sconf);
                    Serial.printf(
                        "  ssid: '%s'\r\n"
                        "  bssid_set: %d\r\n"
                        "  bssid: '%02X:%02X:%02X:%02X:%02X:%02X'\r\n"
                        "  threshold: (%d,%d)\r\n",
                        sconf.ssid,
                        sconf.bssid_set,
                        sconf.bssid[0], sconf.bssid[1], sconf.bssid[2],
                        sconf.bssid[3], sconf.bssid[4], sconf.bssid[5],
                        sconf.threshold.rssi, sconf.threshold.authmode);
                    if (!strcmp((char *)sconf.ssid, wp->ssid)
                            && !strcmp((char *)sconf.password, wp->password)
                            && wifi_station_get_connect_status()) {
                        Serial.printf("Still connecting to %s\r\n", sconf.ssid);
                        return;
                    }
                    os_memset(&sconf, 0, sizeof(sconf));
                    sconf.threshold.rssi = -90;
                    os_memcpy(sconf.ssid, wp->ssid, 32);
                    os_memcpy(sconf.password, wp->password, 64);
                    Serial.printf(
                        "  ssid: '%s'\r\n"
                        "  bssid_set: %d\r\n"
                        "  bssid: '%02X:%02X:%02X:%02X:%02X:%02X'\r\n"
                        "  threshold: (%d,%d)\r\n",
                        sconf.ssid,
                        sconf.bssid_set,
                        sconf.bssid[0], sconf.bssid[1], sconf.bssid[2],
                        sconf.bssid[3], sconf.bssid[4], sconf.bssid[5],
                        sconf.threshold.rssi, sconf.threshold.authmode);
                    Serial.printf("Connecting to ('%s' : '%s')\r\n", sconf.ssid, sconf.password);
                    wifi_station_set_config_current(&sconf);
                    wifi_station_connect();
                    return;
                }
            }
        }
    }
}

static void wifi_check()
{
    static uint8 last_cs = 255;
    uint8 cs = wifi_station_get_connect_status();
    if (cs != last_cs) {
        Serial.printf("Wifi status changed from %d to %d\r\n", last_cs, cs);
        last_cs = cs;
        struct station_config sc;
        wifi_station_get_config(&sc);
        Serial.printf("Wifi %02X:%02X:%02X:%02X:%02X:%02X rssi %d\r\n",
            sc.bssid[0], sc.bssid[1], sc.bssid[2],
            sc.bssid[3], sc.bssid[4], sc.bssid[5],
            wifi_station_get_rssi());
    }
    switch (cs) {
        case STATION_GOT_IP:
            if (!clients) {
                msg_set_state("nosubs");
            } else {
                if (!strcmp(state, "nosubs")) {
                    msg_set_state("idle");
                }
            }
            return;
        case STATION_CONNECTING:
            msg_set_state("cnwifi");
            return;
        default:
            if ((lasttick - lastscan) >= 30000) {
                msg_set_state("nowifi");
                lastscan = lasttick;
                wifi_station_scan(NULL, wifi_scan_done);
                return;
            }
    }
}

static void ICACHE_FLASH_ATTR wifi_event(System_Event_t *evt)
{
    if (evt->event == EVENT_SOFTAPMODE_PROBEREQRECVED) return;
    Serial.printf("event %x\r\n", evt->event);
    switch (evt->event) {
        case EVENT_STAMODE_CONNECTED:
            Serial.printf("connect to ssid %s, channel %d\r\n",
                    evt->event_info.connected.ssid,
                    evt->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED:
            Serial.printf("disconnect from ssid %s, reason %d\r\n",
                    evt->event_info.disconnected.ssid,
                    evt->event_info.disconnected.reason);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            Serial.printf("mode: %d -> %d\r\n",
                    evt->event_info.auth_change.old_mode,
                    evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            Serial.printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR "\r\n",
                    IP2STR(&evt->event_info.got_ip.ip),
                    IP2STR(&evt->event_info.got_ip.mask),
                    IP2STR(&evt->event_info.got_ip.gw));
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
            Serial.printf("station: " MACSTR "join, AID = %d\r\n",
                    MAC2STR(evt->event_info.sta_connected.mac),
                    evt->event_info.sta_connected.aid);
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            Serial.printf("station: " MACSTR "leave, AID = %d\r\n",
                    MAC2STR(evt->event_info.sta_disconnected.mac),
                    evt->event_info.sta_disconnected.aid);
            break;
        default:
            break;
    }
}


static void wifi_setup()
{
    lastscan = 0 - 30000;
    // Read all wifi files from littlefs
    // Get stored wifi connections from flash, set index on wifi files
    // Scan wifi
    // See if wifi is found that matches littlefs files
    //   (opt?) If not see if wifi is found that matches flash (?)
    // Connect to that wifi
    wifi_set_event_handler_cb(wifi_event);

    char wififn[11] = "/wifiA.txt";
    int wifiidx = 0;
#ifndef MQTT_SERVER
    // Skip A because that's our softAP
    wifiidx = 1;
    wififn[5] = 'A' + wifiidx;
#endif
    while (LittleFS.exists(wififn)) {
        File wifitxt = LittleFS.open(wififn, "r");
        String wssid = wifitxt.readStringUntil('\n');
        String wpwd = wifitxt.readStringUntil('\n');
        wifitxt.close();

        if (wssid.length() > 31) {
            debugE("%s ssid too long!", wififn);
        } else if (wssid.length() > 63) {
            debugE("%s password too long!", wififn);
        } else {
            struct wifipwd *wp = new struct wifipwd;
            os_strcpy(wp->ssid, wssid.c_str());
            os_strcpy(wp->password, wpwd.c_str());
            wp->next = wifipwds;
            wifipwds = wp;
        }
        wifiidx++;
        wififn[5] = 'A' + wifiidx;
    }
}

void msg_setup()
{
    LittleFS.begin();

    if (!wifi_station_get_auto_connect()) wifi_station_set_auto_connect(1);
#ifdef MQTT_SERVER
    ap_setup();
    mdns_setup();
    server_setup();
#else
    sta_setup();
#endif
    wifi_setup();
    msg_set_state("nowifi");
}

void msg_send(const char *topic, const char *msg)
{
    struct client *client = clients;
    char sendstate[256];
    int len = 0;
    len = snprintf(sendstate, sizeof(sendstate), "%s %s\r\n", topic, msg);
    if (len > sizeof(sendstate)-1) {
        debugE("msg_send too large: %s %s", topic, msg);
        return;
    }
    while (client) {
        struct espconn *conn = client->conn;
        if (conn->state != ESPCONN_CLOSE) {
            espconn_send(conn, (uint8_t *)sendstate, len);
        }
        client = client->next;
    }
}

void msg_check()
{
    wifi_check();
#ifdef MQTT_SERVER
    mdns.update();
#else
    client_check();
#endif
    // Double pointer to facilitate removal
    struct client **client = &clients;
    char sendstate[256];
    int len = 0;
    while (*client) {
        struct espconn *conn = (*client)->conn;
        if (conn->state == ESPCONN_CLOSE) {
            struct client *toremove = *client;
            *client = (*client)->next;
            Serial.printf("espconn_disconnect(%d.%d.%d.%d:%d) %d\r\n",
                conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
                conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
                conn->proto.tcp->remote_port, conn->link_cnt);
            espconn_abort(conn);
            espconn_delete(conn);
            delete toremove;
        } else {
            if ((*client)->state != state) {
                if (!len) {
                    len = snprintf(sendstate, sizeof(sendstate), "set %s\r\n", state);
                    if (len > 255) len = 255;
                }
                int r = espconn_send(conn, (uint8_t *)sendstate, len);
                if (r == 0) (*client)->state = state;
            }
            client = &((*client)->next);
        }
    }
}

void msg_receive(const char *topic, const char *msg)
{
}
