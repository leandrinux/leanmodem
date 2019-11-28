#include "xmodem.h"

#define CFG_XMODEM_TIMEOUT 2000

void xmodem_recv(Stream *stream, const char *filename) {
/*
  // send C's until there's a response
  // until not eot 
  //  receive packet
  //  respond with ack
  // transfer complete

  Serial.setTimeout(3000);
  while (!Serial.available()) {
    sound(SND_BEEP);
    Serial.print((char)XMODEM_C);
    delay(1000);
  }
  byte code = 0;
  byte error_count = 0;
  XmodemPacket packet;
  while ( (error_count < 5) && (code != XMODEM_EOT)) {
    size_t count = Serial.readBytes((char *)&packet, sizeof(packet));
    tone(CFG_BUZZER_PIN, 1500, 50); delay(50);
    if (count == sizeof(packet)) {
      code = packet.start_of_header;
      Serial.write(XMODEM_ACK);
      sound(SND_GOOD);
    } else {
      sound(SND_ERROR);
      error_count++;
    }
  }
  Serial.setTimeout(1000);
*/
}

void xmodem_send(Stream *stream, const char *filename) {
  
}
