#ifndef other_h
#define other_h

#define XMODEM_SOH 0x01 // start of header
#define XMODEM_EOT 0x04 // end of transmission
#define XMODEM_ACK 0x06 // acknowledge
#define XMODEM_NAK 0x15 // not acknowledge
#define XMODEM_ETB 0x17 // end of transmission block
#define XMODEM_CAN 0x18 // cancel
#define XMODEM_C 0x43   // ascii c
#define XMODEM_BUFFER_SIZE 20
#define XMODEM_MAX_RETRIES 6

#define guard(c,str) if(!(c)){writeln(str);return;}
#define ntohl(n) ((((n&0xFF))<< 24)|(((n&0xFF00))<<8)|(((n&0xFF0000))>>8)|(((n&0xFF000000))>>24))

#endif
