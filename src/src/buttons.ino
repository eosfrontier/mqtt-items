
void buttons_setup()
{
  for (int i = 0; i < BUTTONS_NUM; i++) {
    pinMode(BUTTONS_PINS[i], INPUT_PULLUP);
  }
}

int buttons_last = 0;
int buttons_retry = 0;
int buttons_send = 0;

void buttons_check()
{
  char msgbuf[1025];
  int btset = 0;
  for (int i = 0; i < BUTTONS_NUM; i++) {
    if (digitalRead(BUTTONS_PINS[i]) == LOW) {
      btset |= 1 << i;
    }
  }
  if (btset & ~buttons_last) {
    buttons_retry = BUTTON_RETRIES * BUTTON_RETRY_DELAY + 1;
    buttons_send = btset;
  }
  if (buttons_retry > 0) {
    int idx = sprintf(msgbuf, "button/");
    for (int i = 0; i < BUTTONS_NUM; i++) {
      if (buttons_send & (1 << i)) {
        if ((idx + strlen(BUTTONS_NAMES[i])) > 1024) {
          Serial.print("Button message too long! <<<");
          Serial.write(msgbuf, idx);
          Serial.println(">>>");
          return;
        }
        strcpy(msgbuf+idx, BUTTONS_NAMES[i]);
        idx += strlen(BUTTONS_NAMES[i]);
        msgbuf[idx++] = '_';
      }
    }
    msgbuf[--idx] = 0;
    buttons_retry--;
    if (!(buttons_retry % BUTTON_RETRY_DELAY)) {
      Serial.print("Sending button press "); Serial.print(msgbuf); Serial.print(" try "); Serial.println(buttons_retry);
      msg_send(msgbuf, state);
    }
  }
  buttons_last = btset;
}

// Als er een bericht is binnengekomen met als effect dat onze state verandert,
// gaan we er vanuit dat onze laatste knopdruk goed is aangekomen,
// en dan sturen we onze buttonpress niet opnieuw.
// Nogal een hack ivm race conditions (meer knopdrukken na elkaar)
// Maar voor 99% van de gevallen afdoende
void buttons_ack()
{
  buttons_retry = 0;
}
