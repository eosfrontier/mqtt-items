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

static void msg_handle(const char *cmd, const char *arg)
{
    if (!strcmp(cmd, "set")) {
        for (int i = 0; LEDS_ANIMATIONS[i]; i += 2) {
            if (!strcmp(arg, LEDS_ANIMATIONS[i])) {
                if (state != LEDS_ANIMATIONS[i]) { // Pointer vergelijking omdat ze naar dezelfde string in-memory wijzen
                    leds_set(LEDS_ANIMATIONS[i+1]);
                    state = LEDS_ANIMATIONS[i];
                }
                return;
            }
        }
        leds_set(arg);
        return;
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

    int bln = os_strlen(client->buf);
    while (len > 0) {
        char *nl = (char *)memchr(data, '\n', len);
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
        }
    }
}

static void ICACHE_FLASH_ATTR conn_recon(void *arg, sint8 err)
{
    struct espconn *conn = (struct espconn *)arg;
    struct client *client = (struct client *)conn->reverse;
    client->state = NULL;
}

static void ICACHE_FLASH_ATTR conn_discon(void *arg)
{
    struct espconn *conn = (struct espconn *)arg;
    struct client **client = &clients;
    while (*client && conn->reverse != *client) {
        client = &((*client)->next);
    }
    if (*client) {
        struct client *toremove = *client;
        *client = (*client)->next;
        delete toremove;
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

    client->conn = (struct espconn *)arg;
    client->conn->reverse = client;
    //client->lastsent = 0;
    client->buf[0] = 0;
    client->next = clients;
    client->state = NULL;
    clients = client;

    espconn_set_opt(client->conn, ESPCONN_REUSEADDR|ESPCONN_NODELAY|ESPCONN_KEEPALIVE);
    espconn_regist_recvcb(client->conn, conn_recv);
    //espconn_regist_sentcb(client->conn, conn_sent);
    espconn_regist_reconcb(client->conn, conn_recon);
    espconn_regist_disconcb(client->conn, conn_discon);
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
    espconn_tcp_set_max_con_allow(&server, MAX_CLIENTS);
    espconn_accept(&server);
    espconn_regist_time(&server, 30, 0);
    debugD("Server initialized");
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
    if (strcmp(wificonfig.ssid, wssid) || strcmp(wificonfig.password, wpwd)) {
        strcpy(wificonfig.ssid, wssid.c_str());
        strcpy(wificonfig.password, wpwd.c_str());
        wificonfig.ssid_len = wssif.length();
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

static char wifiidx = 0;

static void wifi_check()
{
    // TODO
}

static void wifi_setup()
{
    // TODO
}

void msg_setup()
{
    LittleFS.begin();
    char wififn[11] = "/wifiA.txt";
    wifiidx = 0;
    while (LittleFS.exists(wififn)) {
        wifiidx++;
        wififn[5] = 'A' + wifiidx;
    }
    // TODO setup ap or client

    if (!wifi_station_get_auto_connect()) wifi_station_set_auto_connect(1);
#ifdef MQTT_SERVER
    ap_setup();
    server_setup();
#else
    sta_setup();
    client_setup();
#endif
    wifi_setup();
    msg_handle("set", "nosubs");
}

void msg_send(const char *topic, const char *msg)
{
}

void msg_check()
{
    wifi_check();
    struct client *client = clients;
    char sendstate[256];
    int len = snprintf(sendstate, sizeof(sendstate), "set %s\n", state);
    if (len > 255) len = 255;
    while (client) {
        if (client->state != state) {
            int r = espconn_send(client->conn, (uint8_t *)sendstate, len);
            if (r == 0) client->state = state;
        }
        client = client->next;
    }
}

void msg_receive(const char *topic, const char *msg)
{
}
