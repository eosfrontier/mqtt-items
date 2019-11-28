#include <Adafruit_NeoPixel.h>

#define R_LED_PIN 10
#define L_LED_PIN 9

#define LED_COUNT 17

#define FIXEDP 256

Adafruit_NeoPixel l_strip(LED_COUNT, L_LED_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel r_strip(LED_COUNT, R_LED_PIN, NEO_RGB + NEO_KHZ800);

uint32_t colors[] = {  0x000000,  0x008844,  0x00cccc,  0x0000ff,  0x0000ff,  0xcc00cc,  0x880044,  0x000000,
                       0x000000,  0x880044,  0xcc00cc,  0x0000ff,  0x0000ff,  0x00cccc,  0x008844,  0x000000,
                       0x000000,  0xff0000,  0x00ff00,  0x0000ff,  0xff0000,  0x00ff00,  0x0000ff,  0x000000 };
int c_len = 8;

int l_line1[] = { 2,3,4,5,6 };
int l_line2[] = { 2,3,4,5,12,7,8,9 };
int l_line3[] = { 2,3,4,13,14,11,10 };
int l_line4[] = { 2,3,4,13,14,11,15,16 };
int *l_lines[] = { l_line1, l_line2, l_line3, l_line4 };
int l_lens[] = { 5, 8, 7, 8 };

int r_line1[] = { 2,3,4,5,6 };
int r_line2[] = { 2,3,4,12,11,7,8,9 };
int r_line3[] = { 2,3,4,12,11,10 };
int r_line4[] = { 2,3,4,12,13,14,15,16 };
int *r_lines[] = { r_line1, r_line2, r_line3, r_line4 };
int r_lens[] = { 5, 8, 6, 8 };

uint32_t interp(uint32_t a, uint32_t b, uint32_t pos) {
  uint32_t rb = (a) & 0xff;
  uint32_t gb = (a >> 8) & 0xff;
  uint32_t bb = (a >> 16) & 0xff;
  uint32_t ra = (b) & 0xff;
  uint32_t ga = (b >> 8) & 0xff;
  uint32_t ba = (b >> 16) & 0xff;
  uint32_t ri = (ra * pos + rb * (FIXEDP-pos-1))/FIXEDP;
  uint32_t gi = (ga * pos + gb * (FIXEDP-pos-1))/FIXEDP;
  uint32_t bi = (ba * pos + bb * (FIXEDP-pos-1))/FIXEDP;
  return (ri + (gi << 8) + (bi << 16));
}

uint32_t get_color(long pos, int coff) {
  if (pos < 0) return 0x000000;
  int off = pos/FIXEDP;
  if (off > (c_len-2)) return 0x000000;
  return interp(colors[coff+off],colors[coff+off+1], pos % FIXEDP);
}

void setup() {
  // put your setup code here, to run once:
  l_strip.begin();
  l_strip.setPixelColor(0, 0xff0000);
  l_strip.setPixelColor(1, 0x0000ff);
  l_strip.show();
  
  r_strip.begin();
  r_strip.setPixelColor(0, 0xcccc00);
  r_strip.setPixelColor(1, 0x00cccc);
  r_strip.show();
  
  //Serial.begin(9600);
  //delay(4000);
  //Serial.println("DBG");
  //Serial.println(sizeof(long), DEC);
}

void loop() {
  long cwid = FIXEDP*3/2;
  long cspd = FIXEDP/5;
  // put your main code here, to run repeatedly:
  while (1) {
    int lline = random(4);
    int rline = random(4);
    int lcol = random(3) * c_len;
    int rcol = random(3) * c_len;
    int *l_line = l_lines[lline];
    int *r_line = r_lines[rline];
    int llen = l_lens[lline];
    int rlen = r_lens[rline];
    int len = rlen > llen ? rlen : llen;
    int cend = -cwid * (c_len-1);
    int cpos = cwid * len;
    while (cpos >= cend) {
      cpos -= cspd;
      for (int j = 0; j < len; j++) {
        l_strip.setPixelColor(l_line[j], get_color(cpos + FIXEDP*j, lcol));
        r_strip.setPixelColor(r_line[j], get_color(cpos + FIXEDP*j, rcol));
      }
      l_strip.show();
      r_strip.show();
      delay(20);
    }
  }
  // Serial.println("Done one round");
}
