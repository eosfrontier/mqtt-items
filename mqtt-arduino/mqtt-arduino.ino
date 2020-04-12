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
int api_check_status = -1;
int avl_num_entries[2] = {0,0};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  lasttick = millis();
  ntp_setup();
  rfid_setup();
  msg_setup();
  leds_setup();
  buttons_setup();
  gpio_setup();
  ota_setup();
  ws_setup();
  api_setup();
}

void loop() {
  ntp_check();
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
    lasttick += (ms_per_frame - elaps);
    if (api_check_status > 0) {
      while (millis() < (lasttick - 1)) {
        delay(1);
        if (api_check_status > 0) { api_check(); }
      }
    } else {
      delay(ms_per_frame - elaps);
    }
  }
}
