#ifdef MQTT_RFID
#include <FS.h>

#define MAX_API_ACCESS 1024

struct api_access {
  uint32_t cardid;
  long character_id;
  bool access;
} api_granted[MAX_API_ACCESS];

int api_num_granted = -1;
unsigned long api_next_check = 0;
String api_token;

void api_got_cardid(uint32_t cardid)
{
  Serial.print("Got card id "); Serial.println(cardid, HEX);
  char cardid_str[9];
  sprintf(cardid_str, "%08x", cardid);
  msg_send("card", cardid_str);
  int l = 0;
  int r = api_num_granted - 1;
  while (r > l) {
    int m = (l+r)/2;
    if (api_granted[m].cardid < cardid) { l = m; } else { r = m; }
  }
  if (api_granted[l].cardid == cardid) {
    if (api_granted[l].access) {
      msg_send("granted", cardid_str);
      leds_set(RFID_LEDS_GRANTED);
    } else {
      msg_send("denied", cardid_str);
      leds_set(RFID_LEDS_DENIED);
    }
  } else {
    msg_send("denied", cardid_str);
    msg_send("unknown", cardid_str);
    leds_set(RFID_LEDS_DENIED);
  }
}

void api_setup()
{
  api_num_granted = -1;
  api_next_check = 0;
  if (SPIFFS.exists("/ApiToken.txt")) {
    File tokentxt = SPIFFS.open("/ApiToken.txt", "r");
    api_token = tokentxt.readStringUntil('\n');
    tokentxt.close();
  } else {
    api_token = "xxxx";
  }
}

bool api_load_acl()
{
  return true;
}

void api_check()
{
  // Don't recheck if there s an animation running
  if ((api_next_check < lasttick) && ((anim_tick == 0) || (api_num_granted >= 0))) {
    api_next_check = lasttick + ((api_num_granted >= 0) ? 30000 : 5000);
    if (WiFi.status() == WL_CONNECTED) {
      if (api_load_acl()) {
        api_next_check = lasttick + 300000;
      }
    }
  }
}

#else
#endif
