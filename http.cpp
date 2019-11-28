#include "http.h"
#include "string.h"
#include "stdlib.h"

unsigned short int default_port(const char scheme[6]) {
  if (!strcmp(scheme, "http")) return 80;
  return 0;
}

bool split_url(UrlComponents *components, const char *url) {
  char *a, *b;
  memset(components, 0, sizeof(UrlComponents));
  if (strlen(url) > 150) return false;
  a = strchr(url, ':'); // look for the : right before http:// or ftp://
  if (!a) return false;
  strncpy(components->scheme, url, a - url);
  if ((a[1] != '/') || (a[2] != '/')) return false;
  a += 3;
  b = strchr(a, ':'); // look for a : indicating a port was explicitly specified
  if (b) {
    strncpy(components->host, a, b-a);
    a = b + 1;
    b = strchr(a, '/'); 
    char portstr[6];
    strncpy(portstr, a, (b) ? b-a : sizeof(portstr));
    components->port = atoi(portstr); 
  } else {
    b = strchr(a, '/');
    strncpy(components->host, a, (b) ? b-a : sizeof(components->host));
  }
  strncpy(components->path, b, sizeof(components->path));
  return true;  
}

bool http_get(WiFiClient *client, const char *url) {
  return http_request(client, url, "GET");
}

bool http_request(WiFiClient *client, const char *url, const char *method) {
  UrlComponents comp;
  if (!split_url(&comp, url)) return false;
  if (!comp.port) comp.port = default_port(comp.scheme);
  if (!comp.port) return false;
  if (!client->connect(comp.host, comp.port)) return false;
  client->println("GET " + String(comp.path) + " HTTP/1.0");
  client->println("Host: " + String(comp.host));
  client->println("Accept-Encoding: identity");
  client->println();
  String header = client->readStringUntil('\n');
  if ((header != "HTTP/1.0 200 OK\r") && (header != "HTTP/1.1 200 OK\r")) return false;
  while (client->readStringUntil('\n') != "\r");
  return true;
}
