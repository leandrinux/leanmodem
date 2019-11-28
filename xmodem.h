#ifndef xmodem_h
#define xmodem_h

#include "Arduino.h"

void xmodem_recv(Stream *stream, const char *filename);
void xmodem_send(Stream *stream, const char *filename);

#endif
