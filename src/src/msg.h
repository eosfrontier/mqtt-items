#ifndef _MSG_H_
#define _MSG_H_

const unsigned int mqtt_port = 1883;

void msg_setup();
void msg_send(const char *topic, const char *msg);
void msg_check();
void msg_receive(const char *topic, const char *msg);

#endif // _MSG_H_
