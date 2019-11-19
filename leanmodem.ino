// (2019) leanmodem by leandrinux (github.com/leandrinux)
// Modern communications for obsolete devices using the ESP8266-based nodemcu
// Have fun! :)
//
//

// DEPENDENCIES ----------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <Regexp.h>
#include <FS.h>

// CONFIGURATION ---------------------------------------------------------------
#define CFG_CONSOLE_BAUD_RATE 1200
#define CFG_STARTUP_DELAY 10
#define CFG_BUZZER_PIN 15
#define CFG_CONFIG_FILENAME "/s/config.ini"
#define CFG_USER_DIRECTORY "/u/"
#define CFG_SYSTEM_DIRECTORY "/s/"

// STRINGS ---------------------------------------------------------------------
#define STR_COMMAND_PROMPT "> "
#define STR_WELCOME "\r\nleanmodem ready. Enter 'help' for assistance."
#define STR_COMMAND_UNRECOGNIZED "Command '%s' unknown"
#define STR_GREETING "Hey! How are you doing?"
#define STR_NOT_IMPLEMENTED "Not implemented yet!"
#define STR_SSID_SET "SSID set to '%s'."
#define STR_NULL_SSID_UNSUPPORTED "Null SSIDs are not supported."
#define STR_PASSWORD_SET "Password set to '%s'."
#define STR_PASSWORD_SET_BLANK "Not using WiFi password."
#define STR_CONNECTING "Connecting..."
#define STR_CONNECTION_ERROR "Failed to connect"
#define STR_SSID_MISSING "Please set an SSID."
#define STR_UNREACHABLE_SSID "The SSID could not be reached."
#define STR_CONNECTED_WITH_IP "Connected to '%s1' with IP %s2"
#define STR_CONNECTION_FAILED "Failed to connect. Check password."
#define STR_INVALID_ARGUMENTS "Invalid arguments."
#define STR_SCANNING "Scanning networks..."
#define STR_NETWORKS_FOUND "Networks found: "
#define STR_NOT_CONNECTED "You are not connected."
#define STR_OPEN_FAILED "Failed to open."
#define STR_OPEN_SUCCESSFUL "Host connected."
#define STR_OK "Ok."
#define STR_NOT_SET "[NOT SET]"
#define STR_YES "YES"
#define STR_NO "NO"
#define STR_NO_CONFIG_FOUND "Configuration not found."
#define STR_CONFIG_SAVED "Configuration saved."
#define STR_CONFIG_LOADED "Configuration loaded."
#define STR_DIRECTORY_USER_TITLE "Listing user files:"
#define STR_DIRECTORY_SYSTEM_TITLE "Listing system files:"
#define STR_DIRECTORY_EMPTY "  [Empty]"
#define STR_DIRECTORY_ENTRY "  %d %s"
#define STR_DIRECTORY_TOTAL "  Total: %d bytes"
#define STR_DIRECTORY_USED "  Used : %d bytes"
#define STR_DIRECTORY_FREE "  Free : %d bytes"
#define STR_FACTORY_PROMPT "This will restore your device to factory settings! Continue? (y/n): "
#define STR_CANCELED "Canceled."
#define STR_PLEASE_WAIT "Please wait."
#define STR_FACTORY_OK "Format complete."
#define STR_FACTORY_FAILED "Format error."
#define STR_DEVICE_WILL_RESTART "Your device will restart soon."
#define STR_USER_INPUT_YES "y"

#define STR_ARGUMENT_CONFIG_SAVE "save"
#define STR_ARGUMENT_CONFIG_LOAD "load"
#define STR_ARGUMENT_FILES_SYSTEM "-s"

#define KEY_SEPARATOR ": "
#define KEY_SSID "SSID"
#define KEY_PASSWORD "PASSWORD"
#define KEY_IS_UNIX_EOL "UNIX_EOL"
#define KEY_IS_SOUND_ENABLED "SOUND_ENABLED"
#define KEY_IS_ANSI_ENABLED "ANSI_ENABLED"
#define KEY_IS_ECHO_ENABLED "ECHO_ENABLED"

#define SND_BEEP 1
#define SND_GOOD 2
#define SND_BAD 3
#define SND_ALERT 4
#define SND_CONNECTED 5

// TYPE DEFINITIONS ------------------------------------------------------------
typedef void (*CommandHandler)(String args);
typedef struct
{
  String command;
  CommandHandler handler;
}  CommandEntry;

// FUNCTION DECLARATIONS -------------------------------------------------------
void cmd_hello(String args);
void cmd_help(String args);
void cmd_ssid(String args);
void cmd_password(String args);
void cmd_connect(String args);
void cmd_sound(String args);
void cmd_ansi(String args);
void cmd_echo(String args);
void cmd_beep(String args);
void cmd_scan(String args);
void cmd_telnet(String args);
void cmd_config(String args);
void cmd_eol(String args);
void cmd_restart(String args);
void cmd_files(String args);
void cmd_factory(String args);

