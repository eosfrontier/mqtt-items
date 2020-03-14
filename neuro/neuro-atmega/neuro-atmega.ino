#include <Adafruit_NeoPixel.h>

bool debugging = false;

#define R_LED_PIN 10
#define L_LED_PIN 9

#define LED_COUNT 17

#define FIXEDP ((long)256)
#define SPDFACT ((long)16)

Adafruit_NeoPixel l_strip(LED_COUNT, L_LED_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel r_strip(LED_COUNT, R_LED_PIN, NEO_RGB + NEO_KHZ800);

const char l_line1[] = { 2,3,4,5,6 };
const char l_line2[] = { 2,3,4,5,12,7,8,9 };
const char l_line3[] = { 2,3,4,13,14,11,10 };
const char l_line4[] = { 2,3,4,13,14,11,15,16 };
// char *l_lines[] = { l_line1, l_line2, l_line3, l_line4 };
// char l_lens[] = { 5, 8, 7, 8 };

const char r_line1[] = { 2,3,4,5,6 };
const char r_line2[] = { 2,3,4,12,11,7,8,9 };
const char r_line3[] = { 2,3,4,12,11,10 };
const char r_line4[] = { 2,3,4,12,13,14,15,16 };
// char *r_lines[] = { r_line1, r_line2, r_line3, r_line4 };
// char r_lens[] = { 5, 8, 6, 8 };

const char *lines[] = { l_line1, l_line2, l_line3, l_line4, r_line1, r_line2, r_line3, r_line4 };
const char ln_lens[] = { 5,8,7,8,5,8,6,8 };

const char inpins[] = { 3,4,5,6,7,8,14,15,16,18,19,20 };

#define NUM_POINTS 2
#define NUM_COLS 5

typedef struct {
    uint32_t r;
    uint32_t g;
    uint32_t b;
} rgb_t;

typedef struct {
    uint32_t hue;
    uint32_t chroma;
    uint32_t luma;
} hcl_t;

struct point {
    unsigned long tick;
    long size;
    hcl_t hcl[NUM_COLS];
    short idx[NUM_COLS];
    short spd;
    char dir;
    unsigned char line; // 1 bit per possible line
    char ppos1; // Position on line
    char ppos2;
    unsigned char mode; //  ??  Bounce oid?
    char dly;
} points[NUM_POINTS];

rgb_t get_hcl_color(hcl_t ca, hcl_t cb, uint32_t pos, uint32_t tot)
{
    debug_p("hue1", ca.hue);
    debug_p("chroma1", ca.chroma);
    debug_p("luma1", ca.luma);
    debug_p("hue2", cb.hue);
    debug_p("chroma2", cb.chroma);
    debug_p("luma2", cb.luma);
    debug_p("pos", pos);
    debug_p("tot", tot);
    long hue =    (cb.hue    * pos + ca.hue    * (tot-pos)) / tot + (36 * FIXEDP); // Is cyclisch, in het positieve trekken (slechte hack maarja)
    long chroma = (cb.chroma * pos + ca.chroma * (tot-pos)) / tot;
    long luma =   (cb.luma   * pos + ca.luma   * (tot-pos)) / tot;
    long x = chroma * (FIXEDP - abs(hue % (FIXEDP*2) - FIXEDP)) / FIXEDP;
    long r = 0, g = 0, b = 0;
    switch ((hue / FIXEDP) % 6) {
        case 0: r = chroma; g = x; break;
        case 1: r = x; g = chroma; break;
        case 2: g = chroma; b = x; break;
        case 3: g = x; b = chroma; break;
        case 4: r = x; b = chroma; break;
        case 5: r = chroma; b = x; break;
    }
    debug_p("x", x);
    debug_p("hue", hue);
    debug_p("chroma", chroma);
    debug_p("luma", luma);
    // Luma : 0.3*R + 0.59*G + 0.11*B
    long m = luma - (r * 77 / FIXEDP) - (g * 151 / FIXEDP) - (b * 28 / FIXEDP);
    r += m;
    g += m;
    b += m;
    debug_p("r", r);
    debug_p("g", g);
    debug_p("b", b);
    if (r < 0) r = 0;
    if (r > 255) r = 255;
    if (g < 0) g = 0;
    if (g > 255) g = 255;
    if (b < 0) b = 0;
    if (b > 255) b = 255;
    rgb_t rgb = { .r = r, .g = g, .b = b };
    //rgb_t rgb = { .r = constrain(r+m, 0, 255), .g = constrain(g+m, 0, 255), .b = constrain(b+m, 0, 255) };
    debug_p("r.", rgb.r);
    debug_p("g.", rgb.g);
    debug_p("b.", rgb.b);
    return rgb;
}

void debug_p(char *msg, long val)
{
    if (debugging) {
        Serial.print(msg);
        Serial.print(" ");
        Serial.println(val);
        delay(1);
    }
}


// See if the point is part of a line and if so return the index on that line
int get_pointindex(int off, int pos, unsigned char ptline)
{
    for (int l = 0; l < 4; l++) {
        int ln = l+off;
        if (ptline & (1<<ln)) {
            char *line = lines[ln];
            for (int i = 0; i < ln_lens[ln]; i++) {
                if (line[i] == pos) {
                    debug_p("line", ln);
                    debug_p("i", i);
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
    uint32_t r = 0, g = 0, b = 0;
    for (int p = 0; p < NUM_POINTS; p++) {
        debug_p("pixel", pos);
        debug_p("point", p);
        struct point *pt = &points[p];
        long idx = get_pointindex(off, pos, pt->line) - pt->ppos1;
        // idx is now integer index of pixel
        debug_p("idx", idx);
        if (idx >= 0 && idx <= (pt->ppos2 - pt->ppos1)) {
            debug_p("idx (1.5)", (idx * FIXEDP) - (pt->ppos2-pt->ppos1)*(FIXEDP/2));
            debug_p("pt->ppos1", pt->ppos1);
            debug_p("pt->ppos2", pt->ppos2);
            debug_p("idx * FIXEDP", idx * FIXEDP);
            debug_p("(pt->ppos2-pt->ppos1)*(FIXEDP/2)", (pt->ppos2-pt->ppos1)*(FIXEDP/2));
            debug_p("(idx * FIXEDP) - (pt->ppos2-pt->ppos1)*(FIXEDP/2)", (idx * FIXEDP) - (pt->ppos2-pt->ppos1)*(FIXEDP/2));
            debug_p("(((tick - pt->tick) * pt->spd) / 16) * pt->dir", (((tick - pt->tick) * pt->spd) / 16) * pt->dir);
            debug_p("(pt->size/2)", (pt->size/2));
            idx = (idx * FIXEDP) - (pt->ppos2-pt->ppos1)*(FIXEDP/2) - (((tick - pt->tick) * pt->spd) / 16 - (pt->size/2)) * pt->dir;
            debug_p("idx (2)", idx);
            debug_p("tick - pt->tick", tick - pt->tick);
            debug_p("pt->spd * pt->dir", pt->spd * pt->dir);
            debug_p("(((tick - pt->tick) * pt->spd * pt->dir))",        (((tick - pt->tick) * pt->spd * pt->dir)));
            debug_p("(((tick - pt->tick) * pt->spd * pt->dir) / 16)", (((tick - pt->tick) * pt->spd) / 16) * pt->dir);
            // Serial.print("Time "); Serial.print(tick - pt->tick); Serial.print(" offset index="); Serial.println(idx);
            // idx is now fixedpoint index of pixel relative to point animation
            if (idx >= pt->idx[0] && idx < pt->idx[NUM_COLS-1]) {
                int i;
                for (i = 1; i < NUM_COLS-1; i++) {
                    if (idx < pt->idx[i]) { break; }
                }
                debug_p("color idx", i);
                // Serial.print("Color number "); Serial.print(i); Serial.print(" ");
                rgb_t itp = get_hcl_color(pt->hcl[i-1], pt->hcl[i], idx - pt->idx[i-1], pt->idx[i] - pt->idx[i-1]);
                r += itp.r;
                g += itp.g;
                b += itp.b;
            }
        }
    }
    debug_p("r(t)",r);
    debug_p("g(t)",g);
    debug_p("b(t)",b);
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    debug_p("r(b)",r);
    debug_p("g(b)",g);
    debug_p("b(b)",b);
    return (r + (g << 8) + (b << 16));
}

void set_point(int p, unsigned long now, short hue, unsigned char line, int dir)
{
    struct point *pt = &points[p];
    pt->tick = now;

    pt->idx[0] = -1500;
    pt->hcl[0].hue = hue+0x500;
    pt->hcl[0].chroma = 0;
    pt->hcl[0].luma = 0;
    pt->idx[1] = -600;
    pt->hcl[1].hue = hue+0x580;
    pt->hcl[1].chroma = 0xff;
    pt->hcl[1].luma = 0;
    pt->idx[2] = 0;
    pt->hcl[2].hue = hue+0x600;
    pt->hcl[2].chroma = 0xff;
    pt->hcl[2].luma = 0xff;
    pt->idx[3] = 600;
    pt->hcl[3].hue = hue+0x680;
    pt->hcl[3].chroma = 0xff;
    pt->hcl[3].luma = 0;
    pt->idx[4] = 1500;
    pt->hcl[4].hue = hue+0x700;
    pt->hcl[4].chroma = 0;
    pt->hcl[4].luma = 0;

    pt->spd = 16;
    pt->dir = dir;
    pt->size = 16 * FIXEDP + 3000;
    pt->line = line;
    pt->ppos1 = 0;
    pt->ppos2 = 16;
    pt->mode = 0;
    pt->dly = 0;
}

void update_points(unsigned long now)
{
  for (int p = 0; p < NUM_POINTS; p++) {
    if (points[p].spd > 0) {
      debug_p("size",points[p].size);
      debug_p("spd", points[p].spd);
      uint32_t aspd = (points[p].size * 16 / (points[p].spd));
      debug_p("aspd", aspd);
      if (points[p].tick + aspd <= now) {
          // Serial.print("Updating "); Serial.print(p); Serial.print(" : tick = "); Serial.print(points[p].tick);
          // Serial.print(" now = "); Serial.print(now); Serial.print(" dir = "); Serial.println((int)points[p].dir);
          points[p].tick += aspd;
          points[p].dir = -points[p].dir;
      }
    }
  }
}

#define DEBOUNCE 100

void rotate_spd(unsigned char dir)
{
  for (int i = 0; i < NUM_POINTS; i++) {
    if (dir) {
      if (points[i].spd < 1024) {
        points[i].spd++;
      }
    } else {
      if (points[i].spd > 4) {
        points[i].spd--;
      }
    }
  }
}

void rotate_hue(unsigned char dir)
{
  for (int i = 0; i < NUM_POINTS; i++) {
    uint32_t wrap = 0;
    for (int h = 0; h < NUM_COLS; h++) {
      if (dir) {
        points[i].hcl[h].hue += 8;
        if (points[i].hcl[h].hue > 0x1800) wrap = 0xfa00;
      } else {
        points[i].hcl[h].hue -= 8;
        if (points[i].hcl[h].hue < 0x600) wrap = 0x0600;
      }
    }
    if (wrap) {
      for (int h = 0; h < NUM_COLS; h++) {
        points[i].hcl[h].hue += wrap;
      }
    }
  }
}

void rotate_knob(int pos, unsigned char dir)
{
  // Serial.print("Rotate knob ");
  // Serial.print(pos);
  // Serial.println(dir ? " Right" : " Left");
  switch(pos) {
    case 0:
      rotate_spd(dir);
      break;
    case 1:
      rotate_hue(dir);
      break;
    case 2:
      break;
    case 3:
      break;
  }
}

void press_knob(int pos)
{
  // Serial.print("Press knob ");
  // Serial.println(pos);
  if (pos == 2) debugging = true;
}

void process_knob(unsigned int pos, unsigned char rot1, unsigned char rot2, unsigned char btn)
{
  static unsigned char knob_r2 = 0;
  static unsigned char knob_r1 = 0;
  static unsigned long knob_press[] = { 0,0,0,0 };
  unsigned long now = millis();
  if (btn) {
    if ((now - knob_press[pos]) > DEBOUNCE) {
      press_knob(pos);       
    }
    knob_press[pos] = now;
  }
  unsigned char rot = rot1 | (rot2<<1);
  unsigned char r1 = (knob_r1 >> (2*pos)) & 3;
  unsigned char r2 = (knob_r2 >> (2*pos)) & 3;
  if (rot != r1) {
    //         00 -> 01 -> 11 -> 10 -> 00
    //   dcba      b^c
    //   0001 = 1   0
    //   0111 = 1   0
    //   1110 = 1   0
    //   1000 = 1   0
    //   0010 = 0   1
    //   1011 = 0   1
    //   1101 = 0   1
    //   0100 = 0   1
    if (rot != r2) {
      // Niet op en neer
      rotate_knob(pos, rot2 ^ (r1 & 1));
    }
    knob_r2 = (knob_r2 & ~(3 << (2*pos))) | r1 << (2*pos);
    knob_r1 = (knob_r1 & ~(3 << (2*pos))) | rot << (2*pos);
  }
}

#define P_BIT(p,bo) (((p) >> (bo)) & 1)

void scan_knobs()
{
  unsigned char pb = PINB;
  unsigned char pe = PINE;
  unsigned char pc = PINC;
  unsigned char pd = PIND;
  unsigned char pf = PINF;
  process_knob(0, P_BIT(pf,6), P_BIT(pf,5), P_BIT(pf,7));
  process_knob(1, P_BIT(pb,2), P_BIT(pb,3), P_BIT(pb,1));
  process_knob(2, P_BIT(pe,6), P_BIT(pb,4), P_BIT(pd,7));
  process_knob(3, P_BIT(pd,4), P_BIT(pd,0), P_BIT(pc,6));

}

void loop()
{
  unsigned long usesum = 0;
  unsigned long usecount = 0;
  while(1) {
    unsigned long now = millis();
    update_points(now);
    for (int j = 2; j < LED_COUNT; j++) {
      //get_pix_color(0, j, now);
      l_strip.setPixelColor(j, get_pix_color(0, j, now));
      //get_pix_color(4, j, now);
      r_strip.setPixelColor(j, get_pix_color(4, j, now));
      scan_knobs();
    }
    l_strip.show();
    r_strip.show();
    if (debugging) {
      delay(2000);
    }
    debugging = false;
    usecount++;
    unsigned long elaps = millis() - now;
    usesum += elaps;
    if (elaps < 20) {
      now += 20;
    } else {
      now += elaps;
    }
    while (now > millis()) {
      delay(1);
      scan_knobs();
    }
    if (usecount >= 100) {
        // Serial.print("Load/100: ");
        // Serial.println(usesum);
        usecount = 0;
        usesum = 0;
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

  for (int i = 0; i < 12; i++) {
    pinMode(inpins[i], INPUT_PULLUP);
  }
  
  if (debugging) {
      Serial.begin(9600);
      delay(2000);
      Serial.println("Starting");
  }
  /*
  for (int i = 0; i < 100; i++) {
  delay(100);
  Serial.print(millis());
  Serial.print(" ");
  Serial.println(i);
  }
  */
  unsigned long now = millis();
  //set_point(0, now, 0x000, 0b11000110,  1);
  //set_point(1, now, 0x200, 0b01101100, -1);
  set_point(0, now, 0x000, 0b10000010,  1);
  set_point(1, now, 0x200, 0b00101000,  1);
}
