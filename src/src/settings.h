#ifndef _SETTINGS_H_
#define _SETTINGS_H_
#include <stddef.h>

#ifdef MQTT_LIGHTS

const int LEDS_NUM = 4;
const int BUTTONS_PINS[] = {};
const char * const  BUTTONS_NAMES[] = {};
#define COLORS_DEFAULT "2000:000000"

#define MQTT_NAME "light"
#endif

#ifdef MQTT_BUTTONS_OUT
const int LEDS_NUM = 3;
const int BUTTONS_PINS[] = {14,12,13};
const char * const  BUTTONS_NAMES[] = {"b1","b2","b3"};
#define COLORS_DEFAULT "2000:000000,001800,000000"

#define MQTT_NAME "buttons_out"
#endif

#ifdef MQTT_BUTTONS_IN
const int LEDS_NUM = 4;
const int BUTTONS_PINS[] = {4,14,12,13};
const char * const  BUTTONS_NAMES[] = {"b4","b3","b2","b1"};
#define COLORS_DEFAULT "2000:000000,000000,001800,180000"

#define MQTT_NAME "buttons_in"
#endif

#ifdef MQTT_SONOFF
#define MQTT_WEBSOCKETS
#define MQTT_GPIO

const int LEDS_NUM = 0;
const int BUTTONS_PINS[] = {0};
const char * const  BUTTONS_NAMES[] = {"b1"};
#define COLORS_DEFAULT "1000:000000"

#define MQTT_NAME "sonoff_" MQTT_SONOFF

const int WS_MESSAGE_RETRIES = 5;
const int WS_MESSAGE_RETRY_DELAY = 10;

// Yes, I know, it's a hack to get ints into a char array
const char * const GPIO_PORTS[] = {
  "led",    (const char *)13, "L",
  "relais", (const char *)12, "H",
  NULL
};

#endif

#ifdef MQTT_RFID
#define MQTT_JSON

const int LEDS_NUM = 24;
const int BUTTONS_PINS[] = {};
const char * const BUTTONS_NAMES[] = {};
#define COLORS_DEFAULT "1000:000000,000008"

#define MQTT_NAME MQTT_RFID
#endif

#ifdef MQTT_RFID
#define MSG_NAME "eos/rfid/" MQTT_NAME
#define OTA_NAME "eos-rfid-" MQTT_NAME
#else
#define MSG_NAME "eos/portal/" MQTT_NAME
#define OTA_NAME "eos-portal-" MQTT_NAME
#endif

#define COLORS_NOWIFI "r 1000:100008,000000 2000:000000 5000:000000 6000:000000,080010 7000:000000 10000:000000"
#define COLORS_NOSUBS "r 500:000108,000000 1000:000000,000810"

#define MSG_NAME_NUM_PARTS 3

const int MAX_CLIENTS = 8;
const int MAX_SUBSCRIBERS = 16;

const int BUTTONS_NUM = sizeof(BUTTONS_PINS)/sizeof(*BUTTONS_PINS);

#ifdef MQTT_RFID
const char * const  LEDS_ANIMATIONS[] = {
  "idle",COLORS_DEFAULT,
  "nowifi",COLORS_NOWIFI,
  "nosubs",COLORS_NOSUBS,
  NULL
};
const char * const RFID_LEDS_GRANTED = "500:00ff00,000000,000000,000000 1000:00ff00,00ff00,000000,000000 1500:00ff00,00ff00,00ff00,000000 2000:00ff00 10000:008000 12000:000008,000000";
const char * const RFID_LEDS_DENIED  = "500:ff0000,000000,000000,000000 800:ff0000,ff0000,000000,000000 1000:ff0000,ff0000,ff0000,000000 1200:ff0000 1500:200000 2000:ff0000 5000:ff0000 6000:000008,000000";
const char * const RFID_LEDS_QUEUE_FULL = "500:005555,000000 1000:000000 1500:000000,005555 2000:000000 2500:000000,000000,000000,001111";
#else

