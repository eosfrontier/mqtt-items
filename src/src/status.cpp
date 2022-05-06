#include <Arduino.h>
#include "settings.h"
#include "status.h"
#include "main.h"
#include "avl.h"
#include "api.h"
#include "ntp.h"
#include "msg.h"

unsigned long laststatus = 0;
const char *tstatus = "status";

void check_status()
{
  if ((millis() - laststatus) > STATUS_FREQ) {
    char msgbuf[1024];
    laststatus = millis();
    unsigned long batt = (unsigned long)analogRead(0) * 600 / 1023;
    snprintf(msgbuf, sizeof(msgbuf), "{\"battery\":%ld.%02ld,\"connected\":true,\"load\":%ld.%02ld,\"avl-entries\":[%d,%d],\"queue\":%d,\"timestamp\":%d,\"heap\":%x}",
      batt/100, batt%100, loadavg/100000, (loadavg%100000)/1000, avl_num_entries[0], avl_num_entries[1], api_queue_size(), ntp_now(), ESP.getFreeHeap());
    msg_send(tstatus, msgbuf);
  }
}
