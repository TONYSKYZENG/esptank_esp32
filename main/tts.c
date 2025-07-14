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
#include "mp3_decoder.h"
bool play_once_flag = true;
static const char *TAG = "PLAY_TTS_EXAMPLE";
static const char *CHINESE_STRINGS = "我准备好了";
uint8_t *mp3_data_start;
uint8_t *mp3_data_end;
int i2s_pipe_idx = 0;
audio_pipeline_handle_t pipeline,pipeline2;
audio_element_handle_t tts_stream_reader, i2s_stream_writer,mp3_decoder,i2s_stream_writer2;
static struct marker {
    int pos;
    const uint8_t *start;
    const uint8_t *end;
} file_marker;
void switchToTTS(void);
void switchToMusic(void);
int mp3_music_read_cb(audio_element_handle_t el, char *buf, int len, TickType_t wait_time, void *ctx)
{
    int read_size = file_marker.end - file_marker.start - file_marker.pos;
    int file_size = file_marker.end - file_marker.start;
    if (read_size == 0) {
        return AEL_IO_DONE;
    } else if (len < read_size) {
        read_size = len;
    }
    memcpy(buf, file_marker.start + file_marker.pos, read_size);
    file_marker.pos += read_size;
    if(file_marker.pos>=file_size){
        file_marker.pos=0;
    }
    return read_size;
}

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

    audio_pipeline_cfg_t pipeline_cfg2 = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline2 = audio_pipeline_init(&pipeline_cfg2);
    mem_assert(pipeline2);

    ESP_LOGI(TAG, "[3.0.2] Create mp3 decoder to decode mp3 file and set custom read callback");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);
    audio_element_set_read_cb(mp3_decoder, mp3_music_read_cb, NULL);



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

    i2s_stream_writer2 = i2s_stream_init(&i2s_cfg);
    i2s_stream_set_clk(i2s_stream_writer2, 32000, 16, 1);

    ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, tts_stream_reader, "tts");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[3.5] Link it together [strings]-->tts_stream-->i2s_stream-->[codec_chip]");
    const char *link_tag[2] = {"tts", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 2);


    audio_pipeline_register(pipeline2, mp3_decoder, "mp3");
    audio_pipeline_register(pipeline2, i2s_stream_writer2, "i2s2");

    ESP_LOGI(TAG, "[2.4] Link it together [mp3_music_read_cb]-->mp3_decoder-->i2s_stream-->[codec_chip]");
    const char *link_tag2[2] = {"mp3", "i2s2"};
    audio_pipeline_link(pipeline2, &link_tag2[0], 2);

    ESP_LOGI(TAG, "[3.6] Set up  uri (tts as tts_stream, and directly output is i2s)");
    tts_stream_set_strings(tts_stream_reader, CHINESE_STRINGS);

    ESP_LOGI(TAG, "[4.0] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[4.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);
   // audio_event_iface_cfg_t evt_cfg2 = AUDIO_EVENT_IFACE_DEFAULT_CFG();
   // audio_event_iface_handle_t evt2 = audio_event_iface_init(&evt_cfg2);

    ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
    ESP_LOGI(TAG, "[5.0] Start audio_pipeline");
    file_marker.start = mp3_data_start_idel;
    file_marker.end = mp3_data_end_idel;
    file_marker.pos = 0;
    audio_pipeline_run(pipeline2);
    i2s_pipe_idx = 1;
    ESP_LOGI(TAG, "[6.0] Listen for all pipeline events");
}
void playTTS(char *s){
    if(i2s_pipe_idx==1) {
        switchToTTS();
        i2s_pipe_idx = 0;
    }
    else{
    audio_pipeline_stop(pipeline2);
    audio_pipeline_wait_for_stop(pipeline2);
    audio_pipeline_terminate(pipeline2);
    }
     i2s_stream_set_clk(i2s_stream_writer, 16000, 16, 1);
     audio_pipeline_reset_ringbuffer(pipeline);
     audio_pipeline_reset_elements(pipeline);
     tts_stream_set_strings(tts_stream_reader, s);
    audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
    audio_pipeline_run(pipeline);
}
void switchToTTS(void){
    ESP_LOGI(TAG, "[5.1] Switch to TTS");
    audio_pipeline_stop(pipeline2);
    audio_pipeline_wait_for_stop(pipeline2);
    audio_pipeline_terminate(pipeline2);
    i2s_stream_set_clk(i2s_stream_writer, 16000, 16, 1);
}
void switchToMusic(void){
    ESP_LOGI(TAG, "[5.2] Switch to Music");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);
    i2s_stream_set_clk(i2s_stream_writer2, 32000, 16, 1);
}
void playMusicLoop(uint8_t *start,uint8_t *end)
{   //switchToTTS();
    if(i2s_pipe_idx==0) {
        switchToMusic();
        i2s_pipe_idx = 1;
    }
    else{
      audio_pipeline_pause(pipeline2);
    i2s_stream_set_clk(i2s_stream_writer, 32000, 16, 1);
    file_marker.start = start;
    file_marker.end = end;
    file_marker.pos = 0;
    audio_pipeline_resume(pipeline2);
    return;
    }
    i2s_stream_set_clk(i2s_stream_writer2, 32000, 16, 1);
    file_marker.start = start;
    file_marker.end = end;
    file_marker.pos = 0;
    audio_pipeline_reset_ringbuffer(pipeline2);
    audio_pipeline_reset_elements(pipeline2);
    audio_pipeline_change_state(pipeline2, AEL_STATE_INIT);
    audio_pipeline_run(pipeline2);
}