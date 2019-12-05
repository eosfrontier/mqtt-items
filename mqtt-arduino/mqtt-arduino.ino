#include <ESP8266WiFi.h>
#include <errno.h>
#include "settings.h"

char ssid[] = "Airy";
char pass[] = "Landryssa";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" WiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  msg_setup();
  leds_setup();
}

void loop() {
  msg_check();
  leds_animate();
  delay(20);
}
