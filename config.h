#ifndef config_h
#define config_h

#include "Arduino.h"

#define CFG_ACTIVITY_LED_PIN 2
#define CFG_BUZZER_PIN 15
#define CFG_CONSOLE_BAUD_RATE 19200
#define CFG_STARTUP_DELAY 10
#define CFG_CONFIG_FILENAME "/s/config.ini"
#define CFG_USER_DIRECTORY "/u/"
#define CFG_SYSTEM_DIRECTORY "/s/"
#define CFG_TELNET_BREAK_KEYCODE 27
#define CFG_TELNET_BUFFER_SIZE 2048
#define CFG_NTP_SERVER "pool.ntp.org"
#define CFG_COPY_BUFFER_SIZE 1024
#define CFG_STOP_KEY 'q'

typedef struct {
  char ssid[31];
  char pass[31];
  char id[11];
  short int timezone;
  short int timeout;
  short unsigned int port;
  bool unix_eol;
  bool sound;
  bool ansi;
  bool echo;
  bool autoconnect;
  bool tcpserver;
} Config;

typedef void DidConfigChangeHandler(const char[12]);

extern Config config;

void config_save();
void config_load();
void config_info(Stream *stream);
bool config_set(String args, DidConfigChangeHandler didConfigChange);

#endif
