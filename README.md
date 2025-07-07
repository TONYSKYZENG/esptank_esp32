| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |


# ESP32 Tank
This is the code for ESP32 mother board, which supports
* 2 channel motor
* 1 single music player

And requires
* ESP-IDF 5.0+,
* 16M flash in esp32 module (for smaller ones, you need to modify menuconfig)
## Overview
The mother board will firstly connect to a wifi (can be configured in menuconfig CONFIG_EXAMPLE_WIFI_PASSWORD), afterwards, it opens a 3333 port for controlling.
- By default, id=RCTank0,passwd=123456789

Please go to Schematic_ESP32_LITE_V2.pdf for Hardware specs and connections
## Modification of music
- 1. Convert to single channel mp3, sample rate 32K
- 2. Place at main/
- 3. Add music to main/CMakeLists.txtx
- 4. Follow sound.h to define and use!

