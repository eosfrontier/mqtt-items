#include <ESP8266WiFi.h>

//#define MQTT_BUTTONS_OUT
//#define MQTT_BUTTONS_IN
//#define MQTT_LIGHTS
//#define MQTT_SONOFF "B"
#define MQTT_RFID "armory"
#include "settings.h"

const char *state = "nosubs";

unsigned long loadavg = 0;
unsigned long lasttick = 0;
unsigned long anim_tick = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  rfid_setup();
  msg_setup();
  leds_setup();
  buttons_setup();
  gpio_setup();
  ota_setup();
  ws_setup();
  api_setup();
  lasttick = millis();
}

void loop() {
  ota_check();
  rfid_check();
  msg_check();
  leds_animate();
  buttons_check();
  gpio_check();
  check_status();
  ws_check();
  api_check();

  unsigned long nexttick = millis();
  unsigned long elaps = nexttick - lasttick;
  lasttick = nexttick;
  loadavg = (loadavg * 99 / 100) + (elaps * 1000 / ms_per_frame);
  if (elaps < ms_per_frame) {
    delay(ms_per_frame - elaps);
    lasttick += (ms_per_frame - elaps);
  }
}
