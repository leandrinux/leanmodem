// (2019) leanmodem by leandrinux (github.com/leandrinux)
// Modern communications for obsolete devices using the ESP8266-based nodemcu
// Have fun! :)
//
//

// DEPENDENCIES ----------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <FS.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <SHA256.h>

#include "strings.h"
#include "config.h"
#include "other.h"
#include "sound.h"
#include "http.h"
#include "pong.h"
#include "ansi.h"
#include "ping.h"
#include "xmodem.h"

// TYPE DEFINITIONS ------------------------------------------------------------
typedef void (*CommandHandler)(String args);

typedef struct {
  String command;
  CommandHandler handler;
  char detail[60];
}  CommandEntry;

// FUNCTION DECLARATIONS -------------------------------------------------------
void cmd_help(String args);
void cmd_connect(String args);
void cmd_scan(String args);
void cmd_telnet(String args);
void cmd_config(String args);
void cmd_restart(String args);
void cmd_files(String args);
void cmd_format(String args);
void cmd_xmodem_recv(String args);
void cmd_xmodem_send(String args);
void cmd_copy(String args);
void cmd_time(String args);
void cmd_erase(String args);
void cmd_hexdump(String args);
void cmd_sha256(String args);
void cmd_pong(String args);
void cmd_clear(String args);
void cmd_ping(String args);
void cmd_ver(String args);
void cmd_setpin(String args);

// CONSTANTS -------------------------------------------------------------------
const CommandEntry CommandEntries[] = {
  { "clear", cmd_clear, "clears the screen" },
  { "config", cmd_config, "manages device settings like ssid, pass, and others"},
  { "connect", cmd_connect, "connects device to wifi network" },
  { "copy", cmd_copy, "copies data from sources like files, tcp sockets or http" },
  { "erase", cmd_erase, "erases a user file" },
  { "files", cmd_files, "lists all stored user files" },
  { "format", cmd_format, "erases all info stored in device" },
  { "help", cmd_help, "the command you just used :)" },
  { "hexdump", cmd_hexdump, "dumps context of file as hexadecimal info" },
  { "ping", cmd_ping, "pings a remote server" },
  { "pong", cmd_pong, "plays a game of pong" },
  { "restart", cmd_restart, "restarts the device" },
  { "scan", cmd_scan, "scans for wireless networks" },
  { "setpin", cmd_setpin, "turns on/off a digital pin on the device" },
  { "sha256", cmd_sha256, "calculates the sha256 hash of a user file" },
  { "telnet", cmd_telnet, "connects to telnet hosts"},
  { "time", cmd_time, "queries an ntp server and displays current time" },
  { "ver", cmd_ver, "returns the build version and about info" },
  { "xrecv", cmd_xmodem_recv, "receives binary files using the xmodem-crc protocol" },
  { "xsend", cmd_xmodem_send, "sends a binary file using the xmodem-crc protocol" }
};

// GLOBAL VARIABLES ------------------------------------------------------------
bool isConnected = false;
Stream *stream = &Serial; 

// FUNCTION IMPLEMENTATIONS ----------------------------------------------------

String readln() { 
  #define KEY_BACKSPACE 0x7f
  const int BUFFER_SIZE = 256; 
  char buffer[BUFFER_SIZE];
  char ch = '\0';
  byte i = 0;
  while ((ch != '\n') && (ch != '\r') && (i < 255)) {
    if (!stream->available()) continue;
    stream->readBytes(&ch, 1);
    if ((ch == '\n') || (ch == '\r')) continue;
    if ((ch == KEY_BACKSPACE)) {
      buffer[i] = '\0';
      if (i) i--;
    } else {
      buffer[i] = ch;
      i++;          
    }
    if (config.echo && (i>0)) Serial.print(ch);
  }
  buffer[i] = '\0';
  if (config.echo) stream->print(config.unix_eol ? "\n" : "\r\n");
  return String(buffer);
}

// COMMAND HANDLERS ------------------------------------------------------------

void cmd_help(String args) {
  stream->println("Valid commands are: ");
  int count = sizeof(CommandEntries)/sizeof(CommandEntry);
  for (int i=0; i<count; i++) {
    CommandEntry *entry = (CommandEntry *)&CommandEntries[i];
    Serial.printf("%10s   %s\r\n", entry->command.c_str(), entry->detail);
  }
}

