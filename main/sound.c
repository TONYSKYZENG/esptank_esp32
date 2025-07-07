#include "sound.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_check.h"
#include "esp_spiffs.h"
#include "mp3dec.h"
#include <stdatomic.h>
// 定义原子变量
atomic_uint should_stop_music = ATOMIC_VAR_INIT(0);
atomic_uint has_stopped_music = ATOMIC_VAR_INIT(1);
#define CONFIG_EXAMPLE_MODE_MUSIC 1
static const char *TAG = "i2s_es8311";
static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;

/* Import music file as buffer */

uint8_t *mp3_data_start;
uint8_t *mp3_data_end;
int shouldStopMusic(void)
{
    int value = atomic_load(&should_stop_music);
    return value;
}
int hasStoppedMusic(void)
{
    int value = atomic_load(&has_stopped_music);
    return value;
}
void stopMusic(void){
     atomic_store(&should_stop_music, 1);
}
void startMusic(void){
     atomic_store(&should_stop_music, 0);
     while (!hasStoppedMusic());
    atomic_store(&has_stopped_music, 0);
    xTaskCreate(i2s_music_mp3, "i2s_music", 4096, NULL, 5, NULL);
}
void playMusicLoop(uint8_t *start,uint8_t *end){
   stopMusic();
    while (!hasStoppedMusic());
    atomic_store(&has_stopped_music, 0);
    mp3_data_start = start;
    mp3_data_end = end;
    atomic_store(&should_stop_music, 0);
    xTaskCreate(i2s_music_mp3, "i2s_music", 4096, NULL, 5, NULL);
}
void playMusicSingle(uint8_t *start,uint8_t *end){
    stopMusic();
     while (!hasStoppedMusic());
    atomic_store(&has_stopped_music, 0);
    mp3_data_start = start;
    mp3_data_end = end;
     atomic_store(&should_stop_music, 0);
    xTaskCreate(i2s_music_mp3_once, "i2s_music_once", 4096, NULL, 5, NULL);
}
static esp_err_t i2s_driver_init(void)
{
#if !defined(CONFIG_EXAMPLE_BSP)
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_MCK_IO,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_DO_IO,
            .din = I2S_DI_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));
#else
    ESP_LOGI(TAG, "Using BSP for HW configuration");
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(EXAMPLE_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };
    std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;
    ESP_ERROR_CHECK(bsp_audio_init(&std_cfg, &tx_handle, &rx_handle));
    ESP_ERROR_CHECK(bsp_audio_poweramp_enable(true));
#endif
    return ESP_OK;
}
size_t file_size(char *fname) {
FILE *fp = fopen(fname, "rb");
long file_size=0;
if (fp) {
    fseek(fp, 0, SEEK_END);  // 移动到文件末尾
    file_size = ftell(fp);  // 获取当前位置（即文件大小）
    fseek(fp, 0, SEEK_SET);  // 移回文件开头
    fclose(fp);
    return file_size;
}
return 0;
}
int16_t *pcm_buffer;
HMP3Decoder hMP3Decoder;
void i2s_music_mp3_inner(void *args)
{
    esp_err_t ret = ESP_OK;
    size_t bytes_write = 0;
     // 初始化MP3解码
    const uint8_t *mp3_ptr = mp3_data_start;
    int bytes_left = mp3_data_end - mp3_data_start;
    unsigned char **inbuf=(unsigned char **)&mp3_ptr;
    while (bytes_left > 0&&(!shouldStopMusic())) {
        int offset = 0;
        int err = MP3Decode(hMP3Decoder, inbuf, &bytes_left, pcm_buffer, offset);

        if (err) {
            printf("MP3解码错误: %d\n", err);
            break;
        }

        // 获取解码后的音频信息
        MP3FrameInfo mp3Info;
        MP3GetLastFrameInfo(hMP3Decoder, &mp3Info);

        // 写入I2S
        size_t bytes_written;
        i2s_channel_write(tx_handle, pcm_buffer, mp3Info.outputSamps * sizeof(int16_t), &bytes_write, 1000);
        
    }
}
void init_mp3(void){
    esp_err_t ret = ESP_OK;
    size_t bytes_write = 0;
     // 初始化MP3解码器
    hMP3Decoder = MP3InitDecoder();
    /* (Optional) Disable TX channel and preload the data before enabling the TX channel,
     * so that the valid data can be transmitted immediately */
    ESP_ERROR_CHECK(i2s_channel_disable(tx_handle));
    //data_ptr += bytes_write;  // Move forward the data pointer

    /* Enable the TX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
     // 初始化MP3解码器
    if (!hMP3Decoder) {
        printf("MP3解码器初始化失败\n");
        return;
    }

    const uint8_t *mp3_ptr = mp3_data_start;
    int bytes_left = mp3_data_end - mp3_data_start;
    pcm_buffer=(int16_t*)malloc(1152*2*sizeof(int16_t)); // 最大PCM帧大小
    unsigned char **inbuf=(unsigned char **)&mp3_ptr;
}

void i2s_music_mp3(void *args)
{
   
    int i=0;
    while (!shouldStopMusic())
    {   ESP_LOGI(TAG, "[music] i2s music played, %d bytes are written.",i);
       i2s_music_mp3_inner(args);
        i++;
      //   vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    atomic_store(&has_stopped_music, 1);
    vTaskDelete(NULL);
}
void i2s_music_mp3_once(void *args)
{
   //ESP_LOGI(TAG, "[music] i2s music played, %d bytes are written.",i);
   i2s_music_mp3_inner(args);
      //   vTaskDelay(1000 / portTICK_PERIOD_MS);
    atomic_store(&has_stopped_music, 1);
    
    vTaskDelete(NULL);
}
    // 清理
   // MP3FreeDecoder(hMP3Decoder);
void initSound(void)
{
    if (i2s_driver_init() != ESP_OK) {
        ESP_LOGE(TAG, "i2s driver init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "i2s driver init success");
    }
    /* Initialize i2c peripheral and config es8311 codec by i2c */
    /*if (es8311_codec_init() != ESP_OK) {
        ESP_LOGE(TAG, "es8311 codec init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "es8311 codec init success");
    }*/

    /* Play a piece of music in music mode */
    init_mp3();
}