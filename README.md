| Supported Targets |ESP32-C3 |
| ----------------- | ----- | 


# ESP32 Train
This is the code for ESP32C3 mother board, which supports
* 2 channel motor
* 1 single music player

And requires
* ESP-IDF 5.0+,
* 4M flash in esp32 module (for smaller ones, you need to modify menuconfig)
## Overview
The mother board will firstly set up a BLE SPP server named xxx_LOC, afterwards, you can use SPP app like bletooth terminals to control
Please go to Schematic_ESP32_LITE_V2.pdf for Hardware specs and connections
## Modification of music
- 1. Convert to single channel mp3, sample rate 32K
- 2. Place at main/
- 3. Add music to main/CMakeLists.txtx
- 4. Follow sound.h to define and use!

