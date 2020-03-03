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

#define NUM_POINTS 2
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

struct point {
    unsigned long tick;
    hcl_t hcl[NUM_COLS];
    short idx[NUM_COLS];
    short spd;
    unsigned char line; // 1 bit per possible line
    char ppos1; // Position on line
    char ppos2;
    unsigned char mode; //  ??  Bounce oid?
    char dly;
} points[NUM_POINTS];

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
    // Luma : 0.3*R + 0.59*G + 0.11*B
    long m = luma - (r * 77 / FIXEDP) - (g * 151 / FIXEDP) - (b * 28 / FIXEDP);
    rgb_t rgb = { .r = constrain(r+m, 0, 255), .g = constrain(g+m, 0, 255), .b = constrain(b+m, 0, 255) };
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
        struct point *pt = points[p];
        int idx = get_pointindex(off, pos, pt->line) - pt->ppos1;
        // idx is now integer index of pixel
        if (idx >= 0 && idx <= (pt->ppos2 - pt->ppos1)) {
            idx = (idx * FIXEDP) - ((tick - pt->tick - (pt->time/2))*pt->spd / FIXEDP);
            // idx is now fixedpoint index of pixel relative to point animation
            if (idx >= pt->idx[0] && idx < pt->idx[NUM_COLS-1]) {
                int i;
                for (i = 1; i < NUM_COLS-1; i++) {
                    if (idx < pt->idx[i]) { break; }
                }
                rgb_t itp = get_hcl_color(pt->hcl[i-1], pt->hcl[i], idx - pt->idx[i-1], pt->idx[i] - pt->idx[i-1]);
                r += itp.r;
                g += itp.g;
                b += itp.b;
            }
        }
    }
    return (min(r, 255) + (min(g, 255) << 8) + (min(b, 255) << 16));
}

void set_point(int p, unsigned long now, short hue, unsigned char line)
{
    struct point *pt = points[p];
    pt->tick = now;

    pt->idx[0] = -500;
    pt->hcl[0].hue = hue;
    pt->hcl[0].chroma = 0xff;
    pt->hcl[0].luma = 0;
    pt->idx[1] = -300;
    pt->hcl[1].hue = hue+0x100;
    pt->hcl[1].chroma = 0xff;
    pt->hcl[1].luma = 0xff;
    pt->idx[2] = 300;
    pt->hcl[2].hue = hue+0x200;
    pt->hcl[2].chroma = 0xff;
    pt->hcl[2].luma = 0xff;
    pt->idx[3] = 500;
    pt->hcl[3].hue = hue+0x300;
    pt->hcl[3].chroma = 0xff;
    pt->hcl[3].luma = 0;

    pt->spd = 1000;
    pt->time = 16000;
    pt->line = line;
    pt->ppos1 = 0;
    pt->ppos2 = 15;
    pt->mode = 0;
    pt->dly = 0;
}

void update_points(unsigned long now)
{
    for (int p = 0; p < NUM_POINTS; p++) {
        if (points[p].tick + points[p].time <= now) {
            points[p].now += points[p].time;
            points[p].spd = -points[p].spd;
        }
    }
}

void setup()
{
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
  unsigned long now = millis();
  set_point(0, now, 0x200, 0b10001000);
  set_point(1, now, 0x500, 0b00010001);
}

void loop()
{
  // put your main code here, to run repeatedly:
  unsigned long now = millis();
  update_points(now);
  for (int j = 2; j <= 16; j++) {
    l_strip.setPixelColor(j, get_pix_color(0, j, now));
    r_strip.setPixelColor(j, get_pix_color(4, j, now));
  }
  l_strip.show();
  r_strip.show();
  delay(20);
  // Serial.println("Done one round");
}

