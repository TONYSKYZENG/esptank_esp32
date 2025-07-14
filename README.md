| Supported Targets |ESP32-C3 |
| ----------------- | ----- | 


# ESP32 Train
This is the code for ESP32C3 mother board, which supports
* 2 channel motor
* 1 single music player

And requires
* ESP-IDF 5.0+,
* 4M flash in esp32 module (for smaller ones, you need to modify menuconfig)
## Patch Required
Our board is not officially supported by ESP-ADF, therefore, a patch is required.

First, unzip the train_sound_mini.zip to 
```
$ENV{ADF_PATH}/components/audio_board/
```

Second, add this to ``$ENV{ADF_PATH}/components/audio_board/CMakeLists.txt``
```
if (CONFIG_TANK_SOUND_MINI_BOARD)
message(STATUS "Current board name is " CONFIG_TRAIN_SOUND_MINI_BOARD)
list(APPEND COMPONENT_ADD_INCLUDEDIRS ./train_sound_mini)
set(COMPONENT_SRCS
./train_sound_mini/board.c
./train_sound_mini/board_pins_config.c
)
endif()
```

Third, add this to ``$ENV{ADF_PATH}/components/audio_board/Kconfig.projbuild``

```
config TRAIN_SOUND_MINI_BOARD
    bool "TRAIN-SOUND-MINI"
```

Lastly, go to  ``idf.py menuconfig`` and check ``CONFIG_TANK_SOUND_MINI_BOARD=y``
## Overview
The mother board will firstly set up a BLE SPP server named xxx_LOC, afterwards, you can use SPP app like bletooth terminals to control
Please go to ESP32C3_CONTROLLER.pdf for Hardware specs and connections
## Modification of music
- 1. Convert to single channel mp3, sample rate 32K
- 2. Place at main/
- 3. Add music to main/CMakeLists.txtx
- 4. Follow sound.h to define and use!

