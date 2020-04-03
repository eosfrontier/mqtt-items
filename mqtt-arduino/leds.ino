#include <Adafruit_NeoPixel.h>

typedef struct {
  unsigned long tm;
  uint32_t color[LEDS_NUM];
} LedAnimation;

Adafruit_NeoPixel ledstrip(LEDS_NUM, LEDS_PIN, NEO_GRB + NEO_KHZ800);
unsigned long tick = 0;
bool anim_rpt = false;
int anim_len = 0;
LedAnimation anim[MAX_ANIM];

uint32_t curcolor[LEDS_NUM];

/*
#define PIN_OUT_SET(b)    (*((volatile uint32_t *)0x60000304)=(1<<(b)))
#define PIN_OUT_CLEAR(b)  (*((volatile uint32_t *)0x60000308)=(1<<(b)))
#define PIN_DIR_OUTPUT(b) (*((volatile uint32_t *)0x60000310)=(1<<(b)))
#define NS_TO_CYCLES(n) ((n) / (1000000000L / F_CPU))
#define STORE_CYCLES(r) __asm__ __volatile__("rsr %0, CCOUNT \n\t" : "=a" (r))

// Delay an arbitrary number of cycles past the specified cycle count.
#define DELAY_CYCLES(_cs, _delay) do { \
  uint32_t _cc; \
  do { \
    STORE_CYCLES(_cc); \
  } while ((_cc - _cs) < _delay); \
} while(0);

#define ENTER_CRITICAL(r) __asm__ __volatile__("rsil %0,15 ; esync":"=a" (r))
#define LEAVE_CRITICAL(w) __asm__ __volatile__("wsr %0,ps ; esync"::"a" (w):"memory")

void send_led_byte(uint8_t bt)
{
  uint32_t cc_delay;
  uint32_t intr_state;
  for (uint8_t i = 8; i > 0; i--) {
    if (bt & (1 << (i-1))) {
      STORE_CYCLES(cc_delay);
      PIN_OUT_SET(LEDS_PIN);
      DELAY_CYCLES(cc_delay, 56);
      PIN_OUT_CLEAR(LEDS_PIN);
      DELAY_CYCLES(cc_delay, 100);
    } else {
      //ENTER_CRITICAL(intr_state);
      STORE_CYCLES(cc_delay);
      PIN_OUT_SET(LEDS_PIN);
      DELAY_CYCLES(cc_delay, 32);
      PIN_OUT_CLEAR(LEDS_PIN);
      //LEAVE_CRITICAL(intr_state);
      DELAY_CYCLES(cc_delay, 90);
    }
  }
}
*/

uint32_t set_interpolate(uint32_t cola, uint32_t colb, unsigned long frac, unsigned long denom, int idx)
{
  uint32_t ga = (cola >> 8) & 0xff;
  uint32_t gb = (colb >> 8) & 0xff;
  uint32_t gi = (gb * frac + ga * (denom-frac))/denom;
  //send_led_byte(gi);

  uint32_t ra = (cola) & 0xff;
  uint32_t rb = (colb) & 0xff;
  uint32_t ri = (rb * frac + ra * (denom-frac))/denom;
  //send_led_byte(ri);

  uint32_t ba = (cola >> 16) & 0xff;
  uint32_t bb = (colb >> 16) & 0xff;
  uint32_t bi = (bb * frac + ba * (denom-frac))/denom;
  //send_led_byte(bi);
  
  curcolor[idx] = (ri + (gi << 8) + (bi << 16));
  ledstrip.setPixelColor(idx, curcolor[idx]);
}

void leds_show()
{
  /*
  uint32_t intr_state;
  ENTER_CRITICAL(intr_state);
  for (int c = 0; c < LEDS_NUM; c++) {
    send_led_byte((curcolor[c] >> 8) & 0xff);
    send_led_byte((curcolor[c]) & 0xff);
    send_led_byte((curcolor[c] >> 16) & 0xff);
  }
  LEAVE_CRITICAL(intr_state);
  */
  ledstrip.show();
}

void leds_setup()
{
  ledstrip.begin();
  //PIN_DIR_OUTPUT(LEDS_PIN);
  leds_set(state);
}

