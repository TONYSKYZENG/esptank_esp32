#ifndef _SOUND_H_
#define _SOUND_H_
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

/* Example configurations */
#define EXAMPLE_RECV_BUF_SIZE   (2400)
#define EXAMPLE_SAMPLE_RATE     (16000)
#define EXAMPLE_MCLK_MULTIPLE   (384) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)



/* I2C port and GPIOs */
#define I2C_NUM         (0)
#define I2S_NUM         (0)
//#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define I2S_MCK_IO      (-1)
#define I2S_BCK_IO      (GPIO_NUM_5)
#define I2S_WS_IO       (GPIO_NUM_6)
#define I2S_DO_IO       (GPIO_NUM_4)
#define I2S_DI_IO       (-1)
void initSound(void);
void stopMusic(void);
/*void startMusic(void);
void i2s_music_mp3(void *args);
void i2s_music_mp3_once (void *args);
*/

void playMusicSingle(uint8_t *start,uint8_t *end);
void playMusicLoop(uint8_t *start,uint8_t *end);
extern const uint8_t mp3_data_start_idel[] asm("_binary_idel_mp3_start");
extern const uint8_t mp3_data_end_idel[] asm("_binary_idel_mp3_end");

extern const uint8_t mp3_data_start_music[] asm("_binary_music_mp3_start");
extern const uint8_t mp3_data_end_music[] asm("_binary_music_mp3_end");

extern const uint8_t mp3_data_start_mg[] asm("_binary_mg_mp3_start");
extern const uint8_t mp3_data_end_mg[] asm("_binary_mg_mp3_end");

extern const uint8_t mp3_data_start_engine[] asm("_binary_engine_mp3_start");
extern const uint8_t mp3_data_end_engine[] asm("_binary_engine_mp3_end");
extern const uint8_t mp3_data_start_cannon[] asm("_binary_cannon_mp3_start");
extern const uint8_t mp3_data_end_cannon[] asm("_binary_cannon_mp3_end");
#endif