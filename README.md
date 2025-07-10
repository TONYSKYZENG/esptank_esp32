

# ESP32 Tank
This is the code for ESP32 mother board, which supports
* 2 channel motor
* 1 Chinese TTS via blue tooth control

And requires
* ESP-IDF 5.0+,
* ESP-ADF
* 4M flash in esp32 module (for smaller ones, you need to modify menuconfig)
## Overview
The mother board will firstly set up a Blue tooth SPP server named XXX_M1A2_TTS, afterwards, you can use SPP app like bletooth terminals to control
Please go to Schematic_ESP32_LITE_V2.pdf for Hardware specs and connections

This example allows to convert Chinese text into Chinese speech. You can send as #XXX# where XXX is your content.

## TTS Specs

### Patch Required
Our board is not officially supported by ESP-ADF, therefore, a patch is required.

First, unzip the tank_sound_mini.zip to 
```
$ENV{ADF_PATH}/components/audio_board/
```

Second, add this to ``$ENV{ADF_PATH}/components/audio_board/CMakeLists.txt``
```
if (CONFIG_TANK_SOUND_MINI_BOARD)
message(STATUS "Current board name is " CONFIG_TANK_SOUND_MINI_BOARD)
list(APPEND COMPONENT_ADD_INCLUDEDIRS ./tank_sound_mini)
set(COMPONENT_SRCS
./tank_sound_mini/board.c
./tank_sound_mini/board_pins_config.c
)
endif()
```

Third, add this to ``$ENV{ADF_PATH}/components/audio_board/Kconfig.projbuild``

```
config TANK_SOUND_MINI_BOARD
    bool "TANK-SOUND-MINI"
```

Lastly, go to  ``idf.py menuconfig`` and check ``CONFIG_TANK_SOUND_MINI_BOARD=y``

### Build and Flash

Refer to the following table for the firmware flash address.

| Flash address | Bin Path |
|---|---|
|0x1000 | build/bootloader/bootloader.bin|
|0x8000 | build/partitions.bin|
|0x10000 | app |
|0x100000 | components/esp-sr/esp-tts/esp_tts_chinese/esp_tts_voice_data_xiaoxin_small.dat|


Select compatible audio board in ``menuconfig > Audio HAL``, build the project and flash it to the board, then run monitor tool to view serial output.


-  Flash the app as normal case, although it will not work after this.

- Do not worry, flash `components/esp-tts/esp_tts_chinese/esp_tts_voice_data_xiaoxin_small.dat` to the partition table address. For instance, use the following in ESP-IDF terminal:

```
esptool.py write_flash 0x100000 esp_tts_voice_data_xiaoxin_small.dat
```


- Reboot, it works now.


## Troubleshooting

- If the TTS example does not play the speech, please check if the ``esp_tts_voice_data_xiaoxin_small`` file has been flashed to the specified address.


## Technical Support and Feedback
Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/viewforum.php?f=20) forum

