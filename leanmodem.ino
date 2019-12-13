// (2019) leanmodem by leandrinux (github.com/leandrinux)
// Modern communications for obsolete devices using the ESP8266-based nodemcu
// Have fun! :)
//
//

// BUILD SETTINGS --------------------------------------------------------------
#define PONG_SUPPORT // comment this out to remove pong from the build
#define TCP_SERVER_SUPPORT // comment this out to remove support for management terminal over TCP

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
#include "ansi.h"
#include "ping.h"
#include "xmodem.h"
#include "datetime.h"

#ifdef PONG_SUPPORT
#include "pong.h"
#endif

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
void cmd_clear(String args);
void cmd_ping(String args);
void cmd_ver(String args);
void cmd_setpin(String args);

#ifdef PONG_SUPPORT
void cmd_pong(String args);
#endif

#ifdef TCP_SERVER_SUPPORT
void cmd_server(String args);
#endif

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
#ifdef PONG_SUPPORT
  { "pong", cmd_pong, "plays a game of pong" },
#endif  
  { "restart", cmd_restart, "restarts the device" },
  { "scan", cmd_scan, "scans for wireless networks" },
#ifdef TCP_SERVER_SUPPORT
  { "server", cmd_server, "turns on/off the tcp server" },
#endif
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
bool tcp_server_active = false;

#ifdef TCP_SERVER_SUPPORT
WiFiServer server(0);
WiFiClient client;
#endif

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
    if (!tcp_server_active && config.echo && (i>0)) stream->print(ch);
  }
  buffer[i] = '\0';
  if (tcp_server_active) {
    stream->read(); // this consumes the LF after CR.
  } else {
    if (config.echo) stream->print(config.unix_eol ? "\n" : "\r\n");    
  }
  return String(buffer);
}

// COMMAND HANDLERS ------------------------------------------------------------

