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
