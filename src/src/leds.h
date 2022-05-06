#ifndef _LEDS_H_
#define _LEDS_H_

void leds_setup();
void leds_animate();
void leds_clear();
void leds_set(const char *color);

extern unsigned long anim_tick;

#endif // _LEDS_H_