void cmd_connect(String args) {
  guard(*config.ssid, STR_NETWORK_SSID_MISSING);
  WiFi.begin(config.ssid, config.pass);
  while (1)  {
    byte status = WiFi.status();
    guards(status != WL_NO_SSID_AVAIL, STR_NETWORK_SSID_UNREACHABLE, SND_ERROR); 
    if (status == WL_CONNECTED) {
      sound(SND_CONNECTED);
      String msg = String(STR_NETWORK_CONNECTED_TO);
      msg.replace("%s1", config.ssid);
      msg.replace("%s2", WiFi.localIP().toString());
      stream->println(msg);
      isConnected = true;
      break;
    }
    guards(status != WL_CONNECT_FAILED, STR_NETWORK_CONNECTION_FAILED, SND_ERROR);
    delay(100);
  }
}

void cmd_scan(String args) {
  stream->println(STR_NETWORK_SCANNING);
  int count = WiFi.scanNetworks();
  stream->println(STR_NETWORK_FOUND + String(count));
  for (int i = 0; i < count; i++) {
    stream->println("   '" + String(WiFi.SSID(i).c_str()) + "', Ch:" + String(WiFi.channel(i)) + ", rssi:" + String(WiFi.RSSI(i)));
  }
  WiFi.scanDelete();
}

void cmd_config(String args) {  
  if (args == STR_CONFIG_ARG_SAVE) {
    config_save();
    return;
  }
  if (args == STR_CONFIG_ARG_LOAD) {
    config_load();
    didConfigChange(NULL);    
    return;
  }
  if (args == "") {
    config_info(&Serial);
    return;
  }
  if (config_set(args, &didConfigChange)) return;
  stream->println(STR_ERROR_INVALID_ARGUMENTS);
}

void didConfigChange(const char propertyName[12]) {
  sound_setMute(!config.sound);
}

void cmd_telnet(String args) {
  guard(isConnected, STR_NETWORK_UNAVAILABLE);
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  String host = "";
  int port = 23;
  int i = args.indexOf(':');
  if (i != -1) {
    port = args.substring(i+1).toInt();
    host = args.substring(0, i);
  } else {
    host = args;
  }  
  WiFiClient client;
  guards(client.connect(host, port), STR_NETWORK_CONNECTION_FAILED, SND_ERROR);
  int key = -1;
  byte buffer[CFG_TELNET_BUFFER_SIZE];
  int count = 0;
  while ((key != CFG_TELNET_BREAK_KEYCODE) && (client.connected() || client.available())) {      
    if (client.available()) {
      count = client.readBytes(buffer, CFG_TELNET_BUFFER_SIZE);
      stream->write(buffer, count);        
    }
    if (stream->available()) {
      count = stream->readBytes(buffer, CFG_TELNET_BUFFER_SIZE);
      client.write(buffer, count);
      key = (count > 0) ? int(buffer[0]) : -1; 
    }
  }
  client.stop();
  delay(1);
  stream->println();
  stream->println(STR_NETWORK_DISCONNECTED);
  sound(SND_BEEP);
}

void cmd_restart(String args) {
  stream->println(STR_SYSTEM_WILL_RESTART);
  delay(500);
  ESP.restart();
}

void cmd_files(String args) {
  String directory;
  if (args == STR_FILES_ARG_SYSTEM) {
    stream->println(STR_FILES_SYSTEM_TITLE);
    directory = CFG_SYSTEM_DIRECTORY;
  } else {
    stream->println(STR_FILES_USER_TITLE);
    directory = CFG_USER_DIRECTORY;    
  }
  Dir dir = SPIFFS.openDir(directory);
  bool empty = true;
  String msg, filename;
  while (dir.next()) {
    empty = false;
    msg = String(STR_FILES_ENTRY);
    filename = dir.fileName().substring(directory.length());
    msg.replace("%s", filename);
    msg.replace("%d", String(dir.fileSize()));
    stream->println(msg);
  }
  if (empty) stream->println(STR_FILES_EMPTY);
  stream->println();
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  msg = String(STR_FILES_TOTAL_BYTES);
  msg.replace("%d", String(fs_info.totalBytes));
  stream->println(msg);
  msg = String(STR_FILES_USED_BYTES);
  msg.replace("%d", String(fs_info.usedBytes));
  stream->println(msg);
  msg = String(STR_FILES_FREE_BYTES);
  msg.replace("%d", String(fs_info.totalBytes - fs_info.usedBytes));
  stream->println(msg);    
}

void cmd_format(String args) {
  sound(SND_ALERT);
  stream->print(STR_FORMAT_PROMPT);
  String userInput = readln();
  guard(userInput == STR_USER_INPUT_YES, STR_FORMAT_CANCELED);
  stream->println(STR_WAIT);
  bool success = SPIFFS.format();
  stream->println(success ? STR_FORMAT_OK : STR_FORMAT_FAILED);
  SPIFFS.end();
  cmd_restart("");
}

