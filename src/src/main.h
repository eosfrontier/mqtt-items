#ifndef _MAIN_H_
#define _MAIN_H_

extern unsigned long loadavg;
extern unsigned long lasttick;
extern const char *state;
extern int api_check_status;
void serprintf(const char *fmt, ...);

#endif // _MAIN_H_
