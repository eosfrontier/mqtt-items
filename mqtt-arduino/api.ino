#ifdef MQTT_RFID
#include <FS.h>
#include <WiFiClientSecure.h>

WiFiClientSecure apiclient;

#define MAX_API_ACCESS 1024

#define API_STATUS_OK 0
#define API_STATUS_HEADERS 1
#define API_STATUS_DATA 2
#define API_STATUS_CHARACTERS 16
#define API_STATUS_META 32

#define API_FLAGS_ACCESS (1<<0)
#define API_FLAGS_CHARID (1<<6)
#define API_FLAGS_FOUND (1<<7)

struct api_access {
  long character_id;
  uint32_t cardid;
  uint8_t flags;
} api_granted[MAX_API_ACCESS];

int api_num_granted = -1;
unsigned long api_next_load_characters = 0;
unsigned long api_next_load_acl = 0;
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

#define API_ORTHANC_SERVER "api.eosfrontier.space"
#define API_ORTHANC_PORT 443

int api_parse_headers()
{
  if (!apiclient.connected()) {
    Serial.println(F("Connection closed while reading headers"));
    return -1;
  }
  if (!apiclient.available()) {
    return (api_check_status & 0xf0) | API_STATUS_HEADERS;
  }
  String http = apiclient.readStringUntil(' ');
  int stnum = apiclient.parseInt();
  String ststring = apiclient.readStringUntil('\n');
  
  Serial.print(F("Orthanc returned ")); Serial.print(stnum); Serial.println(ststring);
  
  while (apiclient.connected()) {
    String line = apiclient.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      break;
    }
  }

  if (stnum < 200 || stnum > 299) {
    apiclient.stop();
    if (stnum == 404) {
      return 0;
    }
    return -1;
  }
  return json_parse_stream();
}

int api_post_json(String path, String json)
{
  if (WiFi.status() != WL_CONNECTED) return false;
  Serial.println(F(" Connecting to Orthanc"));
  if (!apiclient.connect(API_ORTHANC_SERVER, API_ORTHANC_PORT)) {
    Serial.println(F("Api Failed to connect to " API_ORTHANC_SERVER));
    return -1;
  }

  Serial.println(F("  Sending request"));
  apiclient.println(
    "POST https://" API_ORTHANC_SERVER + path + " HTTP/1.0\r\n"
    "Host: " API_ORTHANC_SERVER "\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: " + json.length() + "\r\n\r\n" + json);

  if (!apiclient.connected()) return -1;
  return api_parse_headers();
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
  // Serial.print("DBG: "); Serial.print(depth); Serial.print(","); Serial.print(key); Serial.print(","); Serial.println(val);
  if (key == "characterID") {
    long cid = json_parse_int(val);
    if (cid > 0) {
      json_current.character_id = json_parse_int(val);
      json_current.flags |= API_FLAGS_CHARID;
    }
  }
  if ((key == "card_id") && (val != "null") && (val != "")) {
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
    long cid = json_parse_int(val);
    if (cid > 0) {
      json_current.character_id = json_parse_int(val);
      json_current.flags |= API_FLAGS_ACCESS|API_FLAGS_FOUND;
    }
  }
}

void json_end_object(int depth)
{
  if (json_current.flags & API_FLAGS_FOUND) {
    Serial.print("Got character id "); Serial.print(json_current.character_id); Serial.print(" with card "); Serial.print(json_current.card_id, HEX); Serial.print(" and access "); Serial.println(json_current.flags & API_FLAGS_ACCESS);
  }
}

long api_parse_count;

int json_parse_stream_step()
{
  if (!apiclient.connected()) {
    apiclient.stop();
    Serial.print(F("Parsed ")); Serial.print(api_parse_count); Serial.println(F(" bytes"));
    return 0;
  }
  while (apiclient.available()) {
    json_parse_char(apiclient.read());
    api_parse_count++;
  }
  return (api_check_status & 0xf0) | API_STATUS_DATA;
}

int json_parse_stream()
{
  api_parse_count = 0;
  json_start_parse();
  return json_parse_stream_step();
}

int api_load_acl()
{
  api_check_status = API_STATUS_META;
  return api_post_json("/orthanc/character/meta/",
    String("{\"token\":\"" + api_token + "\",\"meta\":\"roster:access_" MQTT_RFID "\"}"));
}

int api_load_characters()
{
  api_check_status = API_STATUS_CHARACTERS;
  return api_post_json("/orthanc/character/",
    String("{\"token\":\"" + api_token + "\",\"all_characters\":\"all_characters\"}"));
}

void api_setup()
{
  api_num_granted = -1;
  api_next_load_acl = lasttick + 5000;
  api_next_load_characters = lasttick + 3000;
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

int api_do_status()
{
  if ((api_check_status & 0x0f) == API_STATUS_HEADERS) return api_parse_headers();
  if ((api_check_status & 0x0f) == API_STATUS_DATA   ) return json_parse_stream_step();
  return -1;
}

void api_check()
{
  if (api_check_status > 0) {
    api_check_status = api_do_status();
    if (api_check_status <= 0) {
      if (api_next_load_characters < (lasttick + 2000)) { api_next_load_characters = lasttick + 4000; }
      if (api_next_load_acl < (lasttick + 2000)) { api_next_load_acl = lasttick + 2000; }
    }
    return;
  }
  // Don't recheck if there s an animation running
  if ((api_next_load_characters < lasttick) && ((anim_tick == 0) || (api_num_granted < 0))) {
    api_next_load_characters = lasttick + ((api_num_granted >= 0) ? 30000 : 5000);
    if (api_check_status = api_load_characters()) {
      api_next_load_characters = lasttick + 300000;
    }
    if (api_next_load_acl < (lasttick + 2000)) {
      api_next_load_acl = lasttick + 2000;
    }
    return;
  }
  // Don't recheck if there s an animation running
  if ((api_next_load_acl < lasttick) && ((anim_tick == 0) || (api_num_granted < 0))) {
    api_next_load_acl = lasttick + ((api_num_granted >= 0) ? 30000 : 5000);
    if (api_check_status = api_load_acl()) {
      api_next_load_acl = lasttick + 300000;
    }
  }
}

#else
void api_got_cardid(uint32_t cardid) {}
void api_setup() {}
void api_check() {}
#endif
