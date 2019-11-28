#include <Adafruit_NeoPixel.h>

#define R_LED_PIN 10
#define L_LED_PIN 9

#define LED_COUNT 17

#define FIXEDP 256

Adafruit_NeoPixel l_strip(LED_COUNT, L_LED_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel r_strip(LED_COUNT, R_LED_PIN, NEO_RGB + NEO_KHZ800);

uint32_t colors[] = {
  0x000000,
  0x008844,
  0x00cccc,
  0x0000ff,
  0x0000ff,
  0xcc00cc,
  0x880044,
  0x000000
};
int c_len = 8;

int l_line1[] = { 2,3,4,5,6 };
int l_line2[] = { 2,3,4,5,12,7,8,9 };
int l_line3[] = { 2,3,4,13,14,11,10 };
int l_line4[] = { 2,3,4,13,14,11,15,16 };
int *l_lines[] = { l_line1, l_line2, l_line3, l_line4 };
int l_lens[] = { 5, 8, 7, 8 };

uint32_t interp(uint32_t a, uint32_t b, long pos) {
  long rb = (a) & 0xff;
  long gb = (a >> 8) & 0xff;
  long bb = (a >> 16) & 0xff;
  long ra = (b) & 0xff;
  long ga = (b >> 8) & 0xff;
  long ba = (b >> 16) & 0xff;
  long ri = (ra * pos + rb * (FIXEDP-pos-1))/FIXEDP;
  long gi = (ga * pos + gb * (FIXEDP-pos-1))/FIXEDP;
  long bi = (ba * pos + bb * (FIXEDP-pos-1))/FIXEDP;
  return (ri + (gi << 8) + (bi << 16));
}

uint32_t get_color(long pos) {
  if (pos < 0) return 0x000000;
  int off = pos/FIXEDP;
  if (off > (c_len-2)) return 0x000000;
  return interp(colors[off],colors[off+1], pos % FIXEDP);
}

void setup() {
  // put your setup code here, to run once:
  l_strip.begin();
  l_strip.setPixelColor(0, 0x440000);
  l_strip.setPixelColor(1, 0x000044);
  l_strip.show();
  
  r_strip.begin();
  r_strip.setPixelColor(0, 0x333300);
  r_strip.setPixelColor(1, 0x003333);
  r_strip.show();
  
  //Serial.begin(9600);
  //delay(4000);
  //Serial.println("DBG");
  //Serial.println(sizeof(long), DEC);
}

void loop() {
  int lline = 0;
  long lwid = FIXEDP*3/2;
  long lspd = FIXEDP/5;
  long lpos, lend;
  int llen;
  // put your main code here, to run repeatedly:
  while (1) {
    lline = random(4);
    int *l_line = l_lines[lline];
    llen = l_lens[lline];
    lend = -lwid * (c_len-1);
    lpos = lwid * llen;
    while (lpos >= lend) {
      lpos -= lspd;
      for (int j = 0; j < llen; j++) {
        l_strip.setPixelColor(l_line[j], get_color(lpos + FIXEDP*j));
      }
      l_strip.show();
      delay(20);
    }
  }
  // Serial.println("Done one round");
}
