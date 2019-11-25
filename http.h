#ifndef http_h
#define http_h

#include <ESP8266WiFi.h>

typedef struct {
  char path[101];
  char host[41];
  char scheme[6];
  unsigned short int port;
}  UrlComponents;

unsigned short int default_port(const char *scheme[6]);
bool split_url(UrlComponents *components, const char *url);
bool http_get(WiFiClient *client, const char *url);
bool http_request(WiFiClient *client, const char *url, const char *method); 

#endif
