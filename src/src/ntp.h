#ifndef _NTP_H_
#define _NTP_H_

uint32_t ntp_time(uint32_t time);
uint32_t ntp_now();
void ntp_setup();
void ntp_check();

#endif // _NTP_H_
