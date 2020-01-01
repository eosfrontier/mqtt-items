unsigned long laststatus = 0;
const char *tstatus = "status";

void check_status()
{
  if ((millis() - laststatus) > STATUS_FREQ) {
    char msgbuf[1024];
    laststatus = millis();
    unsigned long batt = (unsigned long)analogRead(0) * 600 / 1023;
    sprintf(msgbuf, "{\"battery\":%d.%02d,\"connected\":true,\"load\":%d.%02d}", batt/100, batt%100, loadavg/100000, (loadavg%100000)/1000);
    msg_send(tstatus, msgbuf);
  }
}
