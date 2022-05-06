#ifdef MQTT_JSON

#define JSON_MAX_DEPTH 10
#define JSON_MAX_STRLEN 300

enum json_state_t {
  initial_value,
  object_value,
  want_colon,
  in_array,
  in_object,
  in_key,
  in_string,
  in_value,
  in_escape,
  in_unicode
};
  
struct json_data {
  int depth;
  int pos;
  json_state_t state[JSON_MAX_DEPTH];
  String key[JSON_MAX_DEPTH];
  String val;
} jss;

bool json_start_parse()
{
  jss.depth = 0;
  jss.pos = 0;
  jss.state[jss.depth] = initial_value;
}

bool json_err(const char *wanted, char c)
{
  Serial.print("Json parse error, "); Serial.print(wanted); Serial.print(", got <"); Serial.print(c); Serial.print("> at position "); Serial.println(jss.pos);
  return false;
}

bool json_push_state(json_state_t s, char c);  // Workaround Arduino automatic prototyping
bool json_push_state(json_state_t s, char c)
{
  if (++jss.depth >= JSON_MAX_DEPTH) return json_err("Max depth exceeded", c);
  jss.state[jss.depth] = s;
  return true;
}

bool json_pop_state(char c)
{
  if (jss.depth <= 0) {
    return json_err("CANTHAPPEN ran out of depth", c);
  }
  jss.depth--;
  if (jss.state[jss.depth] == object_value) {
    jss.state[jss.depth] = in_object;
  }
  return true;
}

void json_emit_value()
{
  if ((jss.depth > 0) && (jss.state[jss.depth-1] == object_value)) {
    json_object_value(jss.depth-1, jss.key[jss.depth-1], jss.val);
    // Serial.print("DBG: "); Serial.print(jss.depth); Serial.print(" '"); Serial.print(jss.key[jss.depth-1]); Serial.print("' = '"); Serial.print(jss.val); Serial.println("'");
  } else {
    // Serial.print("DBG: "); Serial.print(jss.depth); Serial.print(" value '"); Serial.print(jss.val); Serial.println("'");
  }
}

bool json_parse_char(char c)
{
  jss.pos++;
  switch (jss.state[jss.depth]) {
    case in_array:
      if (c == ',') return true;
      if (c == ']') {
        return json_pop_state(c);
      }
      // Fallthrough
    case initial_value:
    case object_value:
      if (isspace(c)) return true;
      if (c == '[') {
        return json_push_state(in_array, c);
      }
      if (c == '{') {
        json_begin_object(jss.depth);
        return json_push_state(in_object, c);
      }
      if (c == '"') {
        jss.val = String("");
        return json_push_state(in_string, c);
      }
      if (isdigit(c) || c == '-' || c == 'f' || c == 't' || c == 'n') {
        jss.val = String(c);
        return json_push_state(in_value, c);
      }
      return json_err("expected value start", c);
    case want_colon:
      if (isspace(c)) return true;
      if (c == ':') {
        jss.state[jss.depth] = object_value;
        return true;
      }
      return json_err("expected colon", c);
    case in_object:
      if (isspace(c)) return true;
      if (c == ',') return true;
      if (c == '"') {
        jss.state[jss.depth] = in_key;
        jss.val = String("");
        return true;
      }
      if (c == '}') {
        json_end_object(jss.depth-1);
        return json_pop_state(c);
      }
      return json_err("expected key start", c);
    case in_key:
      if (c == '"') {
        // Serial.print("Object key(");Serial.print(jss.depth);Serial.print(") '");Serial.print(jss.val);Serial.println("'");
        jss.key[jss.depth] = jss.val;
        jss.state[jss.depth] = want_colon;
        return true;
      }
      // Fallthrough
    case in_string:
      if (c == '"') {
        json_emit_value();
        return json_pop_state(c);
      }
      if (c == '\\') {
        return json_push_state(in_escape, c);
      }
      /*
      if (c >= 192) {
        // Ignore unicode
        if (jss.val.length() < JSON_MAX_STRLEN) jss.val += '?';
        return json_push_state(in_unicode, c);
      }
      */
      if (jss.val.length() < JSON_MAX_STRLEN) jss.val += c;
      return true;
    case in_escape:
      if (!json_pop_state(c)) return false;
      if (jss.val.length() < JSON_MAX_STRLEN) {
        if (c == 'n') {
          jss.val += '\n';
        } else if (c == 'r') {
          jss.val += '\r';
        } else if (c == 't') {
          jss.val += '\t';
        } else {
          jss.val += c;
        }
      }
      return true;
    /*
    case in_unicode:
      // Ignore unicode
      if (c < 128) return json_pop_state(c);
      return true;
    */
    case in_value:
      if (!(isalnum(c) || c == '.')) {
        json_emit_value();
        if (!json_pop_state(c)) return false;
        jss.pos--;
        return json_parse_char(c);
      }
      if (jss.val.length() < JSON_MAX_STRLEN) jss.val += c;
      return true;
    default:
      return json_err("CANTHAPPEN Unexpected state", c);
  }
}

#endif
