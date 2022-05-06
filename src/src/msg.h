#ifndef _MSG_H_
#define _MSG_H_

void msg_setup();
void msg_send(const char *topic, const char *msg);
void msg_check();
#endif // _MSG_H_