void leds_animate()
{
  while (tick > 0) { // Voor de repeat, als we voorbij het einde zijn.
    unsigned long elaps = millis() - tick;
    // Look in which step of the animation we are
    for (int t = 1; t < anim_len; t++) {
      if (anim[t].tm > elaps) {
        if (elaps >= anim[t-1].tm) {
          unsigned long frac = elaps - anim[t-1].tm;
          unsigned long denom = anim[t].tm - anim[t-1].tm;
          for (int c = 0; c < LEDS_NUM; c++) {
            set_interpolate(anim[t-1].color[c], anim[t].color[c], frac, denom, c);
          }
          leds_show();
        }
        return;
      }
    }
    if (anim_rpt) {
      for (int c = 0; c < LEDS_NUM; c++) {
        anim[0].color[c] = anim[anim_len-1].color[c];
      }
      tick += anim[anim_len-1].tm;
    } else {
      tick = 0;
      for (int c = 0; c < LEDS_NUM; c++) {
        set_interpolate(anim[anim_len-1].color[c], anim[anim_len-1].color[c], 0, 1, c);
      }
      leds_show();
    }
  }
}

void leds_clear()
{
  anim[0].tm = 0;
  for (int c = 0; c < LEDS_NUM; c++) {
    anim[0].color[c] = curcolor[c];
  }
  for (int st = 1; st < MAX_ANIM; st++) {
    anim[st].tm = 0;
    for (int c = 0; c < LEDS_NUM; c++) {
      anim[st].color[c] = 0;
    }
  }
  anim_len = 0;
  anim_rpt = false;
}

void leds_set(const char *color)
{
  for (int i = 0; LEDS_ANIMATIONS[i]; i += 2) {
    if (!strcmp(color, LEDS_ANIMATIONS[i])) {
      if (state != LEDS_ANIMATIONS[i]) { // Pointer vergelijking omdat ze naar dezelfde string in-memory wijzen
        leds_set(LEDS_ANIMATIONS[i+1]);
        state = LEDS_ANIMATIONS[i];
      }
      return;
    }
  }
  leds_clear();
  int st = 0;
  const char *p = color;
  int ci = -2;
  while (true) {
    char c = tolower(*p++);
    int nm = -1;
    if (isspace(c) || c == 0) {
      if (st > 0 && ci >= 0) {
        ci++;
        for (int c = ci; c < LEDS_NUM; c++) {
          anim[st].color[c] = anim[st].color[c % ci];
        }
      }
      ci = -2;
      if (c == 0) break;
    }
    else if (c >= '0' && c <= '9') nm = c-'0';
    else if (c >= 'a' && c <= 'f') nm = c-'a'+10;
    else if (c == ',' || c == ':' || c == ';') {
      if (ci != -2) ci++;
    } else if (c == 'r') {
      anim_rpt = true;
    } else if (c == '#') { // Special case, shortcut
      if (ci == -2) {
        nm = anim[st].tm + 1000;
      }
    }
    if (nm >= 0) {
      if (ci < 0) {
        if (ci == -2) {
          if (st >= (MAX_ANIM-1)) {
            Serial.print("Color parse fail, too long: <<<"); Serial.print(color); Serial.println(">>>");
            break;
          }
          st++;
          ci = (c == '#' ? 0 : -1);  // Special case
        }
        anim[st].tm = anim[st].tm * 10 + nm;
      } else {
        if (ci < LEDS_NUM) anim[st].color[ci] = anim[st].color[ci] * 16 + nm;
      }
    }
  }
  anim_len = st+1;
  tick = millis();
  /*
  for (st = 0; st < anim_len; st++) {
    Serial.print("DBG: st = "); Serial.print(st);
    Serial.print(", anim[st].tm = "); Serial.print(anim[st].tm);
    Serial.print(", anim[st].color[] = "); Serial.print(anim[st].color[0], HEX);
    Serial.print(","); Serial.print(anim[st].color[1], HEX);
    Serial.print(","); Serial.print(anim[st].color[2], HEX);
    Serial.print(","); Serial.print(anim[st].color[3], HEX);
    Serial.println(" .");
  }
  */
}
