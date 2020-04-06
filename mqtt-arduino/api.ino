#ifdef MQTT_RFID
#include <FS.h>
#include <WiFiClientSecure.h>

WiFiClientSecure apiclient;

#define MAX_API_ACCESS 1024

#define API_FLAGS_ACCESS (1<<0)
#define API_FLAGS_CHARID (1<<6)
#define API_FLAGS_FOUND (1<<7)

struct api_access {
  long character_id;
  uint32_t cardid;
  uint8_t flags;
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
    if (api_granted[l].flags & API_FLAGS_ACCESS) {
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
  api_next_check = lasttick+5000;
  if (SPIFFS.exists("/ApiToken.txt")) {
    File tokentxt = SPIFFS.open("/ApiToken.txt", "r");
    api_token = tokentxt.readStringUntil('\n');
    tokentxt.close();
  } else {
    api_token = "xxxx";
  }
  apiclient.setInsecure();
  apiclient.setBufferSizes(512, 512);
}

#define API_ORTHANC_SERVER "api.eosfrontier.space"
#define API_ORTHANC_PORT 443

int api_post_json(String path, String json)
{
  if (WiFi.status() != WL_CONNECTED) return false;
  if (!apiclient.connect(API_ORTHANC_SERVER, API_ORTHANC_PORT)) {
    Serial.println(F("Api Failed to connect to " API_ORTHANC_SERVER));
    return 499;
  }
  /*
  Serial.print(F("Sending request to orthanc: ")); Serial.println(json);
  Serial.print(F("POST https://" API_ORTHANC_SERVER)); Serial.print(path); Serial.println(F(" HTTP/1.0"));
  Serial.println(F("Host: " API_ORTHANC_SERVER));
  Serial.println(F("Connection: close"));
  Serial.println(F("Content-Type: text/plain"));
  Serial.print(F("Content-Length: ")); Serial.println(json.length());
  Serial.println();
  Serial.println(json);
  */
  
  apiclient.print(F("POST https://" API_ORTHANC_SERVER)); apiclient.print(path); apiclient.println(F(" HTTP/1.0"));
  apiclient.println(F("Host: " API_ORTHANC_SERVER));
  apiclient.println(F("Connection: close"));
  apiclient.println(F("Content-Type: text/plain"));
  apiclient.print(F("Content-Length: ")); apiclient.println(json.length());
  apiclient.println();
  apiclient.println(json);

  String http = apiclient.readStringUntil(' ');
  int stnum = apiclient.parseInt();
  String ststring = apiclient.readStringUntil('\n');
  
  Serial.print(F("Orthanc returned ")); Serial.print(stnum); Serial.println(ststring);
  
  while (apiclient.connected()) {
    String line = apiclient.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      return stnum;
    }
  }
  Serial.println(F("Connection closed while reading headers"));
  return 499;
}

struct {
  long character_id;
  uint32_t card_id;
  uint8_t flags;
} json_current;

void json_begin_object(int depth)
{
  json_current.character_id = 0;
  json_current.card_id = 0;
  json_current.flags = 0;
}

long json_parse_int(String val)
{
  bool neg = false;
  int p = 0;
  int num = 0;
  while (p < val.length()) {
    uint8_t c = val[p];
    if (c == '-') neg = !neg;
    if (isdigit(c)) break;
    p++;
  }
  while (p < val.length()) {
    uint8_t c = val[p] - '0';
    if (c >= 10) break;
    num = num*10 + c;
    p++;
  }
  return (neg ? -num : num);
}

void json_object_value(int depth, String key, String val)
{
  //Serial.print("DBG: "); Serial.print(depth); Serial.print(","); Serial.print(key); Serial.print(","); Serial.println(val);
  if (key == "characterID") {
    json_current.character_id = json_parse_int(val);
    json_current.flags |= API_FLAGS_CHARID;
  }
  if ((key == "card_id") && (val != "null")) {
    uint32_t cid = 0;
    // If it's longer, take the first 8 nibbles, otherwise pad left
    int len = val.length() < 8 ? val.length() : 8;
    // Serial.print("DBG: card_id("); Serial.print(val.length()); Serial.print("="); Serial.print(len); Serial.print(") = '"); Serial.print(val); Serial.println("'");
    int s = 0;
    while (len--) {
      uint8_t c = val[len];
      if (isdigit(c)) {
        c -= '0';
      } else if (c >= 'a' && c <= 'f') {
        c -= ('a'-10);
      } else if (c >= 'A' && c <= 'F') {
        c -= ('A'-10);
      } else {
        Serial.print("Error parsing card ID '"); Serial.print(val); Serial.println("'");
        return;
      }
      cid |= (c << s);
      s += 4;
    }
    json_current.card_id = cid;
    json_current.flags |= API_FLAGS_FOUND;
  }
  if (key == "character_id") {
    json_current.character_id = json_parse_int(val);
    json_current.flags |= API_FLAGS_ACCESS|API_FLAGS_FOUND;
  }
}

void json_end_object(int depth)
{
  if (json_current.flags & API_FLAGS_FOUND) {
    Serial.print("Got character id "); Serial.print(json_current.character_id); Serial.print(" with card "); Serial.print(json_current.card_id, HEX); Serial.print(" and access "); Serial.println(json_current.flags | API_FLAGS_ACCESS);
  }
}

void json_parse_stream()
{
  long count = 0;
  json_start_parse();
  while (apiclient.connected()) {
    while (apiclient.available()) {
      json_parse_char(apiclient.read());
      count++;
    }
    yield();
  }
  Serial.print(F("Parsed ")); Serial.print(count); Serial.println(F(" bytes"));
}

/*
void api_dump_stream()
{
  Serial.println(F("Data from Orthanc:"));
  int wrap = 100;
  int count = 0;
  while (apiclient.connected()) {
    if (apiclient.available()) {
      if (--wrap <= 0) {
        wrap = 100;
        Serial.println();
      }
      char c = apiclient.read();
      Serial.write(c);
      count++;
    } else {
      yield();
    }
  }
  Serial.println();
  Serial.print(F("Received ")); Serial.print(count); Serial.println(F(" bytes"));
}
*/

bool api_load_acl()
{
  int res = api_post_json("/orthanc/character/meta/",
    String("{\"token\":\"" + api_token + "\",\"meta\":\"roster:access_" MQTT_RFID "\"}"));
  if (res < 200 || res > 299) {
    if (res == 404) {
      api_num_granted = 0;
      return true;
    }
    return false;
  }
  json_parse_stream();
  apiclient.stop();
  return true;
}

bool api_load_characters()
{
  int res = api_post_json("/orthanc/character/",
    String("{\"token\":\"" + api_token + "\",\"all_characters\":\"all_characters\"}"));
  if (res < 200 || res > 299) {
    if (res == 404) {
      api_num_granted = 0;
      return true;
    }
    return false;
  }
  json_parse_stream();
  apiclient.stop();
  return true;
}

void api_check()
{
  // Don't recheck if there s an animation running
  if ((api_next_check < lasttick) && ((anim_tick == 0) || (api_num_granted < 0))) {
    api_next_check = lasttick + ((api_num_granted >= 0) ? 30000 : 5000);
    if (api_check_status = api_load_characters()) {
      api_next_check = lasttick + 300000;
    }
  }
}

#else
void api_got_cardid(uint32_t cardid) {}
void api_setup() {}
void api_check() {}
#endif
