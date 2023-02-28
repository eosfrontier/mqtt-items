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

static void msg_handle(char *cmd, char *arg)
{
    if (!strcmp(msg, "set")) {
        leds_set(arg);
    }
}

static void conn_handle(char *line, unsigned int len)
{
    while (len > 0 && isspace(line[len])) len--;
    char *arg = os_strchr(line, ' ');
    if (!arg) return;
    *arg++ = 0;
    while (*arg && isspace(*arg)) arg++;
    msg_handle(line, arg);
}

static struct client {
    struct espconn *conn;
    unsigned long lastsent;
    struct client *next;
    char buf[64 - 12];
} *clients = NULL;

static void conn_recv(void *arg, char *data, unsigned short len)
{
    struct espcponn *conn = (struct espconn *)arg;
    struct client *client = (struct client *)conn->reverse;

    int bln = os_strlen(client->buf);
    while (len > 0) {
        char *nl = (char *)os_memchr(data, '\n', len);
        if (nl) {
            if (bln) {
                size_t sz = nl-data;
                if (sz > (sizeof(client->buf)-bln-1)) sz = (sizeof(client->buf)-bln-1);
                os_memcpy(client->buf+bln, data, sz);
                client->buf[bln+sz] = 0;
                conn_handle(client->buf);
                client->buf[0] = 0;
                bln = 0;
            } else {
                *nl = 0;
                conn_handle(data);
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

static void conn_recon(void *arg, sint8 err)
{
}

static void conn_discon(void *arg)
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


static void conn_connected(void *arg)
{
    struct client *client = new struct client;

    client->conn = (struct espconn *)arg;
    client->conn->reverse = client;
    client->lastsent = 0;
    client->buf[0] = 0;
    client->next = clients;
    clients = client;

    espconn_set_opt(client->conn, ESPCONN_REUSEADDR|ESPCONN_NODELAY|ESPCONN_KEEPALIVE);
    espconn_regist_recvcb(client->conn, conn_recv);
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
    if (!ip_info.ip.addr) wifi_get_ip_info(STATION_IF, &ip_info);
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

#endif

static char wifiidx = 0;

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
#ifdef MQTT_SERVER
    server_setup();
#else
    client_setup();
#endif
}

void msg_send(const char *topic, const char *msg)
{
}

void msg_check()
{
}

void msg_receive(const char *topic, const char *msg)
{
}