void cmd_copy(String args) {
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  int i = args.indexOf(' ');
  guard(i != -1, STR_ERROR_INVALID_ARGUMENTS);
  String inputArg = args.substring(0, i);
  String streamArg = args.substring(i+1);
  Stream *input = NULL;
  Stream *stream = NULL;
  File inputFile;
  File streamFile;
  WiFiClient inputClient;
  guard(inputArg != STR_COPY_STANDARD_OUTPUT, STR_ERROR_INVALID_ARGUMENTS);
  guard(streamArg != STR_COPY_STANDARD_INPUT, STR_ERROR_INVALID_ARGUMENTS);
  
  if (inputArg == STR_COPY_STANDARD_INPUT) {
    input = &Serial;
  } else if (inputArg.startsWith(STR_COPY_TCP_PREFIX)) {
    guard(isConnected, STR_NETWORK_UNAVAILABLE);
    String uri = inputArg.substring(String(STR_COPY_TCP_PREFIX).length());
    int i = uri.indexOf(':');
    guard(i >= 0, STR_COPY_PORT_MISSING);
    String host = uri.substring(0, i);
    int port = uri.substring(i+1).toInt();
    guard(inputClient.connect(host, port), STR_NETWORK_HOST_UNAVAILABLE);
    input = &inputClient;
  } else if (inputArg.startsWith(STR_COPY_HTTP_PREFIX)) {
    guard(isConnected, STR_NETWORK_UNAVAILABLE);
    char url[256];
    inputArg.toCharArray(url, 256);
    guard(http_get(&inputClient, url), STR_NETWORK_CONNECTION_FAILED);
    input = &inputClient;
  } else {
    String filename = String(CFG_USER_DIRECTORY) + inputArg;
    guard(SPIFFS.exists(filename), STR_FILES_NOT_FOUND);
    inputFile = SPIFFS.open(filename, "r");
    if (inputFile) input = &inputFile;
  }
  
  if (streamArg == STR_COPY_STANDARD_OUTPUT) {
    stream = &Serial;
  } else {
    String filename = String(CFG_USER_DIRECTORY) + streamArg;
    streamFile = SPIFFS.open(filename, "w");   
    if (streamFile) stream = &streamFile;
  }
  
  guard(input != NULL, STR_ERROR_INVALID_ARGUMENTS);
  guard(stream != NULL, STR_ERROR_INVALID_ARGUMENTS);
  input->setTimeout(config.timeout);
  size_t count = 1;
  char buffer[CFG_COPY_BUFFER_SIZE];
  digitalWrite(CFG_ACTIVITY_LED_PIN, HIGH);
  while (count > 0) {
    count = input->readBytes(buffer, CFG_COPY_BUFFER_SIZE);
    stream->write(buffer, count);
  }
  digitalWrite(CFG_ACTIVITY_LED_PIN, LOW);
  input->setTimeout(1000);
  inputFile.close();
  streamFile.close();
}

void cmd_xmodem_recv(String args) {
  xmodem_recv(&Serial, (const char*)args.c_str());
}

void cmd_xmodem_send(String args) {
  xmodem_send(&Serial, (const char*)args.c_str());  
}

void cmd_time(String args) {
  guard(isConnected, STR_NETWORK_UNAVAILABLE);
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, CFG_NTP_SERVER, config.timezone * 3600);
  timeClient.begin();
  timeClient.update();
  stream->println(timeClient.getFullFormattedTime());
  timeClient.end();
}

void cmd_erase(String args) {
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  String filename = CFG_USER_DIRECTORY + args;
  guard(SPIFFS.exists(filename), STR_FILES_NOT_FOUND);
  stream->println(SPIFFS.remove(filename) ? STR_FILES_ERASE_SUCCESS : STR_ERROR_UNKNOWN);
}

void cmd_hexdump(String args) {
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  String filename = CFG_USER_DIRECTORY + args;
  guard(SPIFFS.exists(filename), STR_FILES_NOT_FOUND);
  #define CFG_HEXDUMP_LENGTH 16
  char buffer[CFG_HEXDUMP_LENGTH];
  char str[9];
  File file = SPIFFS.open(filename, "r");
  int count = file.readBytes(buffer, CFG_HEXDUMP_LENGTH);
  int offset = 0;
  byte val;
  digitalWrite(CFG_ACTIVITY_LED_PIN, HIGH);  
  while (count) {
    sprintf(str, "%08X", offset);
    stream->print(String(str) + ": ");  
    offset += count;
    for (byte i=0;i<count;i++) {
      val = buffer[i];
      sprintf(str, "%02X", val);
      stream->print(str);
      if ((i+1)%4==0) stream->print(" ");
    }
    val = 36 - (count * 2 + count / 4);
    while (val>0) { stream->print(" "); val--; }
    stream->print("|");
    for (byte i=0;i<count;i++) {
      val = buffer[i];
      stream->print(((val>=0x20)&&(val<=0x7e)) ? String((char)val) : ".");
    }
    stream->print("|");
    stream->println();
    count = file.readBytes(buffer, sizeof(buffer));
  }
  sprintf(str, "%08X", offset);
  digitalWrite(CFG_ACTIVITY_LED_PIN, LOW);
  stream->println(String(str));  
  file.close();
}