void cmd_help(String args) {
  stream->println("Valid commands are: ");
  int count = sizeof(CommandEntries)/sizeof(CommandEntry);
  for (int i=0; i<count; i++) {
    CommandEntry *entry = (CommandEntry *)&CommandEntries[i];
    stream->printf("%10s   %s\r\n", entry->command.c_str(), entry->detail);
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
    config_info(stream);
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
  WiFi.disconnect();
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
  stream->println();
  Dir dir = SPIFFS.openDir(directory);
  bool empty = true;
  String filename;
  char msg[51];
  while (dir.next()) {
    empty = false;
    filename = dir.fileName().substring(directory.length());
    sprintf(msg, STR_FILES_ENTRY, dir.fileSize(), filename.c_str());    
    stream->println(msg);
  }
  if (empty) stream->println(STR_FILES_EMPTY);
  stream->println();
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  sprintf(msg, STR_FILES_USED_BYTES, fs_info.usedBytes);  
  stream->println(msg);
  sprintf(msg, STR_FILES_TOTAL_BYTES, fs_info.totalBytes);  
  stream->println(msg);
  sprintf(msg, STR_FILES_FREE_BYTES, fs_info.totalBytes - fs_info.usedBytes);  
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
  String sourceArg = args.substring(0, i);
  String targetArg = args.substring(i+1);
  Stream *source = NULL;
  Stream *target = NULL;
  File sourceFile;
  File targetFile;
  WiFiClient sourceClient;
  guard(sourceArg != STR_COPY_STANDARD_OUTPUT, STR_ERROR_INVALID_ARGUMENTS);
  guard(targetArg != STR_COPY_STANDARD_INPUT, STR_ERROR_INVALID_ARGUMENTS);
  if (sourceArg == STR_COPY_STANDARD_INPUT) {
    source = stream;
  } else if (sourceArg.startsWith(STR_COPY_TCP_PREFIX)) {
    guard(isConnected, STR_NETWORK_UNAVAILABLE);
    String uri = sourceArg.substring(String(STR_COPY_TCP_PREFIX).length());
    int i = uri.indexOf(':');
    guard(i >= 0, STR_COPY_PORT_MISSING);
    String host = uri.substring(0, i);
    int port = uri.substring(i+1).toInt();
    guard(sourceClient.connect(host, port), STR_NETWORK_HOST_UNAVAILABLE);
    source = &sourceClient;
  } else if (sourceArg.startsWith(STR_COPY_HTTP_PREFIX)) {
    guard(isConnected, STR_NETWORK_UNAVAILABLE);
    char url[256];
    sourceArg.toCharArray(url, 256);
    guard(http_get(&sourceClient, url), STR_NETWORK_CONNECTION_FAILED);
    source = &sourceClient;
  } else {
    String filename = String(CFG_USER_DIRECTORY) + sourceArg;
    guard(SPIFFS.exists(filename), STR_FILES_NOT_FOUND);
    sourceFile = SPIFFS.open(filename, "r");
    if (sourceFile) source = &sourceFile;
  }
  
  if (targetArg == STR_COPY_STANDARD_OUTPUT) {
    target = stream;
  } else {
    String filename = String(CFG_USER_DIRECTORY) + targetArg;
    targetFile = SPIFFS.open(filename, "w");   
    if (targetFile) target = &targetFile;
  }
  
  guard(source != NULL, STR_ERROR_INVALID_ARGUMENTS);
  guard(target != NULL, STR_ERROR_INVALID_ARGUMENTS);
  source->setTimeout(config.timeout);
  size_t count = 1;
  char buffer[CFG_COPY_BUFFER_SIZE];
  bool usingUserInput = source == stream;
  bool usingUserOutput = target == stream;
  bool user_stopped = false;
  long bytes_copied = 0;
  digitalWrite(CFG_ACTIVITY_LED_PIN, HIGH);
  while ((count > 0) && !user_stopped) {
    user_stopped = !usingUserInput && stream->available() && (stream->read() == CFG_STOP_KEY);
    if (!usingUserOutput) stream->print(String(bytes_copied)+" bytes copied.\r");
    count = source->readBytes(buffer, CFG_COPY_BUFFER_SIZE);
    target->write(buffer, count);
    bytes_copied += count;
  }
  digitalWrite(CFG_ACTIVITY_LED_PIN, LOW);
  source->setTimeout(1000);
  sourceFile.close();
  targetFile.close();
  if (user_stopped) stream->println(STR_STOPPED);
  if (!usingUserOutput) stream->println();
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
  stream->println(getFullFormattedTime(&timeClient));
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
  bool user_stopped = false;
  digitalWrite(CFG_ACTIVITY_LED_PIN, HIGH);  
  while (count && !user_stopped) {
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
    user_stopped = stream->available() && (stream->read() == CFG_STOP_KEY);
    count = file.readBytes(buffer, sizeof(buffer));
  }
  if (user_stopped) {
    stream->println(STR_STOPPED); 
  } else {
    sprintf(str, "%08X", offset);  
    stream->println(String(str));  
  }
  digitalWrite(CFG_ACTIVITY_LED_PIN, LOW);
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

#ifdef PONG_SUPPORT
void cmd_pong(String args) {
  guard(config.ansi, STR_NOT_SUPPORTED);
  pong_start(stream);
}
#endif

void cmd_clear(String args) {
  guard(config.ansi, STR_NOT_SUPPORTED);
  ansi_clear();
  ansi_goto(1, 1);  
}

void cmd_ping(String args) {
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  guard(isConnected, STR_NETWORK_UNAVAILABLE);
  ping(stream, (char *)args.c_str()); 
}

void cmd_ver(String args) {
  stream->println(STR_SYSTEM_ABOUT);
  stream->println(STR_SYSTEM_BUILD);
}

void cmd_setpin(String args) {
  const byte valid_pins[] = { 0, 2, 5, 4, 14, 12, 13 };
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

#ifdef TCP_SERVER_SUPPORT
void cmd_server(String args) {
  guard(isConnected, STR_NETWORK_UNAVAILABLE);
  guard(args != "", STR_ERROR_INVALID_ARGUMENTS);
  if (args == STR_USER_INPUT_ON) {
    if (tcp_server_active) {
      stream->println(STR_SERVER_ALREADY_ON);
    } else {
      char msg[81];
      stream->println(STR_SERVER_DISCLAIMER);
      sprintf(msg, STR_SERVER_STARTING_UP, WiFi.localIP().toString().c_str(), config.port);
      stream->println(msg);
      server.begin(config.port);
      tcp_server_active = true;      
    }
    return;
  }
  if (args == STR_USER_INPUT_OFF) {
    if (tcp_server_active) {
      stream->println(STR_SERVER_GOODBYE);
      client.stop();
      server.begin(0);
      tcp_server_active = false;
    } else {
      stream->println(STR_SERVER_ALREADY_OFF);
    }
    return;
  }
  stream->println(STR_ERROR_INVALID_ARGUMENTS);
}
#endif

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
  
  sound_setPin(CFG_BUZZER_PIN);
  stream->println(STR_SYSTEM_INIT_FS);
  SPIFFS.begin();
  SPIFFS.gc();
  cmd_config(STR_CONFIG_ARG_LOAD);
  if (config.autoconnect && *config.ssid) {
    stream->println(STR_SYSTEM_AUTOCONNECT);
    cmd_connect("");
    #ifdef TCP_SERVER_SUPPORT
    if (config.tcpserver && isConnected) {
      cmd_server(STR_USER_INPUT_ON);
    }
    #endif
  }
  stream->println(STR_SYSTEM_POST);  
}

void process_command(Stream *s) {
  stream = s;
  ansi_set_stream(s);
  if (config.echo) {
    stream->print(config.id);
    stream->print(STR_SYSTEM_PROMPT);
  }
  String userInput = readln();
  if (userInput != "") parseCommand(userInput);  
}

// INITIALIZATION AND MAIN LOOP ------------------------------------------------

void setup() {
  boot();
}

void loop() {
#ifdef TCP_SERVER_SUPPORT
  if (tcp_server_active) {
    if (client) {
      process_command(&client);                 
    } else {
      client = server.available();   
    }
    if (Serial.available() && (Serial.read() == CFG_STOP_KEY)) {
      cmd_server(STR_USER_INPUT_OFF);
    }
 } else {
  process_command(&Serial);           
 }
#else
  process_command(&Serial);        
#endif  
}
