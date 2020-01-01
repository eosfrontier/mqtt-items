#include <ESP8266WiFi.h>

//#define MQTT_BUTTONS_OUT
//#define MQTT_BUTTONS_IN
#define MQTT_LIGHTS
#include "settings.h"

char ssid[] = "Airy";
char pass[] = "Landryssa";

const char *state = "nowifi";

unsigned long loadavg = 0;
unsigned long lasttick = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  WiFi.begin(ssid,pass);
  /*
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" WiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  */

  msg_setup();
  leds_setup();
  buttons_setup();
  lasttick = millis();
}

void loop() {
  msg_check();
  leds_animate();
  buttons_check();
  check_status();

  unsigned long nexttick = millis();
  unsigned long elaps = nexttick - lasttick;
  lasttick = nexttick;
  loadavg = (loadavg * 99 / 100) + (elaps * 1000 / ms_per_frame);
  if (elaps < ms_per_frame) {
    delay(ms_per_frame - elaps);
    lasttick += (ms_per_frame - elaps);
  }
}
