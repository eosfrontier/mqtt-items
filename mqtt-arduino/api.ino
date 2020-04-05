#ifdef MQTT_RFID

void api_got_cardid(uint32_t cardid)
{
  Serial.print("Card TODO, got card id "); Serial.println(cardid, HEX);
}

#endif