// CONSTANTS -------------------------------------------------------------------
const int COMMAND_COUNT = 16;
const CommandEntry COMMAND_ENTRIES[COMMAND_COUNT] = {
  { "help", cmd_help },
  { "hello", cmd_hello },
  { "ssid", cmd_ssid },
  { "password", cmd_password },
  { "connect", cmd_connect },
  { "sound", cmd_sound },
  { "ansi", cmd_ansi },
  { "echo", cmd_echo },
  { "beep", cmd_beep },
  { "scan", cmd_scan },
  { "telnet", cmd_telnet },
  { "config", cmd_config },
  { "eol", cmd_eol },
  { "restart", cmd_restart },
  { "files", cmd_files },
  { "factory", cmd_factory }
};

// GLOBAL VARIABLES ------------------------------------------------------------
String ssid = "";
String password = "";
bool isUnixEOL = false;
bool isSoundEnabled = true;
bool isAnsiEnabled = false;
bool isConnected = false;
bool isEchoEnabled = true;

// FUNCTION IMPLEMENTATIONS ----------------------------------------------------

String readln() {
  const int BUFFER_SIZE = 256; 
  char buffer[BUFFER_SIZE];
  char ch = '\0';
  byte i = 0;
  while ((ch != '\n') && (ch != '\r') && (i < 255)) {
    if (Serial.available()) {
      Serial.readBytes(&ch, 1);
      if ((ch != '\n') && (ch != '\r')) {
        buffer[i] = ch;
        if (isEchoEnabled) Serial.print(ch);
        i++;        
      }
    }
  }
  buffer[i] = '\0';
  if (isEchoEnabled) Serial.print(isUnixEOL ? "\n" : "\r\n");
  return String(buffer);
}

void write(String s) {
  Serial.print(s);
}

void writeln(String s) {
  Serial.print(s);
  Serial.print(isUnixEOL ? "\n" : "\r\n");
}

void sound(byte type) {
  if (!isSoundEnabled) return;
  switch (type) {
    case SND_BEEP:
      tone(CFG_BUZZER_PIN, 1000, 100);
      break;
    case SND_GOOD:
      tone(CFG_BUZZER_PIN, 500, 100);
      break;
    case SND_BAD:
      tone(CFG_BUZZER_PIN, 400, 100);
      tone(CFG_BUZZER_PIN, 400, 100);
      tone(CFG_BUZZER_PIN, 400, 100);
      break;
    case SND_ALERT:
      for (int i=500;i<1000;i+=10) { tone(CFG_BUZZER_PIN, i, 3); delay(3); }
      for (int i=1000;i>500;i-=10) { tone(CFG_BUZZER_PIN, i, 3); delay(3); }
      break;
    case SND_CONNECTED:
      tone(CFG_BUZZER_PIN, 800, 30); delay(30);
      tone(CFG_BUZZER_PIN, 1000, 30); delay(30);
      tone(CFG_BUZZER_PIN, 1200, 30); delay(30);
      break;
  }
}

String sanitize(String input) {
  if (isAnsiEnabled) {
    return input;
  } else {
    char buffer[255];
    input.toCharArray(buffer, 255);
    MatchState ms(buffer);
    
    //ms.GlobalReplace("(\x9B|\x1B\[)[0-?]*[ -\/]*[@-~]", "");
    ms.GlobalReplace("(\\x9B|\\x1B\\[)[0-?]*[ -\\/]*[@-~]", "");
    return String(buffer);    
  }
}

void saveStr(File file, String key, String value) {
  file.println(key + KEY_SEPARATOR + value);
}

void saveBool(File file, String key, bool value) {
  file.println(key + KEY_SEPARATOR + (value ? STR_YES : STR_NO));
}

void boot() {
  writeln(STR_WELCOME);
  File file = SPIFFS.open(CFG_CONFIG_FILENAME, "r");
  bool configExists = file != NULL;
  file.close();
  if (configExists) {
    cmd_loadconfig("");
    if (ssid != "") cmd_connect("");
  }
}

// COMMAND HANDLERS ------------------------------------------------------------

void cmd_hello(String args) {
  writeln(STR_GREETING);
}

void cmd_help(String args) {
  writeln(STR_NOT_IMPLEMENTED);
}

void cmd_ssid(String args) {
  if (args == "") {
    writeln(STR_NULL_SSID_UNSUPPORTED);
    return;
  }
  ssid = args;
  String msg = String(STR_SSID_SET);
  msg.replace("%s", ssid);
  writeln(msg);
}

void cmd_password(String args) {
  password = args;
  if (args == "") {
    writeln(STR_PASSWORD_SET_BLANK);
  } else {
    String msg = String(STR_PASSWORD_SET);
    msg.replace("%s", password);
    writeln(msg);
  }
}

