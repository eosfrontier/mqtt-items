#include "gpio.h"
#ifdef MQTT_GPIO

void gpio_setup()
{
  for (int i = 0; GPIO_PORTS[i]; i += 3) {
    pinMode((int)GPIO_PORTS[i+1], OUTPUT);
    digitalWrite((int)GPIO_PORTS[i+1], GPIO_PORTS[i+2][0] == 'L');
  }
}

void gpio_check()
{
}

void gpio_set(const char *topic, const char *msg)
{
  Serial.print("Setting GPIO "); Serial.print(topic); Serial.print(" to "); Serial.println(msg);
  for (int i = 0; GPIO_PORTS[i]; i += 3) {
    if (!strcmp(topic, GPIO_PORTS[i])) {
      int hilo = GPIO_PORTS[i+2][0] == 'L';
      if (msg[0] == 'H') { hilo = !hilo; }
      Serial.print("Setting GPIO PIN "); Serial.print((int)GPIO_PORTS[i+1]); Serial.print(" to digital "); Serial.println(hilo);
      digitalWrite((int)GPIO_PORTS[i+1], hilo);
    }
  }
}

#else
void gpio_setup() {}
void gpio_check() {}
void gpio_set(const char *topic, const char *msg) {}
#endif
