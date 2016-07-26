## ESP8266 WiFi Serial Manager

The [ESP8266 Arduino](https://github.com/esp8266/Arduino) package provides a great and familiar to use ecosystem for developing code on the chip. However, most examples for WiFi network connection and management involve building and uploading new code every time you want to change the network settings. That's just more than should be required if you want to simply connect an existing and complete project to a new network. There are some [examples](https://github.com/tzapu/WiFiManager) of ESP8266 WiFi managers already but all seem to go with the mor complicated process of entering AP mode, connecting to the ESP8266 with another computer, loading a webpage and configuring via that page. This library, however, seeks to simplify that with providing an easy to use interactive serial console for all your WiFi network management needs. It supports open, WEP, WPA, and even WPS connections as well as can commit the configuration to EEPROM to be automatically connected on reboot.

### Usage

#### Serial Connections

You can connect to the interactive management console on the ESP8266 via a serial connection, either using an FTDI adapter or any on board serial to USB connection. On all platforms, using the Arduino Serial Monitor is always on option. Just open the monitor, set the baud rate to the rate you set in Serial.begin(). Note that entering options into the management console requires sending the return key which in the Arduino Serial Monitor requires setting line ending option in the bottom right of the monitor window. Any option other than "No line ending" will work.

##### Putty

On Windows, putty is a good option and you can connect to the serial port via:

```putty -serial <COM_PORT> -sercfg 115200```

##### Screen

On Linux and Mac, the best option we've found is to use the screen command which if not already available can be installed via your native package manager. It is almost always:

```sudo <apt-get|yum|dnf|etc> install screen```

Once you have screen, connect to the serial port via:

```screen /dev/tty<device> 115200```

To find, the actual device path, the easiest way is generally to connect the ESP8266 device and then run:

```dmesg | grep /dev/tty*```

Then look for a line detailing the connection of a new USB Serial device which will list the full device path.

Once connected the board will not automatically reset like when using the Arduino Serial Monitor

### ESPSerialWiFiManager Class

#### ESPSerialWiFiManager(int eeprom_size, int eeprom_offset)

The constructor can typically be called with no parameters but because EEPROM works a little differently on the ESP8266, two optional parameters are provided.

- **eeprom_size**: (optional) Size in bytes of the desired EEPROM. Typically between 512 and 4096. The ESP8266 does not actually have an EEPROM and instead it's faked by writing to flash. Only use this if you are already using EEPROM in your code and need to specify something other than the default 512 bytes.
- **eeprom_offset**: (optional) Offset in bytes at which the ESPSerialWiFiManager config should be stored. If, for example, your code uses 100 bytes of EEPROM, starting at byte index 0, set this offset to something greater than 100 so that ESPSerialWiFiManager does not overwrite your code's EEPROM data.

*If you are using EEPROM in your code, you must remove the call to EEPROM.begin() and instead let ESPSerialWiFiManager handle it for you!*

#### begin(String ssid, String pass)

This must be called before run_menu() and will configure the EEPROM then connect to any stored WiFi network automatically, if previously saved.

- **ssid**: (optional) If provided ESPSerialWiFiManager will attempt to connect to this SSID instead of anything stored in EEPROM.
- **pass**: (optional) If provided ESPSerialWiFiManager will attempt to connect to the above SSID using this password. If not provided, it is assumed that the network is unsecured.

The optional parameters are useful for providing a default that can then be changed on the fly via the serial menu. If this connection fails, it will attempt a connection from details stored in EEPROM, if available. But this will *always* be tried first so it is best when used only if the interactive config is to be used temporarily.

#### run_menu(int timeout)

Launches the interactive serial console. This can be called anywhere in the code. For example, call at the end of setup() with a timeout to provide a one-time option to configure the WiFi on boot. Or call it inside loop() when a button is pressed. See the [examples](https://github.com/ManiacalLabs/ESPSerialWiFiManager/tree/master/examples) for more.

- **timeout**: (optional) If provided, the interactive console will timeout after this many seconds and return control to the main program.
