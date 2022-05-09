#include "gpio.h"
#include "settings.h"
#ifdef MQTT_GPIO
#include <Arduino.h>
#include "main.h"

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
  serprintf("Setting GPIO %s to %s", topic, msg);
  for (int i = 0; GPIO_PORTS[i]; i += 3) {
    if (!strcmp(topic, GPIO_PORTS[i])) {
      int hilo = GPIO_PORTS[i+2][0] == 'L';
      if (msg[0] == 'H') { hilo = !hilo; }
      serprintf("Setting GPIO PIN %d to digital %d", (int)GPIO_PORTS[i+1], hilo);
      digitalWrite((int)GPIO_PORTS[i+1], hilo);
    }
  }
}

#else
void gpio_setup() {}
void gpio_check() {}
void gpio_set(const char *topic, const char *msg) {}
#endif
