#include <ESP8266WiFi.h>

//#define MQTT_BUTTONS_OUT
#define MQTT_BUTTONS_IN
//#define MQTT_LIGHTS
#include "settings.h"

#ifdef MQTT_LIGHTS
#define MQTT_SOFTAP
#endif

const char *state = "nosubs";

unsigned long loadavg = 0;
unsigned long lasttick = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  msg_setup();
  leds_setup();
  buttons_setup();
  ota_setup();
  // ws_setup();
  lasttick = millis();
}

void loop() {
  ota_check();
  msg_check();
  leds_animate();
  buttons_check();
  check_status();
  // ws_check();

  unsigned long nexttick = millis();
  unsigned long elaps = nexttick - lasttick;
  lasttick = nexttick;
  loadavg = (loadavg * 99 / 100) + (elaps * 1000 / ms_per_frame);
  if (elaps < ms_per_frame) {
    delay(ms_per_frame - elaps);
    lasttick += (ms_per_frame - elaps);
  }
}
