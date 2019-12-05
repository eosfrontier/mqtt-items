const int LEDS_NUM = 4;
const int LEDS_PIN = 0;
const uint32_t LEDS_COLORS[] = { 0x000000, 0x000000, 0x000000, 0x000000 };

const int BUTTONS_PINS[] = {};
const char * BUTTONS_NAMES[] = {};
const int BUTTONS_NUM = sizeof(BUTTONS_PINS)/sizeof(*BUTTONS_PINS);

#define MSG_NAME "eos/portal/light"
const char * MSG_MAPPING[] = {"eos/portal/buttons_in/button/b1","set flashred"};
