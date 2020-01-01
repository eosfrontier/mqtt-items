
#ifdef MQTT_LIGHTS
const int LEDS_NUM = 4;
const int BUTTONS_PINS[] = {};
const char * BUTTONS_NAMES[] = {};
#define COLORS_DEFAULT "2000:000000"

#define MSG_NAME "eos/portal/light"
#endif

#ifdef MQTT_BUTTONS_OUT
const int LEDS_NUM = 3;
const int BUTTONS_PINS[] = {14,12,13};
const char * BUTTONS_NAMES[] = {"b1","b2","b3"};
#define COLORS_DEFAULT "2000:180000,001800,000020"

#define MSG_NAME "eos/portal/buttons_out"
#endif

#ifdef MQTT_BUTTONS_IN
const int LEDS_NUM = 4;
const int BUTTONS_PINS[] = {4,14,12,13};
const char * BUTTONS_NAMES[] = {"b4","b3","b2","b1"};
#define COLORS_DEFAULT "2000:101000,000020,001800,180000"

#define MSG_NAME "eos/portal/buttons_in"
#endif

#define MSG_NAME_NUM_PARTS 3

const int MAX_SUBSCRIBERS = 10;

const int BUTTONS_NUM = sizeof(BUTTONS_PINS)/sizeof(*BUTTONS_PINS);

const int LEDS_PIN = 0;

const char * LEDS_ANIMATIONS[] = {
  "green","r 1000:00ff00,00aa00,005500 2000:00aa00,005500,00ff00 3000:005500,00ff00,00aa00",
  "red","r 500:ff0000 800:000000,550000,aa0000,ff0000 1000:000000 1200:ff0000,aa0000,550000,000000 1500:ff0000",
  "idle",COLORS_DEFAULT,
  NULL
};
const int MAX_ANIM = 16;

#ifdef MQTT_LIGHTS
const char * MSG_MAPPING[] = {
  "eos/portal/*/button/b1","idle",MSG_NAME "/set","red",
  "eos/portal/*/button/b2","idle",MSG_NAME "/set","green",
  "eos/portal/*/button/*","*",MSG_NAME "/set","idle",
  NULL
};
const char * MSG_SUBSCRIPTIONS[] = {
  "eos/portal/*/button/*",
  NULL
};
#else
const char * MSG_MAPPING[] = {
  "eos/portal/light/ack","*",MSG_NAME "/set",NULL,
  NULL
};
const char * MSG_SUBSCRIPTIONS[] = {
  "eos/portal/light/ack",
  NULL
};
#endif

const int BUTTON_RETRIES = 5;
const int BUTTON_RETRY_DELAY = 10;

const int STATUS_FREQ = 10 * 1000;
const int FPS = 40;

const int MSG_TIMEOUT = 60 * 1000;

const int ms_per_frame = 1000/FPS;
