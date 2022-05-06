#ifdef MQTT_RFID
#include <FS.h>
#include <WiFiClientSecure.h>

WiFiClientSecure apiclient;

#define API_STATUS_OK 0
#define API_STATUS_HEADERS 1
#define API_STATUS_DATA 2

#define API_FLAGS_ACCESS (1<<0)
#define API_FLAGS_CHARID (1<<8)
#define API_FLAGS_CARDID (1<<9)

#define API_CHARACTERS 0
#define API_META       1
#define API_SEND       2

#define API_PARSE_NUM        3


#define API_QUEUE_MAX 32
#define API_QUEUE_FULL (((api_queue_end + 1) % API_QUEUE_MAX) == api_queue_start)

struct api_queue {
    uint32_t time;
    long character_id;
    bool access;
} api_queue[API_QUEUE_MAX];

uint8_t api_queue_start, api_queue_end;

char api_queue_size()
{
  return ((api_queue_end + API_QUEUE_MAX - api_queue_start) % API_QUEUE_MAX);
}

int api_failcount[API_PARSE_NUM];
unsigned long api_next_load[API_PARSE_NUM];
String api_token;

void api_add_queue(long character_id, bool access)
{
    api_queue[api_queue_end].time = ntp_now();
    api_queue[api_queue_end].character_id = character_id;
    api_queue[api_queue_end].access = access;
    api_queue_end = (api_queue_end + 1) % API_QUEUE_MAX;
    api_next_load[API_SEND] = lasttick + 1000;
}

