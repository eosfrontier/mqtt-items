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

static unsigned long lastblink = 0;
static char blinker[2] = "L";

void gpio_check()
{
    if (lasttick > lastblink) {
        if (!strcmp(state, "nowifi")) {
            blinker[0] ^= ('H' ^ 'L');
            gpio_set("led", blinker);
            lastblink = lasttick + 2000;
        } else if (!strcmp(state, "nosubs")) {
            blinker[0] ^= ('H' ^ 'L');
            gpio_set("led", blinker);
            lastblink = lasttick + 500;
        } else {
            lastblink = lasttick + 5000;
        }
    }
}

void gpio_set(const char *topic, const char *msg)
{
  debugI("Setting GPIO %s to %s", topic, msg);
  for (int i = 0; GPIO_PORTS[i]; i += 3) {
    if (!strcmp(topic, GPIO_PORTS[i])) {
      int hilo = GPIO_PORTS[i+2][0] == 'L';
      if (msg[0] == 'H') { hilo = !hilo; }
      debugD("Setting GPIO PIN %d to digital %d", (int)GPIO_PORTS[i+1], hilo);
      digitalWrite((int)GPIO_PORTS[i+1], hilo);
    }
  }
}

#else
void gpio_setup() {}
void gpio_check() {}
void gpio_set(const char *topic, const char *msg) {}
#endif
