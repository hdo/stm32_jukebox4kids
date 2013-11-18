#include "ff.h"

#ifndef __AUDIO_H
#define __AUDIO_H

#define AUDIO_STATUS_ERROR 0
#define AUDIO_STATUS_STOPPED 1
#define AUDIO_STATUS_PAUSED 2
#define AUDIO_STATUS_PLAYING 3
#define AUDIO_STATUS_FINISHED 4

void audio_init();
void audio_reset();
void audio_process(uint32_t msticks);
void audio_stop();
void audio_pause();
void audio_resume();
void audio_play_file(char* fname);
uint8_t audio_get_status();
void audio_set_volume(uint8_t vol);
uint8_t audio_get_volume();

#endif /* end __AUDIO_H */