void cmd_connect(String args) {
  if (ssid=="") {
    writeln(STR_SSID_MISSING);
    return;
  }
  writeln(STR_CONNECTING);
  WiFi.begin(ssid, password);
  while (1)  {
    byte status = WiFi.status();
    if (status == WL_NO_SSID_AVAIL) {
      writeln(STR_UNREACHABLE_SSID);
      break;
    }
    if (status == WL_CONNECTED) {
      sound(SND_CONNECTED);
      String msg = String(STR_CONNECTED_WITH_IP);
      msg.replace("%s1", ssid);
      msg.replace("%s2", WiFi.localIP().toString());
      writeln(msg);
      isConnected = true;
      break;
    }
    if (status == WL_CONNECT_FAILED) {
      writeln(STR_CONNECTION_FAILED);
      break;
    }
    delay(500);
  }

}

void cmd_sound(String args) {
  if (args == "on") { 
    isSoundEnabled = true;
    writeln(STR_OK);
    return;
  }
  if (args == "off") {
    isSoundEnabled = false;
    writeln(STR_OK);
    return;
  }
  writeln(STR_INVALID_ARGUMENTS);
}

void cmd_ansi(String args) {
  if (args == "on") { 
    isAnsiEnabled = true;
    writeln(STR_OK);
    return;
  }
  if (args == "off") {
    isAnsiEnabled = false;
    writeln(STR_OK);
    return;
  }
  writeln(STR_INVALID_ARGUMENTS);
}

void cmd_echo(String args) {
  if (args == "on") { 
    isEchoEnabled = true;
    writeln(STR_OK);
    return;
  }
  if (args == "off") {
    isEchoEnabled = false;
    writeln(STR_OK);
    return;
  }
  writeln(STR_INVALID_ARGUMENTS);
}
void cmd_eol(String args) {
  if (args == "unix") { 
    isUnixEOL = true;
    writeln(STR_OK);
    return;
  }
  if (args == "windows") {
    isUnixEOL = false;
    writeln(STR_OK);
    return;
  }
  writeln(STR_INVALID_ARGUMENTS);
}

void cmd_beep(String args) {
  writeln(STR_OK);
  sound(SND_BEEP);
}

void cmd_scan(String args) {
  writeln(STR_SCANNING);
  int count = WiFi.scanNetworks();
  writeln(STR_NETWORKS_FOUND + String(count));
  for (int i = 0; i < count; i++) {
    writeln("   '" + String(WiFi.SSID(i).c_str()) + "', Ch:" + String(WiFi.channel(i)) + ", rssi:" + String(WiFi.RSSI(i)));
  }
  WiFi.scanDelete();
}

void cmd_config(String args) {
  if (args == STR_ARGUMENT_CONFIG_SAVE) {
    cmd_saveconfig("");
    return;
  }
  if (args == STR_ARGUMENT_CONFIG_LOAD) {
    cmd_loadconfig("");
    return;
  }
  if (args == "") {
    writeln("Network Info:");
    writeln("  SSID       : " + ((ssid == "") ? String(STR_NOT_SET) : "'" + ssid + "'"));
    writeln("  Password   : " + ((password == "") ? String(STR_NOT_SET) : "'" + password + "'"));
    writeln("  Connected to WiFi : " + String(isConnected ? "YES" : "NO") );
    writeln("");
    writeln("System flags:");
    writeln("  Sound      : " + String(isSoundEnabled ? "ON" : "OFF") );
    writeln("  ANSI Codes : " + String(isAnsiEnabled ? "ON" : "OFF") );  
    writeln("  User Echo  : " + String(isEchoEnabled ? "ON" : "OFF") );
    writeln("");
    writeln("Other info:");
    writeln("  Line Endings: " + String (isUnixEOL ? "UNIX" : "WINDOWS"));
    writeln("");    
    return;
  }
  writeln(STR_INVALID_ARGUMENTS);
}

void cmd_telnet(String args) {
  if (!isConnected) {
    writeln(STR_NOT_CONNECTED);
    return;
  }
  if (args == "") {
    writeln(STR_INVALID_ARGUMENTS);
    return;
  }
  String host = "";
  int port = 80;
  int i = args.indexOf(':');
  if (i != -1) {
    port = args.substring(i+1).toInt();
    host = args.substring(0, i);
  } else {
    host = args;
  }

  host = "towel.blinkenlights.nl"; // "outremer.fsxnet.nz";
  port = 23;

  writeln("Opening " + host + " at port " + String(port));

  WiFiClient client;
  if (client.connect(host, port)) {
    writeln(STR_OPEN_SUCCESSFUL);

    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        String sanitized = sanitize(line);
        writeln(sanitized);
      }
    }

  } else {
    writeln(STR_OPEN_FAILED);
  }

}

