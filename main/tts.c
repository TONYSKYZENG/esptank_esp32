/* Play pcm stream from tts stream, which can input chinese string

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "tts_stream.h"
#include "i2s_stream.h"
#include "esp_peripherals.h"
#include "board.h"
#include "tts.h"
bool play_once_flag = true;
static const char *TAG = "PLAY_TTS_EXAMPLE";
static const char *CHINESE_STRINGS = "我准备好了";
audio_pipeline_handle_t pipeline;
audio_element_handle_t tts_stream_reader, i2s_stream_writer;
void initTTS(void){
     esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "[1.0] Init Peripheral Set");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[2.0] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[3.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[3.1] Create tts stream to read data from chinese strings");
    tts_stream_cfg_t tts_cfg = TTS_STREAM_CFG_DEFAULT();
    tts_cfg.type = AUDIO_STREAM_READER;
    tts_stream_reader = tts_stream_init(&tts_cfg);

    ESP_LOGI(TAG, "[3.2] Create i2s stream to write data to codec chip");
#if CONFIG_ESP32_C3_LYRA_V2_BOARD
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_PDM_TX_CFG_DEFAULT();
#else
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
#endif
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    i2s_stream_set_clk(i2s_stream_writer, 16000, 16, 1);

    ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, tts_stream_reader, "tts");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[3.5] Link it together [strings]-->tts_stream-->i2s_stream-->[codec_chip]");
    const char *link_tag[2] = {"tts", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 2);

    ESP_LOGI(TAG, "[3.6] Set up  uri (tts as tts_stream, and directly output is i2s)");
    tts_stream_set_strings(tts_stream_reader, CHINESE_STRINGS);

    ESP_LOGI(TAG, "[4.0] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[4.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    ESP_LOGI(TAG, "[5.0] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    ESP_LOGI(TAG, "[6.0] Listen for all pipeline events");
}
void playTTS(char *s){
     audio_pipeline_reset_ringbuffer(pipeline);
     audio_pipeline_reset_elements(pipeline);
     tts_stream_set_strings(tts_stream_reader, s);
    audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
    audio_pipeline_run(pipeline);
}