const char * const  LEDS_ANIMATIONS[] = {
  "inc","r 1000:00ff00,00aa00,005500 2000:00aa00,005500,00ff00 3000:005500,00ff00,00aa00",
  "out","r 1000:00ff00,00aa00,005500 2000:00aa00,005500,00ff00 3000:005500,00ff00,00aa00",
  "red","r 500:ff0000 800:000000,550000,aa0000,ff0000 1000:000000 1200:ff0000,aa0000,550000,000000 1500:ff0000",
  "idle",COLORS_DEFAULT,
  "nowifi",COLORS_NOWIFI,
  "nosubs",COLORS_NOSUBS,
  NULL
};
#endif

const int LEDS_PIN = 0;
const int MAX_ANIM = 16;

#ifdef MQTT_LIGHTS
const char * const  MSG_MAPPING[] = {
  "eos/portal/buttons_in/button/b1","idle",MSG_NAME "/set","red",
  "eos/portal/buttons_in/button/b2","idle",MSG_NAME "/set","inc",
  "eos/portal/buttons_out/button/b2","idle",MSG_NAME "/set","out",
  "eos/portal/*/button/*","*",MSG_NAME "/set","idle",
  "eos/portal/*/beacon","*",MSG_NAME "/set",NULL,
  NULL
};
const char * const  MSG_SUBSCRIPTIONS[] = {
  "eos/portal/*/button/*",
  "eos/portal/*/beacon",
  NULL
};
#elif defined(MQTT_GPIO)
const char * const  MSG_MAPPING[] = {
  "eos/portal/light/ack","inc",MSG_NAME "/gpio/led","H",
  "eos/portal/light/ack","red",MSG_NAME "/gpio/led","H",
  "eos/portal/light/ack","out",MSG_NAME "/gpio/led","H",
  "eos/portal/light/ack","*",MSG_NAME "/gpio/led","L",
  // Chaining led to relais
  MSG_NAME "/gpio/led", "*", MSG_NAME "/gpio/relais", NULL,
  NULL
};
const char * const  MSG_SUBSCRIPTIONS[] = {
  "eos/portal/light/ack",
  NULL
};
#elif defined(MQTT_RFID)
const char * const  MSG_MAPPING[] = {
  NULL
};
const char * const  MSG_SUBSCRIPTIONS[] = {
  NULL
};
#else
const char * const  MSG_MAPPING[] = {
  "eos/portal/light/ack","*",MSG_NAME "/set",NULL,
  NULL
};
const char * const  MSG_SUBSCRIPTIONS[] = {
  "eos/portal/light/ack",
  NULL
};
#endif

#ifdef MQTT_WEBSOCKETS
const char * const  WS_BROADCAST_RECEIVE[] = {
  "bcportalinc", "beacon", "inc",
  "bcportalincdanger", "beacon", "red",
  "bcportalout", "beacon", "out",
  NULL
};
const char * const  WS_BROADCAST_SEND[] = {
  "inc", "42[\"broadcastSend\",{\"title\":\"Scheduled Incoming Portal Activation\","
         "\"file\":\"bcportalinc\",\"priority\":3,\"duration\":\"30000\",\"colorscheme\":\"0\"}]",
  "red", "42[\"broadcastSend\",{\"title\":\"Unscheduled Incoming Portal Activation\","
         "\"file\":\"bcportalincdanger\",\"priority\":3,\"duration\":\"30000\",\"colorscheme\":\"0\"}]",
  "out", "42[\"broadcastSend\",{\"title\":\"Portal Outgoing\","
         "\"file\":\"bcportalout\",\"priority\":3,\"duration\":\"17500\",\"colorscheme\":\"0\"}]",
  NULL
};
const char * const WS_BROADCAST_ACK = "eos/portal/light/ack";
#endif

const int BUTTON_RETRIES = 5;
const int BUTTON_RETRY_DELAY = 10;

const int STATUS_FREQ = 60 * 1000;
const int FPS = 40;

const int MSG_TIMEOUT = 125 * 1000;      // Forget subscription (if not resubscribed) after 2 minutes
const int MSG_SUB_INTERVAL = 20 * 1000;  // Resubscribe every 20 seconds
const int MSG_ACK_INTERVAL = 2 * 1000;   // Send last status every 2 seconds

const int ms_per_frame = 1000/FPS;

#endif // _SETTINGS_H_
