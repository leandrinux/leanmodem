#include "config.h"
#include <FS.h>
#include "strings.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>

#define CFG_STRING(val) ((*val) ? val : STR_CONFIG_NOT_SET)
#define CFG_BOOL(val) ((val) ? STR_YES : STR_NO)

Config config = { "", "", "default", 0, 3000, false, true, true, true, true };
bool isConfigNotSaved = false;

void config_save() {
  File file = SPIFFS.open(CFG_CONFIG_FILENAME, "w");
  file.write((byte *)&config, sizeof(config));
  file.close();
  isConfigNotSaved = false;
}

void config_load() {
  if (!SPIFFS.exists(CFG_CONFIG_FILENAME)) return;
  File file = SPIFFS.open(CFG_CONFIG_FILENAME, "r");
  file.read((byte *)&config, sizeof(config));
  file.close();
}

void stream_printf(Stream *stream, const char *fmt, ...) {
  char str[50];
  sprintf(str, fmt);
  stream->print(str);
}

void config_info(Stream *stream) {
  bool isConnected = WiFi.status() == WL_CONNECTED;
  stream_printf(stream, "SYSTEM INFO\r\n");
  stream_printf(stream, "  Device ID  : %s\r\n", CFG_STRING(config.id));
  stream_printf(stream, "  Timezone   : %d\r\n", config.timezone);
  stream_printf(stream, "  Uptime     : %d seconds\r\n", millis()/1000);
  stream_printf(stream, "  Build Date : %s\r\n", __TIMESTAMP__);
  stream_printf(stream, "\r\n");
  stream_printf(stream, "NETWORK INFO\r\n");
  stream_printf(stream, "  SSID     : %s\r\n", CFG_STRING(config.ssid));
  stream_printf(stream, "  Password : %s\r\n", CFG_STRING(config.pass));
  stream_printf(stream, "  WiFi     : %s\r\n", CFG_BOOL(isConnected));
  stream_printf(stream, "  Timeout  : %d\r\n", config.timeout);
  stream_printf(stream, "\r\n");
  stream_printf(stream, "SYSTEM FLAGS\r\n");
  stream_printf(stream, "  Sound       : %s\r\n", CFG_BOOL(config.sound));
  stream_printf(stream, "  ANSI Codes  : %s\r\n", CFG_BOOL(config.ansi));  
  stream_printf(stream, "  User Echo   : %s\r\n", CFG_BOOL(config.echo));
  stream_printf(stream, "  Autoconnect : %s\r\n", CFG_BOOL(config.autoconnect));
  stream_printf(stream, "\r\n");
  if (isConfigNotSaved) {
    stream->println("\r\nYou made configuration changes, use 'config save' to make them permanent.");
  }
}

bool config_set(String args, DidConfigChangeHandler didConfigChange) {
  int i = args.indexOf(' ');
  if (i != -1) {
    String command = args.substring(0, i);
    if (command == "set") {
      String temp = args.substring(i+1);
      i = temp.indexOf(' ');
      String propertyName = (i<0) ? temp : temp.substring(0, i);
      String propertyValue = (i<0) ? "" : temp.substring(i+1);
      enum FieldType { string = 0, number = 1, boolean = 2 };
      const int q = 10;
      const struct {
        char propertyName[12];
        void *pointer;
        FieldType fieldType;
      } SETTINGS[q] = {
        { "id", &config.id, string },
        { "ssid", &config.ssid, string },
        { "pass", &config.pass, string },
        { "sound", &config.sound, boolean },
        { "ansi", &config.ansi, boolean },
        { "echo", &config.echo, boolean },
        { "autoconnect", &config.autoconnect, boolean },
        { "unix", &config.unix_eol, boolean },
        { "timezone", &config.timezone, number },
        { "timeout", &config.timeout, number }
      };
      int j = 0;
      while ((j < q) && (String(SETTINGS[j].propertyName) != propertyName)) { j++; };
      if (j < q) {
        switch (SETTINGS[j].fieldType) {
          case string:
            propertyValue.toCharArray((char *)SETTINGS[j].pointer, 30);
            if (didConfigChange) didConfigChange(SETTINGS[j].propertyName);
            isConfigNotSaved = true;
            return true;
          case number:
            *(short int *)SETTINGS[j].pointer = propertyValue.toInt();
            if (didConfigChange) didConfigChange(SETTINGS[j].propertyName);
            isConfigNotSaved = true;
            return true;
          case boolean:
            if ( (propertyValue != STR_USER_INPUT_YES) && (propertyValue != STR_USER_INPUT_NO)) break;
            *(char *)SETTINGS[j].pointer = propertyValue == STR_USER_INPUT_YES;
            if (didConfigChange) didConfigChange(SETTINGS[j].propertyName);
            isConfigNotSaved = true;
            return true;
        }
      }
    }
  }
  return false;
}
