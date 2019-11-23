#define SND_BEEP 1
#define SND_GOOD 2
#define SND_ERROR 3
#define SND_ALERT 4
#define SND_CONNECTED 5

#define guard(c,s) if(!(c)){writeln(s);return;}
#define ntohl(n) ((((n&0xFF))<< 24)|(((n&0xFF00))<<8)|(((n&0xFF0000))>>8)|(((n&0xFF000000))>>24))
