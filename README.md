# leanmodem

Connect your old computers to the internet and more!

leanmodem was made for the nodemcu and the ESP8266. It allows you to connect to the internet via a simple text interface. You can also use it to store files from the old computer into the device and viceversa.

Adding support for a full traditional landline modem is on the roadmap. Once it's done your old computer will be able to use it without knowing it's actually using the internet.

## Getting Started

* Download the full sketch.
* Download the dependencies.
* Compile and install it to your nodemcu.
* Have fun!

### Prerequisites

You need to have the Arduino IDE installed and properly set up for compiling programs and installing them in the nodemcu.

### Installing

1. Download the full code, unzip it in a folder inside your `arduino`.
2. Set up your IDE so that you can install programs in the nodemcu. [Read this guide in case you need help.](https://www.instructables.com/id/Steps-to-Setup-Arduino-IDE-for-NODEMCU-ESP8266-WiF/)
3. In case you didn't follow the guide, make sure that you have installed the support for your nodemcu. Go to `Tools -> Board ... -> Board Manager` and install esp8266 2.5.2 by ESP8266 Community.
4. Go to `Tools -> Library Manager` and install the dependencies for leanmodem. They are:
  * `ESP8266-ping` 2.0.1 by Alessio Leoncini
  * `NTPClient` 3.2.0 by Fabrice Weinberg
  * `Crypto` 0.2.0 by Rhys Weatherley
  * `WiFiEsp` 2.2.2 by bportaluri

  Please be aware that using newer versions may cause incompatibility issues. Let me know if you have any problems.
5. Since this project uses SPIFFS you will need to install the proper tools. [Go to their repository and follow the instructions.](https://github.com/esp8266/arduino-esp8266fs-plugin)
6. Set up your Arduino IDE with the following settings:
  * Board: `Generic ESP8266 Module`
  * Flash Size: `4M (1M SPIFFS)`
7. You should be ready to compile and install the project into your nodemcu. Again, let me know if you have issues. That will help me to improve these instructions, if necessary.

## Usage

leanmodem is using by plugging the nodemcu to your computer using a micro-USB cable. Then, you need a terminal emulator such as [PuTTY](https://www.putty.org/) to connect to the proper COM port that was automatically assigned to the nodemcu after you connected it. Make sure that you set the appropriate speed as set in the `CFG_CONSOLE_BAUD_RATE` option in `config.h`. At time of this writing, the speed is 19200 bauds.

Once you see the command prompt you will be able to use the following commands. Make sure you always use lowercase characters, otherwise your commands won't be recognized.

# clear
Clears the screen. Only works if your Terminal supports ANSI codes and your ANSI flag is set to yes.

# config [nothing/load/save/set]
Allows you to see the current settings, change, load and save them.

* `config`: displays the device configuration
* `config load`: loads configuration from system file
* `config save`: saves configuration to system file
* `config set [setting] [value]`: Sets [value] to [setting]. `String` means the argument requires values to be text. `Boolean` means the argument has to be `yes` or `no`. `Number` means the argument must be a number between -32768 and 32787. Valid settings are:
  * **id** [String] the name of your device.
  * **ssid** [String] the SSID of your wireless network.
  * **pass** [String] the password for your wireless network. BE AWARE that your WiFi password will be stored in plaintext in your device, and you can even see it among the other configuration values.
  * **sound** [Boolean] Enables sounds via the pin specified in `CFG_BUZZER_PIN`.
  * **ansi** [Boolean] Enables leanmodem to use ANSI codes for enhanced terminal output, with colors and more complex interfaces.
  * **echo** [Boolean] When set to `yes` this setting returns all user input to your terminal, displaying it on the screen.
  * **autoconnect** [Boolean] when set to `yes` leanmodem will automatically connect to the wireless network during startup.
  * **unix** [Boolean] when set to `yes`, all line-endings will be LF only (Unix compatible). If set to `no` line-endings will be CRLF (Windows compatible).
  * **timezone** [Number] sets the current timezone. This will affect how time is displayed by your device.
  * **timeout** [Number] number of milliseconds that the device waits before cutting communication with a remote device. A larger number will make it more resistant to errors but it will make it more unresponsive to the user. A smaller number makes it faster and more responsive, but it could result in communications being cut before it's due (this is a risk with older computers or during user interaction).

# connect
Connects to the wireless network using the SSID and password previously set with `config`.

# copy [source] [target]
This is one of the most powerful commands in leanmodem. It copies information from `[source]` to `[target]`. For instance, it copies user input to a file, or data from a file to another (making a duplicate), or even information from a TCP socket, or data downloaded via HTTP. Check out these examples:
* `copy stdin somefile`: read information from the user and writes it down to `somefile`. Once you are done typing just wait for it to timeout and it will save and close the output.
* `copy somefile anotherfile`: creates a new file called `anotherfile` with the contents of `somefile`.
* `copy somefile stdout`: retrieves the contents of somefile and dumps them to your terminal, displaying it. Be aware that displaying binary files might confuse or crash your terminal.
* `copy tcp://192.168.0.1:8080 somefile`: it opens a raw TCP socket to `192.168.0.1` at port `8080` and dumps everything that it receives through it to `somefile`.
* `copy http://www.example.com/ website`: connects to the provided URL using default port 80, downloads the webpage and dumps it in a local file called `website`. Be aware, only HTTP/1.0 is supported. HTTPS is not supported... yet.

# erase [filename]
Erases the specified local file

# files [-s]
Displays the files stored in the local filesystem. By default it displays user files, but you can use the `-s` argument to see the system files. System files are usually not accessible by the user by using the terminal alone.

# format
Erases all the information in the device, including the system configuration. It restores the device to its "factory" settings.

# help
Displays a list of all available commands and a short explanation for each one.

# hexdump [filename]
Displays the contents of the specified file using the classic hex editor style.

# ping [host]
Pings the remote host using the standard ICMP method.

# pong
It allows you to play pong! Yes, you read that correctly. Try it out! You need an ANSI compatible terminal.

# restart
Restarts the device

# scan
Scans for nearby wireless networks

# setpin [pin number] [yes/no]
Sets a GPIO pin to digital output and sets it HIGH or LOW depending on the provided parameter. For the nodemcu the only available pins are 0, 2, 5, 4, 14, 12 and 13. [Check out this website](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/) for more information about usable digital output pins in the nodemcu.

# sha256 [filename]
Calculates the sha256 hash of the specified local file.

# telnet [host]
It connects via WiFi to the specified telnet host. This is great for old computers to connect to Internet-based old-style bulletin boards (BBS).

# time
It queries the current time using NTP and displays it on the screen using the device's `timezone` setting.

# ver
Returns the about information and the build date.

# xrecv [filename]
It receives a file from the computer using the old standard xmodem-crc protocol. This was tested with old programs like Windows 3.0 Terminal application.

# xsend [filename]
Sends the specified local filename to the connected computer using the xmodem-crc binary protocol.

## Authors

* **Leandro Tami** - [reach me on Twitter by my handle @leandrinux](https://twitter.com/leandrinux)

## License

This project is licensed under the MIT License:
```
Copyright 2019 Leandro Tami

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

## Acknowledgments

To all those people who worked so hard to make the software and hardware that allowed me to build this and have so much fun.
