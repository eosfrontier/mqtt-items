#include "settings.h"
#include "buttons.h"
#ifdef MQTT_BUTTONS
#include <stdint.h>
#include <Arduino.h>
#include "msg.h"
#include "main.h"

void buttons_setup()
{
  for (int i = 0; i < BUTTONS_NUM; i++) {
    pinMode(BUTTONS_PINS[i], INPUT_PULLUP);
  }
}

static int buttons_last = 0;

void buttons_check()
{
  int btset = 0;
  for (int i = 0; i < BUTTONS_NUM; i++) {
    if (digitalRead(BUTTONS_PINS[i]) == LOW) {
      btset |= 1 << i;
    }
  }
  int btpush = btset & ~buttons_last;
  buttons_last = btset;
  if (btpush) {
    // Button(s) changed to pressed state since last check
    for (int i = 0; BUTTONS_ACTIONS[i]; i += 3) {
      if ((BUTTONS_ACTIONS[i][0] == '*') || !strcmp(state, BUTTONS_ACTIONS[i])) {
        if (btpush & (int)BUTTONS_ACTIONS[i+1]) {
          if (BUTTONS_ACTIONS[i+2]) {
            msg_send("set", BUTTONS_ACTIONS[i+2]);
          }
          break;
        }
      }
    }
  }
}
#else // MQTT_BUTTONS
void buttons_setup() {}
void buttons_check() {}
#endif // MQTT_BUTTONS