void api_got_cardid(uint32_t cardid)
{
  Serial.print("Got card id "); Serial.println(cardid, HEX);
  if (API_QUEUE_FULL) {
      Serial.println("QUEUE FULL");
      leds_set(RFID_LEDS_QUEUE_FULL);
  }
  char cardid_str[9];
  sprintf(cardid_str, "%08x", cardid);
  msg_send("card", cardid_str);
  avl_access_t *entry = avl_find(cardid, 0);
  if (entry) {
    long characterid = entry->data.character_id;
    avl_access_t *accessentry = avl_find((uint32_t)characterid, 1);
    uint32_t access = (accessentry ? accessentry->data.access : 0);
    char characterid_str[12];
    sprintf(characterid_str, "%d", characterid);
    // Serial.print("Found character id "); Serial.print(characterid_str); Serial.print(" with access "); Serial.print(access); Serial.println(".");
    if (access) {
      msg_send("granted", characterid_str);
      leds_set(RFID_LEDS_GRANTED);
    } else {
      msg_send("denied", characterid_str);
      leds_set(RFID_LEDS_DENIED);
    }
    api_add_queue(characterid, access);
  } else {
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
    return API_STATUS_HEADERS;
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
  Serial.println(
    "POST https://" API_ORTHANC_SERVER + path + " HTTP/1.0\r\n"
    "Host: " API_ORTHANC_SERVER "\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: " + json.length() + "\r\n\r\n" + json);
  apiclient.println(
    "POST https://" API_ORTHANC_SERVER + path + " HTTP/1.0\r\n"
    "Host: " API_ORTHANC_SERVER "\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: " + json.length() + "\r\n\r\n" + json);

  if (!apiclient.connected()) return -1;
  return api_parse_headers();
}

avl_access_t json_current;

void json_begin_object(int depth)
{
  json_current.key.v = 0;
  json_current.data.v = 0;
  json_current.bitfield = 0;
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

char api_parse_type;

void json_object_value(int depth, String key, String val)
{
  // Serial.print("DBG: "); Serial.print(depth); Serial.print(","); Serial.print(key); Serial.print(","); Serial.println(val);
  if (key == "characterID") {
    long cid = json_parse_int(val);
    if (cid > 0) {
      json_current.data.character_id = json_parse_int(val);
      json_current.bitfield |= API_FLAGS_CHARID;
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
    json_current.key.card_id = cid;
    json_current.bitfield |= API_FLAGS_CARDID;
  }
  if (key == "character_id") {
    long cid = json_parse_int(val);
    if (cid > 0) {
      json_current.key.character_id = json_parse_int(val);
      json_current.data.access = 3;
      json_current.bitfield |= API_FLAGS_ACCESS;
    }
  }
}

void json_end_object(int depth)
{
  if (api_parse_type == API_CHARACTERS) {
    if ((json_current.bitfield & (API_FLAGS_CARDID|API_FLAGS_CHARID)) == (API_FLAGS_CARDID|API_FLAGS_CHARID)) {
      // Serial.print("Got character id "); Serial.print(json_current.data.character_id); Serial.print(" with card "); Serial.println(json_current.key.card_id, HEX);
      avl_insert(&json_current, 0);
    }
  }
  if (api_parse_type == API_META) {
    if ((json_current.bitfield & (API_FLAGS_ACCESS)) == (API_FLAGS_ACCESS)) {
      // Serial.print("Got character id "); Serial.print(json_current.key.character_id); Serial.print(" with access "); Serial.println(json_current.data.access);
      avl_insert(&json_current, 1);
    }
  }
}

long api_parse_count;

int json_parse_stream_step()
{
  if (!apiclient.connected()) {
    apiclient.stop();
    Serial.print(F("Parsed ")); Serial.print(api_parse_count); Serial.println(F(" bytes"));
    avl_print_status();
    if (api_parse_type == API_CHARACTERS) {
      api_failcount[API_CHARACTERS] = 4;
      api_next_load[API_CHARACTERS] = lasttick + 1800000;
    }
    if (api_parse_type == API_META) {
      api_failcount[API_META] = 4;
      api_next_load[API_META] = lasttick + 300000;
      avl_degrade_access();
    }
    if (api_parse_type == API_SEND) {
      api_failcount[API_SEND] = 4;
      api_next_load[API_SEND] = lasttick + 1000;
      api_queue_start = (api_queue_start + 1) % API_QUEUE_MAX;
    }
    return 0;
  }
  while (apiclient.available()) {
    json_parse_char(apiclient.read());
    api_parse_count++;
  }
  return API_STATUS_DATA;
}

int json_parse_stream()
{
  api_parse_count = 0;
  json_start_parse();
  return json_parse_stream_step();
}

int api_load_meta()
{
  api_parse_type = API_META;
  return api_post_json("/orthanc/character/meta/",
    String("{\"token\":\"" + api_token + "\",\"meta\":\"roster:access_" MQTT_RFID "\"}"));
}

int api_load_characters()
{
  api_parse_type = API_CHARACTERS;
  return api_post_json("/orthanc/character/",
    String("{\"token\":\"" + api_token + "\",\"all_characters\":\"all_characters\"}"));
}

int api_send_queue()
{
    struct api_queue *ent = &api_queue[api_queue_start];
    api_parse_type = API_SEND;
    Serial.print("Want to send access of "); Serial.print(ent->character_id); Serial.print(" at time "); Serial.print(ent->time); Serial.print(" Access: "); Serial.println(ent->access ? "Granted" : "DENIED");
    if (ent->time < 1500000000) {
      ent->time = ntp_time(ent->time);
      if (ent->time < 1500000000) {
        // We do not have NTP time so fail
        return -1;
      }
    }
    return api_post_json("/orthanc/character/meta/update.php",
        String("{\"token\":\"" + api_token +
            "\",\"id\":"+ent->character_id+
            ",\"meta\":[{\"name\":\"access:" MQTT_RFID "\",\"value\":\""+
            ent->time + (ent->access ? ":Granted" : ":DENIED") + "\",\"oldvalue\":\""+
            ent->time + (ent->access ? ":Granted" : ":DENIED") + "\"}]}"));
}

BearSSL::Session api_session;

void api_setup()
{
  api_failcount[API_CHARACTERS] = 0;
  api_failcount[API_META] = 0;
  api_failcount[API_SEND] = 0;
  api_next_load[API_CHARACTERS] = lasttick + 3000;
  api_next_load[API_META] = lasttick + 5000;
  api_next_load[API_SEND] = lasttick + 1000;
  if (SPIFFS.exists("/ApiToken.txt")) {
    File tokentxt = SPIFFS.open("/ApiToken.txt", "r");
    api_token = tokentxt.readStringUntil('\n');
    tokentxt.close();
  } else {
    api_token = "xxxx";
  }
  if (SPIFFS.exists("/EosPubKey.txt")) {
    File pubkeytxt = SPIFFS.open("/EosPubKey.txt", "r");
    String eos_pubkey = pubkeytxt.readString();
    pubkeytxt.close();
    BearSSL::PublicKey *eos_key = new BearSSL::PublicKey(eos_pubkey.c_str());
    apiclient.setKnownKey(eos_key);
    //Serial.print("Public key:"); Serial.println(eos_key);
  } else {
    apiclient.setInsecure();
  }
  apiclient.setSession(&api_session);

  apiclient.setBufferSizes(512, 512);
  api_queue_start = 0;
  api_queue_end = 0;
}

int api_do_status()
{
  if (api_check_status == API_STATUS_HEADERS) return api_parse_headers();
  if (api_check_status == API_STATUS_DATA   ) return json_parse_stream_step();
  return -1;
}

void api_check()
{
  if (api_check_status > 0) {
    api_check_status = api_do_status();
    if (api_check_status <= 0) {
      if (api_failcount[api_parse_type] > 0) {
        api_failcount[api_parse_type]--;
      }
      for (int i = 0; i < API_PARSE_NUM; i++) {
        unsigned long next = lasttick + ((i == api_parse_type) ? 2000 : 4000);
        if (api_next_load[i] < next) {
          api_next_load[i] = next;
        }
        if (api_failcount[i] > 0) {
          api_check_status = 0;
        }
      }
    }
    return;
  }
  // Don't send if there's an animation running
  if ((api_queue_start != api_queue_end) && ((api_next_load[API_SEND] < lasttick) && ((anim_tick == 0) || (API_QUEUE_FULL)))) {
      api_next_load[API_SEND] = lasttick + 5000;
      api_check_status = api_send_queue();
  }
  // Don't recheck if there's an animation running
  if ((api_next_load[API_CHARACTERS] < lasttick) && ((anim_tick == 0) || (api_failcount[API_CHARACTERS] <= 0))) {
    api_next_load[API_CHARACTERS] = lasttick + ((api_failcount[API_CHARACTERS] > 0) ? 30000 : 5000);
    api_check_status = api_load_characters();
    if (api_next_load[API_META] < (lasttick + 2000)) {
      api_next_load[API_META] = lasttick + 2000;
    }
    return;
  }
  // Don't recheck if there's an animation running
  if ((api_next_load[API_META] < lasttick) && ((anim_tick == 0) || (api_failcount[API_META] <= 0))) {
    api_next_load[API_META] = lasttick + ((api_failcount[API_META] >= 0) ? 30000 : 5000);
    api_check_status = api_load_meta();
  }
}

#else
void api_got_cardid(uint32_t cardid) {}
void api_setup() {}
void api_check() {}
char api_queue_size() { return 0; }
#endif