void cmd_sha256(String args) {
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  String filename = CFG_USER_DIRECTORY + args;
  guard(SPIFFS.exists(filename), STR_FILES_NOT_FOUND);
  File file = SPIFFS.open(filename, "r");
  const int BUFFER_SIZE = 1024;
  char buffer[BUFFER_SIZE];
  int count = file.readBytes(buffer, BUFFER_SIZE);
  SHA256 sha;
  while (count) {
    sha.update(&buffer, count);
    count = file.readBytes(buffer, BUFFER_SIZE);
  }
  int size = sha.hashSize();
  byte hash[size];
  char str[3];
  sha.finalize(hash, size);
  for (int i=0;i<size;i++) {
    sprintf(str, "%02X", hash[i]);
    stream->print(str);
  }
  stream->println();
  file.close();
}

void cmd_pong(String args) {
  guard(config.ansi, STR_NOT_SUPPORTED);
  pong_start(&Serial);
}

void cmd_clear(String args) {
  guard(config.ansi, STR_NOT_SUPPORTED);
  ansi_clear();
  ansi_goto(1, 1);  
}

void cmd_ping(String args) {
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  guard(isConnected, STR_NETWORK_UNAVAILABLE);
  ping(&Serial, (char *)args.c_str()); 
}

void cmd_ver(String args) {
  stream->println(STR_SYSTEM_ABOUT);
  stream->println(STR_SYSTEM_BUILD);
}

void cmd_setpin(String args) {
  const byte valid_pins[] = { 5, 4, 0, 2, 14, 12, 13 };
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  int i = args.indexOf(' ');
  guard(i != -1, STR_ERROR_INVALID_ARGUMENTS);
  byte pin_number = args.substring(0, i).toInt();
  guard(i > 0, STR_ERROR_INVALID_ARGUMENTS);
  String setting = args.substring(i+1);
  guard((setting == STR_USER_INPUT_YES) || (setting == STR_USER_INPUT_NO), STR_ERROR_INVALID_ARGUMENTS);
  for (byte i=0; (i<sizeof(valid_pins)) && (valid_pins[i] != pin_number); i++);
  guard(i < sizeof(valid_pins), STR_SET_PIN_NOT_ALLOWED);
  pinMode(pin_number, OUTPUT);
  digitalWrite(pin_number, (setting == STR_USER_INPUT_YES) ? HIGH : LOW);
  char msg[21];
  sprintf(msg, STR_SET_PIN_OK, pin_number, setting.c_str());
  stream->println(msg);
}

CommandHandler findCommand(String cmd) {
  int i = 0;
  int count = sizeof(CommandEntries)/sizeof(CommandEntry);
  while ((i<count) && (!cmd.equals(CommandEntries[i].command))) { i++; }
  return (i<count) ? CommandEntries[i].handler : NULL;
}

void parseCommand(String userInput) {
  String cmd;
  String args;
  int i = userInput.indexOf(' ');
  if (i != -1) {
    cmd = userInput.substring(0,i);
    args = userInput.substring(i+1);
  } else {
    cmd = userInput;
    args = "";
  }
  cmd.replace("\r","");
  CommandHandler handler = findCommand(cmd);
  if (handler == NULL) {
    String errorMessage = String(STR_ERROR_COMMAND_UNRECOGNIZED);
    errorMessage.replace("%s", cmd);
    stream->println(errorMessage);
    return;
  }
  handler(args);
}

void boot() {
  Serial.begin(CFG_CONSOLE_BAUD_RATE);
  delay(CFG_STARTUP_DELAY);
  stream->println(STR_SYSTEM_PRE);
  pinMode(CFG_ACTIVITY_LED_PIN, OUTPUT);
  digitalWrite(CFG_ACTIVITY_LED_PIN, LOW);
  ansi_set_stream(&Serial);
  sound_setPin(CFG_BUZZER_PIN);
  SPIFFS.begin();
  SPIFFS.gc();
  cmd_config(STR_CONFIG_ARG_LOAD);
  if (config.autoconnect && *config.ssid) cmd_connect("");
  stream->println(STR_SYSTEM_POST);  
}

// INITIALIZATION AND MAIN LOOP ------------------------------------------------

void setup() {
  boot();
}

void loop() {
  if (config.echo) Serial.print(STR_SYSTEM_PROMPT);
  String userInput = readln();
  if (userInput != "") parseCommand(userInput);
}
