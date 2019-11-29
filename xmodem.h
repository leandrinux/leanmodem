#ifndef xmodem_h
#define xmodem_h

#include "Arduino.h"

// xmodem-crc packet
typedef struct {
  byte start_of_header;
  byte packet_number;
  byte packet_number_chk;
  byte data[128];
  word crc;
}  __attribute__((packed)) XmodemPacket;

void xmodem_recv(Stream *stream, const char *filename);
void xmodem_send(Stream *stream, const char *filename);

#endif