void cmd_saveconfig(String args) {
  File file = SPIFFS.open(CFG_CONFIG_FILENAME, "w");
  saveStr(file, KEY_SSID, ssid);
  saveStr(file, KEY_PASSWORD, password);
  saveBool(file, KEY_IS_UNIX_EOL, isUnixEOL);
  saveBool(file, KEY_IS_SOUND_ENABLED, isSoundEnabled);
  saveBool(file, KEY_IS_ANSI_ENABLED, isAnsiEnabled);  
  file.close();
  writeln(STR_CONFIG_SAVED);
}

void cmd_loadconfig(String args) {
  File file = SPIFFS.open(CFG_CONFIG_FILENAME, "r");
  if (!file) {
    writeln(STR_NO_CONFIG_FOUND);
    return;
  }
  int separatorLength = String(KEY_SEPARATOR).length();
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line[line.length()-1] = '\0';
    int i = line.indexOf(KEY_SEPARATOR);
    String key = line.substring(0, i);
    String value = line.substring(i + separatorLength);
    if (key == String(KEY_SSID)) {
      ssid = value;
      continue;
    }
    if (key == String(KEY_PASSWORD)) {
      password = value;
      continue;
    }
    if (key == String(KEY_IS_UNIX_EOL)) {
      isUnixEOL = value == STR_YES;
      continue;
    }
    if (key == String(KEY_IS_SOUND_ENABLED)) {
      isSoundEnabled = value == STR_YES;
      continue;
    }
    if (key == String(KEY_IS_ANSI_ENABLED)) {
      isAnsiEnabled = value == STR_YES;
      continue;
    }
    if (key == String(KEY_IS_ECHO_ENABLED)) {
      isEchoEnabled = value == STR_YES;
      continue;
    }
  }
  file.close();
  writeln(STR_CONFIG_LOADED);
}

void cmd_restart(String args) {
  ESP.restart();
}

void cmd_files(String args) {
  String directory;
  if (args == STR_ARGUMENT_FILES_SYSTEM) {
    writeln(STR_DIRECTORY_SYSTEM_TITLE);
    directory = CFG_SYSTEM_DIRECTORY;
  } else {
    writeln(STR_DIRECTORY_USER_TITLE);
    directory = CFG_USER_DIRECTORY;    
  }
  Dir dir = SPIFFS.openDir(directory);
  bool empty = true;
  while (dir.next()) {
    empty = false;
    String msg = String(STR_DIRECTORY_ENTRY);
    String filename = dir.fileName().substring(directory.length());
    msg.replace("%s", filename);
    msg.replace("%d", String(dir.fileSize()));
    writeln(msg);
  }
  if (empty) writeln(STR_DIRECTORY_EMPTY); 
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  String msg;
  msg = String(STR_DIRECTORY_TOTAL);
  msg.replace("%d", String(fs_info.totalBytes));
  writeln(msg);
  msg = String(STR_DIRECTORY_USED);
  msg.replace("%d", String(fs_info.usedBytes));
  writeln(msg);
  msg = String(STR_DIRECTORY_FREE);
  msg.replace("%d", String(fs_info.totalBytes - fs_info.usedBytes));
  writeln(msg);    
}

void cmd_factory(String args) {
  sound(SND_ALERT);
  write(STR_FACTORY_PROMPT);
  String userInput = readln();
  if (userInput == STR_USER_INPUT_YES) {
    writeln(STR_PLEASE_WAIT);
    bool success = SPIFFS.format();
    writeln(success ? STR_FACTORY_OK : STR_FACTORY_FAILED);
    writeln(STR_DEVICE_WILL_RESTART);
    SPIFFS.end();
    delay(1000);
    cmd_restart("");
  } else {
    if (!isEchoEnabled) writeln("");
    writeln(STR_CANCELED);
  }
}

CommandHandler findCommand(String cmd) {
  int i = 0;
  while ((i<COMMAND_COUNT) && (!cmd.equals(COMMAND_ENTRIES[i].command))) { i++; }
  if (i<COMMAND_COUNT) 
    return COMMAND_ENTRIES[i].handler;
  else
    return NULL;
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
    String errorMessage = String(STR_COMMAND_UNRECOGNIZED);
    errorMessage.replace("%s", cmd);
    writeln(errorMessage);
    return;
  }
  handler(args);
}

// INITIALIZATION AND MAIN LOOP ------------------------------------------------

void setup() {
  Serial.begin(CFG_CONSOLE_BAUD_RATE);
  SPIFFS.begin();
  delay(CFG_STARTUP_DELAY);
  boot();
}

void loop() {
  if (isEchoEnabled) Serial.print(STR_COMMAND_PROMPT);
  String userInput = readln();
  if (userInput != "") parseCommand(userInput);
}
