const int LEDS_NUM = 4;
const int LEDS_PIN = 0;

const char * LEDS_ANIMATIONS[] = {
  "green","r 1000:00ff00,00aa00,005500 2000:00aa00,005500,00ff00 3000:005500,00ff00,00aa00",
  "red","r r 500:ff0000 800:000000,550000,aa0000,ff0000 1000:000000 1200:ff0000,aa0000,550000,000000 1500:ff0000",
  "","2000:000000"
};
const int max_anim = 16;

const int BUTTONS_PINS[] = {};
const char * BUTTONS_NAMES[] = {};
const int BUTTONS_NUM = sizeof(BUTTONS_PINS)/sizeof(*BUTTONS_PINS);

#define MSG_NAME "eos/portal/light"
const char * MSG_MAPPING[] = {
  "eos/portal/buttons_in/button/b1",NULL,MSG_NAME "/set","red",
  "eos/portal/buttons_in/button/b2",NULL,MSG_NAME "/set","green",
  "eos/portal/buttons_in/button/b3",NULL,MSG_NAME "/set",""
};

const int fps = 40;
