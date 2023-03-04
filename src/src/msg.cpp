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
            if (sz > (sizeof(client->buf)-1)) sz = sizeof(client->buf)-1;
            os_memcpy(client->buf+bln, data, sz);
            client->buf[bln+sz] = 0;
            //Serial.printf("buf = '%s'\r\n", client->buf);
            len = 0;
        }
    }
}

static void ICACHE_FLASH_ATTR conn_recon(void *arg, sint8 err)
{
    struct espconn *conn = (struct espconn *)arg;
    Serial.printf("conn_recon(%d.%d.%d.%d:%d)\r\n",
        conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
        conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
        conn->proto.tcp->remote_port);
    struct client *client = (struct client *)conn->reverse;
    client->state = NULL;
}

static void ICACHE_FLASH_ATTR conn_discon(void *arg)
{
    struct espconn *conn = (struct espconn *)arg;

    Serial.printf("conn_discon(%d.%d.%d.%d:%d)\r\n",
        conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
        conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
        conn->proto.tcp->remote_port);

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

    espconn_set_opt(conn, ESPCONN_REUSEADDR|ESPCONN_NODELAY|ESPCONN_KEEPALIVE);
    espconn_regist_recvcb(conn, conn_recv);
    //espconn_regist_sentcb(conn, conn_sent);
    espconn_regist_reconcb(conn, conn_recon);
    espconn_regist_disconcb(conn, conn_discon);
    espconn_regist_time(conn, 10, 1);

    Serial.printf("Connected client (%d.%d.%d.%d:%d)\r\n",
        conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
        conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
        conn->proto.tcp->remote_port);

    for (client = clients; client; client = client->next) {
        conn = client->conn;
        Serial.printf("- Client: (%d.%d.%d.%d:%d)\r\n",
            conn->proto.tcp->remote_ip[0], conn->proto.tcp->remote_ip[1],
            conn->proto.tcp->remote_ip[2], conn->proto.tcp->remote_ip[3],
            conn->proto.tcp->remote_port);
        Serial.printf("  Conn state: %d\r\n", conn->state);
        Serial.printf("  Link cnt: %d\r\n ", conn->link_cnt);
        Serial.printf("  Client State: '%s'\r\n ", client->state ? client->state : "");
        Serial.printf("  Buf: '%s'\r\n", client->buf);
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
    espconn_regist_connectcb(&server, conn_connected);
    espconn_regist_reconcb(&server, conn_recon);
    espconn_regist_disconcb(&server, conn_discon);
    espconn_tcp_set_max_con_allow(&server, MAX_CLIENTS);
    espconn_accept(&server);
    espconn_regist_time(&server, 30, 0);
    Serial.printf("Server initialized\r\n");
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

#else

static void client_mdns_query()
{
    char packet[128];
    const char *server = "eos-portal";
    const char *proto = "tcp";
    const char *local = "local";
    snprintf(packet, sizeof(packet),
        "%c%c%c%c"
        "%c%c%c%c"
        "%c%c%c%c"
        "%c_%s"
        "%c_%s"
        "%c%s"
        "%c"
        "%c%c%c%c",
        0,0,0,0,
        0,1,0,0,
        0,0,0,0,
        strlen(server)+1, server,
        strlen(proto)+1, proto,
        strlen(local), local,
        0,
        0, 12, 0, 1
        );

    struct ip_info ip_info;
    wifi_get_ip_info(STATION_IF, &ip_info);
    if (!ip_info.ip.addr) wifi_get_ip_info(SOFTAP_IF, &ip_info);
    ip_addr_t ifaddr = { .addr = ip_info.ip.addr };
}

static void client_setup()
{
    static esp_tcp tcp = {
        .remote_port = mqtt_port
    };
    static struct espconn server = {
        .type = ESPCONN_TCP,
        .state = ESPCONN_NONE,
        .proto = { .tcp = &tcp }
    };
}

static void sta_setup()
{
    wifi_set_opmode(STATION_MODE);
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
        // Loop over wp first because that defines prio
        for (struct wifipwd *wp = wifipwds; wp; wp = wp->next) {
            for (struct bss_info *bss = (struct bss_info *)arg; bss; bss = bss->next.stqe_next) {
                if (!strcmp(wp->ssid, (char *)bss->ssid)) {
                    struct station_config sconf;
                    sconf.bssid_set = 0;
                    os_memcpy(&sconf.ssid, wp->ssid, 32);
                    os_memcpy(&sconf.password, wp->password, 64);
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
        case STATION_WRONG_PASSWORD:
            // It also gives state wrong_password when still connecting
            if ((lasttick - lastscan) < 30000) {
                msg_set_state("cnwifi");
                break;
            }
        default:
            msg_set_state("nowifi");
            if ((lasttick - lastscan) >= 10000) {
                lastscan = lasttick;
                wifi_station_scan(NULL, wifi_scan_done);
                return;
            }
    }
}

static void wifi_setup()
{
    lastscan = 0 - 10000;
    // Read all wifi files from littlefs
    // Get stored wifi connections from flash, set index on wifi files
    // Scan wifi
    // See if wifi is found that matches littlefs files
    //   (opt?) If not see if wifi is found that matches flash (?)
    // Connect to that wifi

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
    server_setup();
#else
    sta_setup();
    client_setup();
#endif
    wifi_setup();
    msg_set_state("nowifi");
}

void msg_send(const char *topic, const char *msg)
{
}

void msg_check()
{
    wifi_check();
    struct client *client = clients;
    char sendstate[256];
    int len = 0;
    while (client) {
        if (client->state != state) {
            if (!len) {
                len = snprintf(sendstate, sizeof(sendstate), "set %s\r\n", state);
                if (len > 255) len = 255;
            }
            int r = espconn_send(client->conn, (uint8_t *)sendstate, len);
            if (r == 0) client->state = state;
        }
        client = client->next;
    }
}

void msg_receive(const char *topic, const char *msg)
{
}
