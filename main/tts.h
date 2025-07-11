#ifndef _TTS_H_
#define _TTS_H_
void initTTS(void);
void playTTS(char *s);

void playMusicLoop(uint8_t *start,uint8_t *end);
extern const uint8_t mp3_data_start_idel[] asm("_binary_idel_mp3_start");
extern const uint8_t mp3_data_end_idel[] asm("_binary_idel_mp3_end");

extern const uint8_t mp3_data_start_music[] asm("_binary_music_mp3_start");
extern const uint8_t mp3_data_end_music[] asm("_binary_music_mp3_end");

extern const uint8_t mp3_data_start_mg[] asm("_binary_mg_mp3_start");
extern const uint8_t mp3_data_end_mg[] asm("_binary_mg_mp3_end");

extern const uint8_t mp3_data_start_cannon[] asm("_binary_cannon_mp3_start");
extern const uint8_t mp3_data_end_cannon[] asm("_binary_cannon_mp3_end");
#endif