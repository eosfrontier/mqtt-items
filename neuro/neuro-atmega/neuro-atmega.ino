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

char l_line1[] = { 2,3,4,5,6 };
char l_line2[] = { 2,3,4,5,12,7,8,9 };
char l_line3[] = { 2,3,4,13,14,11,10 };
char l_line4[] = { 2,3,4,13,14,11,15,16 };
char *l_lines[] = { l_line1, l_line2, l_line3, l_line4 };
char l_lens[] = { 5, 8, 7, 8 };

char r_line1[] = { 2,3,4,5,6 };
char r_line2[] = { 2,3,4,12,11,7,8,9 };
char r_line3[] = { 2,3,4,12,11,10 };
char r_line4[] = { 2,3,4,12,13,14,15,16 };
char *r_lines[] = { r_line1, r_line2, r_line3, r_line4 };
char r_lens[] = { 5, 8, 6, 8 };

char lines[] = { l_line1, l_line2, l_line3, l_line4, r_line1, r_line2, r_line3, r_line4 };
char ln_lens[] = { 5,8,7,8,5,8,6,8 };

#define NUM_POINTS 4
#define NUM_COLS 4

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb_t;

typedef struct {
    short hue;
    short chroma;
    short luma;
} hcl_t;

struct anim {
    unsigned long tick;
    hcl_t hcl[NUM_COLS];
    short idx[NUM_COLS+1];
    short spd;
    unsigned char line; // 1 bit per possible line
    char ppos1; // Position on line
    char ppos2;
    unsigned char mode; //  ??  Bounce oid?
    char dly;
} anims[NUM_POINTS];

rgb_t get_hcl_color(hcl_t a, hcl_t b, uint32_t pos, uint32_t tot)
{
    long hue = (a.hue * pos + b.hue * (tot-pos-1)) / tot + (36 * FIXEDP); // Is cyclisch, in het positieve trekken (slechte hack maarja)
    long chroma = (a.chroma * pos + b.chroma * (tot-pos-1)) / tot;
    long luma = (a.luma * pos + b.luma * (tot-pos-1)) / tot;
    long x = chroma * (1 - abs(hue % FIXEDP - FIXEDP)) / FIXEDP;
    long r = 0, g = 0, b = 0;
    switch ((hue / FIXEDP) % 6) {
        case 0: r = c; g = x;
        case 1: r = x; g = c
        case 2: g = c; b = x;
        case 3: g = x; b = c;
        case 4: r = x; b = c;
        case 5: r = c; b = x;
    }
    // Luma 
    long m = luma - (r * 77 / FIXEDP) - (g * 151 / FIXEDP) - (b * 28 / FIXEDP);
    r += m;
    g += m;
    b += m;
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;
    rgb_t rgb = { .r = r, .g = g, .b = b };
    return rgb;
}

// See if the point is part of a line and if so return the index on that line
int get_pointindex(int off, int pos, unsigned char *ptline)
{
    for (int l = 0; l < 4; l++) {
        int ln = l+off;
        if (ptline & (1<<l)) {
            char *line = lines[ln];
            for (int i = 0; i < ln_lens[ln]; i++) {
                if (line[i] == pos) {
                    return (off ? 16-i : i);
                }
            }
        }
    }
    return -1;
}

// Add the color of all moving points for the given led pixel
uint32_t get_pix_color(int off, int pos, long tick)
{
    short r = 0, g = 0, b = 0;
    for (int p = 0; p < NUM_POINTS; p++) {
        struct anim *pt = anims[p];
        int idx = get_pointindex(off, pos, pt->line) - pt->ppos1;
        // idx is now integer index of pixel
        if (idx >= 0 && idx <= (pt->ppos2 - pt->ppos1)) {
            idx = (idx * FIXEDP) + (tick - pt->tick)*FIXEDP / pt->spd;
            // idx is now fixedpoint index of pixel relative to point animation
            if (idx >= 0 && idx < pt->idx[NUM_COLS]) {
                for (int i = 0; i < NUM_COLS; i++) {
                    if (pt->idx[i] < idx) { break; }
                }
                rgb_t itp = get_hcl_color(pt->hcl[i], pt->hcl[i+1], idx - pt->idx[i], pt->idx[i+1] - pt->idx[i]);
                r += itp.r;
                g += itp.g;
                b += itp.b;
            }
        }
    }
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    return (r + (g << 8) + (b << 16));
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
      unsigned long now = millis();
      for (int j = 2; j <= 16; j++) {
        l_strip.setPixelColor(j, get_pix_color(0, j, now));
        r_strip.setPixelColor(j, get_pix_color(4, j, now));
      }
      l_strip.show();
      r_strip.show();
      delay(20);
      delay(20);
  }
  // Serial.println("Done one round");
}

